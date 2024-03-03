#pragma once

#include <windef.h>

struct config_s {
  wchar_t wszPathToSystemXInput1_4   [MAX_PATH] = L"";
} extern config;