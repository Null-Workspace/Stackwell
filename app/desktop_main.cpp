#ifdef _WIN32

#include <Windows.h>
#include <filesystem>
#include <exception>
#include <string>

#include <stackwell/core/application.hpp>
#include <stackwell/ui/suf/desktop_window.hpp>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command)
{
    stackwell::Application application;

    try
    {
        std::wstring executable(32768, L'\0');
        const DWORD length = GetModuleFileNameW(
            nullptr, executable.data(), static_cast<DWORD>(executable.size()));

        if (length == 0 || length >= executable.size())
        {
            MessageBoxW(nullptr, L"Unable to determine the application path.",
                L"Stackwell", MB_ICONERROR | MB_OK);
            return 1;
        }

        executable.resize(length);
        application.initialize(std::filesystem::path(executable));

        stackwell::suf::DesktopWindow window(application);
        if (!window.create(instance, show_command))
        {
            application.shutdown();
            MessageBoxW(nullptr, L"Unable to create the Stackwell desktop window.",
                L"Stackwell", MB_ICONERROR | MB_OK);
            return 1;
        }

        const int result = window.run();
        application.shutdown();
        return result;
    }
    catch (const std::exception& error)
    {
        const std::string message = std::string("Stackwell could not start: ") + error.what();
        MessageBoxA(nullptr, message.c_str(), "Stackwell", MB_ICONERROR | MB_OK);
        application.shutdown();
        return 1;
    }
}

#endif
