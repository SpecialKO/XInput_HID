#pragma once

using GetInputReport_pfn = bool (*)(void*);
bool _GetInputReportStub (void* pDevice);

struct hid_device_file_s {
  wchar_t wszDevicePath [MAX_PATH] = { };
  HANDLE  hDeviceFile              = INVALID_HANDLE_VALUE;

  struct {
    struct {
      HANDLE hThread               = INVALID_HANDLE_VALUE;
      HANDLE hDataRequest          = INVALID_HANDLE_VALUE;
    } input, output;

    struct {
      HANDLE hDisconnect           = INVALID_HANDLE_VALUE;
      HANDLE hReconnect            = INVALID_HANDLE_VALUE;
      HANDLE hShutdown             = INVALID_HANDLE_VALUE;
    } events;
  } threads;

  bool    bConnected               = false;
  bool    bWireless                = false;

  struct {
    USHORT vid = 0;
    USHORT pid = 0;
  } devinfo;

  struct {
    XINPUT_STATE current         = { };
    XINPUT_STATE prev            = { };
    DWORD        input_timestamp =  0 ;
  } state;

  struct {
    DWORD todo = 0;
  } battery;

  GetInputReport_pfn get_input_report = _GetInputReportStub;

  bool GetInputReport (void)
  {
    return
      get_input_report (this);
  }

  bool disconnect (void);
  bool reconnect  (HANDLE hDeviceFile = INVALID_HANDLE_VALUE);
};

bool SK_DualSense_GetInputReportUSB (void *pGenericDev);
bool SK_DualSense_GetInputReportBt  (void *pGenericDev);

bool SK_DualShock4_GetInputReportUSB (void *pGenericDev);
bool SK_DualShock4_GetInputReportBt  (void *pGenericDev);

bool SK_SwitchPro_GetInputReportUSB (void *pGenericDev);
bool SK_SwitchPro_GetInputReportBt  (void *pGenericDev);

bool SK_XInput_UpdatePolledDataAndTimestamp ( hid_device_file_s *pDevice,
                                              bool               bActive,
                                              bool              &bNewData );

void SK_HID_SpawnInputReportThread  (hid_device_file_s* pDevice);
void SK_HID_SpawnOutputReportThread (hid_device_file_s* pDevice);
bool SK_HID_PauseReportThreads      (hid_device_file_s* pDevice);
bool SK_HID_ResumeReportThreads     (hid_device_file_s* pDevice);
bool SK_HID_ShutdownReportThreads   (hid_device_file_s* pDevice);