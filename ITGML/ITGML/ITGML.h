// ITGML.h : Header file

#pragma once

#include "resource.h"
#include <windows.h>
#include <string>

// Case Identifiers. UINT_PTR is used for HMENU compatibility
constexpr UINT_PTR CMD_BROWSE_EXECUTABLE = 1;  // Browse for Executable
constexpr UINT_PTR CMD_LAUNCH_ITGMANIA = 2;    // Launch ITGmania
constexpr UINT_PTR CMD_BOOST_PRIORITY = 6;     // Boost game priority
constexpr UINT_PTR CMD_PATCH_SL_VER = 7;       // Patch SL version check
constexpr UINT_PTR CMD_RESTRICT_CPU = 8;       // Restrict to single CPU

// Constants
#define REGISTRY_KEY L"SOFTWARE\\ITGML"
#define REGISTRY_VALUE_NAME L"ExecutablePath"
#define MAX_LOADSTRING 100

// Global Variables
extern HINSTANCE hInst;
extern wchar_t szTitle[MAX_LOADSTRING];
extern wchar_t szWindowClass[MAX_LOADSTRING];
extern wchar_t selectedExePath[MAX_PATH];
extern int selectedPriority;
extern bool restrictToSingleCPU;
extern bool priorityBooster;

// Function Declarations
void CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int id, int yPos, LPCWSTR text);
//void SpoofExecutableVersion(const std::wstring& exePath);
void LoadSettingsFromRegistry();

// Entry Point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

// GUI Functions
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

// Executable Modifiers
//void BrowseForExecutable(HWND hWnd);
//void LaunchITGmania();

inline int getYPosition() {
	static int initialOffset = 155;
	return initialOffset += 30;
}