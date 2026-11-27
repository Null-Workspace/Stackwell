#include <CLI/CLI.hpp>
#include <stackwell/core/application.hpp>
#include <stackwell/core/logger.hpp>
#include <stackwell/commands/command_registry.hpp>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif
namespace {
std::filesystem::path executable_path(const char* argv0) {
#ifdef _WIN32
 std::vector<wchar_t> buffer(32768);
 const DWORD length=GetModuleFileNameW(nullptr,buffer.data(),static_cast<DWORD>(buffer.size()));
 if(length==0 || length>=buffer.size()) throw std::runtime_error("Unable to determine executable path");
 return std::filesystem::path(std::wstring(buffer.data(),length));
#elif defined(__APPLE__)
 std::uint32_t size=0; _NSGetExecutablePath(nullptr,&size); std::vector<char> buffer(size);
 if(_NSGetExecutablePath(buffer.data(),&size)!=0) throw std::runtime_error("Unable to determine executable path");
 return std::filesystem::weakly_canonical(buffer.data());
#elif defined(__linux__)
 std::vector<char> buffer(4096);
 const auto length=readlink("/proc/self/exe",buffer.data(),buffer.size()-1);
 if(length<0) throw std::runtime_error("Unable to determine executable path");
 buffer[static_cast<std::size_t>(length)]='\0'; return std::filesystem::path(buffer.data());
#else
 return std::filesystem::absolute(argv0);
#endif
}
}
int main(int argc,char* argv[]) {
 stackwell::Application application;
 try {
  application.initialize(executable_path(argv[0]));
  CLI::App cli{"Stackwell - plugin-first application host"};
  stackwell::register_commands(cli,application.get_plugin_manager(),application.get_config().application.version);
  int result=0;
  try { cli.parse(argc,argv); } catch(const CLI::ParseError& error) { result=cli.exit(error); }
  application.shutdown(); return result;
 } catch(const std::exception& error) {
  if(auto logger=stackwell::Logger::get()) logger->critical("Fatal error: {}",error.what());
  else std::cerr<<"Stackwell: "<<error.what()<<'\n';
  application.shutdown(); return 1;
 }
}
