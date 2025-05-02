// RegistryAccess.cpp
#include "RegistryAccess.h"
#include <windows.h>
#include <winreg.h>

extern bool priorityBooster;
extern bool restrictToSingleCPU;
extern DWORD priorityBoosterValue;
extern DWORD restrictToSingleCPUValue;
extern DWORD dataSize;

constexpr DWORD DWORD_TRUE = 1;
constexpr DWORD DWORD_FALSE = 0;

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

bool SaveSettingsToRegistry() {
	HKEY hKey;
	bool returnable_value = false;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
		DWORD priorityBoosterValue = priorityBooster ? 1 : 0;
		DWORD restrictToSingleCPUValue = restrictToSingleCPU ? 1 : 0;

		RegSetValueExW(hKey, L"PriorityBooster", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&priorityBoosterValue), sizeof(priorityBoosterValue));
		RegSetValueExW(hKey, L"RestrictToSingleCPU", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&restrictToSingleCPUValue), sizeof(restrictToSingleCPUValue));

		RegCloseKey(hKey);
		returnable_value = true;
	}
	else {
		returnable_value = false;
	}
	return returnable_value;
}