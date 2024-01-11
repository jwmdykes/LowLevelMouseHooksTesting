#include <iostream>
#include <locale>
#include <string>
#include <codecvt>

#include <Windows.h>

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK hMouseHook;

HWND hWnd;
POINT startingPoint;

void setup_wcout()
{
    SetConsoleOutputCP(CP_UTF8);
    std::ios_base::sync_with_stdio(false);

    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);
}

int main() {

    setup_wcout();
    std::wcout << L"Starting my awesome hook app!" << std::endl;

    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
    if (!hMouseHook) {
        MessageBox(NULL, L"Failed to install hook!", L"Error", MB_ICONERROR);
        return -1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hMouseHook);
    return 0;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return CallNextHookEx(0, nCode, wParam, lParam);
    }

    switch (wParam)
    {
    case WM_RBUTTONDOWN:
    {

        MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
        POINT pt = mouseStruct->pt;
        HWND hWnd = WindowFromPoint(pt);

        if (hWnd == nullptr)
        {
            std::wcout << L"No window found." << std::endl;
            break;
        }

        wchar_t windowText[256] {};

        if (GetWindowTextW(hWnd, windowText, 256) == 0)
        {
            std::wcout << L"Failed to find title!" << std::endl;
            break;
        }

        std::wcout << L"title>>\t" << windowText << std::endl;
    }
    break;
    default:
        break;
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKbdProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT kbdStruct;
    HKL keyboardLayout;
    keyboardLayout = GetKeyboardLayout(0);

    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            std::wcout << L"Got key: " << (wchar_t)MapVirtualKeyExW(kbdStruct.vkCode, MAPVK_VK_TO_CHAR, keyboardLayout) << std::endl;
            if (kbdStruct.vkCode == VK_F1)
            {
                MessageBox(NULL, L"F1 was pressed!", L"key pressed", MB_ICONINFORMATION);
            }
        }
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}