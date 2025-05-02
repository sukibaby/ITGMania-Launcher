
#include "ITGML.h"
#include "Tweaks.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#include <winreg.h>

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
		case CMD_BROWSE_EXECUTABLE:
			BrowseForExecutable(hWnd);
			break;
		case CMD_LAUNCH_ITGMANIA:
			LaunchITGmania();
			break;
		case CMD_BOOST_PRIORITY: {
			LRESULT state = SendMessage(GetDlgItem(hWnd, CMD_BOOST_PRIORITY), BM_GETCHECK, 0, 0);
			priorityBooster = (state == BST_CHECKED);
			selectedPriority = priorityBooster ? REALTIME_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
			break;
		}
		case CMD_PATCH_SL_VER:
			if (wcslen(selectedExePath) > 0) {
				SpoofExecutableVersion(selectedExePath);
			}
			else {
				MessageBox(hWnd, L"Please select an executable first.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
		case CMD_RESTRICT_CPU: {
			LRESULT state = SendMessage(GetDlgItem(hWnd, CMD_RESTRICT_CPU), BM_GETCHECK, 0, 0);
			restrictToSingleCPU = (state == BST_CHECKED);
			break;
		}
		default:
			MessageBox(hWnd, L"Unknown command.", L"Error", MB_OK | MB_ICONERROR);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		HKEY hKey;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
			DWORD priorityBoosterValue = priorityBooster ? 1 : 0;
			DWORD restrictToSingleCPUValue = restrictToSingleCPU ? 1 : 0;

			RegSetValueEx(hKey, L"PriorityBooster", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&priorityBoosterValue), sizeof(priorityBoosterValue));
			RegSetValueEx(hKey, L"RestrictToSingleCPU", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&restrictToSingleCPUValue), sizeof(restrictToSingleCPUValue));

			RegCloseKey(hKey);
		}
		else {
			MessageBox(hWnd, L"Failed to save settings to the registry.", L"Error", MB_OK | MB_ICONERROR);
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