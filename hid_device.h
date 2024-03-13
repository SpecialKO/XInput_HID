#pragma once

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

bool SK_DualSense_GetInputReportUSB (void *pGenericDev);
bool SK_DualSense_GetInputReportBt  (void *pGenericDev);

bool SK_DualShock4_GetInputReportUSB (void *pGenericDev);
bool SK_DualShock4_GetInputReportBt  (void *pGenericDev);

bool SK_SwitchPro_GetInputReportUSB (void *pGenericDev);
bool SK_SwitchPro_GetInputReportBt  (void *pGenericDev);