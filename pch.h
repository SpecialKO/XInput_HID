// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS

// add headers that you want to pre-compile here
#include "framework.h"
#include "config.h"
#include <Shlwapi.h>



#include <concurrent_vector.h>
#include <SetupAPI.h>

#include <initguid.h>
#include <hidclass.h>
#include <hidsdi.h>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")

#define XUSER_MAX_COUNT               4
#define XUSER_INDEX_ANY               0x000000FF

#define XINPUT_GAMEPAD_DPAD_UP        0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN      0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT      0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT     0x0008

#define XINPUT_GAMEPAD_START          0x0010
#define XINPUT_GAMEPAD_BACK           0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB     0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB    0x0080

#define XINPUT_GAMEPAD_LEFT_SHOULDER  0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XINPUT_GAMEPAD_GUIDE          0x0400  // XInputEx

#define XINPUT_GAMEPAD_LEFT_TRIGGER   0x10000
#define XINPUT_GAMEPAD_RIGHT_TRIGGER  0x20000

#define XINPUT_GAMEPAD_A              0x1000
#define XINPUT_GAMEPAD_B              0x2000
#define XINPUT_GAMEPAD_X              0x4000
#define XINPUT_GAMEPAD_Y              0x8000

#define XINPUT_GETSTATE_ORDINAL           MAKEINTRESOURCEA (002)
#define XINPUT_SETSTATE_ORDINAL           MAKEINTRESOURCEA (003)
#define XINPUT_GETCAPABILITIES_ORDINAL    MAKEINTRESOURCEA (004)
#define XINPUT_ENABLE_ORDINAL             MAKEINTRESOURCEA (005)
#define XINPUT_GETSTATEEX_ORDINAL         MAKEINTRESOURCEA (100)
#define XINPUT_POWEROFF_ORDINAL           MAKEINTRESOURCEA (103)
#define XINPUT_GETCAPABILITIES_EX_ORDINAL MAKEINTRESOURCEA (108)


#define XINPUT_DEVTYPE_GAMEPAD             0x01
#define XINPUT_DEVSUBTYPE_UNKNOWN          0x00
#define XINPUT_DEVSUBTYPE_GAMEPAD          0x01
#define XINPUT_DEVSUBTYPE_WHEEL            0x02
#define XINPUT_DEVSUBTYPE_ARCADE_STICK     0x03
#define XINPUT_DEVSUBTYPE_FLIGHT_STICK     0x04
#define XINPUT_DEVSUBTYPE_DANCE_PAD        0x05
#define XINPUT_DEVSUBTYPE_GUITAR           0x06
#define XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE 0x07
#define XINPUT_DEVSUBTYPE_DRUM_KIT         0x08
#define XINPUT_DEVSUBTYPE_GUITAR_BASS      0x0B
#define XINPUT_DEVSUBTYPE_ARCADE_PAD       0x13

#define XINPUT_FLAG_GAMEPAD           0x01

#define BATTERY_DEVTYPE_GAMEPAD       0x00
#define BATTERY_DEVTYPE_HEADSET       0x01

#define BATTERY_TYPE_DISCONNECTED     0x00
#define BATTERY_TYPE_WIRED            0x01
#define BATTERY_TYPE_ALKALINE         0x02
#define BATTERY_TYPE_NIMH             0x03
#define BATTERY_TYPE_UNKNOWN          0xFF

#define BATTERY_LEVEL_EMPTY           0x00
#define BATTERY_LEVEL_LOW             0x01
#define BATTERY_LEVEL_MEDIUM          0x02
#define BATTERY_LEVEL_FULL            0x03

#define XINPUT_CAPS_FFB_SUPPORTED     0x0001
#define XINPUT_CAPS_WIRELESS          0x0002
#define XINPUT_CAPS_VOICE_SUPPORTED   0x0004
#define XINPUT_CAPS_PMD_SUPPORTED     0x0008
#define XINPUT_CAPS_NO_NAVIGATION     0x0010

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

typedef struct _XINPUT_GAMEPAD {
  WORD  wButtons;
  BYTE  bLeftTrigger;
  BYTE  bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT sThumbRX;
  SHORT sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_GAMEPAD_EX {
  WORD  wButtons;
  BYTE  bLeftTrigger;
  BYTE  bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT sThumbRX;
  SHORT sThumbRY;
  DWORD dwUnknown;
} XINPUT_GAMEPAD_EX, *PXINPUT_GAMEPAD_EX;

typedef struct _XINPUT_STATE {
  DWORD          dwPacketNumber;
  XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_STATE_EX {
  DWORD             dwPacketNumber;
  XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX, *PXINPUT_STATE_EX;

typedef struct _XINPUT_VIBRATION {
  WORD wLeftMotorSpeed;
  WORD wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES {
  BYTE             Type;
  BYTE             SubType;
  WORD             Flags;
  XINPUT_GAMEPAD   Gamepad;
  XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

typedef struct _XINPUT_BATTERY_INFORMATION {
  BYTE BatteryType;
  BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

typedef struct _XINPUT_KEYSTROKE {
  WORD  VirtualKey;
  WCHAR Unicode;
  WORD  Flags;
  BYTE  UserIndex;
  BYTE  HidCode;
} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;

typedef struct _XINPUT_CAPABILITIES_EX {
  XINPUT_CAPABILITIES Capabilities;
  WORD                VendorId;
  WORD                ProductId;
  WORD                ProductVersion;
  WORD                unk1;
  DWORD               unk2;
} XINPUT_CAPABILITIES_EX, *PXINPUT_CAPABILITIES_EX;

using XInputGetState_pfn        = DWORD (WINAPI *)(
  _In_  DWORD        dwUserIndex,
  _Out_ XINPUT_STATE *pState
);

using XInputGetStateEx_pfn      = DWORD (WINAPI *)(
  _In_  DWORD            dwUserIndex,
  _Out_ XINPUT_STATE_EX *pState
);

using XInputSetState_pfn        = DWORD (WINAPI *)(
  _In_    DWORD             dwUserIndex,
  _Inout_ XINPUT_VIBRATION *pVibration
);

using XInputGetCapabilities_pfn = DWORD (WINAPI *)(
  _In_  DWORD                dwUserIndex,
  _In_  DWORD                dwFlags,
  _Out_ XINPUT_CAPABILITIES *pCapabilities
);

using XInputGetCapabilitiesEx_pfn = DWORD (WINAPI *)(
  _In_  DWORD dwReserved,
  _In_  DWORD dwUserIndex,
  _In_  DWORD dwFlags,
  _Out_ XINPUT_CAPABILITIES_EX *pCapabilitiesEx
);

using XInputGetBatteryInformation_pfn = DWORD (WINAPI *)(
  _In_  DWORD                       dwUserIndex,
  _In_  BYTE                        devType,
  _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation
);

using XInputGetKeystroke_pfn = DWORD (WINAPI *)(
  DWORD             dwUserIndex,
  DWORD             dwReserved,
  PXINPUT_KEYSTROKE pKeystroke
);

using XInputEnable_pfn = void (WINAPI *)(
  _In_ BOOL enable
);

using XInputPowerOff_pfn = DWORD (WINAPI *)(
  _In_ DWORD dwUserIndex
);

#define XINPUT_GETSTATE_ORDINAL           MAKEINTRESOURCEA (002)
#define XINPUT_SETSTATE_ORDINAL           MAKEINTRESOURCEA (003)
#define XINPUT_GETCAPABILITIES_ORDINAL    MAKEINTRESOURCEA (004)
#define XINPUT_ENABLE_ORDINAL             MAKEINTRESOURCEA (005)
#define XINPUT_GETSTATEEX_ORDINAL         MAKEINTRESOURCEA (100)
#define XINPUT_POWEROFF_ORDINAL           MAKEINTRESOURCEA (103)
#define XINPUT_GETCAPABILITIES_EX_ORDINAL MAKEINTRESOURCEA (108)


using GetInputReport_pfn = bool (*)(void*);
bool _GetInputReportStub (void* pDevice);

struct hid_device_file_s {
  wchar_t wszDevicePath [MAX_PATH] = { };
  HANDLE  hDeviceFile              = INVALID_HANDLE_VALUE;
  bool    bConnected               = false;
  bool    bWireless                = false;

  struct {
    USHORT vid;
    USHORT pid;
  } devinfo;

  struct {
    XINPUT_STATE current;
    XINPUT_STATE prev;
  } state;

  struct {
    DWORD todo;
  } battery;

  GetInputReport_pfn get_input_report = _GetInputReportStub;

  bool GetInputReport (void)
  {
    return
      get_input_report (this);
  }
};

#endif //PCH_H
