// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#pragma comment (lib, "Shlwapi.lib")

config_s config           = { };
HMODULE  hModRealXInput14 =  0 ;

volatile LONG s_Init = 0;

DWORD
WINAPI
XInput_HID_InitThread (LPVOID)
{
  hModRealXInput14 =
    LoadLibraryW (config.wszPathToSystemXInput1_4);

  //while (true)
  //{
    void SK_HID_SetupPlayStationControllers (void);
         SK_HID_SetupPlayStationControllers ();

  //  SleepEx (5000UL, FALSE);
  //}

  WriteRelease (&s_Init, 2);

  return 0;
}

BOOL
APIENTRY
DllMain ( HMODULE hModule,
          DWORD   ul_reason_for_call,
          LPVOID  lpReserved )
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
#ifdef _M_IX86
      GetSystemWow64DirectoryW (config.wszPathToSystemXInput1_4,   MAX_PATH);
#else
      GetSystemDirectoryW      (config.wszPathToSystemXInput1_4,   MAX_PATH);
#endif
      PathAppendW              (config.wszPathToSystemXInput1_4,   L"XInput1_4.dll");

      if (! InterlockedCompareExchange (&s_Init, true, false))
      {
#if 1
        XInput_HID_InitThread (nullptr);
#else
        CloseHandle (
          CreateThread ( nullptr, 0x0, XInput_HID_InitThread,
                         nullptr, 0x0, nullptr )
        );
#endif
      }

      DisableThreadLibraryCalls (hModule);
    } break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;

    case DLL_PROCESS_DETACH:
      while (ReadAcquire (&s_Init) == 1)
        Sleep (1);

      FreeLibrary (hModRealXInput14);
      break;
  }

  return TRUE;
}