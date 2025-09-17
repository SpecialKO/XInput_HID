#pragma once

#include <vector>

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
    DWORD        last_idle_check =  0 ; // Last time idle power-off was tested
  } state;

  struct {
    DWORD todo = 0;
  } battery;

  std::vector <BYTE> input_report;
  std::vector <BYTE> output_report;
  std::vector <BYTE> feature_report;

  GetInputReport_pfn get_input_report = _GetInputReportStub;

  bool GetInputReport (void)
  {
    static bool          s_Once = false;
    if (! std::exchange (s_Once , true))
    {
      DWORD dwBytesRead = 0;
      LONG  lImmediate  = 0;

      DeviceIoControl (
        hDeviceFile, IOCTL_HID_SET_POLL_FREQUENCY_MSEC,
          &lImmediate, sizeof (LONG), nullptr, 0, &dwBytesRead, nullptr );
    }

    return get_input_report != nullptr ?
           get_input_report (this)     :
           false;
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

bool SK_HID_ProcessChordInput (XINPUT_STATE& state, hid_device_file_s *pDevice = nullptr);

void SK_HID_SpawnInputReportThread  (hid_device_file_s* pDevice);
void SK_HID_SpawnOutputReportThread (hid_device_file_s* pDevice);
bool SK_HID_PauseReportThreads      (hid_device_file_s* pDevice);
bool SK_HID_ResumeReportThreads     (hid_device_file_s* pDevice);
bool SK_HID_ShutdownReportThreads   (hid_device_file_s* pDevice);

#define SK_HID_VID_8BITDO            0x2dc8
#define SK_HID_VID_LOGITECH          0x046d
#define SK_HID_VID_MICROSOFT         0x045e
#define SK_HID_VID_NINTENDO          0x057e
#define SK_HID_VID_NVIDIA            0x0955
#define SK_HID_VID_RAZER             0x1532
#define SK_HID_VID_SONY              0x054c
#define SK_HID_VID_VALVE             0x28de

#define SK_HID_PID_XUSB              0x02a1 // Xbox 360 Controller Protocol
#define SK_HID_PID_XBOXGIP           0x02ff // Xbox One Controller Protocol
#define SK_HID_PID_STEAM_VIRTUAL     0x11ff // Steam Emulated Controller

#define SK_HID_PID_DUALSHOCK3        0x0268
#define SK_HID_PID_DUALSHOCK4        0x05c4
#define SK_HID_PID_DUALSHOCK4_REV2   0x09cc
#define SK_HID_PID_DUALSHOCK4_DONGLE 0x0BA0
#define SK_HID_PID_DUALSENSE         0x0ce6
#define SK_HID_PID_DUALSENSE_EDGE    0x0df2