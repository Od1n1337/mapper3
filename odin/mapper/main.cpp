#ifndef KDLIBMODE

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

#include "include/mapper.hpp"
#include "include/driver_array.h"

HANDLE iqvw64e_device_handle;


LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		Log(L"[mapper] Crash at addr 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << L" by 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::endl);
	else
		Log(L"[mapper] Crash" << std::endl);

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;
}

int paramExists(const int argc, wchar_t** argv, const wchar_t* param) {
	size_t plen = wcslen(param);
	for (int i = 1; i < argc; i++) {
		if (wcslen(argv[i]) == plen + 1ull && _wcsicmp(&argv[i][1], param) == 0 && argv[i][0] == '/') {
			return i;
		}
		else if (wcslen(argv[i]) == plen + 2ull && _wcsicmp(&argv[i][2], param) == 0 && argv[i][0] == '-' && argv[i][1] == '-') {
			return i;
		}
	}
	return -1;
}

void help() {
	Log(L"\r\n\r\n[!] Incorrect Usage!" << std::endl);
	Log(L"[mapper] Usage: mapper.exe [--free][--mdl][--PassAllocationPtr] driver\n\nor drag the .sys into mapper.exe" << std::endl);
}

bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	UNREFERENCED_PARAMETER(mdlptr);
	Log("[mapper] Callback example called" << std::endl);

	return true;
}

int wmain(const int argc, wchar_t** argv) {
	SetUnhandledExceptionFilter(SimplestCrashHandler);

	bool free = paramExists(argc, argv, L"free") > 0;
	bool mdlMode = paramExists(argc, argv, L"mdl") > 0;
	bool indPagesMode = paramExists(argc, argv, L"indPages") > 0;
	bool passAllocationPtr = paramExists(argc, argv, L"PassAllocationPtr") > 0;

	if (free) {
		Log(L"[mapper] Free pool memory after usage enabled" << std::endl);
	}

	if (mdlMode) {
		Log(L"[mapper] Mdl memory usage enabled" << std::endl);
	}

	if (indPagesMode) {
		Log(L"[mapper] Allocate Independent Pages mode enabled" << std::endl);
	}

	if (passAllocationPtr) {
		Log(L"[mapper] Pass Allocation Ptr as first param enabled" << std::endl);
	}

	iqvw64e_device_handle = intel_driver::Load();

	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE)
		return -1;

	std::vector<uint8_t> raw_image(driver_array, driver_array + driver_array_size); // Load the driver from the byte array

	saturn::AllocationMode mode = saturn::AllocationMode::AllocatePool;

	if (mdlMode && indPagesMode) {
		Log(L"[mapper] Too many allocation modes" << std::endl);
	    intel_driver:Unload:(iqvw64e_device_handle);
		std::cin.get();
		return -1;
	}
	else if (mdlMode) {
		mode = saturn::AllocationMode::AllocateMdl;
	}
	else if (indPagesMode) {
		mode = saturn::AllocationMode::AllocateIndependentPages;
	}

	NTSTATUS exitCode = 0;
	if (!saturn::MapDriver(iqvw64e_device_handle, raw_image.data(), raw_image.size(), 0, free, true, mode, passAllocationPtr, callbackExample, &exitCode)) {
		Log(L"[mapper] Failed to map driver" << std::endl);
		std::cin.get();
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		Log(L"[mapper] Warning! failed to fully unload vulnerable driver " << std::endl);
		std::cin.get();
	}
	Log(L"[mapper] Successfully Mapped Driver." << std::endl);
	std::cin.get();
}

#endif