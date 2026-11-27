
#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <string>
#include <vector>

#include <stackwell/core/application.hpp>
#include <stackwell/access_recovery/access_recovery.hpp>

namespace stackwell::suf
{
    class DesktopWindow
    {
    public:
        explicit DesktopWindow(Application& application);
        ~DesktopWindow();

        DesktopWindow(const DesktopWindow&) = delete;
        DesktopWindow& operator=(const DesktopWindow&) = delete;

        bool create(HINSTANCE instance, int show_command);
        int run();

    private:
        enum class Page
        {
            Overview,
            Plugins,
            AccessRecovery,
            Diagnostics,
            Settings
        };

        static LRESULT CALLBACK window_proc(
            HWND window,
            UINT message,
            WPARAM wparam,
            LPARAM lparam);

        LRESULT handle_message(
            UINT message,
            WPARAM wparam,
            LPARAM lparam);

        bool create_device_resources();
        void discard_device_resources();

        void render();
        void draw_page();

        void draw_text(
            const std::wstring& text,
            float x,
            float y,
            float width,
            float height,
            float size,
            D2D1_COLOR_F color,
            bool semibold = false);

        void draw_text_centered(
            const std::wstring& text,
            float x,
            float y,
            float width,
            float height,
            float size,
            D2D1_COLOR_F color,
            bool semibold = false);

        void fill_rect(
            float x,
            float y,
            float width,
            float height,
            D2D1_COLOR_F color,
            float radius = 8.0f);

        void draw_icon(
            int icon,
            float x,
            float y,
            D2D1_COLOR_F color);

        void handle_click(float x, float y);
        void inspect_file();
        void inspect_policies();
        void set_page(Page page);

        Application& application_;

        HWND window_ = nullptr;
        HINSTANCE instance_ = nullptr;

        Page page_ = Page::Overview;

        std::wstring status_ = L"Ready";
        std::vector<std::wstring> report_lines_;

        POINT mouse_position_{ -1, -1 };
        bool mouse_tracking_ = false;

        Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory_;
        Microsoft::WRL::ComPtr<IDWriteFactory> write_factory_;

        Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> target_;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;

        Microsoft::WRL::ComPtr<IDWriteTextFormat> text_regular_;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> text_semibold_;
    };
}

#endif
