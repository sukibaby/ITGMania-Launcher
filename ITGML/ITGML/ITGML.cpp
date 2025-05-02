//
// ITGML.cpp
//

#include "framework.h"
#include "ITGML.h"
#include "GUI.h"
#include "RegistryAccess.h"
#include <windows.h>
#include <winreg.h>
#include <shlobj.h>  // For SHBrowseForFolder and SHGetPathFromIDList
#pragma comment(lib, "winmm.lib")

constexpr DWORD DWORD_FALSE = 0;
constexpr DWORD DWORD_TRUE = 1;
constexpr int WINDOW_WIDTH = 250;
constexpr int WINDOW_HEIGHT = 350;

HINSTANCE hInst = nullptr;
wchar_t szTitle[MAX_LOADSTRING] = L"";
wchar_t szWindowClass[MAX_LOADSTRING] = L"";
wchar_t selectedExePath[MAX_PATH] = L"";
int selectedPriority = NORMAL_PRIORITY_CLASS;
bool restrictToSingleCPU = false;
bool priorityBooster = false;
DWORD priorityBoosterValue = DWORD_FALSE;
DWORD restrictToSingleCPUValue = DWORD_FALSE;
DWORD dataSize = sizeof(DWORD);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Load application title and window class name
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ITGML, szWindowClass, MAX_LOADSTRING);

    // Register the window class
    if (!MyRegisterClass(hInstance)) {
        return FALSE;
    }

    // Initialize the application instance
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    // Start the message loop
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ITGML));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ITGML));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

#define CREATE_BUTTON(text, x, y, width, height, parent, id, instance) \
    CreateWindowW(L"BUTTON", text, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, \
                  x, y, width, height, parent, reinterpret_cast<HMENU>(id), instance, nullptr)

#define CREATE_CHECKBOX(text, x, y, width, height, parent, id, instance) \
    CreateWindowW(L"BUTTON", text, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, \
                  x, y, width, height, parent, reinterpret_cast<HMENU>(id), instance, nullptr)

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;

    // Load settings from the registry
    LoadSettingsFromRegistry();

    // Create the main application window
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    // Center the buttons and checkboxes
    RECT rect;
    GetClientRect(hWnd, &rect);
    int centerX = (rect.right - rect.left) / 2;

    // Create buttons
    CREATE_BUTTON(L"Browse for Executable", centerX - (BUTTON_WIDTH / 2), 50,
        BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, CMD_BROWSE_EXECUTABLE, hInstance);

    CREATE_BUTTON(L"Launch ITGmania", centerX - (BUTTON_WIDTH / 2), 90,
        BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, CMD_LAUNCH_ITGMANIA, hInstance);

    CREATE_BUTTON(L"Patch SL ver. check", centerX - (BUTTON_WIDTH / 2), 130,
        BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, CMD_PATCH_SL_VER, hInstance);

    // Create checkboxes
    HWND hBoostPriorityCheckbox = CREATE_CHECKBOX(L"Boost game priority",
        (WINDOW_WIDTH - CHECKBOX_WIDTH) / 2, getYPosition(),
        CHECKBOX_WIDTH, CHECKBOX_HEIGHT, hWnd, CMD_BOOST_PRIORITY, hInstance);

    HWND hRestrictCPUCheckbox = CREATE_CHECKBOX(L"Restrict to single CPU",
        (WINDOW_WIDTH - CHECKBOX_WIDTH) / 2, getYPosition(),
        CHECKBOX_WIDTH, CHECKBOX_HEIGHT, hWnd, CMD_RESTRICT_CPU, hInstance);

    // Set the initial state of the checkboxes based on the registry values
    SendMessageW(hBoostPriorityCheckbox, BM_SETCHECK, priorityBooster ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hRestrictCPUCheckbox, BM_SETCHECK, restrictToSingleCPU ? BST_CHECKED : BST_UNCHECKED, 0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}