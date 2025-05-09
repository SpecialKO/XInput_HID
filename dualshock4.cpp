#include "pch.h"

enum Direction : uint8_t {
  North     = 0,
  NorthEast,
  East,
  SouthEast,
  South,
  SouthWest,
  West,
  NorthWest,
  Neutral   = 8
};

enum PowerState : uint8_t {
  Discharging         = 0x00, // Use PowerPercent
  Charging            = 0x01, // Use PowerPercent
  Complete            = 0x02, // PowerPercent not valid? assume 100%?
  AbnormalVoltage     = 0x0A, // PowerPercent not valid?
  AbnormalTemperature = 0x0B, // PowerPercent not valid?
  ChargingError       = 0x0F  // PowerPercent not valid?
};

enum LightBrightness : uint8_t {
  Bright    = 0,
  Mid,
  Dim,
  NoLightAction3,
  NoLightAction4,
  NoLightAction5,
  NoLightAction6,
  NoLightAction7 = 7
};

enum LightFadeAnimation : uint8_t {
  Nothing = 0,
  FadeIn, // from black to blue
  FadeOut // from blue to black
};

struct TouchFingerData { // 4
/*0.0*/ uint32_t Index       : 7;
/*0.7*/ uint32_t NotTouching : 1;
/*1.0*/ uint32_t FingerX     : 12;
/*2.4*/ uint32_t FingerY     : 12;
};

struct TouchData { // 9
/*0*/ TouchFingerData Finger [2];
/*8*/ uint8_t         Timestamp;
};

#pragma pack(push,1)
template <int N> struct BTCRC {
  uint8_t  Buff [N-4];
  uint32_t CRC;
};
#pragma pack(pop)

#pragma pack(push,1)
template <int N> struct BTAudio {
  uint16_t FrameNumber;
  uint8_t  AudioTarget; // 0x02 speaker?, 0x24 headset?, 0x03 mic?
  uint8_t  SBCData [N-3];
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_SetStateData // 47
{
  uint8_t EnableRumbleUpdate        : 1;
  uint8_t EnableLedUpdate           : 1;
  uint8_t EnableLedBlink            : 1;
  uint8_t EnableExtWrite            : 1;
  uint8_t EnableVolumeLeftUpdate    : 1;
  uint8_t EnableVolumeRightUpdate   : 1;
  uint8_t EnableVolumeMicUpdate     : 1;
  uint8_t EnableVolumeSpeakerUpdate : 1;
  uint8_t UNK_RESET1                : 1; // unknown reset, both set high by Remote Play
  uint8_t UNK_RESET2                : 1; // unknown reset, both set high by Remote Play
  uint8_t UNK1                      : 1;
  uint8_t UNK2                      : 1;
  uint8_t UNK3                      : 1;
  uint8_t UNKPad                    : 3;
  uint8_t Empty1;
  uint8_t RumbleRight;              // weak
  uint8_t RumbleLeft;               // strong
  uint8_t LedRed;
  uint8_t LedGreen;
  uint8_t LedBlue;
  uint8_t LedFlashOnPeriod;
  uint8_t LedFlashOffPeriod;
  uint8_t ExtDataSend [8];               // sent to I2C EXT port, stored in 8x8 byte block
  uint8_t VolumeLeft;                    // 0x00 - 0x4F inclusive
  uint8_t VolumeRight;                   // 0x00 - 0x4F inclusive
  uint8_t VolumeMic;                     // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
  uint8_t VolumeSpeaker;                 // 0x00 - 0x4F
  uint8_t UNK_AUDIO1                : 7; // clamped to 1-64 inclusive, appears to be set to 5 for audio
  uint8_t UNK_AUDIO2                : 1; // unknown, appears to be set to 1 for audio
  uint8_t Pad [8];
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_SetStateDataBt // 74
{
  uint8_t EnableRumbleUpdate        : 1;
  uint8_t EnableLedUpdate           : 1;
  uint8_t EnableLedBlink            : 1;
  uint8_t EnableExtWrite            : 1;
  uint8_t EnableVolumeLeftUpdate    : 1;
  uint8_t EnableVolumeRightUpdate   : 1;
  uint8_t EnableVolumeMicUpdate     : 1;
  uint8_t EnableVolumeSpeakerUpdate : 1;
  uint8_t UNK_RESET1                : 1; // unknown reset, both set high by Remote Play
  uint8_t UNK_RESET2                : 1; // unknown reset, both set high by Remote Play
  uint8_t UNK1                      : 1;
  uint8_t UNK2                      : 1;
  uint8_t UNK3                      : 1;
  uint8_t UNKPad                    : 3;
  uint8_t Empty1;
  uint8_t RumbleRight;              // Weak
  uint8_t RumbleLeft;               // Strong
  uint8_t LedRed;
  uint8_t LedGreen;
  uint8_t LedBlue;
  uint8_t LedFlashOnPeriod;
  uint8_t LedFlashOffPeriod;
  uint8_t ExtDataSend [8];               // sent to I2C EXT port, stored in 8x8 byte block
  uint8_t VolumeLeft;                    // 0x00 - 0x4F inclusive
  uint8_t VolumeRight;                   // 0x00 - 0x4F inclusive
  uint8_t VolumeMic;                     // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
  uint8_t VolumeSpeaker;                 // 0x00 - 0x4F
  uint8_t UNK_AUDIO1                : 7; // clamped to 1-64 inclusive, appears to be set to 5 for audio
  uint8_t UNK_AUDIO2                : 1; // unknown, appears to be set to 1 for audio
  uint8_t Pad [52];
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_ReportOut11 // 78
{
  union {
    BTCRC <78> CRC;
    struct {
      uint8_t ReportID;        // 0x11
      uint8_t PollingRate : 6; // note 0 appears to be clamped to 1
      uint8_t EnableCRC   : 1;
      uint8_t EnableHID   : 1;
      uint8_t EnableMic   : 3; // somehow enables mic, appears to be 3 bit flags
      uint8_t UnkA4       : 1;
      uint8_t UnkB1       : 1;
      uint8_t UnkB2       : 1; // seems to always be 1
      uint8_t UnkB3       : 1;
      uint8_t EnableAudio : 1;
      union {
        SK_HID_DualShock4_SetStateDataBt State;
        BTAudio <75> Audio;
      };
    } Data;
  };
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_BasicGetStateData {
/*0  */ uint8_t   LeftStickX;
/*1  */ uint8_t   LeftStickY;
/*2  */ uint8_t   RightStickX;
/*3  */ uint8_t   RightStickY;
/*4.0*/ Direction DPad           : 4;
/*4.4*/ uint8_t   ButtonSquare   : 1;
/*4.5*/ uint8_t   ButtonCross    : 1;
/*4.6*/ uint8_t   ButtonCircle   : 1;
/*4.7*/ uint8_t   ButtonTriangle : 1;
/*5.0*/ uint8_t   ButtonL1       : 1;
/*5.1*/ uint8_t   ButtonR1       : 1;
/*5.2*/ uint8_t   ButtonL2       : 1;
/*5.3*/ uint8_t   ButtonR2       : 1;
/*5.4*/ uint8_t   ButtonShare    : 1;
/*5.5*/ uint8_t   ButtonOptions  : 1;
/*5.6*/ uint8_t   ButtonL3       : 1;
/*5.7*/ uint8_t   ButtonR3       : 1;
/*6.0*/ uint8_t   ButtonHome     : 1;
/*6.1*/ uint8_t   ButtonPad      : 1;
/*6.2*/ uint8_t   Counter        : 6; // always 0 on USB, counts up with some skips on BT
/*7  */ uint8_t   TriggerLeft;
/*8  */ uint8_t   TriggerRight;
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_GetStateData : SK_HID_DualShock4_BasicGetStateData {
/* 9  */ uint16_t Timestamp; // in 5.33us units?
/*11  */ uint8_t  Temperature;
/*12  */ int16_t  AngularVelocityX;
/*14  */ int16_t  AngularVelocityZ;
/*16  */ int16_t  AngularVelocityY;
/*18  */ int16_t  AccelerometerX;
/*20  */ int16_t  AccelerometerY;
/*22  */ int16_t  AccelerometerZ;
/*24  */ uint8_t  ExtData [5]; // range can be set by EXT device
/*29  */ uint8_t  PowerPercent      : 4; // 0x00-0x0A or 0x01-0x0B if plugged int
/*29.4*/ uint8_t  PluggedPowerCable : 1;
/*29.5*/ uint8_t  PluggedHeadphones : 1;
/*29.6*/ uint8_t  PluggedMic        : 1;
/*29,7*/ uint8_t  PluggedExt        : 1;
/*30.0*/ uint8_t  UnkExt1           : 1; // ExtCapableOfExtraData?
/*30.1*/ uint8_t  UnkExt2           : 1; // ExtHasExtraData?
/*30.2*/ uint8_t  NotConnected      : 1; // Used by dongle to indicate no controller
/*30.3*/ uint8_t  Unk1              : 5;
/*31  */ uint8_t  Unk2; // unused?
/*32  */ uint8_t  TouchCount;
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_GetStateDataUSB : SK_HID_DualShock4_GetStateData {
  TouchData TouchData [3];
  uint8_t   Pad       [3];
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualShock4_GetStateDataBt : SK_HID_DualShock4_GetStateData {
  TouchData TouchData [4];
  uint8_t Pad         [6];
};
#pragma pack(pop)


bool
SK_DualShock4_GetInputReportUSB (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  ZeroMemory ( pDevice->input_report.data (),
               pDevice->input_report.size () );

  BYTE* report = pDevice->input_report.data ();

  // HID Input Report 0x1 (USB)
  report [0] = 0x1;

  DWORD dwBytesRead = 0;
  bool  bNewData    = false;

  if (ReadFile (pDevice->hDeviceFile, report, (DWORD)pDevice->input_report.size (), &dwBytesRead, nullptr))
  {
    SK_HID_DualShock4_GetStateDataUSB *pData =
      (SK_HID_DualShock4_GetStateDataUSB *)&report [1];

    pDevice->state.current.Gamepad = { };

    pDevice->state.current.Gamepad.bLeftTrigger =
      static_cast <BYTE> (pData->TriggerLeft);
    pDevice->state.current.Gamepad.bRightTrigger =
      static_cast <BYTE> (pData->TriggerRight);

    pDevice->state.current.Gamepad.sThumbLX =
      static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pData->LeftStickX) - 128) / 128.0);
    pDevice->state.current.Gamepad.sThumbLY =
      static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pData->LeftStickY)      ) / 128.0);
    pDevice->state.current.Gamepad.sThumbRX =
      static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pData->RightStickX) - 128) / 128.0);
    pDevice->state.current.Gamepad.sThumbRY =
      static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pData->RightStickY)      ) / 128.0);

    switch (pData->DPad)
    {
      case North:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
      case NorthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
                      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
      case East:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
      case SouthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
                      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
      case South:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
      case SouthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
                      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
      case West:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
      case NorthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
                      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
      case Neutral:
      default:
        // Centered value, do nothing
        break;
    }
    
    if (pData->ButtonSquare   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_X;
    if (pData->ButtonCross    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_A;
    if (pData->ButtonCircle   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_B;
    if (pData->ButtonTriangle != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

    if (pData->ButtonL1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
    if (pData->ButtonR1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
    if (pData->ButtonL2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_TRIGGER;
    if (pData->ButtonR2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_TRIGGER;
    if (pData->ButtonL3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
    if (pData->ButtonR3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

    if (pData->ButtonShare    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
    if (pData->ButtonOptions  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

    if (pData->ButtonHome     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

    float LX    = pDevice->state.current.Gamepad.sThumbLX;
    float LY    = pDevice->state.current.Gamepad.sThumbLY;
    float normL = sqrtf ( LX*LX + LY*LY );

    float RX    = pDevice->state.current.Gamepad.sThumbRX;
    float RY    = pDevice->state.current.Gamepad.sThumbRY;
    float normR = sqrtf ( RX*RX + RY*RY );

    if (! SK_XInput_UpdatePolledDataAndTimestamp (
                   pDevice, (
                   pDevice->state.current.Gamepad.wButtons     !=                                    0 ||
                   pDevice->state.current.Gamepad.wButtons     != pDevice->state.prev.Gamepad.wButtons ||
                                                          normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  * 2 ||
                                                          normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2 ||
                   pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2 ||
                   pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2
                            ), bNewData          )
       )
    {
      return false;
    }

#if 0
    // Common to DualSense and DualShock4, but no representation in XInput
    pData->ButtonPad != 0;
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
          L"HID Input Error (ReadFile - DualShock4 [USB])",
            MB_OK
      );
  }
  
  return false;
}



bool
SK_DualShock4_GetInputReportBt (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  ZeroMemory ( pDevice->input_report.data (),
               pDevice->input_report.size () );

  BYTE* report = pDevice->input_report.data ();

  // HID Input Report 0x11 (Bluetooth)
  report [0] = 0x11;

  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, (DWORD)pDevice->input_report.size (), &dwBytesRead, nullptr))
  {
    pDevice->state.current.Gamepad = { };

    BYTE* pInputRaw =
      (BYTE *)&report [0];

    SK_HID_DualShock4_GetStateDataBt *pData =
      (SK_HID_DualShock4_GetStateDataBt *)&pInputRaw [3];

    bool bSimple = false;

    // We're in simplified mode...
    if (pInputRaw [0] == 0x1)
    {
      bSimple = true;
    }

    bool bNewData = false;
    
    if (! bSimple)
    {
      pDevice->state.current.Gamepad.bLeftTrigger =
        static_cast <BYTE> (pData->TriggerLeft);
      pDevice->state.current.Gamepad.bRightTrigger =
        static_cast <BYTE> (pData->TriggerRight);

      pDevice->state.current.Gamepad.sThumbLX =
        static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pData->LeftStickX) - 128) / 128.0);
      pDevice->state.current.Gamepad.sThumbLY =
        static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pData->LeftStickY)      ) / 128.0);
      pDevice->state.current.Gamepad.sThumbRX =
        static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pData->RightStickX) - 128) / 128.0);
      pDevice->state.current.Gamepad.sThumbRY =
        static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pData->RightStickY)      ) / 128.0);

      switch (pData->DPad)
      {
        case North:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
        case NorthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
        case East:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
        case SouthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
        case South:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
        case SouthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
        case West:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
        case NorthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
        case Neutral:
        default:
          // Centered value, do nothing
          break;
      }
      
      if (pData->ButtonSquare   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_X;
      if (pData->ButtonCross    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_A;
      if (pData->ButtonCircle   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_B;
      if (pData->ButtonTriangle != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

      if (pData->ButtonL1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
      if (pData->ButtonR1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
      if (pData->ButtonL2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_TRIGGER;
      if (pData->ButtonR2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_TRIGGER;
      if (pData->ButtonL3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
      if (pData->ButtonR3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

      if (pData->ButtonShare    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
      if (pData->ButtonOptions  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

      if (pData->ButtonHome     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

      float LX    = pDevice->state.current.Gamepad.sThumbLX;
      float LY    = pDevice->state.current.Gamepad.sThumbLY;
      float normL = sqrtf ( LX*LX + LY*LY );

      float RX    = pDevice->state.current.Gamepad.sThumbRX;
      float RY    = pDevice->state.current.Gamepad.sThumbRY;
      float normR = sqrtf ( RX*RX + RY*RY );

      if (! SK_XInput_UpdatePolledDataAndTimestamp (
                     pDevice, (
                     pDevice->state.current.Gamepad.wButtons     !=                                    0 ||
                     pDevice->state.current.Gamepad.wButtons     != pDevice->state.prev.Gamepad.wButtons ||
                                                            normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  * 2 ||
                                                            normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2 ||
                     pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2 ||
                     pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2
                              ), bNewData          )
         )
      {
        return false;
      }
    }

    else
    {
      SK_HID_DualShock4_BasicGetStateData *pSimpleData =
        (SK_HID_DualShock4_BasicGetStateData *)&pInputRaw [1];

#if 0
      // The analog axes
      memset (
        &pSimpleData->LeftStickX,  128,   4);
      memset (
        &pSimpleData->TriggerLeft,   0,   2);
         pSimpleData->Counter++;
         pSimpleData->DPad               = Neutral;
         pSimpleData->ButtonSquare       = 0;
         pSimpleData->ButtonCross        = 0;
         pSimpleData->ButtonCircle       = 0;
         pSimpleData->ButtonTriangle     = 0;
         pSimpleData->ButtonL1           = 0;
         pSimpleData->ButtonR1           = 0;
         pSimpleData->ButtonL2           = 0;
         pSimpleData->ButtonR2           = 0;
         pSimpleData->ButtonShare        = 0;
         pSimpleData->ButtonOptions      = 0;
         pSimpleData->ButtonL3           = 0;
         pSimpleData->ButtonR3           = 0;
         pSimpleData->ButtonHome         = 0;
         pSimpleData->ButtonPad          = 0;
#endif

      pDevice->state.current.Gamepad.bLeftTrigger =
        static_cast <BYTE> (pSimpleData->TriggerLeft);
      pDevice->state.current.Gamepad.bRightTrigger =
        static_cast <BYTE> (pSimpleData->TriggerRight);

      pDevice->state.current.Gamepad.sThumbLX =
        static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pSimpleData->LeftStickX) - 128) / 128.0);
      pDevice->state.current.Gamepad.sThumbLY =
        static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pSimpleData->LeftStickY)      ) / 128.0);
      pDevice->state.current.Gamepad.sThumbRX =
        static_cast <SHORT> (32767.0 * static_cast <double> (      static_cast <LONG> (pSimpleData->RightStickX) - 128) / 128.0);
      pDevice->state.current.Gamepad.sThumbRY =
        static_cast <SHORT> (32767.0 * static_cast <double> (128 - static_cast <LONG> (pSimpleData->RightStickY)      ) / 128.0);

      switch (pSimpleData->DPad)
      {
        case North:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
        case NorthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
        case East:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
        case SouthEast: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
        case South:     pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;  break;
        case SouthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
        case West:      pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;  break;
        case NorthWest: pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
                        pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;    break;
        case Neutral:
        default:
          // Centered value, do nothing
          break;
      }
      
      if (pSimpleData->ButtonSquare   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_X;
      if (pSimpleData->ButtonCross    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_A;
      if (pSimpleData->ButtonCircle   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_B;
      if (pSimpleData->ButtonTriangle != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

      if (pSimpleData->ButtonL1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
      if (pSimpleData->ButtonR1       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
      if (pSimpleData->ButtonL2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_TRIGGER;
      if (pSimpleData->ButtonR2       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_TRIGGER;
      if (pSimpleData->ButtonL3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
      if (pSimpleData->ButtonR3       != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

      if (pSimpleData->ButtonShare    != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
      if (pSimpleData->ButtonOptions  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

      if (pSimpleData->ButtonHome     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

      float LX    = pDevice->state.current.Gamepad.sThumbLX;
      float LY    = pDevice->state.current.Gamepad.sThumbLY;
      float normL = sqrtf ( LX*LX + LY*LY );

      float RX    = pDevice->state.current.Gamepad.sThumbRX;
      float RY    = pDevice->state.current.Gamepad.sThumbRY;
      float normR = sqrtf ( RX*RX + RY*RY );

      if (! SK_XInput_UpdatePolledDataAndTimestamp (
                     pDevice, (
                     pDevice->state.current.Gamepad.wButtons     !=                                    0 ||
                     pDevice->state.current.Gamepad.wButtons     != pDevice->state.prev.Gamepad.wButtons ||
                                                            normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  * 2 ||
                                                            normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2 ||
                     pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2 ||
                     pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2
                              ), bNewData          )
         )
      {
        return false;
      }
    }

#if 0
    // Common to DualSense and DualShock4, but no representation in XInput
    pData->ButtonPad != 0;
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
          L"HID Input Error (ReadFile - DualShock4 [Bt])",
            MB_OK
      );
  }
  
  return false;
}