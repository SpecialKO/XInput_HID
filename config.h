#pragma once

#include <windef.h>

struct config_s {
  wchar_t wszPathToSystemXInput1_4 [MAX_PATH] =   L"";
  DWORD   dwIdleTimeoutInSeconds              =   450; // 7.5 minutes
  DWORD   bSpecialTriangleShutsOff            =  TRUE;
  DWORD   bSpecialCrossActivatesScreenSaver   = FALSE;
  DWORD   bPowerOffControllersBeforeSleep     =  TRUE;
  DWORD   dwSteamInputKillSwitchChord         =     0;
} extern config;