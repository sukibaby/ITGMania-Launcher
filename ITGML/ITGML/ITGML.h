// ITGML.h : Header file

#pragma once

#include "resource.h"
#include <windows.h>
#include <string>

// Constants
#define REGISTRY_KEY L"SOFTWARE\\ITGML"
#define REGISTRY_VALUE_NAME L"ExecutablePath"
#define MAX_LOADSTRING 100

// Global Variables
extern HINSTANCE hInst;
extern WCHAR szTitle[MAX_LOADSTRING];
extern WCHAR szWindowClass[MAX_LOADSTRING];
extern WCHAR selectedExePath[MAX_PATH];
extern int selectedPriority;

// Function Declarations
int getYPosition();
void CreateCheckbox(HWND hWnd, HINSTANCE hInstance, int id, int yPos, LPCWSTR text);
void SpoofExecutableVersion(const std::wstring& exePath);

// Entry Point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

// GUI Functions
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Executable Modifiers
void BrowseForExecutable(HWND hWnd);
void LaunchITGmania();