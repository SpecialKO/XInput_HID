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

void CalcAnalogStick2
(
  float &pOutX,       // out: resulting stick X value
  float &pOutY,       // out: resulting stick Y value
  uint16_t x,              // in: initial stick X value
  uint16_t y,              // in: initial stick Y value
  uint16_t x_calc [3],     // calc -X, CenterX, +X
  uint16_t y_calc [3]      // calc -Y, CenterY, +Y
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

bool
SK_SwitchPro_GetInputReportUSB (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  static thread_local uint8_t report [2048] = { };
                  ZeroMemory (report, 2048);

  // HID Input Report 0x30 (USB)
  report [0] = 0x30;

  bool  bNewData    = false;
  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, 2048, &dwBytesRead, nullptr))
  {
    SK_HID_SwitchPro_GetStateData *pData =
      (SK_HID_SwitchPro_GetStateData *)&report [1];

    pDevice->state.current.Gamepad = { };

    pDevice->state.current.Gamepad.bLeftTrigger =
      static_cast <BYTE> (pData->ButtonZL * 255);
    pDevice->state.current.Gamepad.bRightTrigger =
      static_cast <BYTE> (pData->ButtonZR * 255);

    uint8_t* pStickData = pData->LeftAnalog;
    uint16_t LeftStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t LeftStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

             pStickData  = pData->RightAnalog;
    uint16_t RightStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t RightStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

    float fLeftStickX,  fLeftStickY;
    float fRightStickX, fRightStickY;

    uint16_t calibrated_ranges [3] = { -32768, 0, 32767 };

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

    //if (memcmp (&pDevice->state.current.Gamepad, &pDevice->state.prev.Gamepad, sizeof (XINPUT_GAMEPAD)))
    if (pDevice->state.current.Gamepad.wButtons)/// ||
               //( ( abs (pDevice->state.current.Gamepad.sThumbLX) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbLY) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbRX) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbRY) > 5000 ||
               //         pDevice->state.current.Gamepad.bLeftTrigger  > 30 ||
               //         pDevice->state.current.Gamepad.bRightTrigger > 30 ) && memcmp (&pDevice->state.current.Gamepad, &pDevice->state.prev.Gamepad, sizeof (XINPUT_GAMEPAD)) ))
    {
      pDevice->state.prev = pDevice->state.current;
      pDevice->state.current.dwPacketNumber++;

      bNewData = true;
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
      if (dwLastErr != ERROR_INVALID_HANDLE)
        CloseHandle (std::exchange (pDevice->hDeviceFile, INVALID_HANDLE_VALUE));

      pDevice->hDeviceFile =
               CreateFileW ( pDevice->wszDevicePath,
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                               FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                 nullptr, OPEN_EXISTING, 0x0, nullptr );

      if (pDevice->hDeviceFile == INVALID_HANDLE_VALUE)
        pDevice->bConnected = false;

      return false;
    }

    if (dwLastErr == ERROR_INVALID_USER_BUFFER ||
        dwLastErr == ERROR_INVALID_PARAMETER)
    {
      pDevice->bConnected = false;

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
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  static thread_local uint8_t report [2048] = { };
                  ZeroMemory (report, 2048);

  // HID Input Report 0x30 (USB)
  report [0] = 0x30;

  bool  bNewData    = false;
  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, 2048, &dwBytesRead, nullptr))
  {
    SK_HID_SwitchPro_GetStateData *pData =
      (SK_HID_SwitchPro_GetStateData *)&report [1];

    pDevice->state.current.Gamepad = { };

    pDevice->state.current.Gamepad.bLeftTrigger =
      static_cast <BYTE> (pData->ButtonZL * 255);
    pDevice->state.current.Gamepad.bRightTrigger =
      static_cast <BYTE> (pData->ButtonZR * 255);

    uint8_t* pStickData = pData->LeftAnalog;
    uint16_t LeftStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t LeftStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

             pStickData  = pData->RightAnalog;
    uint16_t RightStickX =  pStickData [0]       | ((pStickData [1] & 0xF) << 8);
    uint16_t RightStickY = (pStickData [1] >> 4) |  (pStickData [2]        << 4);

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

    if (pDevice->state.current.Gamepad.wButtons)
               //( ( abs (pDevice->state.current.Gamepad.sThumbLX) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbLY) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbRX) > 5000 ||
               //    abs (pDevice->state.current.Gamepad.sThumbRY) > 5000 ||
               //         pDevice->state.current.Gamepad.bLeftTrigger  > 30 ||
               //         pDevice->state.current.Gamepad.bRightTrigger > 30 ) && memcmp (&pDevice->state.current.Gamepad, &pDevice->state.prev.Gamepad, sizeof (XINPUT_GAMEPAD)) ))
    {
      pDevice->state.prev = pDevice->state.current;
      pDevice->state.current.dwPacketNumber++;

      bNewData = true;
    }

#if 0
    // Share button has no representation in XInput
    pData->ButtonShare != 0;
#endif

    if ( (pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_Y    ) != 0 &&
         (pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE) != 0 )
    {
      if (SK_Bluetooth_PowerOffGamepad (pDevice))
      {
        pDevice->bConnected = false;

        return false;
      }
    }

    return false;
  }

  else
  {
    DWORD dwLastErr =
      GetLastError ();

    if (dwLastErr == ERROR_DEVICE_NOT_CONNECTED ||
        dwLastErr == ERROR_INVALID_HANDLE)
    {
      if (dwLastErr != ERROR_INVALID_HANDLE)
        CloseHandle (std::exchange (pDevice->hDeviceFile, INVALID_HANDLE_VALUE));

      pDevice->hDeviceFile =
               CreateFileW ( pDevice->wszDevicePath,
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                               FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                 nullptr, OPEN_EXISTING, 0x0, nullptr );

      if (pDevice->hDeviceFile == INVALID_HANDLE_VALUE)
        pDevice->bConnected = false;

      return false;
    }

    if (dwLastErr == ERROR_INVALID_USER_BUFFER ||
        dwLastErr == ERROR_INVALID_PARAMETER)
    {
      pDevice->bConnected = false;

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