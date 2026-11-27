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
#include <stackwell/core/application.hpp>
namespace stackwell::suf {
class DesktopWindow {
public:
 explicit DesktopWindow(Application& application);
 ~DesktopWindow();
 DesktopWindow(const DesktopWindow&) = delete;
 DesktopWindow& operator=(const DesktopWindow&) = delete;
 bool create(HINSTANCE instance, int show_command);
 int run();
private:
 enum class Page { Overview, Plugins, Settings };
 static LRESULT CALLBACK window_proc(HWND, UINT, WPARAM, LPARAM);
 LRESULT handle_message(UINT, WPARAM, LPARAM);
 bool create_device_resources();
 void discard_device_resources();
 void render();
 void draw_page();
 void draw_text(const std::wstring&, float, float, float, float, float, D2D1_COLOR_F, bool = false);
 void fill_rect(float, float, float, float, D2D1_COLOR_F, float = 7.0f);
 void set_page(Page);
 Application& application_;
 HWND window_ = nullptr;
 HINSTANCE instance_ = nullptr;
 Page page_ = Page::Overview;
 POINT mouse_{-1, -1};
 bool tracking_mouse_ = false;
 Microsoft::WRL::ComPtr<ID2D1Factory> d2d_factory_;
 Microsoft::WRL::ComPtr<IDWriteFactory> write_factory_;
 Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> target_;
 Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_;
 Microsoft::WRL::ComPtr<IDWriteTextFormat> regular_;
 Microsoft::WRL::ComPtr<IDWriteTextFormat> semibold_;
};
}
#endif
