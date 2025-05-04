// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <atlbase.h>

#pragma comment (lib, "Shlwapi.lib")

config_s config           = { };
HMODULE  hModRealXInput14 =  0 ;
HANDLE   __SK_DLL_TeardownEvent;

DWORD
WINAPI
XInput_HID_InitThread (LPVOID);

BOOL
APIENTRY
DllMain ( HMODULE hModule,
          DWORD   ul_reason_for_call,
          LPVOID  lpReserved )
{
  std::ignore = lpReserved;

  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      __SK_DLL_TeardownEvent =
        CreateEventW (nullptr, TRUE, FALSE, nullptr);

#ifdef _M_IX86
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_4,   MAX_PATH);
#else
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_4,   MAX_PATH);
#endif
      PathAppendW              (config.wszPathToSystemXInput1_4,   L"XInput1_4.dll");

#if 1
      XInput_HID_InitThread (nullptr);

      GetModuleHandleEx ( GET_MODULE_HANDLE_EX_FLAG_PIN |
                          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hModule, &hModule);
#else
      CloseHandle (
        CreateThread ( nullptr, 0x0, XInput_HID_InitThread,
                       nullptr, 0x0, nullptr )
      );
#endif
    } break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;

    case DLL_PROCESS_DETACH:
      while (hModRealXInput14 == nullptr)
        Sleep (1);

      if (           hModRealXInput14 != (HMODULE)-1)
        FreeLibrary (hModRealXInput14);
      break;
  }

  return TRUE;
}