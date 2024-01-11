#include <iostream>
#include <locale>
#include <string>
#include <codecvt>

#include <Windows.h>
#include <windowsx.h>

POINT clickOffset{};

HWND g_hwnd;

bool g_altKeyPressed = false;
const float g_cornerSize = 0.3;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (pKeyboardStruct->vkCode == VK_LMENU) { // VK_MENU is the virtual key code for Alt
                g_altKeyPressed = true;
            }
        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (pKeyboardStruct->vkCode == VK_LMENU) {
                g_altKeyPressed = false;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK GetResizeRegion(HWND hWnd, POINT cursorPos /* in screen coordinates*/, float cornerSize) {
    // Get the dimensions of the window
    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);

    // Calculate the corner threshold (20% of the window's dimensions)
    int cornerWidth = (windowRect.right - windowRect.left) * cornerSize;
    int cornerHeight = (windowRect.bottom - windowRect.top) * cornerSize;

    // Check if the cursor is within the corner regions
    if (cursorPos.x >= windowRect.right - cornerWidth) {
        if (cursorPos.y >= windowRect.bottom - cornerHeight) {
            return HTBOTTOMRIGHT;
        }
        if (cursorPos.y <= windowRect.top + cornerHeight) {
            return HTTOPRIGHT;
        }
    }
    if (cursorPos.x <= windowRect.left + cornerWidth) {
        if (cursorPos.y >= windowRect.bottom - cornerHeight) {
            return HTBOTTOMLEFT;
        }
        if (cursorPos.y <= windowRect.top + cornerHeight) {
            return HTTOPLEFT;
        }
    }

    // Continue with your existing logic for edges
    int leftDistance = cursorPos.x - windowRect.left;
    int rightDistance = windowRect.right - cursorPos.x;
    int topDistance = cursorPos.y - windowRect.top;
    int bottomDistance = windowRect.bottom - cursorPos.y;

    int minDistance = min(min(leftDistance, rightDistance), min(topDistance, bottomDistance));

    if (minDistance == leftDistance) return HTLEFT;
    else if (minDistance == rightDistance) return HTRIGHT;
    else if (minDistance == topDistance) return HTTOP;
    else if (minDistance == bottomDistance) return HTBOTTOM;

    return HTCLIENT;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NCHITTEST:
    {
        if (!g_altKeyPressed) return HTCAPTION;
        // Get the cursor position in screen coordinates
        POINT cursorPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        return GetResizeRegion(hWnd, cursorPos, g_cornerSize);
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN: {
        // note that we don't need to handle the WM_LBUTTONUP event since
        // windows handles the resizing and moving of the window itself.
        if (g_altKeyPressed)
        {
            POINT cursorPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ClientToScreen(hWnd, &cursorPos);
            return SendMessage(hWnd, WM_NCLBUTTONDOWN, GetResizeRegion(hWnd, cursorPos, g_cornerSize), lParam);
        }
        else
        {
            return SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }
    }
    case WM_SIZE: {
        InvalidateRect(hWnd, nullptr, TRUE);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        COLORREF color = RGB(172, 216, 230);
        HBRUSH hBrush = CreateSolidBrush(color);
        FillRect(hdc, &ps.rcPaint, hBrush);
        EndPaint(hWnd, &ps);
        return 0;
    }
    default:
        //std::wcout << uMsg << L": got some other event!" << std::endl;
        
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void setup_wcout()
{
    SetConsoleOutputCP(CP_UTF8);
    std::ios_base::sync_with_stdio(false);

    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);
}

int main()
{
    setup_wcout();


    WNDCLASS wc{};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"MyWindowClass";

    RegisterClass(&wc);

    g_hwnd = CreateWindowExW(
        0,                              // Optional window styles.
        L"MyWindowClass",                // Window class
        L"My Window",                    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,

        NULL,       // Parent window    
        NULL,       // Menu
        wc.hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (g_hwnd == NULL) {
        return 0;
    }

    ShowWindow(g_hwnd, SW_SHOW);

    HHOOK hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (hKeyboardHook == NULL) {
        // Handle error
        return -1;
    }


    MSG msg {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}