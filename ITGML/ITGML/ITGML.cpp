//
// ITGML.cpp
//

#include "framework.h"
#include <shellapi.h>
#include "ITGML.h"
#include <windows.h>
#include <iostream>
#include <commdlg.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <winreg.h>
#include <string>
#include <fstream>
#include <shlobj.h> // For SHBrowseForFolder and SHGetPathFromIDList

#define REGISTRY_KEY L"SOFTWARE\\ITGML"
#define REGISTRY_VALUE_NAME L"ExecutablePath"

#define MAX_LOADSTRING 100
#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 30
#define CHECKBOX_WIDTH 190
#define CHECKBOX_HEIGHT 30
#define CHECKBOX_SPACING 10

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

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 250, 400,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	RECT rect;
	GetClientRect(hWnd, &rect);
	int centerX = (rect.right - rect.left) / 2;

	CreateWindowW(L"BUTTON", L"Browse for Executable",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 50,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, reinterpret_cast<HMENU>(1), hInstance, nullptr);

	CreateWindowW(L"BUTTON", L"Launch ITGmania",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 90,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, reinterpret_cast<HMENU>(2), hInstance, nullptr);

	CreateWindowW(L"BUTTON", L"Patch SL ver. check",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		centerX - (BUTTON_WIDTH / 2), 130,
		BUTTON_WIDTH, BUTTON_HEIGHT,
		hWnd, reinterpret_cast<HMENU>(7), hInstance, nullptr);

	CreateCheckbox(hWnd, hInstance, 6, getYPosition(), L"Boost game priority");
	CreateCheckbox(hWnd, hInstance, 8, getYPosition(), L"Restrict to single CPU");

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HFONT hFont = nullptr;

	switch (message) {
	case WM_CREATE:
		hFont = CreateFont(-16, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, L"");
		break;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		SetBkMode(hdc, TRANSPARENT);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		RECT rect;
		GetClientRect(hWnd, &rect);

		LPCWSTR titleText = L"ITGMANIA LAUNCHER";
		RECT textRect = { 0, 0, rect.right, 50 };
		DrawText(hdc, titleText, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		SelectObject(hdc, hOldFont);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		switch (wmId) {
		case 1:
			BrowseForExecutable(hWnd);
			break;
		case 2:
			LaunchITGmania();
			break;
		case 6: {
			LRESULT state = SendMessage(GetDlgItem(hWnd, 6), BM_GETCHECK, 0, 0);
			selectedPriority = (state == BST_CHECKED) ? REALTIME_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
			break;
		}
		case 7:
			if (wcslen(selectedExePath) > 0) {
				SpoofExecutableVersion(selectedExePath);
			}
			else {
				MessageBox(hWnd, L"Please select an executable first.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
		case 8: {
			LRESULT state = SendMessage(GetDlgItem(hWnd, 8), BM_GETCHECK, 0, 0);
			if (state == BST_CHECKED) {
				restrictToSingleCPU = true;
			}
			else {
				restrictToSingleCPU = false;
			}
			break;
		}
		default:
			MessageBox(hWnd, L"Unknown command.", L"Error", MB_OK | MB_ICONERROR);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	case WM_DESTROY:
		if (hFont) {
			DeleteObject(hFont);
		}
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/*****************************************************************/
/******************** Executable Modifiers ***********************/
/*****************************************************************/
void BrowseForExecutable(HWND hWnd) {
	OPENFILENAME ofn;
	WCHAR szFile[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
	ofn.lpstrFilter = L"Executable Files\0*.exe\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE) {
		wcscpy_s(selectedExePath, ofn.lpstrFile);

		HKEY hKey;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
			size_t dataSize = (wcslen(selectedExePath) + 1) * sizeof(WCHAR);
			if (dataSize > MAXDWORD) {
				MessageBox(nullptr, L"Path size exceeds the maximum allowed size. Please report this to the ITGmania Launcher GitHub.", L"Error", MB_OK | MB_ICONERROR);
			}
			else {
				RegSetValueEx(hKey, REGISTRY_VALUE_NAME, 0, REG_SZ, reinterpret_cast<const BYTE*>(selectedExePath), static_cast<DWORD>(dataSize));
			}
			RegCloseKey(hKey);
		}
	}
}

// Launch ITGmania
void LaunchITGmania() {
	// Check the registry if we have a path stored
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD bufferSize = sizeof(selectedExePath);
		if (RegQueryValueEx(hKey, REGISTRY_VALUE_NAME, nullptr, nullptr, (LPBYTE)selectedExePath, &bufferSize) != ERROR_SUCCESS) {
			selectedExePath[0] = L'\0';
		}
		RegCloseKey(hKey);
	}

	if (wcslen(selectedExePath) == 0) {
		MessageBox(nullptr, L"Please select the executable first.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Set the thread scheduler to 1ms interval
	timeBeginPeriod(1);

	// Try to launch the exe
	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
	sei.lpFile = selectedExePath;
	sei.lpParameters = L"";
	sei.nShow = SW_SHOWNORMAL;
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;

	if (!ShellExecuteEx(&sei)) {
		std::wcerr << L"Failed to launch application. Error: " << GetLastError() << std::endl;
		return;
	}

	// Handle anything we wanted to wait for until after the game was launched
	if (restrictToSingleCPU)
	{
		SetProcessAffinityMask(sei.hProcess, 1);
	}
	SetPriorityClass(sei.hProcess, selectedPriority);
	WaitForSingleObject(sei.hProcess, INFINITE);
	CloseHandle(sei.hProcess);
}

void SpoofExecutableVersion(const std::wstring& exePath) {
	MessageBox(nullptr, L"This will patch out the version check in any Simply Love theme.\nThe next step will ask you to locate your Simply Love folder.\nIf you use Zmod, then use your Zmod folder.", L"Info", MB_OK | MB_ICONINFORMATION);

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Select the Simply Love (or Zmod) folder";
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (!pidl) {
		MessageBox(nullptr, L"Cancelling without modifying anything.", L"Info", MB_OK | MB_ICONINFORMATION);
		return;
	}

	WCHAR scriptsFolderPath[MAX_PATH];
	if (!SHGetPathFromIDList(pidl, scriptsFolderPath)) {
		MessageBox(nullptr, L"Failed to retrieve the Simply Love folder path.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	CoTaskMemFree(pidl);

	std::wstring luaFilePath = std::wstring(scriptsFolderPath) + L"\\Scripts\\SL-SupportHelpers.lua";

	std::wifstream inputFile(luaFilePath);
	if (!inputFile.is_open()) {
		MessageBox(nullptr, L"Failed to open SL-SupportHelpers.lua for reading.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	std::wstring fileContent;
	std::wstring line;
	bool lineModified = false;

	while (std::getline(inputFile, line)) {
		if (line.find(L"local MinimumVersion") != std::wstring::npos) {
			line = L"local MinimumVersion = {0, 0, 1}";
			lineModified = true;
		}
		fileContent += line + L"\n";
	}

	inputFile.close();

	if (!lineModified) {
		MessageBox(nullptr, L"Target line not found in SL-SupportHelpers.lua.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	std::wofstream outputFile(luaFilePath, std::ios::trunc);
	if (!outputFile.is_open()) {
		MessageBox(nullptr, L"Failed to open SL-SupportHelpers.lua for writing.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	outputFile << fileContent;
	outputFile.close();

	MessageBox(nullptr, L"SL-SupportHelpers.lua modified successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
}

/*****************************************************************/
/************************ Helper Functions ***********************/
/*****************************************************************/
void CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int id, int yOffset, LPCWSTR text) {
	CreateWindowW(L"BUTTON", text,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
		(250 - CHECKBOX_WIDTH) / 2, yOffset,
		CHECKBOX_WIDTH, CHECKBOX_HEIGHT,
		hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInstance, nullptr);
}

int getYPosition() {
	static int initialOffset = 155;
	return initialOffset += 30;
}
