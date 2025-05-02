//
// ITGML.cpp
//

#include "framework.h"
#include "ITGML.h"
#include "GUI.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#include <winreg.h>
#include <shlobj.h> // For SHBrowseForFolder and SHGetPathFromIDList

#define REGISTRY_KEY L"SOFTWARE\\ITGML"
#define REGISTRY_VALUE_NAME L"ExecutablePath"

#define MAX_LOADSTRING 100
#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 30
#define CHECKBOX_WIDTH 190
#define CHECKBOX_HEIGHT 30
#define CHECKBOX_SPACING 10

constexpr DWORD DWORD_FALSE = 0;
constexpr DWORD DWORD_TRUE = 1;
bool restrictToSingleCPU = false;
bool priorityBooster = false;
DWORD priorityBoosterValue = DWORD_FALSE;
DWORD restrictToSingleCPUValue = DWORD_FALSE;
DWORD dataSize = sizeof(DWORD);
HINSTANCE hInst = nullptr;
wchar_t szTitle[MAX_LOADSTRING] = L"";
wchar_t szWindowClass[MAX_LOADSTRING] = L"";
wchar_t selectedExePath[MAX_PATH] = L"";
int selectedPriority = NORMAL_PRIORITY_CLASS;

/*****************************************************************/
/************************** Entry Point **************************/
/*****************************************************************/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_ITGML, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

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
/************************** GUI Functions ************************/
/*****************************************************************/
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ITGML));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;

	LoadSettingsFromRegistry();

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 250, 350,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	RECT rect;
	GetClientRect(hWnd, &rect);
	int centerX = (rect.right - rect.left) / 2;

	// Create buttons
	CreateWindowW(L"BUTTON", L"Browse for Executable",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 50,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, (HMENU)CMD_BROWSE_EXECUTABLE, hInstance, nullptr);

	CreateWindowW(L"BUTTON", L"Launch ITGmania",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 90,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, (HMENU)CMD_LAUNCH_ITGMANIA, hInstance, nullptr);

	CreateWindowW(L"BUTTON", L"Patch SL ver. check",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 130,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, (HMENU)CMD_PATCH_SL_VER, hInstance, nullptr);

	// Create checkboxes
	HWND hBoostPriorityCheckbox = CreateWindowW(L"BUTTON", L"Boost game priority",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		(250 - CHECKBOX_WIDTH) / 2, getYPosition(),
		CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
		hWnd, (HMENU)CMD_BOOST_PRIORITY, hInstance, nullptr);

	HWND hRestrictCPUCheckbox = CreateWindowW(L"BUTTON", L"Restrict to single CPU",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		(250 - CHECKBOX_WIDTH) / 2, getYPosition(),
		CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
		hWnd, (HMENU)CMD_RESTRICT_CPU, hInstance, nullptr);

	// Set the initial state of the checkboxes based on the loaded settings
	SendMessage(hBoostPriorityCheckbox, BM_SETCHECK, priorityBooster ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hRestrictCPUCheckbox, BM_SETCHECK, restrictToSingleCPU ? BST_CHECKED : BST_UNCHECKED, 0);


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int id, int yOffset, LPCWSTR text) {
	CreateWindowW(L"BUTTON", text,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		(250 - CHECKBOX_WIDTH) / 2, yOffset,
		CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
		hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance, nullptr);
}

void LoadSettingsFromRegistry() {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Read PriorityBooster value
		if (RegQueryValueEx(hKey, L"PriorityBooster", nullptr, nullptr, reinterpret_cast<LPBYTE>(&priorityBoosterValue), &dataSize) == ERROR_SUCCESS) {
			priorityBooster = (priorityBoosterValue == DWORD_TRUE);
		}

		// Read RestrictToSingleCPU value
		if (RegQueryValueEx(hKey, L"RestrictToSingleCPU", nullptr, nullptr, reinterpret_cast<LPBYTE>(&restrictToSingleCPUValue), &dataSize) == ERROR_SUCCESS) {
			restrictToSingleCPU = (restrictToSingleCPUValue == DWORD_TRUE);
		}

		RegCloseKey(hKey);
	}
}