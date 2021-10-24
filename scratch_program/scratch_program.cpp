// scratch_program.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "scratch_program.h"

// Global Variables:
HINSTANCE g_hinst; // current instance

class Window
{
public:
    HWND GetHWND() { return m_hwnd; }
protected:
    virtual LRESULT HandleMessage(
        UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void PaintContent(PAINTSTRUCT* pps) {}
    virtual LPCTSTR ClassName() = 0;
    virtual BOOL WinRegisterClass(WNDCLASS* pwc)
    {
        return RegisterClass(pwc);
    }
    virtual ~Window() {}
    HWND WinCreateWindow(DWORD dwExStyle, LPCTSTR pszName,
        DWORD dwStyle, int x, int y, int cx, int cy,
        HWND hwndParent, HMENU hmenu)
    {
        Register();
        return CreateWindowEx(dwExStyle, ClassName(), pszName, dwStyle,
            x, y, cx, cy, hwndParent, hmenu, g_hinst, this);
    }
private:
    void Register();
    void OnPaint();
    void OnPrintClient(HDC hdc);
    static LRESULT CALLBACK s_WndProc(HWND hwnd,
        UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
    HWND m_hwnd;
};

void Window::Register()
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Window::s_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = g_hinst;
    wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SCRATCHPROGRAM));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SCRATCHPROGRAM);
    wcex.lpszClassName = ClassName();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex);
}
LRESULT CALLBACK Window::s_WndProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* self;
    if(uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        self = reinterpret_cast<Window*>(lpcs->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA,
            reinterpret_cast<LPARAM>(self));
    }
    else
    {
        self = reinterpret_cast<Window*>
            (GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    if(self)
    {
        return self->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
LRESULT Window::HandleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;
    switch(uMsg)
    {
    case WM_NCDESTROY:
        lres = DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
        delete this;
        return lres;
    case WM_PAINT:
        OnPaint();
        return 0;
    case WM_PRINTCLIENT:
        OnPrintClient(reinterpret_cast<HDC>(wParam));
        return 0;
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
void Window::OnPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);
    PaintContent(&ps);
    EndPaint(m_hwnd, &ps);
}
void Window::OnPrintClient(HDC hdc)
{
    PAINTSTRUCT ps;
    ps.hdc = hdc;
    GetClientRect(m_hwnd, &ps.rcPaint);
    PaintContent(&ps);
}


class RootWindow : public Window
{
public:
    virtual LPCTSTR ClassName() { return TEXT("Scratch"); }
    static RootWindow* Create();
protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate();
private:
    static LRESULT CALLBACK About(HWND hwnd,
        UINT uMsg, WPARAM wParam, LPARAM lParam);
    typedef Window super;
    HWND m_hwndChild;
};
LRESULT RootWindow::OnCreate()
{
    return 0;
}
// Message handler for about box.
LRESULT CALLBACK RootWindow::About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch(uMsg)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
LRESULT RootWindow::HandleMessage(
    UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch(wmId)
        {
        case IDM_ABOUT:
            DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hwnd, RootWindow::About);
            break;
        case IDM_EXIT:
            DestroyWindow(m_hwnd);
            break;
        default:
            return DefWindowProc(m_hwnd, wParam, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
        return OnCreate();
    case WM_NCDESTROY:
        // Death of the root window ends the thread
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if(m_hwndChild)
        {
            SetWindowPos(m_hwndChild, NULL, 0, 0,
                GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    case WM_SETFOCUS:
        if(m_hwndChild)
        {
            SetFocus(m_hwndChild);
        }
        return 0;
    }
    return super::HandleMessage(uMsg, wParam, lParam);
}
RootWindow* RootWindow::Create()
{
    RootWindow* self = new RootWindow();
    if(self && self->WinCreateWindow(0,
        TEXT("Scratch"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL))
    {
        return self;
    }
    delete self;
    return NULL;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    int result = 0;
    g_hinst = hInstance;
    if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        // Ensure that the common control DLL is loaded, and then create the header control.
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = (ICC_COOL_CLASSES | ICC_DATE_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES);
        InitCommonControlsEx(&icex);
        RootWindow* prw = RootWindow::Create();
        if(prw)
        {
            HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCRATCHPROGRAM));
            ShowWindow(prw->GetHWND(), nCmdShow);
            MSG msg;
            while(GetMessage(&msg, NULL, 0, 0))
            {
                if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            result = (int)msg.wParam;
        }
        CoUninitialize();
    }
    return result;
}