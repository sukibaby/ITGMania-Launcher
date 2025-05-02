//
// ITGML.cpp
//

#include "framework.h"
#include "ITGML.h"
#include "GUI.h"
#include <windows.h>
#include <winreg.h>
#include <shlobj.h>  // For SHBrowseForFolder and SHGetPathFromIDList
#pragma comment(lib, "winmm.lib")

// Constants
constexpr DWORD DWORD_FALSE = 0;
constexpr DWORD DWORD_TRUE = 1;
constexpr int WINDOW_WIDTH = 250;
constexpr int WINDOW_HEIGHT = 350;

// Global Variables
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

/*****************************************************************/
/************************** Entry Point **************************/
/*****************************************************************/
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

    // Load accelerators and start the message loop
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

/*****************************************************************/
/************************** Window Class *************************/
/*****************************************************************/
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

/*****************************************************************/
/************************** Instance Init ************************/
/*****************************************************************/
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
    CreateWindowW(L"BUTTON", L"Browse for Executable",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        centerX - (BUTTON_WIDTH / 2), 50,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd, reinterpret_cast<HMENU>(CMD_BROWSE_EXECUTABLE), hInstance, nullptr);

    CreateWindowW(L"BUTTON", L"Launch ITGmania",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        centerX - (BUTTON_WIDTH / 2), 90,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd, reinterpret_cast<HMENU>(CMD_LAUNCH_ITGMANIA), hInstance, nullptr);

    CreateWindowW(L"BUTTON", L"Patch SL ver. check",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        centerX - (BUTTON_WIDTH / 2), 130,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd, reinterpret_cast<HMENU>(CMD_PATCH_SL_VER), hInstance, nullptr);

    // Create checkboxes
    HWND hBoostPriorityCheckbox = CreateWindowW(L"BUTTON", L"Boost game priority",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        (WINDOW_WIDTH - CHECKBOX_WIDTH) / 2, getYPosition(),
        CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
        hWnd, reinterpret_cast<HMENU>(CMD_BOOST_PRIORITY), hInstance, nullptr);

    HWND hRestrictCPUCheckbox = CreateWindowW(L"BUTTON", L"Restrict to single CPU",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        (WINDOW_WIDTH - CHECKBOX_WIDTH) / 2, getYPosition(),
        CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
        hWnd, reinterpret_cast<HMENU>(CMD_RESTRICT_CPU), hInstance, nullptr);

    // Set the initial state of the checkboxes based on the loaded settings
    SendMessageW(hBoostPriorityCheckbox, BM_SETCHECK, priorityBooster ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hRestrictCPUCheckbox, BM_SETCHECK, restrictToSingleCPU ? BST_CHECKED : BST_UNCHECKED, 0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

/*****************************************************************/
/************************ Helper Functions ***********************/
/*****************************************************************/
void LoadSettingsFromRegistry() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        // Read PriorityBooster value
        if (RegQueryValueExW(hKey, L"PriorityBooster", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(&priorityBoosterValue), &dataSize) == ERROR_SUCCESS) {
            priorityBooster = (priorityBoosterValue == DWORD_TRUE);
        }

        // Read RestrictToSingleCPU value
        if (RegQueryValueExW(hKey, L"RestrictToSingleCPU", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(&restrictToSingleCPUValue), &dataSize) == ERROR_SUCCESS) {
            restrictToSingleCPU = (restrictToSingleCPUValue == DWORD_TRUE);
        }

        RegCloseKey(hKey);
    }
}
