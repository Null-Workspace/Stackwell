#include <stackwell/commands/info_command.hpp>
#include <iostream>
namespace stackwell {
void register_info_command(CLI::App& app, const std::string& version) {
 auto* command = app.add_subcommand("info", "Show Stackwell host information");
 command->callback([version]() {
  std::cout << "Stackwell " << version << '\n'
            << "Plugin-first application host\nC++20\n";
 });
}
}
