#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace stackwell
{
    struct FileSecurityReport
    {
        std::filesystem::path path;
        bool inspected = false;
        bool owner_available = false;
        std::string owner_sid;
        bool dacl_present = false;
        bool dacl_defaulted = false;
        bool dacl_protected = false;
        std::size_t ace_count = 0;
        std::string message;
    };

    struct PolicyFinding
    {
        std::string hive;
        std::string key;
        std::string value_name;
        unsigned long value = 0;
        std::string interpretation;
    };

    class AccessRecovery
    {
    public:
        [[nodiscard]] FileSecurityReport inspect_path(
            const std::filesystem::path& path
        ) const;

        [[nodiscard]] std::vector<PolicyFinding> inspect_known_policies() const;

        [[nodiscard]] static bool platform_supported() noexcept;
    };
}
