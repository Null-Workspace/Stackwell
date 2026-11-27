
#include <stackwell/access_recovery/access_recovery.hpp>

#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <Aclapi.h>
#include <Sddl.h>

namespace
{
    // Converts a null-terminated UTF-16 string to UTF-8.
    std::string WideToUtf8(const wchar_t* value)
    {
        if (value == nullptr)
        {
            return {};
        }

        const int required = WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value,
            -1,
            nullptr,
            0,
            nullptr,
            nullptr
        );

        if (required <= 0)
        {
            return {};
        }

        // The required size includes the terminating null character.
        std::string result(
            static_cast<std::size_t>(required),
            '\0'
        );

        const int converted = WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value,
            -1,
            result.data(),
            required,
            nullptr,
            nullptr
        );

        if (converted <= 0)
        {
            return {};
        }

        result.resize(static_cast<std::size_t>(converted - 1));

        return result;
    }
}

#endif // _WIN32

namespace stackwell
{
    bool AccessRecovery::platform_supported() noexcept
    {
#ifdef _WIN32
        return true;
#else
        return false;
#endif
    }

    FileSecurityReport AccessRecovery::inspect_path(
        const std::filesystem::path& path
    ) const
    {
        FileSecurityReport report;
        report.path = path;

        std::error_code fs_error;

        if (!std::filesystem::exists(path, fs_error))
        {
            report.message = fs_error
                ? "Unable to access the path: " + fs_error.message()
                : "The path does not exist.";

            return report;
        }

#ifndef _WIN32

        report.message =
            "NTFS security inspection is currently supported only on Windows.";

        return report;

#else

        PSID owner = nullptr;
        PACL dacl = nullptr;
        PSECURITY_DESCRIPTOR descriptor = nullptr;

        const DWORD result = GetNamedSecurityInfoW(
            const_cast<LPWSTR>(path.c_str()),
            SE_FILE_OBJECT,
            OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
            &owner,
            nullptr,
            &dacl,
            nullptr,
            &descriptor
        );

        if (result != ERROR_SUCCESS)
        {
            report.message =
                "GetNamedSecurityInfoW failed with Windows error " +
                std::to_string(result) + ".";

            return report;
        }

        report.inspected = true;

        // Owner SID
        if (owner != nullptr)
        {
            LPWSTR sid_text = nullptr;

            if (ConvertSidToStringSidW(owner, &sid_text) != FALSE)
            {
                report.owner_sid = WideToUtf8(sid_text);
                report.owner_available = !report.owner_sid.empty();

                LocalFree(sid_text);
            }
        }

        // DACL information
        BOOL dacl_present = FALSE;
        BOOL dacl_defaulted = FALSE;

        if (GetSecurityDescriptorDacl(
                descriptor,
                &dacl_present,
                &dacl,
                &dacl_defaulted
            ) != FALSE)
        {
            report.dacl_present = dacl_present != FALSE;
            report.dacl_defaulted = dacl_defaulted != FALSE;

            if (dacl_present && dacl != nullptr)
            {
                report.ace_count =
                    static_cast<std::size_t>(dacl->AceCount);
            }
        }

        // DACL inheritance protection
        SECURITY_DESCRIPTOR_CONTROL control = 0;
        DWORD revision = 0;

        if (GetSecurityDescriptorControl(
                descriptor,
                &control,
                &revision
            ) != FALSE)
        {
            report.dacl_protected =
                (control & SE_DACL_PROTECTED) != 0;
        }

        LocalFree(descriptor);

        report.message =
            "Security descriptor inspected. No permissions were changed.";

        return report;

#endif
    }

    std::vector<PolicyFinding>
    AccessRecovery::inspect_known_policies() const
    {
        std::vector<PolicyFinding> findings;

#ifndef _WIN32

        return findings;

#else

        struct PolicyDefinition
        {
            HKEY root;
            const wchar_t* hive;
            const wchar_t* subkey;
            const wchar_t* value_name;

            const char* display_key;
            const char* display_value;
            const char* description;
        };

        const PolicyDefinition definitions[] =
        {
            {
                HKEY_CURRENT_USER,
                L"HKCU",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableTaskMgr",
                "Policies\\System",
                "DisableTaskMgr",
                "Task Manager restriction flag is configured."
            },
            {
                HKEY_CURRENT_USER,
                L"HKCU",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableRegistryTools",
                "Policies\\System",
                "DisableRegistryTools",
                "Registry Editor restriction flag is configured."
            },
            {
                HKEY_CURRENT_USER,
                L"HKCU",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableCMD",
                "Policies\\System",
                "DisableCMD",
                "Command Prompt restriction flag is configured."
            },
            {
                HKEY_LOCAL_MACHINE,
                L"HKLM",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableTaskMgr",
                "Policies\\System",
                "DisableTaskMgr",
                "Machine-level Task Manager restriction flag is configured."
            },
            {
                HKEY_LOCAL_MACHINE,
                L"HKLM",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableRegistryTools",
                "Policies\\System",
                "DisableRegistryTools",
                "Machine-level Registry Editor restriction flag is configured."
            },
            {
                HKEY_LOCAL_MACHINE,
                L"HKLM",
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                L"DisableCMD",
                "Policies\\System",
                "DisableCMD",
                "Machine-level Command Prompt restriction flag is configured."
            }
        };

        for (const auto& definition : definitions)
        {
            HKEY key = nullptr;

            const LONG open_result = RegOpenKeyExW(
                definition.root,
                definition.subkey,
                0,
                KEY_QUERY_VALUE,
                &key
            );

            if (open_result != ERROR_SUCCESS)
            {
                continue;
            }

            DWORD value = 0;
            DWORD type = 0;
            DWORD size = sizeof(value);

            const LONG query_result = RegQueryValueExW(
                key,
                definition.value_name,
                nullptr,
                &type,
                reinterpret_cast<LPBYTE>(&value),
                &size
            );

            RegCloseKey(key);

            if (query_result != ERROR_SUCCESS ||
                type != REG_DWORD ||
                size != sizeof(value))
            {
                continue;
            }

            PolicyFinding finding;

            finding.hive = WideToUtf8(definition.hive);
            finding.key = definition.display_key;
            finding.value_name = definition.display_value;
            finding.value = value;

            finding.interpretation = value == 0
                ? "The policy value exists and is set to 0; verify against the intended configuration."
                : definition.description;

            findings.push_back(std::move(finding));
        }

        return findings;

#endif
    }
}
