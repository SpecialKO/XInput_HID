#pragma once

#include <windef.h>

struct config_s {
  wchar_t wszPathToSystemXInput1_4 [MAX_PATH] = L"";
  DWORD   dwIdleTimeoutInSeconds              = 450; // 7.5 minutes
  BOOL    bSpecialTriangleShutsOff            = TRUE;
} extern config;