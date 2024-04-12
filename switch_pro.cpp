#include "pch.h"

#include <windef.h>
#include <cstdint>
#include <string>

#pragma pack(push,1)
struct SK_HID_SwitchPro_GetStateData // 49 (0x30)
{
/* 0  */ uint8_t Sequence;

/* 1.0*/ uint8_t BatteryCharging : 1;
/* 1.1*/ uint8_t BatteryLevel    : 3; // 8: Full, 6: Medium, 4: Low, 2: Critical, 0: Empty
/* 1.4*/ uint8_t ConInfo         : 4; // (ConInfo >> 1) & 3 - 3: JoyCon, 0: Pro/ChrGrip,
                                      //  ConInfo       & 1 - 1: Switch/USB
/* 2.0*/ uint8_t ButtonY         : 1;
/* 2.1*/ uint8_t ButtonX         : 1;
/* 2.2*/ uint8_t ButtonB         : 1;
/* 2.3*/ uint8_t ButtonA         : 1;
/* 2.4*/ uint8_t ButtonSR_R      : 1;
/* 2.5*/ uint8_t ButtonSL_R      : 1;
/* 2.6*/ uint8_t ButtonR         : 1;
/* 2.7*/ uint8_t ButtonZR        : 1;
/* 3.0*/ uint8_t ButtonMinus     : 1;
/* 3.1*/ uint8_t ButtonPlus      : 1;
/* 3.2*/ uint8_t ButtonRStick    : 1;
/* 3.3*/ uint8_t ButtonLStick    : 1;
/* 3.4*/ uint8_t ButtonHome      : 1;
/* 3.5*/ uint8_t ButtonShare     : 1;
/* 3.6*/ uint8_t ButtonUNK       : 1;
/* 3.7*/ uint8_t ButtonChrg      : 1;
/* 4.0*/ uint8_t ButtonDown      : 1;
/* 4.1*/ uint8_t ButtonUp        : 1;
/* 4.2*/ uint8_t ButtonRight     : 1;
/* 4.3*/ uint8_t ButtonLeft      : 1;
/* 4.4*/ uint8_t ButtonSR_L      : 1;
/* 4.5*/ uint8_t ButtonSL_L      : 1;
/* 4.6*/ uint8_t ButtonL         : 1;
/* 4.7*/ uint8_t ButtonZL        : 1;

/* 5  */ uint8_t LeftAnalog  [3];
/* 8  */ uint8_t RightAnalog [3];

/*11  */ uint8_t Vibration;

/*12  */ uint8_t Unused0;
/*13  */ uint8_t SixAxis [36];
};

#pragma pack(push,1)
struct SK_HID_SwitchPro_OutputPacket
{
  uint8_t ReportID;
  uint8_t Subtype;
  uint8_t Data [47];
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SK_HID_SwitchPro_PacketResponse // 64 (0x81)
{
  uint8_t ReportID;
  uint8_t Subtype;
  uint8_t Data [62];
};
#pragma pack(pop)

void CalcAnalogStick2
(
  float &pOutX,       // out: resulting stick X value
  float &pOutY,       // out: resulting stick Y value
  int16_t x,              // in: initial stick X value
  int16_t y,              // in: initial stick Y value
  int16_t x_calc [3],     // calc -X, CenterX, +X
  int16_t y_calc [3]      // calc -Y, CenterY, +Y
)
{
  float x_f, y_f;

  // convert to float based on calibration and valid ranges per +/-axis
  x = std::clamp (x, x_calc [0], x_calc [2]);
  y = std::clamp (y, y_calc [0], y_calc [2]);

  if (x >= x_calc [1])
  {
  	x_f = (float)(x - x_calc [1]) / (float)(x_calc [2] - x_calc [1]);
  }

  else
  {
  	x_f = -((float)(x - x_calc [1]) / (float)(x_calc [0] - x_calc [1]));
  }

  if (y >= y_calc [1])
  {
  	y_f = (float)(y - y_calc [1]) / (float)(y_calc [2] - y_calc [1]);
  }

  else
  {
  	y_f = -((float)(y - y_calc [1]) / (float)(y_calc [0] - y_calc [1]));
  }

  pOutX = x_f;
  pOutY = y_f;
}

// Sub-types of the 0x80 output report, used for initialization.
const uint8_t kSubTypeRequestMac        = 0x01;
const uint8_t kSubTypeHandshake         = 0x02;
const uint8_t kSubTypeBaudRate          = 0x03;
const uint8_t kSubTypeDisableUsbTimeout = 0x04;
const uint8_t kSubTypeEnableUsbTimeout  = 0x05;

typedef enum {
    k_eSwitchInputReportIDs_SubcommandReply       = 0x21,
    k_eSwitchInputReportIDs_FullControllerState   = 0x30,
    k_eSwitchInputReportIDs_SimpleControllerState = 0x3F,
    k_eSwitchInputReportIDs_CommandAck            = 0x81,
} ESwitchInputReportIDs;

typedef enum {
    k_eSwitchOutputReportIDs_RumbleAndSubcommand = 0x01,
    k_eSwitchOutputReportIDs_Rumble              = 0x10,
    k_eSwitchOutputReportIDs_Proprietary         = 0x80,
} ESwitchOutputReportIDs;

typedef enum {
    k_eSwitchSubcommandIDs_BluetoothManualPair = 0x01,
    k_eSwitchSubcommandIDs_RequestDeviceInfo   = 0x02,
    k_eSwitchSubcommandIDs_SetInputReportMode  = 0x03,
    k_eSwitchSubcommandIDs_SetHCIState         = 0x06,
    k_eSwitchSubcommandIDs_SPIFlashRead        = 0x10,
    k_eSwitchSubcommandIDs_SetPlayerLights     = 0x30,
    k_eSwitchSubcommandIDs_SetHomeLight        = 0x38,
    k_eSwitchSubcommandIDs_EnableIMU           = 0x40,
    k_eSwitchSubcommandIDs_SetIMUSensitivity   = 0x41,
    k_eSwitchSubcommandIDs_EnableVibration     = 0x48,
} ESwitchSubcommandIDs;

static int output_counter = 0;

bool
WriteSubcommand (hid_device_file_s* pDevice, uint8_t sub_command, uint8_t val)
{
  DWORD dwBytesRead = 0;

  BYTE output [64] = { };
  BYTE input  [64] = { };

  // Serial subcommands also carry vibration data. Configure the vibration
  // portion of the report for a neutral vibration effect (zero amplitude).
  // https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md#output-0x12
  output [ 1] = static_cast <uint8_t>(output_counter++ & 0xff);
  output [ 2] = 0x00;
  output [ 3] = 0x01;
  output [ 4] = 0x40;
  output [ 5] = 0x40;
  output [ 6] = 0x00;
  output [ 7] = 0x01;
  output [ 8] = 0x40;
  output [ 9] = 0x40;
  output [10] = sub_command;
  output [11] = val;
//DCHECK_LT(bytes.size() + kSubCommandDataOffset, output_report_size_bytes_);
//base::ranges::copy(bytes, &report_bytes[kSubCommandDataOffset - 1]);

  output [0] = 0x1;

  if (! HidD_SetOutputReport (pDevice->hDeviceFile, output, (ULONG)pDevice->output_report.size ()))
  {
    MessageBox (
      nullptr, std::to_wstring (GetLastError ()).c_str (),
        L"HID Init Error (HidD_SetOutputReport - Switch Pro [Usb])",
          MB_OK
    );

    return false;
  }

  return true;
}

bool
SendPacket (hid_device_file_s* pDevice, uint8_t type)
{
  DWORD dwBytesRead = 0;

  BYTE output [64] = { };
  BYTE input  [64] = { };

  SK_HID_SwitchPro_OutputPacket* out =
    (SK_HID_SwitchPro_OutputPacket *)output;
  SK_HID_SwitchPro_PacketResponse* ack =
    (SK_HID_SwitchPro_PacketResponse *)input;

  out->ReportID = 0x80;
  out->Subtype  = type;

  if (! HidD_SetOutputReport (pDevice->hDeviceFile, output, (ULONG)pDevice->output_report.size ()))
  //if (! WriteFile (pDevice->hDeviceFile, output, pDevice->output_report.size (), &dwBytesRead, nullptr))
  {
      MessageBox (
        nullptr, std::to_wstring (GetLastError ()).c_str (),
          L"HID Init Error (HidD_SetOutputReport - Switch Pro [Usb])",
            MB_OK
      );

    return false;
  }

  ack->ReportID = 0x81;
  ack->Subtype  = type;

  //if (! HidD_GetInputReport (pDevice->hDeviceFile, input, pDevice->input_report.size ()))//, &dwBytesRead,    nullptr))
  if (! ReadFile (pDevice->hDeviceFile,   input, 64, &dwBytesRead,    nullptr))
   {
       MessageBox (
         nullptr, std::to_wstring (GetLastError ()).c_str (),
           L"HID Init Error (ReadFile - Switch Pro [Usb])",
             MB_OK
       );

     return false;
   }

  return true;
}

bool
SK_SwitchPro_GetInputReportUSB (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  // Handshake needed
  //
  if ( pDevice->state.current.dwPacketNumber == 0 &&
       pDevice->state.prev.   dwPacketNumber == 0 &&
       pDevice->state.input_timestamp        == 0 )
  {
    pDevice->state.current.dwPacketNumber = 0;
    pDevice->state.prev.   dwPacketNumber = 0;
    pDevice->state.input_timestamp        = 1;

#if 0
    //if (! SendPacket (pDevice, kSubTypeRequestMac))
    //  MessageBox (NULL, L"Oh no", L"Oh no", MB_OK);
    //if (! SendPacket (pDevice, kSubTypeHandshake))
    //  MessageBox (NULL, L"Oh no", L"Oh no", MB_OK);
    //if (! SendPacket (pDevice, kSubTypeBaudRate))
    //  MessageBox (NULL, L"Oh no", L"Oh no", MB_OK);
    //if (! SendPacket (pDevice, kSubTypeHandshake))
    //  MessageBox (NULL, L"Oh no", L"Oh no", MB_OK);

    WriteSubcommand (pDevice, k_eSwitchSubcommandIDs_SetInputReportMode, k_eSwitchInputReportIDs_FullControllerState);

    MessageBox (NULL, L"Oh Yeah!", L"Oh Yeah!", MB_OK);
#endif
  }

  BYTE report [64] = { };
  //ZeroMemory ( pDevice->input_report.data (),
  //             pDevice->input_report.size () );

  //BYTE* report = pDevice->input_report.data ();

  // HID Input Report 0x30 (USB)
  report [0] = 0x30;

  bool  bNewData    = false;
  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, 64, &dwBytesRead, nullptr))
  {
    SK_HID_SwitchPro_GetStateData *pData =
      (SK_HID_SwitchPro_GetStateData *)&report [1];

    pDevice->state.current.Gamepad = { };

    pDevice->state.current.Gamepad.bLeftTrigger =
      static_cast <BYTE> (pData->ButtonZL * 255);
    pDevice->state.current.Gamepad.bRightTrigger =
      static_cast <BYTE> (pData->ButtonZR * 255);

    uint8_t* pStickData = pData->LeftAnalog;
    int16_t  LeftStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    int16_t  LeftStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

             pStickData = pData->RightAnalog;
    int16_t RightStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    int16_t RightStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

    float fLeftStickX,  fLeftStickY;
    float fRightStickX, fRightStickY;

    int16_t calibrated_ranges [3] = { -32768, 0, 32767 };

    CalcAnalogStick2 (fLeftStickX, fLeftStickY, LeftStickX, LeftStickY, calibrated_ranges,
                                                                        calibrated_ranges );

    CalcAnalogStick2 (fRightStickX, fRightStickY, RightStickX, RightStickY, calibrated_ranges,
                                                                            calibrated_ranges );

    pDevice->state.current.Gamepad.sThumbLX = 0;
      //static_cast <SHORT> (32767.0 * fLeftStickX);
    pDevice->state.current.Gamepad.sThumbLY = 0;
      //static_cast <SHORT> (32767.0 * fLeftStickY);
    pDevice->state.current.Gamepad.sThumbRX = 0;
      //static_cast <SHORT> (32767.0 * fRightStickX);
    pDevice->state.current.Gamepad.sThumbRY = 0;
      //static_cast <SHORT> (32767.0 * fRightStickY);

    if (pData->ButtonUp     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
    if (pData->ButtonDown   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
    if (pData->ButtonLeft   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
    if (pData->ButtonRight  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
    
    if (pData->ButtonY      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_X;
    if (pData->ButtonB      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_A;
    if (pData->ButtonA      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_B;
    if (pData->ButtonX      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

    if (pData->ButtonL      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
    if (pData->ButtonR      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
    if (pData->ButtonZL     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_TRIGGER;
    if (pData->ButtonZR     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_TRIGGER;
    if (pData->ButtonLStick != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
    if (pData->ButtonRStick != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

    if (pData->ButtonMinus  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
    if (pData->ButtonPlus   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

    if (pData->ButtonHome   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

    if (! SK_XInput_UpdatePolledDataAndTimestamp ( pDevice,
                                                   pDevice->state.current.Gamepad.wButtons != 0 ||
                                                   pDevice->state.current.Gamepad.wButtons !=
                                                   pDevice->state.prev.Gamepad.wButtons,
                                                   bNewData ) )
    {
      return false;
    }

#if 0
    // Share button has no representation in XInput
    pData->ButtonShare != 0;
#endif

    return bNewData;
  }

  else
  {
    DWORD dwLastErr =
      GetLastError ();

    if (dwLastErr == ERROR_DEVICE_NOT_CONNECTED ||
        dwLastErr == ERROR_INVALID_HANDLE)
    {
      pDevice->disconnect ();

      return false;
    }

    if (dwLastErr == ERROR_INVALID_USER_BUFFER)
    {
      pDevice->disconnect ();

      return false;
    }

    static auto x =
      MessageBox (
        nullptr, std::to_wstring (dwLastErr).c_str (),
          L"HID Input Error (ReadFile - Switch Pro [USB])",
            MB_OK
      );
  }
  
  return false;
}



bool
SK_SwitchPro_GetInputReportBt (void *pGenericDev)
{
  return false;

  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  //ZeroMemory ( pDevice->input_report.data (),
  //             pDevice->input_report.size () );

  BYTE report [512] = { };//pDevice->input_report.data ();

  // HID Input Report 0x30 (USB)
  report [0] = 0x30;

  bool  bNewData    = false;
  DWORD dwBytesRead = 0;

  //if (ReadFile (pDevice->hDeviceFile, report, 49, &dwBytesRead, nullptr))  
  if (HidD_GetInputReport (pDevice->hDeviceFile, report, (ULONG)pDevice->input_report.size ()))
  {
    SK_HID_SwitchPro_GetStateData *pData =
      (SK_HID_SwitchPro_GetStateData *)&report [1];

    pDevice->state.current.Gamepad = { };

    pDevice->state.current.Gamepad.bLeftTrigger =
      static_cast <BYTE> (pData->ButtonZL * 255);
    pDevice->state.current.Gamepad.bRightTrigger =
      static_cast <BYTE> (pData->ButtonZR * 255);

#if 0
    uint8_t* pStickData = pData->LeftAnalog;
    uint16_t LeftStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t LeftStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

             pStickData  = pData->RightAnalog;
    uint16_t RightStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t RightStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);
#endif

    pDevice->state.current.Gamepad.sThumbLX = 0;
      //static_cast <SHORT> (32767.0 * static_cast <double> (        static_cast <LONG> (LeftStickX ) - 32768) / 32768.0);
    pDevice->state.current.Gamepad.sThumbLY = 0;
      //static_cast <SHORT> (32767.0 * static_cast <double> (32768 - static_cast <LONG> (LeftStickY )        ) / 32768.0);
    pDevice->state.current.Gamepad.sThumbRX = 0;
      //static_cast <SHORT> (32767.0 * static_cast <double> (        static_cast <LONG> (RightStickX) - 32768) / 32768.0);
    pDevice->state.current.Gamepad.sThumbRY = 0;
      //static_cast <SHORT> (32767.0 * static_cast <double> (32768 - static_cast <LONG> (RightStickY)        ) / 32768.0);

    if (pData->ButtonUp     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
    if (pData->ButtonDown   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
    if (pData->ButtonLeft   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
    if (pData->ButtonRight  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
    
    if (pData->ButtonY      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_X;
    if (pData->ButtonB      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_A;
    if (pData->ButtonA      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_B;
    if (pData->ButtonX      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

    if (pData->ButtonL      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
    if (pData->ButtonR      != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
    if (pData->ButtonZL     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_TRIGGER;
    if (pData->ButtonZR     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_TRIGGER;
    if (pData->ButtonLStick != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
    if (pData->ButtonRStick != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

    if (pData->ButtonMinus  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
    if (pData->ButtonPlus   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

    if (pData->ButtonHome   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

    if (! SK_XInput_UpdatePolledDataAndTimestamp ( pDevice,
                                                   pDevice->state.current.Gamepad.wButtons != 0 ||
                                                   pDevice->state.current.Gamepad.wButtons !=
                                                   pDevice->state.prev.Gamepad.wButtons,
                                                   bNewData ) )
    {
      return false;
    }

#if 0
    // Share button has no representation in XInput
    pData->ButtonShare != 0;
#endif

    if (                                                              bNewData &&
         (pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_Y    ) != 0 &&
         (pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE) != 0 )
    {
      if (SK_Bluetooth_PowerOffGamepad (pDevice))
      {
        return false;
      }
    }

    return true;
  }

  else
  {
    DWORD dwLastErr =
      GetLastError ();

    if (dwLastErr == ERROR_DEVICE_NOT_CONNECTED ||
        dwLastErr == ERROR_INVALID_HANDLE)
    {
      pDevice->disconnect ();

      return false;
    }

    if (dwLastErr == ERROR_INVALID_USER_BUFFER)
    {
      pDevice->disconnect ();

      return false;
    }

    static auto x =
      MessageBox (
        nullptr, std::to_wstring (dwLastErr).c_str (),
          L"HID Input Error (ReadFile - Switch Pro [Bt])",
            MB_OK
      );
  }
  
  return false;
}