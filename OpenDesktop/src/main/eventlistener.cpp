#include "eventlistener.h"
#include <bitset>
#include "explorer.h"
static UINT CustomMessage;
static UINT TaskbarCreated;
HWND Listener;
LRESULT CALLBACK WindowProc(
  _In_ HWND   hwnd,
  _In_ UINT   uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
        ){
    if(uMsg==TaskbarCreated)Explorer::init();
    if(uMsg==CustomMessage){
        //Building the mouse event to send.
        QPoint pt(LOWORD(lParam),HIWORD(lParam));
        Explorer::DispatchMouseMessage(pt,wParam);
    };
    return DefWindowProcA(hwnd,uMsg,wParam,lParam);
}
bool StartListener(){
    CustomMessage=RegisterWindowMessageA("CustomDesktopBackground");
    TaskbarCreated=RegisterWindowMessageA("TaskbarCreated");
    WNDCLASS winclass;
    winclass.style=NULL;
    winclass.lpfnWndProc=WindowProc;
    winclass.cbClsExtra=NULL;
    winclass.cbWndExtra=NULL;
    winclass.hInstance=GetModuleHandleA(nullptr);
    winclass.hIcon=nullptr;
    winclass.hCursor=nullptr;
    winclass.hbrBackground=nullptr;
    winclass.lpszMenuName=nullptr;
    winclass.lpszClassName=L"DesktopMessageListener";
    RegisterClass(&winclass);
    Listener=CreateWindowA(
      "DesktopMessageListener",
      nullptr,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,CW_USEDEFAULT,
      CW_USEDEFAULT,CW_USEDEFAULT,
      nullptr,nullptr,GetModuleHandleA(nullptr),nullptr);
    if(Listener==nullptr)return false;
    ShowWindow(Listener,SW_HIDE);
    return true;
}
