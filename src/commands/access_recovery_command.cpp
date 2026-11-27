#include <stackwell/commands/access_recovery_command.hpp>

#include <stackwell/access_recovery/access_recovery.hpp>

#include <iostream>
#include <string>

namespace stackwell
{
    namespace
    {
        void print_file_report(const FileSecurityReport& report)
        {
            std::cout << "\nNTFS security inspection\n";
            std::cout << "------------------------\n";
            std::cout << "Path: " << report.path.string() << '\n';
            std::cout << "Inspected: " << (report.inspected ? "yes" : "no") << '\n';

            if (report.owner_available)
            {
                std::cout << "Owner SID: " << report.owner_sid << '\n';
            }
            else
            {
                std::cout << "Owner SID: unavailable\n";
            }

            if (report.inspected)
            {
                std::cout << "DACL present: " << (report.dacl_present ? "yes" : "no") << '\n';
                std::cout << "DACL defaulted: " << (report.dacl_defaulted ? "yes" : "no") << '\n';
                std::cout << "DACL protected from inheritance: "
                          << (report.dacl_protected ? "yes" : "no") << '\n';
                std::cout << "ACE count: " << report.ace_count << '\n';
            }

            std::cout << "Result: " << report.message << "\n\n";
        }

        void print_policy_report(const std::vector<PolicyFinding>& findings)
        {
            std::cout << "\nKnown Windows policy values\n";
            std::cout << "---------------------------\n";

            if (findings.empty())
            {
                std::cout << "No configured values from the current inspection list were found.\n";
                std::cout << "This does not prove that no restrictions exist.\n\n";
                return;
            }

            for (const auto& finding : findings)
            {
                std::cout << finding.hive << '\\' << finding.key
                          << '\\' << finding.value_name << '\n';
                std::cout << "  DWORD: " << finding.value << '\n';
                std::cout << "  Note: " << finding.interpretation << "\n\n";
            }
        }
    }

    void register_access_recovery_command(CLI::App& app)
    {
        auto* command = app.add_subcommand(
            "access-recovery",
            "Inspect file permissions and selected Windows policy settings (read-only)"
        );

        auto* scan = command->add_subcommand(
            "scan",
            "Inspect the security descriptor of one file or directory"
        );

        std::string path;
        scan->add_option("--path", path, "File or directory to inspect")
            ->required();

        scan->callback([&path]()
        {
            AccessRecovery recovery;
            print_file_report(recovery.inspect_path(path));
        });

        auto* policies = command->add_subcommand(
            "policies",
            "Inspect selected Windows policy registry values (read-only)"
        );

        policies->callback([]()
        {
            if (!AccessRecovery::platform_supported())
            {
                std::cout << "Policy inspection is currently supported only on Windows.\n";
                return;
            }

            AccessRecovery recovery;
            print_policy_report(recovery.inspect_known_policies());
        });

        command->require_subcommand(1);
    }
}
