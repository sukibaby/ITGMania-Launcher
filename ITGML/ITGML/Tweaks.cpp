
#include "framework.h"
#include <shellapi.h>
#include "ITGML.h"
#include <windows.h>
#include <iostream>
#include <commdlg.h>
#include <mmsystem.h> // for timeBeginPeriod
#pragma comment(lib, "winmm.lib")
#include <winreg.h>
#include <string>
#include <fstream>
#include <shlobj.h> // For SHBrowseForFolder and SHGetPathFromIDList


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