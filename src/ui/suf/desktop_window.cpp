#ifdef _WIN32
#include <stackwell/ui/suf/desktop_window.hpp>
#include <d2d1helper.h>
#include <windowsx.h>
#include <algorithm>
#include <string>

namespace stackwell::suf {
namespace {
constexpr float sidebar_width=224.0f;
constexpr float title_height=42.0f;
constexpr float header_height=126.0f;
constexpr float min_width=820.0f;
constexpr float min_height=560.0f;
const auto background=D2D1::ColorF(0x101113);
const auto sidebar=D2D1::ColorF(0x15171A);
const auto surface=D2D1::ColorF(0x1B1E22);
const auto hover=D2D1::ColorF(0x262A2F);
const auto border=D2D1::ColorF(0x2B2F34);
const auto primary=D2D1::ColorF(0xECEDEF);
const auto secondary=D2D1::ColorF(0x9299A2);
std::wstring widen(const std::string& value) {
 if(value.empty()) return {};
 const int n=MultiByteToWideChar(CP_UTF8,0,value.data(),static_cast<int>(value.size()),nullptr,0);
 if(n<=0) return L"(unavailable)";
 std::wstring result(static_cast<std::size_t>(n),L'\0');
 MultiByteToWideChar(CP_UTF8,0,value.data(),static_cast<int>(value.size()),result.data(),n);
 return result;
}
}
DesktopWindow::DesktopWindow(Application& app):application_(app){}
DesktopWindow::~DesktopWindow(){discard_device_resources();}

bool DesktopWindow::create(HINSTANCE instance,int show_command) {
 instance_=instance;
 WNDCLASSEXW wc{}; wc.cbSize=sizeof(wc); wc.style=CS_HREDRAW|CS_VREDRAW;
 wc.lpfnWndProc=window_proc; wc.hInstance=instance;
 wc.hCursor=LoadCursorW(nullptr,IDC_ARROW); wc.lpszClassName=L"StackwellDesktop";
 if(!RegisterClassExW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return false;
 RECT r{0,0,1120,720};
 AdjustWindowRectEx(&r,WS_POPUP|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU,FALSE,0);
 window_=CreateWindowExW(0,wc.lpszClassName,L"Stackwell",
  WS_POPUP|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU,
  CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,
  nullptr,nullptr,instance,this);
 if(!window_) return false;
 ShowWindow(window_,show_command); UpdateWindow(window_); return true;
}
int DesktopWindow::run() {
 MSG msg{}; BOOL result=FALSE;
 while((result=GetMessageW(&msg,nullptr,0,0))>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 return result==-1?1:static_cast<int>(msg.wParam);
}
LRESULT CALLBACK DesktopWindow::window_proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
 auto* self=reinterpret_cast<DesktopWindow*>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
 if(msg==WM_NCCREATE){auto* cs=reinterpret_cast<CREATESTRUCTW*>(lp);self=static_cast<DesktopWindow*>(cs->lpCreateParams);self->window_=hwnd;SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}
 return self?self->handle_message(msg,wp,lp):DefWindowProcW(hwnd,msg,wp,lp);
}
LRESULT DesktopWindow::handle_message(UINT msg,WPARAM wp,LPARAM lp) {
 switch(msg) {
 case WM_NCCALCSIZE: if(wp) return 0; break;
 case WM_GETMINMAXINFO:{auto* m=reinterpret_cast<MINMAXINFO*>(lp);m->ptMinTrackSize={static_cast<LONG>(min_width),static_cast<LONG>(min_height)};return 0;}
 case WM_NCHITTEST:{
  POINT p{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};ScreenToClient(window_,&p);
  RECT r{};GetClientRect(window_,&r);const LONG b=7;
  if(!IsZoomed(window_)){
   bool l=p.x<b,rr=p.x>=r.right-b,t=p.y<b,bt=p.y>=r.bottom-b;
   if(t&&l)return HTTOPLEFT;if(t&&rr)return HTTOPRIGHT;if(bt&&l)return HTBOTTOMLEFT;if(bt&&rr)return HTBOTTOMRIGHT;
   if(l)return HTLEFT;if(rr)return HTRIGHT;if(t)return HTTOP;if(bt)return HTBOTTOM;
  }
  if(p.y<static_cast<LONG>(title_height)&&p.x<r.right-132)return HTCAPTION;
  return HTCLIENT;
 }
 case WM_PAINT:{PAINTSTRUCT ps{};BeginPaint(window_,&ps);render();EndPaint(window_,&ps);return 0;}
 case WM_SIZE:if(target_)target_->Resize(D2D1::SizeU(LOWORD(lp),HIWORD(lp)));InvalidateRect(window_,nullptr,FALSE);return 0;
 case WM_MOUSEMOVE:{
  mouse_={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
  if(!tracking_mouse_){TRACKMOUSEEVENT t{sizeof(t),TME_LEAVE,window_,0};TrackMouseEvent(&t);tracking_mouse_=true;}
  InvalidateRect(window_,nullptr,FALSE);return 0;
 }
 case WM_MOUSELEAVE:tracking_mouse_=false;mouse_={-1,-1};InvalidateRect(window_,nullptr,FALSE);return 0;
 case WM_LBUTTONUP:{
  const float x=static_cast<float>(GET_X_LPARAM(lp)),y=static_cast<float>(GET_Y_LPARAM(lp));
  RECT r{};GetClientRect(window_,&r);
  if(y<title_height){
   if(x>r.right-44)DestroyWindow(window_);
   else if(x>r.right-88)ShowWindow(window_,IsZoomed(window_)?SW_RESTORE:SW_MAXIMIZE);
   else if(x>r.right-132)ShowWindow(window_,SW_MINIMIZE);
   return 0;
  }
  if(x<sidebar_width){
   if(y>=174&&y<222)set_page(Page::Overview);
   else if(y>=222&&y<270)set_page(Page::Plugins);
   else if(y>=270&&y<318)set_page(Page::Settings);
  }
  return 0;
 }
 case WM_ERASEBKGND:return 1;
 case WM_DESTROY:PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(window_,msg,wp,lp);
}
bool DesktopWindow::create_device_resources() {
 if(target_)return true;
 if(FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,d2d_factory_.ReleaseAndGetAddressOf())))return false;
 if(FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),reinterpret_cast<IUnknown**>(write_factory_.ReleaseAndGetAddressOf())))){discard_device_resources();return false;}
 RECT r{};GetClientRect(window_,&r);
 auto size=D2D1::SizeU(static_cast<UINT32>(std::max<LONG>(1,r.right)),static_cast<UINT32>(std::max<LONG>(1,r.bottom)));
 if(FAILED(d2d_factory_->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),D2D1::HwndRenderTargetProperties(window_,size),target_.GetAddressOf()))){discard_device_resources();return false;}
 if(FAILED(target_->CreateSolidColorBrush(primary,brush_.GetAddressOf()))){discard_device_resources();return false;}
 if(FAILED(write_factory_->CreateTextFormat(L"Segoe UI",nullptr,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,12.0f,L"en-us",regular_.GetAddressOf()))){discard_device_resources();return false;}
 if(FAILED(write_factory_->CreateTextFormat(L"Segoe UI",nullptr,DWRITE_FONT_WEIGHT_SEMI_BOLD,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,12.0f,L"en-us",semibold_.GetAddressOf()))){discard_device_resources();return false;}
 regular_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);semibold_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
 return true;
}
void DesktopWindow::discard_device_resources(){regular_.Reset();semibold_.Reset();brush_.Reset();target_.Reset();write_factory_.Reset();d2d_factory_.Reset();}
void DesktopWindow::fill_rect(float x,float y,float w,float h,D2D1_COLOR_F c,float radius){
 if(!target_||!brush_)return;brush_->SetColor(c);
 auto rect=D2D1::RoundedRect(D2D1::RectF(x,y,x+w,y+h),radius,radius);
 target_->FillRoundedRectangle(rect,brush_.Get());
}
void DesktopWindow::draw_text(const std::wstring& text,float x,float y,float w,float h,float size,D2D1_COLOR_F color,bool bold){
 if(!target_||!brush_||text.empty())return;
 auto format=bold?semibold_:regular_;format->SetFontSize(size);brush_->SetColor(color);
 target_->DrawText(text.c_str(),static_cast<UINT32>(text.size()),format.Get(),D2D1::RectF(x,y,x+w,y+h),brush_.Get(),D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
void DesktopWindow::render() {
 if(!create_device_resources())return;
 target_->BeginDraw();target_->Clear(background);
 RECT r{};GetClientRect(window_,&r);const float w=static_cast<float>(r.right),h=static_cast<float>(r.bottom);
 fill_rect(0,0,w,title_height,D2D1::ColorF(0x121416),0);
 fill_rect(0,title_height,sidebar_width,h-title_height,sidebar,0);
 brush_->SetColor(border);
 target_->DrawLine(D2D1::Point2F(sidebar_width,title_height),D2D1::Point2F(sidebar_width,h),brush_.Get(),1);
 target_->DrawLine(D2D1::Point2F(sidebar_width,title_height+header_height-title_height),D2D1::Point2F(w,title_height+header_height-title_height),brush_.Get(),1);
 draw_text(L"STACKWELL",18,12,150,20,11,primary,true);
 draw_text(L"PLUGIN-FIRST HOST",sidebar_width+30,13,220,18,9,secondary,true);
 const float bx=w-132;
 for(int i=0;i<3;++i){float x=bx+i*44.0f;bool over=mouse_.x>=x&&mouse_.x<x+44&&mouse_.y>=0&&mouse_.y<title_height;if(over)fill_rect(x,5,42,32,i==2?D2D1::ColorF(0x383838):hover,4);
  brush_->SetColor(primary);
  if(i==0)target_->DrawLine(D2D1::Point2F(x+15,21),D2D1::Point2F(x+29,21),brush_.Get(),1.5f);
  if(i==1)target_->DrawRectangle(D2D1::RectF(x+15,13,x+29,27),brush_.Get(),1.5f);
  if(i==2){target_->DrawLine(D2D1::Point2F(x+16,14),D2D1::Point2F(x+28,26),brush_.Get(),1.5f);target_->DrawLine(D2D1::Point2F(x+28,14),D2D1::Point2F(x+16,26),brush_.Get(),1.5f);}
 }
 fill_rect(26,68,3,34,primary,1);draw_text(L"STACKWELL",40,66,160,26,18,primary,true);
 draw_text(L"APPLICATION HOST",41,94,160,16,9,secondary,true);
 draw_text(L"NAVIGATION",25,150,170,18,9,secondary,true);
 const struct{Page page;const wchar_t* name;float y;} nav[]={{Page::Overview,L"Overview",182},{Page::Plugins,L"Plugins",230},{Page::Settings,L"Settings",278}};
 for(const auto& item:nav){bool over=mouse_.x>=12&&mouse_.x<210&&mouse_.y>=item.y-7&&mouse_.y<item.y+30;
  if(page_==item.page||over)fill_rect(12,item.y-7,198,38,page_==item.page?hover:surface,6);
  if(page_==item.page)fill_rect(12,item.y+2,3,20,primary,1);
  draw_text(item.name,34,item.y,160,22,12,page_==item.page?primary:secondary,page_==item.page);
 }
 const wchar_t* title=page_==Page::Plugins?L"Plugin Manager":page_==Page::Settings?L"Settings":L"Overview";
 draw_text(title,sidebar_width+30,78,400,30,20,primary,true);
 draw_page();
 const HRESULT hr=target_->EndDraw();if(hr==D2DERR_RECREATE_TARGET)discard_device_resources();
}
void DesktopWindow::draw_page() {
 RECT r{};GetClientRect(window_,&r);const float left=sidebar_width+30,top=header_height+title_height+22,avail=std::max(120.0f,static_cast<float>(r.right)-left-30);
 if(page_==Page::Overview){
  draw_text(L"Welcome to Stackwell",left,top,500,34,24,primary,true);
  draw_text(L"A clean host. Functionality comes from plugins.",left,top+40,650,22,12,secondary);
  const auto& plugins=application_.get_plugin_manager().get_plugins();
  const float gap=14,card=(avail-gap*2)/3;
  const struct{const wchar_t* label;std::wstring value;const wchar_t* note;} cards[]={
   {L"PLUGIN FILES",std::to_wstring(plugins.size()),L"Discovered in plugin directory"},
   {L"INITIALIZED",std::to_wstring(application_.get_plugin_manager().get_loaded_count()),L"Running plugin modules"},
   {L"HOST VERSION",widen(application_.get_config().application.version),L"Current Stackwell build"}
  };
  for(int i=0;i<3;++i){float x=left+i*(card+gap);fill_rect(x,top+92,card,126,surface);
   draw_text(cards[i].label,x+17,top+110,card-34,18,9,secondary,true);
   draw_text(cards[i].value,x+17,top+140,card-34,38,25,primary,true);
   draw_text(cards[i].note,x+17,top+184,card-34,20,10,secondary);
  }
  fill_rect(left,top+240,avail,116,surface);
  draw_text(L"Host status",left+18,top+258,220,22,14,primary,true);
  draw_text(L"Core initialized",left+18,top+292,260,20,11,secondary);
  draw_text(L"All product features are supplied by installed plugins.",left+18,top+320,avail-36,20,11,secondary);
 } else if(page_==Page::Plugins){
  const auto& plugins=application_.get_plugin_manager().get_plugins();
  draw_text(L"Installed modules",left,top,400,28,21,primary,true);
  draw_text(L"One plugin is one native library file in the plugins directory.",left,top+36,avail,20,11,secondary);
  float y=top+76;
  if(plugins.empty()){
   fill_rect(left,y,avail,92,surface);draw_text(L"No plugins installed",left+18,y+18,320,22,14,primary,true);
   draw_text(L"Add a plugin DLL to the configured plugins directory, then restart Stackwell.",left+18,y+49,avail-36,20,11,secondary);
  }
  for(const auto& p:plugins){
   fill_rect(left,y,avail,88,surface);
   draw_text(widen(p.name.empty()?"Unknown plugin":p.name),left+18,y+12,avail-36,22,14,primary,true);
   draw_text(widen(p.description.empty()?p.path.filename().string():p.description),left+18,y+39,avail-36,18,10,secondary);
   std::string meta="v"+p.version+"  /  "+plugin_state_to_string(p.state);
   if(!p.error.empty())meta+="  /  "+p.error;
   draw_text(widen(meta),left+18,y+62,avail-36,18,9,p.state==PluginState::Failed?D2D1::ColorF(0xB0B0B0):secondary);
   y+=98;
  }
 } else {
  draw_text(L"Host configuration",left,top,420,28,21,primary,true);
  draw_text(L"Runtime settings are stored in the config file.",left,top+36,avail,20,11,secondary);
  fill_rect(left,top+76,avail,100,surface);
  draw_text(L"Configuration",left+18,top+94,220,20,13,primary,true);
  draw_text(L"resources/config/config.json",left+18,top+126,avail-36,20,11,secondary);
  fill_rect(left,top+192,avail,100,surface);
  draw_text(L"Plugin directory",left+18,top+210,220,20,13,primary,true);
  draw_text(widen(application_.get_config().plugins.directory.string()),left+18,top+242,avail-36,20,11,secondary);
 }
}
void DesktopWindow::set_page(Page page){page_=page;InvalidateRect(window_,nullptr,FALSE);}
}
#endif
