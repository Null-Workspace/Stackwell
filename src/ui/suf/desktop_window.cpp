
#ifdef _WIN32


#include <stackwell/ui/suf/desktop_window.hpp>

#include <commdlg.h>
#include <windowsx.h>
#include <d2d1helper.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>

#pragma comment(lib, "Comdlg32.lib")

namespace stackwell::suf
{
    namespace
    {
        constexpr float SidebarWidth = 232.0f;
        constexpr float TitleBarHeight = 40.0f;
        constexpr float HeaderHeight = 116.0f;

        constexpr float WindowWidth = 1180.0f;
        constexpr float WindowHeight = 760.0f;

        constexpr float MinimumWindowWidth = 900.0f;
        constexpr float MinimumWindowHeight = 620.0f;
        constexpr float ResizeBorder = 8.0f;

        const D2D1_COLOR_F Background = D2D1::ColorF(0x101113);
        const D2D1_COLOR_F Sidebar = D2D1::ColorF(0x15171A);
        const D2D1_COLOR_F TitleBar = D2D1::ColorF(0x121416);

        const D2D1_COLOR_F Surface = D2D1::ColorF(0x1B1E22);
        const D2D1_COLOR_F SurfaceHover = D2D1::ColorF(0x24282D);

        const D2D1_COLOR_F Border = D2D1::ColorF(0x2A2E33);

        const D2D1_COLOR_F Primary = D2D1::ColorF(0xE8EAED);
        const D2D1_COLOR_F Secondary = D2D1::ColorF(0x9299A2);

        const D2D1_COLOR_F Accent = D2D1::ColorF(0xDADADA);
        const D2D1_COLOR_F AccentSoft = D2D1::ColorF(0x303236);

        const D2D1_COLOR_F Green = D2D1::ColorF(0xC8C8C8);
        const D2D1_COLOR_F Red = D2D1::ColorF(0x8A8A8A);

        const D2D1_COLOR_F CloseHover = D2D1::ColorF(0x3A3A3A);

        std::wstring widen(const std::string& value)
        {
            if (value.empty())
                return {};

            const int count = MultiByteToWideChar(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                nullptr,
                0);

            if (count <= 0)
                return L"(text unavailable)";

            std::wstring result(
                static_cast<std::size_t>(count),
                L'\0');

            MultiByteToWideChar(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                result.data(),
                count);

            return result;
        }
    }

    DesktopWindow::DesktopWindow(Application& application)
        : application_(application)
    {
    }

    DesktopWindow::~DesktopWindow()
    {
        discard_device_resources();
    }

    bool DesktopWindow::create(HINSTANCE instance, int show_command)
    {
        instance_ = instance;

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = window_proc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        wc.hbrBackground = nullptr;
        wc.lpszClassName = L"StackwellDesktopWindow";

        if (!RegisterClassExW(&wc) &&
            GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }

        RECT rect{
            0,
            0,
            static_cast<LONG>(WindowWidth),
            static_cast<LONG>(WindowHeight)
        };

        const DWORD window_style =
            WS_POPUP |
            WS_THICKFRAME |
            WS_MINIMIZEBOX |
            WS_MAXIMIZEBOX |
            WS_SYSMENU;

        AdjustWindowRectEx(
            &rect,
            window_style,
            FALSE,
            0);

        window_ = CreateWindowExW(
            0,
            wc.lpszClassName,
            L"Stackwell",
            window_style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            instance,
            this);

        if (!window_)
            return false;

        ShowWindow(window_, show_command);
        UpdateWindow(window_);

        return true;
    }

    int DesktopWindow::run()
    {
        MSG message{};
        BOOL result = FALSE;

        while ((result = GetMessageW(
            &message,
            nullptr,
            0,
            0)) > 0)
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return result == -1
            ? 1
            : static_cast<int>(message.wParam);
    }

    LRESULT CALLBACK DesktopWindow::window_proc(
        HWND window,
        UINT message,
        WPARAM wparam,
        LPARAM lparam)
    {
        DesktopWindow* self =
            reinterpret_cast<DesktopWindow*>(
                GetWindowLongPtrW(window, GWLP_USERDATA));

        if (message == WM_NCCREATE)
        {
            const auto* create =
                reinterpret_cast<CREATESTRUCTW*>(lparam);

            self = static_cast<DesktopWindow*>(
                create->lpCreateParams);

            self->window_ = window;

            SetWindowLongPtrW(
                window,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(self));
        }

        return self
            ? self->handle_message(message, wparam, lparam)
            : DefWindowProcW(window, message, wparam, lparam);
    }

    LRESULT DesktopWindow::handle_message(
        UINT message,
        WPARAM wparam,
        LPARAM lparam)
    {
        switch (message)
        {
        case WM_NCCALCSIZE:
            if (wparam)
                return 0;
            break;

        case WM_GETMINMAXINFO:
        {
            auto* info =
                reinterpret_cast<MINMAXINFO*>(lparam);

            info->ptMinTrackSize.x =
                static_cast<LONG>(MinimumWindowWidth);

            info->ptMinTrackSize.y =
                static_cast<LONG>(MinimumWindowHeight);

            return 0;
        }

        case WM_NCHITTEST:
        {
            POINT point{
                GET_X_LPARAM(lparam),
                GET_Y_LPARAM(lparam)
            };

            ScreenToClient(window_, &point);

            RECT client{};
            GetClientRect(window_, &client);

            const LONG width = client.right;
            const LONG height = client.bottom;
            const LONG border =
                static_cast<LONG>(ResizeBorder);

            if (!IsZoomed(window_))
            {
                const bool left = point.x < border;
                const bool right = point.x >= width - border;
                const bool top = point.y < border;
                const bool bottom = point.y >= height - border;

                if (top && left) return HTTOPLEFT;
                if (top && right) return HTTOPRIGHT;
                if (bottom && left) return HTBOTTOMLEFT;
                if (bottom && right) return HTBOTTOMRIGHT;

                if (left) return HTLEFT;
                if (right) return HTRIGHT;
                if (top) return HTTOP;
                if (bottom) return HTBOTTOM;
            }

            // Window controls remain client-side.
            if (point.y < static_cast<LONG>(TitleBarHeight) &&
                point.x >= width - 126)
            {
                return HTCLIENT;
            }

            // The rest of the title bar moves the window.
            if (point.y < static_cast<LONG>(TitleBarHeight))
                return HTCAPTION;

            return HTCLIENT;
        }

        case WM_NCLBUTTONDBLCLK:
            if (wparam == HTCAPTION)
            {
                ShowWindow(
                    window_,
                    IsZoomed(window_) ? SW_RESTORE : SW_MAXIMIZE);

                return 0;
            }
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps{};

            BeginPaint(window_, &ps);
            render();
            EndPaint(window_, &ps);

            return 0;
        }

        case WM_SIZE:
            if (target_)
            {
                target_->Resize(D2D1::SizeU(
                    LOWORD(lparam),
                    HIWORD(lparam)));
            }

            InvalidateRect(window_, nullptr, FALSE);
            return 0;

        case WM_MOUSEMOVE:
        {
            mouse_position_.x = GET_X_LPARAM(lparam);
            mouse_position_.y = GET_Y_LPARAM(lparam);

            if (!mouse_tracking_)
            {
                TRACKMOUSEEVENT tracking{
                    sizeof(TRACKMOUSEEVENT),
                    TME_LEAVE,
                    window_,
                    0
                };

                TrackMouseEvent(&tracking);
                mouse_tracking_ = true;
            }

            InvalidateRect(window_, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSELEAVE:
            mouse_tracking_ = false;
            mouse_position_ = POINT{ -1, -1 };

            InvalidateRect(window_, nullptr, FALSE);
            return 0;

        case WM_LBUTTONDBLCLK:
            if (GET_Y_LPARAM(lparam) <
                static_cast<int>(TitleBarHeight))
            {
                ShowWindow(
                    window_,
                    IsZoomed(window_) ? SW_RESTORE : SW_MAXIMIZE);

                return 0;
            }
            break;

        case WM_LBUTTONUP:
        {
            const float x =
                static_cast<float>(GET_X_LPARAM(lparam));

            const float y =
                static_cast<float>(GET_Y_LPARAM(lparam));

            RECT client{};
            GetClientRect(window_, &client);

            if (y < TitleBarHeight)
            {
                if (x >= client.right - 42.0f)
                {
                    DestroyWindow(window_);
                }
                else if (x >= client.right - 84.0f)
                {
                    ShowWindow(
                        window_,
                        IsZoomed(window_) ? SW_RESTORE : SW_MAXIMIZE);
                }
                else if (x >= client.right - 126.0f)
                {
                    ShowWindow(window_, SW_MINIMIZE);
                }

                return 0;
            }

            handle_click(x, y);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(
                window_,
                message,
                wparam,
                lparam);
        }

        return DefWindowProcW(
            window_,
            message,
            wparam,
            lparam);
    }

    bool DesktopWindow::create_device_resources()
    {
        if (target_)
            return true;

        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            d2d_factory_.ReleaseAndGetAddressOf())))
        {
            discard_device_resources();
            return false;
        }

        if (FAILED(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(
                write_factory_.ReleaseAndGetAddressOf()))))
        {
            discard_device_resources();
            return false;
        }

        RECT rect{};
        GetClientRect(window_, &rect);

        const auto size = D2D1::SizeU(
            static_cast<UINT32>(
                std::max<LONG>(1, rect.right - rect.left)),
            static_cast<UINT32>(
                std::max<LONG>(1, rect.bottom - rect.top)));

        if (FAILED(d2d_factory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(window_, size),
            target_.GetAddressOf())))
        {
            discard_device_resources();
            return false;
        }

        if (FAILED(target_->CreateSolidColorBrush(
            Primary,
            brush_.GetAddressOf())))
        {
            discard_device_resources();
            return false;
        }

        target_->SetAntialiasMode(
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        if (FAILED(write_factory_->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"en-us",
            text_regular_.GetAddressOf())))
        {
            discard_device_resources();
            return false;
        }

        if (FAILED(write_factory_->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"en-us",
            text_semibold_.GetAddressOf())))
        {
            discard_device_resources();
            return false;
        }

        return true;
    }

    void DesktopWindow::discard_device_resources()
    {
        text_regular_.Reset();
        text_semibold_.Reset();

        brush_.Reset();
        target_.Reset();

        write_factory_.Reset();
        d2d_factory_.Reset();
    }

    void DesktopWindow::fill_rect(
        float x,
        float y,
        float width,
        float height,
        D2D1_COLOR_F color,
        float radius)
    {
        if (!target_ || !brush_ || width <= 0.0f || height <= 0.0f)
            return;

        brush_->SetColor(color);

        const auto rect = D2D1::RoundedRect(
            D2D1::RectF(
                x,
                y,
                x + width,
                y + height),
            radius,
            radius);

        target_->FillRoundedRectangle(
            rect,
            brush_.Get());
    }

    void DesktopWindow::draw_text(
        const std::wstring& text,
        float x,
        float y,
        float width,
        float height,
        float size,
        D2D1_COLOR_F color,
        bool semibold)
    {
        if (!target_ || !brush_ || !write_factory_ ||
            width <= 0.0f || height <= 0.0f)
        {
            return;
        }

        Microsoft::WRL::ComPtr<IDWriteTextFormat> format;

        if (FAILED(write_factory_->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            semibold
                ? DWRITE_FONT_WEIGHT_SEMI_BOLD
                : DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us",
            format.GetAddressOf())))
        {
            return;
        }

        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        brush_->SetColor(color);

        target_->DrawText(
            text.c_str(),
            static_cast<UINT32>(text.size()),
            format.Get(),
            D2D1::RectF(
                x,
                y,
                x + width,
                y + height),
            brush_.Get(),
            D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    void DesktopWindow::draw_text_centered(
        const std::wstring& text,
        float x,
        float y,
        float width,
        float height,
        float size,
        D2D1_COLOR_F color,
        bool semibold)
    {
        if (!target_ || !brush_ || !write_factory_ ||
            width <= 0.0f || height <= 0.0f)
        {
            return;
        }

        Microsoft::WRL::ComPtr<IDWriteTextFormat> format;

        if (FAILED(write_factory_->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            semibold
                ? DWRITE_FONT_WEIGHT_SEMI_BOLD
                : DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us",
            format.GetAddressOf())))
        {
            return;
        }

        format->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_CENTER);

        format->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        format->SetWordWrapping(
            DWRITE_WORD_WRAPPING_NO_WRAP);

        brush_->SetColor(color);

        target_->DrawText(
            text.c_str(),
            static_cast<UINT32>(text.size()),
            format.Get(),
            D2D1::RectF(
                x,
                y,
                x + width,
                y + height),
            brush_.Get(),
            D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    void DesktopWindow::draw_icon(
        int icon,
        float x,
        float y,
        D2D1_COLOR_F color)
    {
        if (!target_ || !brush_)
            return;

        brush_->SetColor(color);

        constexpr float stroke = 1.7f;

        auto line = [&](float x1, float y1, float x2, float y2)
        {
            target_->DrawLine(
                D2D1::Point2F(x + x1, y + y1),
                D2D1::Point2F(x + x2, y + y2),
                brush_.Get(),
                stroke);
        };

        auto rect = [&](float x1, float y1, float x2, float y2)
        {
            target_->DrawRectangle(
                D2D1::RectF(
                    x + x1,
                    y + y1,
                    x + x2,
                    y + y2),
                brush_.Get(),
                stroke);
        };

        auto circle = [&](float cx, float cy, float radius)
        {
            target_->DrawEllipse(
                D2D1::Ellipse(
                    D2D1::Point2F(x + cx, y + cy),
                    radius,
                    radius),
                brush_.Get(),
                stroke);
        };

        switch (icon)
        {
        case 0: // Overview
            rect(2, 2, 8, 8);
            rect(12, 2, 18, 8);
            rect(2, 12, 8, 18);
            rect(12, 12, 18, 18);
            break;

        case 1: // Plugins
            rect(3, 3, 17, 17);
            line(10, 3, 10, 17);
            line(3, 10, 17, 10);
            break;

        case 2: // Access Recovery
            line(10, 2, 18, 5);
            line(18, 5, 17, 13);
            line(17, 13, 10, 19);
            line(10, 19, 3, 13);
            line(3, 13, 2, 5);
            line(2, 5, 10, 2);
            line(6, 10, 9, 13);
            line(9, 13, 14, 7);
            break;

        case 3: // Diagnostics
            line(1, 10, 5, 10);
            line(5, 10, 8, 4);
            line(8, 4, 12, 16);
            line(12, 16, 15, 10);
            line(15, 10, 19, 10);
            break;

        case 4: // Settings
            circle(10, 10, 7);
            circle(10, 10, 2.5f);
            line(10, 0, 10, 3);
            line(10, 17, 10, 20);
            line(0, 10, 3, 10);
            line(17, 10, 20, 10);
            break;

        case 8: // Minimize
            line(4, 10, 16, 10);
            break;

        case 9: // Maximize
            rect(4, 4, 16, 16);
            break;

        case 10: // Close
            line(5, 5, 15, 15);
            line(15, 5, 5, 15);
            break;
        }
    }

    void DesktopWindow::render()
    {
        if (!create_device_resources())
            return;

        target_->BeginDraw();
        target_->Clear(Background);

        RECT client{};
        GetClientRect(window_, &client);

        const float width =
            static_cast<float>(client.right);

        const float height =
            static_cast<float>(client.bottom);

        // Main surfaces.
        fill_rect(
            0,
            0,
            width,
            TitleBarHeight,
            TitleBar,
            0);

        fill_rect(
            0,
            TitleBarHeight,
            SidebarWidth,
            height - TitleBarHeight,
            Sidebar,
            0);

        fill_rect(
            SidebarWidth,
            TitleBarHeight,
            width - SidebarWidth,
            HeaderHeight - TitleBarHeight,
            Background,
            0);

        // Separators.
        brush_->SetColor(Border);

        target_->DrawLine(
            D2D1::Point2F(SidebarWidth, TitleBarHeight),
            D2D1::Point2F(SidebarWidth, height),
            brush_.Get(),
            1.0f);

        target_->DrawLine(
            D2D1::Point2F(SidebarWidth, HeaderHeight),
            D2D1::Point2F(width, HeaderHeight),
            brush_.Get(),
            1.0f);

        // Title bar.
        draw_text(
            L"STACKWELL",
            18,
            10,
            150,
            20,
            11,
            Primary,
            true);

        draw_text(
            L"STACKWELL DESKTOP",
            SidebarWidth + 34,
            10,
            220,
            18,
            10,
            Secondary,
            true);

        const float control_y = 5.0f;
        const float control_w = 42.0f;

        const float close_x = width - control_w;
        const float max_x = close_x - control_w;
        const float min_x = max_x - control_w;

        const bool close_hover =
            mouse_position_.x >= close_x &&
            mouse_position_.x < width &&
            mouse_position_.y >= 0 &&
            mouse_position_.y < TitleBarHeight;

        const bool max_hover =
            mouse_position_.x >= max_x &&
            mouse_position_.x < close_x &&
            mouse_position_.y >= 0 &&
            mouse_position_.y < TitleBarHeight;

        const bool min_hover =
            mouse_position_.x >= min_x &&
            mouse_position_.x < max_x &&
            mouse_position_.y >= 0 &&
            mouse_position_.y < TitleBarHeight;

        if (min_hover)
        {
            fill_rect(
                min_x,
                control_y,
                control_w,
                30,
                SurfaceHover,
                4);
        }

        if (max_hover)
        {
            fill_rect(
                max_x,
                control_y,
                control_w,
                30,
                SurfaceHover,
                4);
        }

        if (close_hover)
        {
            fill_rect(
                close_x,
                control_y,
                control_w,
                30,
                CloseHover,
                4);
        }

        draw_icon(
            8,
            min_x + 11,
            control_y + 5,
            Primary);

        draw_icon(
            9,
            max_x + 11,
            control_y + 5,
            Primary);

        draw_icon(
            10,
            close_x + 11,
            control_y + 5,
            Primary);

        // Sidebar brand.
        fill_rect(
            27,
            67,
            3,
            36,
            Accent,
            2);

        draw_text(
            L"STACKWELL",
            40,
            64,
            160,
            28,
            19,
            Primary,
            true);

        draw_text(
            L"SYSTEM ADMINISTRATION",
            41,
            93,
            170,
            18,
            9,
            Secondary,
            true);

        draw_text(
            L"WORKSPACE",
            26,
            152,
            180,
            20,
            10,
            Secondary,
            true);

        struct NavItem
        {
            Page page;
            const wchar_t* label;
            float y;
            int icon;
        };

        const NavItem items[] = {
            { Page::Overview,       L"Overview",        182, 0 },
            { Page::Plugins,        L"Plugins",         230, 1 },
            { Page::AccessRecovery, L"Access Recovery", 278, 2 },
            { Page::Diagnostics,    L"Diagnostics",     326, 3 },
            { Page::Settings,       L"Settings",        374, 4 }
        };

        for (const auto& item : items)
        {
            const bool hovered =
                mouse_position_.x >= 12 &&
                mouse_position_.x <= 220 &&
                mouse_position_.y >= item.y - 7 &&
                mouse_position_.y <= item.y + 31;

            if (page_ == item.page)
            {
                fill_rect(
                    12,
                    item.y - 7,
                    208,
                    38,
                    SurfaceHover,
                    7);

                fill_rect(
                    12,
                    item.y + 2,
                    3,
                    20,
                    Accent,
                    2);
            }
            else if (hovered)
            {
                fill_rect(
                    12,
                    item.y - 7,
                    208,
                    38,
                    Surface,
                    7);
            }

            const auto icon_color =
                page_ == item.page || hovered
                    ? Primary
                    : Secondary;

            draw_icon(
                item.icon,
                29,
                item.y + 1,
                icon_color);

            draw_text(
                item.label,
                56,
                item.y,
                154,
                24,
                13,
                icon_color,
                page_ == item.page);
        }

        // Current page title.
        const wchar_t* title = L"Overview";

        switch (page_)
        {
        case Page::Plugins:
            title = L"Plugin Manager";
            break;

        case Page::AccessRecovery:
            title = L"Access Recovery";
            break;

        case Page::Diagnostics:
            title = L"Diagnostics";
            break;

        case Page::Settings:
            title = L"Settings";
            break;

        default:
            break;
        }

        draw_text(
            title,
            SidebarWidth + 34,
            79,
            420,
            28,
            19,
            Primary,
            true);

        draw_page();

        // Status is intentionally hidden when it is "Ready".
        if (status_ != L"Ready")
        {
            draw_text(
                status_,
                SidebarWidth + 34,
                height - 30,
                width - SidebarWidth - 68,
                18,
                9,
                Secondary);
        }

        const HRESULT result = target_->EndDraw();

        if (result == D2DERR_RECREATE_TARGET)
            discard_device_resources();
    }

    void DesktopWindow::draw_page()
    {
        const float left = SidebarWidth + 34;
        const float top = HeaderHeight + 30;

        RECT client{};
        GetClientRect(window_, &client);

        const float available =
            std::max(100.0f,
                static_cast<float>(client.right) - left - 34.0f);

        if (page_ == Page::Overview)
        {
            draw_text(
                L"Welcome back",
                left,
                top,
                400,
                34,
                25,
                Primary,
                true);

            draw_text(
                L"Your Stackwell environment at a glance.",
                left,
                top + 39,
                520,
                24,
                13,
                Secondary);

            const float gap = 16.0f;
            const float card =
                std::max(100.0f, (available - gap * 2) / 3.0f);

            const auto& plugins =
                application_.get_plugin_manager().get_plugins();

            const std::wstring plugin_count =
                std::to_wstring(plugins.size());

            const std::wstring loaded_count =
                std::to_wstring(
                    application_.get_plugin_manager().get_loaded_count());

            const struct
            {
                const wchar_t* label;
                std::wstring value;
                const wchar_t* note;
            } cards[] = {
                { L"APPLICATION", L"0.0.1", L"Current version" },
                { L"PLUGINS", plugin_count, L"Discovered modules" },
                { L"ACTIVE", loaded_count, L"Loaded plugins" }
            };

            for (int i = 0; i < 3; ++i)
            {
                const float x =
                    left + i * (card + gap);

                fill_rect(
                    x,
                    top + 100,
                    card,
                    122,
                    Surface);

                draw_text(
                    cards[i].label,
                    x + 18,
                    top + 118,
                    card - 36,
                    18,
                    10,
                    Secondary,
                    true);

                draw_text(
                    cards[i].value,
                    x + 18,
                    top + 147,
                    card - 36,
                    40,
                    27,
                    Primary,
                    true);

                draw_text(
                    cards[i].note,
                    x + 18,
                    top + 190,
                    card - 36,
                    18,
                    11,
                    Secondary);
            }

            fill_rect(
                left,
                top + 244,
                available,
                170,
                Surface);

            draw_text(
                L"Quick access",
                left + 20,
                top + 263,
                300,
                24,
                15,
                Primary,
                true);

            draw_text(
                L"Manage plugins and inspect Windows access settings from the navigation.",
                left + 20,
                top + 300,
                available - 40,
                24,
                12,
                Secondary);

            draw_text(
                L"Core status",
                left + 20,
                top + 350,
                140,
                20,
                11,
                Secondary);

            draw_text(
                L"Initialized",
                left + 140,
                top + 350,
                160,
                20,
                11,
                Green,
                true);
        }
        else if (page_ == Page::Plugins)
        {
            const auto& plugins =
                application_.get_plugin_manager().get_plugins();

            draw_text(
                L"Installed plugins",
                left,
                top,
                400,
                30,
                21,
                Primary,
                true);

            draw_text(
                L"Modules discovered by the Stackwell plugin manager.",
                left,
                top + 35,
                560,
                22,
                12,
                Secondary);

            float y = top + 82;

            if (plugins.empty())
            {
                fill_rect(
                    left,
                    y,
                    available,
                    84,
                    Surface);

                draw_text(
                    L"No plugins found",
                    left + 18,
                    y + 18,
                    300,
                    24,
                    14,
                    Primary,
                    true);

                draw_text(
                    L"Place plugin modules in the configured plugins directory.",
                    left + 18,
                    y + 46,
                    available - 36,
                    20,
                    11,
                    Secondary);
            }

            for (const auto& plugin : plugins)
            {
                fill_rect(
                    left,
                    y,
                    available,
                    92,
                    Surface);

                draw_text(
                    widen(plugin.name),
                    left + 18,
                    y + 14,
                    available - 36,
                    24,
                    14,
                    Primary,
                    true);

                draw_text(
                    widen(plugin.description),
                    left + 18,
                    y + 43,
                    available - 36,
                    18,
                    11,
                    Secondary);

                draw_text(
                    widen(
                        std::string("v") +
                        plugin.version +
                        "  /  " +
                        plugin_state_to_string(plugin.state)),
                    left + 18,
                    y + 66,
                    available - 36,
                    18,
                    10,
                    plugin.state == PluginState::Failed
                        ? Red
                        : Green);

                y += 104;
            }
        }
        else if (page_ == Page::AccessRecovery)
        {
            draw_text(
                L"Inspect access configuration",
                left,
                top,
                560,
                32,
                22,
                Primary,
                true);

            draw_text(
                L"Review file permissions and selected Windows policy values safely.",
                left,
                top + 38,
                available - 10,
                22,
                12,
                Secondary);

            const float card_gap = 16.0f;

            const float card_width =
                (available - card_gap) / 2.0f;

            const float card_y = top + 82;

            const bool file_hover =
                mouse_position_.x >= left + 18 &&
                mouse_position_.x <= left + 222 &&
                mouse_position_.y >= card_y + 108 &&
                mouse_position_.y <= card_y + 150;

            const float second_x =
                left + card_width + card_gap;

            const bool policy_hover =
                mouse_position_.x >= second_x + 18 &&
                mouse_position_.x <= second_x + 222 &&
                mouse_position_.y >= card_y + 108 &&
                mouse_position_.y <= card_y + 150;

            // File inspection.
            fill_rect(
                left,
                card_y,
                card_width,
                156,
                Surface);

            fill_rect(
                left,
                card_y,
                34,
                2,
                Border,
                1);

            draw_text(
                L"01  /  FILE SECURITY",
                left + 20,
                card_y + 18,
                card_width - 40,
                18,
                9,
                Secondary,
                true);

            draw_text(
                L"NTFS inspection",
                left + 18,
                card_y + 43,
                card_width - 36,
                25,
                16,
                Primary,
                true);

            draw_text(
                L"Inspect owner, DACL and ACE metadata for a selected file.",
                left + 18,
                card_y + 74,
                card_width - 36,
                30,
                11,
                Secondary);

            fill_rect(
                left + 18,
                card_y + 108,
                204,
                42,
                file_hover ? SurfaceHover : Surface,
                7);

            draw_text_centered(
                L"Choose file",
                left + 18,
                card_y + 108,
                204,
                42,
                11,
                Primary,
                true);

            // Policy inspection.
            fill_rect(
                second_x,
                card_y,
                card_width,
                156,
                Surface);

            fill_rect(
                second_x,
                card_y,
                34,
                2,
                Border,
                1);

            draw_text(
                L"02  /  WINDOWS POLICY",
                second_x + 20,
                card_y + 18,
                card_width - 40,
                18,
                9,
                Secondary,
                true);

            draw_text(
                L"Policy inspection",
                second_x + 18,
                card_y + 43,
                card_width - 36,
                25,
                16,
                Primary,
                true);

            draw_text(
                L"Check a focused set of known user and machine policy values.",
                second_x + 18,
                card_y + 74,
                card_width - 36,
                30,
                11,
                Secondary);

            fill_rect(
                second_x + 18,
                card_y + 108,
                204,
                42,
                policy_hover ? SurfaceHover : Surface,
                7);

            draw_text_centered(
                L"Inspect policies",
                second_x + 18,
                card_y + 108,
                204,
                42,
                11,
                Primary,
                true);

            // Results.
            const float result_y = card_y + 176;

            fill_rect(
                left,
                result_y,
                available,
                190,
                Surface);

            draw_text(
                L"Inspection results",
                left + 18,
                result_y + 16,
                300,
                24,
                15,
                Primary,
                true);

            fill_rect(
                left + available - 92,
                result_y + 16,
                74,
                22,
                AccentSoft,
                11);

            draw_text(
                report_lines_.empty()
                    ? L"STANDBY"
                    : L"COMPLETED",
                left + available - 84,
                result_y + 20,
                62,
                14,
                8,
                Accent,
                true);

            if (report_lines_.empty())
            {
                draw_text(
                    L"No inspection has been run yet.",
                    left + 18,
                    result_y + 58,
                    available - 36,
                    22,
                    12,
                    Secondary);

                draw_text(
                    L"Choose one of the read-only checks above to view its findings here.",
                    left + 18,
                    result_y + 86,
                    available - 36,
                    22,
                    11,
                    Secondary);
            }
            else
            {
                float y = result_y + 52;

                for (const auto& line : report_lines_)
                {
                    draw_text(
                        line,
                        left + 18,
                        y,
                        available - 36,
                        19,
                        10,
                        Secondary);

                    y += 22;

                    if (y > result_y + 176)
                        break;
                }
            }
        }
        else if (page_ == Page::Diagnostics)
        {
            draw_text(
                L"Diagnostics",
                left,
                top,
                420,
                30,
                21,
                Primary,
                true);

            draw_text(
                L"Runtime information from the active Stackwell Core instance.",
                left,
                top + 36,
                700,
                22,
                12,
                Secondary);

            fill_rect(
                left,
                top + 82,
                available,
                130,
                Surface);

            draw_text(
                L"Core",
                left + 18,
                top + 100,
                160,
                22,
                13,
                Primary,
                true);

            draw_text(
                L"Initialized successfully",
                left + 18,
                top + 132,
                340,
                20,
                12,
                Green);

            draw_text(
                L"Plugin manager",
                left + 18,
                top + 166,
                160,
                20,
                12,
                Secondary);

            draw_text(
                std::to_wstring(
                    application_.get_plugin_manager().get_loaded_count()) +
                    L" loaded module(s)",
                left + 180,
                top + 166,
                260,
                20,
                12,
                Primary);
        }
        else
        {
            draw_text(
                L"Application settings",
                left,
                top,
                420,
                30,
                21,
                Primary,
                true);

            draw_text(
                L"Configuration is currently managed by resources/config/config.json.",
                left,
                top + 36,
                760,
                22,
                12,
                Secondary);

            fill_rect(
                left,
                top + 82,
                available,
                92,
                Surface);

            draw_text(
                L"Configuration file",
                left + 18,
                top + 100,
                240,
                22,
                13,
                Primary,
                true);

            draw_text(
                L"resources/config/config.json",
                left + 18,
                top + 134,
                available - 36,
                20,
                11,
                Secondary);
        }
    }

    void DesktopWindow::handle_click(float x, float y)
    {
        if (x < SidebarWidth)
        {
            if (y >= 175 && y < 223)
                set_page(Page::Overview);

            else if (y >= 223 && y < 271)
                set_page(Page::Plugins);

            else if (y >= 271 && y < 319)
                set_page(Page::AccessRecovery);

            else if (y >= 319 && y < 367)
                set_page(Page::Diagnostics);

            else if (y >= 367 && y < 415)
                set_page(Page::Settings);

            return;
        }

        if (page_ == Page::AccessRecovery)
        {
            RECT client{};
            GetClientRect(window_, &client);

            const float left = SidebarWidth + 34;
            const float top = HeaderHeight + 30;

            const float available =
                std::max(100.0f,
                    static_cast<float>(client.right) - left - 34.0f);

            const float card_width =
                (available - 16.0f) / 2.0f;

            const float card_y = top + 82;

            if (x >= left + 18 &&
                x <= left + 222 &&
                y >= card_y + 108 &&
                y <= card_y + 150)
            {
                inspect_file();
            }
            else if (x >= left + card_width + 34 &&
                x <= left + card_width + 222 &&
                y >= card_y + 108 &&
                y <= card_y + 150)
            {
                inspect_policies();
            }
        }
    }

    void DesktopWindow::inspect_file()
    {
        wchar_t path[MAX_PATH]{};

        OPENFILENAMEW dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = window_;
        dialog.lpstrFile = path;
        dialog.nMaxFile = MAX_PATH;
        dialog.lpstrTitle = L"Select a file to inspect";

        dialog.Flags =
            OFN_FILEMUSTEXIST |
            OFN_PATHMUSTEXIST |
            OFN_NOCHANGEDIR;

        if (!GetOpenFileNameW(&dialog))
            return;

        AccessRecovery recovery;

        const auto report =
            recovery.inspect_path(std::filesystem::path(path));

        report_lines_.clear();

        report_lines_.push_back(
            L"Path: " +
            std::filesystem::path(path).filename().wstring());

        report_lines_.push_back(
            std::wstring(L"Inspection: ") +
            (report.inspected ? L"completed" : L"not completed"));

        report_lines_.push_back(
            L"Owner SID: " +
            (report.owner_available
                ? widen(report.owner_sid)
                : L"unavailable"));

        report_lines_.push_back(
            std::wstring(L"DACL present: ") +
            (report.dacl_present ? L"yes" : L"no"));

        report_lines_.push_back(
            L"ACE count: " +
            std::to_wstring(report.ace_count));

        report_lines_.push_back(
            L"Result: " +
            widen(report.message));

        status_ =
            L"File security inspection finished (read-only)";

        InvalidateRect(window_, nullptr, FALSE);
    }

    void DesktopWindow::inspect_policies()
    {
        AccessRecovery recovery;

        const auto findings =
            recovery.inspect_known_policies();

        report_lines_.clear();

        if (findings.empty())
        {
            report_lines_.push_back(
                L"No configured values from the inspection list were found.");
        }

        for (const auto& finding : findings)
        {
            report_lines_.push_back(
                widen(
                    finding.hive +
                    "\\" +
                    finding.key +
                    "\\" +
                    finding.value_name));

            report_lines_.push_back(
                L"DWORD: " +
                std::to_wstring(finding.value));
        }

        status_ =
            L"Policy inspection finished (read-only)";

        InvalidateRect(window_, nullptr, FALSE);
    }

    void DesktopWindow::set_page(Page page)
    {
        page_ = page;
        status_ = L"Ready";

        InvalidateRect(window_, nullptr, FALSE);
    }
}

#endif
