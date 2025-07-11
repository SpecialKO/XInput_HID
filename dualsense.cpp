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

enum MuteLight : uint8_t {
  Off       = 0,
  On,
  Breathing,
  DoNothing, // literally nothing, this input is ignored,
             // though it might be a faster blink in other versions
  NoMuteAction4,
  NoMuteAction5,
  NoMuteAction6,
  NoMuteAction7 = 7
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

// Derived from (2024-02-07):
// 
//   https://controllers.fandom.com/wiki/Sony_DualSense#Likely_Interface
//
#pragma pack(push,1)
struct SK_HID_DualSense_SetStateData // 47
{
/* 0.0*/ uint8_t EnableRumbleEmulation         : 1; // Suggest halving rumble strength
/* 0.1*/ uint8_t UseRumbleNotHaptics           : 1; // 
/*    */                                       
/* 0.2*/ uint8_t AllowRightTriggerFFB          : 1; // Enable setting RightTriggerFFB
/* 0.3*/ uint8_t AllowLeftTriggerFFB           : 1; // Enable setting LeftTriggerFFB
/*    */                                       
/* 0.4*/ uint8_t AllowHeadphoneVolume          : 1; // Enable setting VolumeHeadphones
/* 0.5*/ uint8_t AllowSpeakerVolume            : 1; // Enable setting VolumeSpeaker
/* 0.6*/ uint8_t AllowMicVolume                : 1; // Enable setting VolumeMic
/*    */                                       
/* 0.7*/ uint8_t AllowAudioControl             : 1; // Enable setting AudioControl section
/* 1.0*/ uint8_t AllowMuteLight                : 1; // Enable setting MuteLightMode
/* 1.1*/ uint8_t AllowAudioMute                : 1; // Enable setting MuteControl section
/*    */                                       
/* 1.2*/ uint8_t AllowLedColor                 : 1; // Enable RGB LED section
/*    */                                       
/* 1.3*/ uint8_t ResetLights                   : 1; // Release the LEDs from Wireless firmware control
/*    */                                            // When in wireless mode this must be signaled to control LEDs
/*    */                                            // This cannot be applied during the BT pair animation.
/*    */                                            // SDL2 waits until the SensorTimestamp value is >= 10200000
/*    */                                            // before pulsing this bit once.
/*    */
/* 1.4*/ uint8_t AllowPlayerIndicators         : 1; // Enable setting PlayerIndicators section
/* 1.5*/ uint8_t AllowHapticLowPassFilter      : 1; // Enable HapticLowPassFilter
/* 1.6*/ uint8_t AllowMotorPowerLevel          : 1; // MotorPowerLevel reductions for trigger/haptic
/* 1.7*/ uint8_t AllowAudioControl2            : 1; // Enable setting AudioControl2 section
/*    */                                       
/* 2  */ uint8_t RumbleEmulationRight;              // emulates the light weight
/* 3  */ uint8_t RumbleEmulationLeft;               // emulated the heavy weight
/*    */
/* 4  */ uint8_t VolumeHeadphones;                  // max 0x7f
/* 5  */ uint8_t VolumeSpeaker;                     // PS5 appears to only use the range 0x3d-0x64
/* 6  */ uint8_t VolumeMic;                         // not linear, seems to max at 64, 0 is not fully muted
/*    */
/*    */ // AudioControl
/* 7.0*/ uint8_t MicSelect                     : 2; // 0 Auto
/*    */                                            // 1 Internal Only
/*    */                                            // 2 External Only
/*    */                                            // 3 Unclear, sets external mic flag but might use internal mic, do test
/* 7.2*/ uint8_t EchoCancelEnable              : 1;
/* 7.3*/ uint8_t NoiseCancelEnable             : 1;
/* 7.4*/ uint8_t OutputPathSelect              : 2; // 0 L_R_X
/*    */                                            // 1 L_L_X
/*    */                                            // 2 L_L_R
/*    */                                            // 3 X_X_R
/* 7.6*/ uint8_t InputPathSelect               : 2; // 0 CHAT_ASR
/*    */                                            // 1 CHAT_CHAT
/*    */                                            // 2 ASR_ASR
/*    */                                            // 3 Does Nothing, invalid
/*    */
/* 8  */ MuteLight MuteLightMode;
/*    */
/*    */ // MuteControl
/* 9.0*/ uint8_t TouchPowerSave                : 1;
/* 9.1*/ uint8_t MotionPowerSave               : 1;
/* 9.2*/ uint8_t HapticPowerSave               : 1; // AKA BulletPowerSave
/* 9.3*/ uint8_t AudioPowerSave                : 1;
/* 9.4*/ uint8_t MicMute                       : 1;
/* 9.5*/ uint8_t SpeakerMute                   : 1;
/* 9.6*/ uint8_t HeadphoneMute                 : 1;
/* 9.7*/ uint8_t HapticMute                    : 1; // AKA BulletMute
/*    */
/*10  */ uint8_t  RightTriggerFFB [11];
/*21  */ uint8_t  LeftTriggerFFB  [11];
/*32  */ uint32_t HostTimestamp;                    // mirrored into report read
/*    */
/*    */ // MotorPowerLevel
/*36.0*/ uint8_t TriggerMotorPowerReduction    : 4; // 0x0-0x7 (no 0x8?) Applied in 12.5% reductions
/*36.4*/ uint8_t RumbleMotorPowerReduction     : 4; // 0x0-0x7 (no 0x8?) Applied in 12.5% reductions
/*    */
/*    */ // AudioControl2
/*37.0*/ uint8_t SpeakerCompPreGain            : 3; // additional speaker volume boost
/*37.3*/ uint8_t BeamformingEnable             : 1; // Probably for MIC given there's 2, might be more bits, can't find what it does
/*37.4*/ uint8_t UnkAudioControl2              : 4; // some of these bits might apply to the above
/*    */
/*38.0*/ uint8_t AllowLightBrightnessChange    : 1; // LED_BRIHTNESS_CONTROL
/*38.1*/ uint8_t AllowColorLightFadeAnimation  : 1; // LIGHTBAR_SETUP_CONTROL
/*38.2*/ uint8_t EnableImprovedRumbleEmulation : 1; // Use instead of EnableRumbleEmulation
                                                    // requires FW >= 0x0224
                                                    // No need to halve rumble strength
/*38.3*/ uint8_t UNKBITC                       : 5; // unused
/*    */
/*39.0*/ uint8_t HapticLowPassFilter           : 1;
/*39.1*/ uint8_t UNKBIT                        : 7;
/*    */
/*40  */ uint8_t UNKBYTE;                           // previous notes suggested this was HLPF, was probably off by 1
/*    */
/*41  */ LightFadeAnimation LightFadeAnimation;
/*42  */ LightBrightness    LightBrightness;
/*    */
/*    */ // PlayerIndicators
/*    */ // These bits control the white LEDs under the touch pad.
/*    */ // Note the reduction in functionality for later revisions.
/*    */ // Generation 0x03 - Full Functionality
/*    */ // Generation 0x04 - Mirrored Only
/*    */ // Suggested detection: (HardwareInfo & 0x00FFFF00) == 0X00000400
/*    */ //
/*    */ // Layout used by PS5:
/*    */ // 0x04 - -x- -  Player 1
/*    */ // 0x06 - x-x -  Player 2
/*    */ // 0x15 x -x- x  Player 3
/*    */ // 0x1B x x-x x  Player 4
/*    */ // 0x1F x xxx x  Player 5* (Unconfirmed)
/*    */ //
/*    */ //                                         // HW 0x03 // HW 0x04
/*43.0*/ uint8_t PlayerLight1                  : 1; // x --- - // x --- x
/*43.1*/ uint8_t PlayerLight2                  : 1; // - x-- - // - x-x -
/*43.2*/ uint8_t PlayerLight3                  : 1; // - -x- - // - -x- -
/*43.3*/ uint8_t PlayerLight4                  : 1; // - --x - // - x-x -
/*43.4*/ uint8_t PlayerLight5                  : 1; // - --- x // x --- x
/*43.5*/ uint8_t PlayerLightFade               : 1; // if low player lights fade in, if high player lights instantly change
/*43.6*/ uint8_t PlayerLightUNK                : 2;
/*    */
/*    */ // RGB LED
/*44  */ uint8_t LedRed;
/*45  */ uint8_t LedGreen;
/*46  */ uint8_t LedBlue;
// Structure ends here though on BT there is padding and a CRC, see ReportOut31
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualSense_BtReport_0x31
{
  union {
    BTCRC <78> CRC;
    struct {
      uint8_t  ReportID;      // 0x31
      uint8_t  UNK1      : 1; // -+
      uint8_t  EnableHID : 1; //  | - these 3 bits seem to act as an enum
      uint8_t  UNK2      : 1; // -+
      uint8_t  UNK3      : 1;
      uint8_t  SeqNo     : 4; // increment for every write // we have no proof of this, need to see some PS5 captures
      SK_HID_DualSense_SetStateData
               State;
    } Data;
  };
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualSense_GetSimpleStateDataBt // 9
{
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
/*6.1*/ uint8_t   ButtonHome     : 1;
/*6.2*/ uint8_t   ButtonPad      : 1;
/*6.3*/ uint8_t   Counter        : 6;
/*7  */ uint8_t   TriggerLeft;
/*8  */ uint8_t   TriggerRight;
// anything beyond this point, if set, is invalid junk data that was not cleared
};
#pragma pack(pop)

#pragma pack(push,1)
struct SK_HID_DualSense_GetStateData // 63
{
/* 0  */ uint8_t    LeftStickX;
/* 1  */ uint8_t    LeftStickY;
/* 2  */ uint8_t    RightStickX;
/* 3  */ uint8_t    RightStickY;
/* 4  */ uint8_t    TriggerLeft;
/* 5  */ uint8_t    TriggerRight;
/* 6  */ uint8_t    SeqNo;                   // always 0x01 on BT
/* 7.0*/ Direction  DPad                : 4;
/* 7.4*/ uint8_t    ButtonSquare        : 1;
/* 7.5*/ uint8_t    ButtonCross         : 1;
/* 7.6*/ uint8_t    ButtonCircle        : 1;
/* 7.7*/ uint8_t    ButtonTriangle      : 1;
/* 8.0*/ uint8_t    ButtonL1            : 1;
/* 8.1*/ uint8_t    ButtonR1            : 1;
/* 8.2*/ uint8_t    ButtonL2            : 1;
/* 8.3*/ uint8_t    ButtonR2            : 1;
/* 8.4*/ uint8_t    ButtonCreate        : 1;
/* 8.5*/ uint8_t    ButtonOptions       : 1;
/* 8.6*/ uint8_t    ButtonL3            : 1;
/* 8.7*/ uint8_t    ButtonR3            : 1;
/* 9.0*/ uint8_t    ButtonHome          : 1;
/* 9.1*/ uint8_t    ButtonPad           : 1;
/* 9.2*/ uint8_t    ButtonMute          : 1;
/* 9.3*/ uint8_t    UNK1                : 1; // appears unused
/* 9.4*/ uint8_t    ButtonLeftFunction  : 1; // DualSense Edge
/* 9.5*/ uint8_t    ButtonRightFunction : 1; // DualSense Edge
/* 9.6*/ uint8_t    ButtonLeftPaddle    : 1; // DualSense Edge
/* 9.7*/ uint8_t    ButtonRightPaddle   : 1; // DualSense Edge
/*10  */ uint8_t    UNK2;                    // appears unused
/*11  */ uint32_t   UNK_COUNTER;             // Linux driver calls this reserved, tools leak calls the 2 high bytes "random"
/*15  */ int16_t    AngularVelocityX;
/*17  */ int16_t    AngularVelocityZ;
/*19  */ int16_t    AngularVelocityY;
/*21  */ int16_t    AccelerometerX;
/*23  */ int16_t    AccelerometerY;
/*25  */ int16_t    AccelerometerZ;
/*27  */ uint32_t   SensorTimestamp;
/*31  */ int8_t     Temperature;                  // reserved2 in Linux driver
/*32  */ TouchData  TouchData;
/*41.0*/ uint8_t    TriggerRightStopLocation : 4; // trigger stop can be a range from 0 to 9 (F/9.0 for Apple interface)
/*41.4*/ uint8_t    TriggerRightStatus       : 4;
/*42.0*/ uint8_t    TriggerLeftStopLocation  : 4;
/*42.4*/ uint8_t    TriggerLeftStatus        : 4; // 0 feedbackNoLoad
                                                  // 1 feedbackLoadApplied
                                                  // 0 weaponReady
                                                  // 1 weaponFiring
                                                  // 2 weaponFired
                                                  // 0 vibrationNotVibrating
                                                  // 1 vibrationIsVibrating
/*43  */ uint32_t   HostTimestamp;                // mirrors data from report write
/*47.0*/ uint8_t    TriggerRightEffect       : 4; // Active trigger effect, previously we thought this was status max
/*47.4*/ uint8_t    TriggerLeftEffect        : 4; // 0 for reset and all other effects
                                                  // 1 for feedback effect
                                                  // 2 for weapon effect
                                                  // 3 for vibration
/*48  */ uint32_t   DeviceTimeStamp;
/*52.0*/ uint8_t    PowerPercent             : 4; // 0x00-0x0A
/*52.4*/ PowerState PowerState               : 4;
/*53.0*/ uint8_t    PluggedHeadphones        : 1;
/*53.1*/ uint8_t    PluggedMic               : 1;
/*53.2*/ uint8_t    MicMuted                 : 1; // Mic muted by powersave/mute command
/*53.3*/ uint8_t    PluggedUsbData           : 1;
/*53.4*/ uint8_t    PluggedUsbPower          : 1;
/*53.5*/ uint8_t    PluggedUnk1              : 3;
/*54.0*/ uint8_t    PluggedExternalMic       : 1; // Is external mic active (automatic in mic auto mode)
/*54.1*/ uint8_t    HapticLowPassFilter      : 1; // Is the Haptic Low-Pass-Filter active?
/*54.2*/ uint8_t    PluggedUnk3              : 6;
/*55  */ uint8_t    AesCmac [8];
};
#pragma pack(pop)

void SK_DualSense_HardCodedEdgeStuff (hid_device_file_s* pDevice)
{
  static DWORD
        dwLastTest = 0;
  DWORD dwTimeNow  = timeGetTime ();

  if (dwTimeNow < std::exchange (dwLastTest, dwTimeNow) + 50)
    return;

  static bool bIsEdge            = false;
  static HWND hWndLastForeground = 0;

  HWND hWndForeground =
    GetForegroundWindow ();

  if (hWndForeground != hWndLastForeground)
  {
    bIsEdge            = false;
    hWndLastForeground = hWndForeground;

    DWORD                                      dwPid = 0;
    GetWindowThreadProcessId (hWndForeground, &dwPid);

    std::string name;

    HANDLE hProcess =
      OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);

    if (hProcess != 0)
    {
      DWORD buffSize = 1023;
      CHAR  buffer    [1024] = { };

      if (QueryFullProcessImageNameA (hProcess, 0, buffer, &buffSize))
      {                                        // Special behavior for Steam versions of SKIF
        bIsEdge = StrStrIA (buffer, "edge");// && PathFileExists (L"steam_appid.txt");
      }

      CloseHandle (hProcess);
    }
  }

  if (! bIsEdge)
    return;

  SetFocus (hWndForeground);
  
  if (memcmp (&pDevice->state.prev.Gamepad, &pDevice->state.current.Gamepad, sizeof (XINPUT_GAMEPAD)))
  {
    if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey (VK_RIGHT, 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_RIGHT, bScancode, dwFlags,                   0);
      keybd_event (VK_RIGHT, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey (VK_LEFT, 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_LEFT, bScancode, dwFlags,                   0);
      keybd_event (VK_LEFT, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey (VK_UP, 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_UP, bScancode, dwFlags,                   0);
      keybd_event (VK_UP, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey (VK_DOWN, 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_DOWN, bScancode, dwFlags,                   0);
      keybd_event (VK_DOWN, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_A) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_A)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey (VK_SPACE, 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_SPACE, bScancode, dwFlags,                   0);
      keybd_event (VK_SPACE, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_Y) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_Y)))
    {
      BYTE bScancode =
        (BYTE)MapVirtualKey ('F', 0);
    
      DWORD dwFlags =
        ( bScancode & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0) :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event ('F', bScancode, dwFlags,                   0);
      keybd_event ('F', bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_X) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_X)))
    {
      POINT          ptCursor = { };
      GetCursorPos (&ptCursor);
    
      INPUT input [2]            = {         };
            input [0].type       = INPUT_MOUSE;
            input [0].mi.dx      = 0;
            input [0].mi.dy      = 0;
            input [0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    
      SendInput (1, input, sizeof (INPUT));
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)))
    {
      BYTE bScancode0 =
        (BYTE)MapVirtualKey (VK_SHIFT, 0);
      BYTE bScancode1 =
        (BYTE)MapVirtualKey ('N', 0);
    
      DWORD dwFlags0 =
        ( bScancode0 & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0)  :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
      DWORD dwFlags1 =
        ( bScancode1 & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0)  :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_SHIFT, bScancode0, dwFlags0,                   0);
      keybd_event ('N',      bScancode1, dwFlags1,                   0);
      keybd_event (VK_SHIFT, bScancode0, dwFlags0 | KEYEVENTF_KEYUP, 0);
      keybd_event ('N',      bScancode1, dwFlags1 | KEYEVENTF_KEYUP, 0);
    }
    
    else if ((pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) && (! (pDevice->state.prev.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)))
    {
      BYTE bScancode0 =
        (BYTE)MapVirtualKey (VK_SHIFT, 0);
      BYTE bScancode1 =
        (BYTE)MapVirtualKey ('P', 0);
    
      DWORD dwFlags0 =
        ( bScancode0 & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0)  :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
      DWORD dwFlags1 =
        ( bScancode1 & 0xE0 ) == 0   ?
          static_cast <DWORD> (0x0)  :
          static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);
    
      keybd_event (VK_SHIFT, bScancode0, dwFlags0,                   0);
      keybd_event ('P',      bScancode1, dwFlags1,                   0);
      keybd_event (VK_SHIFT, bScancode0, dwFlags0 | KEYEVENTF_KEYUP, 0);
      keybd_event ('P',      bScancode1, dwFlags1 | KEYEVENTF_KEYUP, 0);
    }
  }
}

bool
SK_DualSense_GetInputReportUSB (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  ZeroMemory ( pDevice->input_report.data (),
               pDevice->input_report.size () );

  BYTE* report = pDevice->input_report.data ();

  // HID Input Report 0x1 (USB)
  report [0] = 0x1;

  bool  bNewData = false;
  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, (DWORD)pDevice->input_report.size (), &dwBytesRead, nullptr))
  {
    SK_HID_DualSense_GetStateData *pData =
      (SK_HID_DualSense_GetStateData *)&report [1];

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

    if (pData->ButtonCreate   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
    if (pData->ButtonOptions  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

    if (pData->ButtonHome     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

    float LX    = pDevice->state.current.Gamepad.sThumbLX;
    float LY    = pDevice->state.current.Gamepad.sThumbLY;
    float normL = sqrtf ( LX*LX + LY*LY );

    float RX    = pDevice->state.current.Gamepad.sThumbRX;
    float RY    = pDevice->state.current.Gamepad.sThumbRY;
    float normR = sqrtf ( RX*RX + RY*RY );

    if ( pData->DeviceTimeStamp == 0 ||
         ! SK_XInput_UpdatePolledDataAndTimestamp (
                pDevice, (
                pDevice->state.current.Gamepad.wButtons     !=                                    0 ||
                pDevice->state.current.Gamepad.wButtons     != pDevice->state.prev.Gamepad.wButtons ||
                                                       normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE   ||
                                                       normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE  ||
                pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD     ||
                pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD
                         ), bNewData              )
       )
    {
      return false;
    }

#if 0
    if (pDevice->state.current.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE)
    {
      static constexpr DWORD dwRepeatRate = 200UL;

      if (pDevice->state.current.Gamepad.sThumbLY > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*3) && abs (pDevice->state.current.Gamepad.sThumbLX) < (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*2))
      {
        static DWORD dwLastActivation = 0;

        if (dwLastActivation < timeGetTime () - dwRepeatRate)
        {   dwLastActivation = timeGetTime ();
          BYTE bScancode =
            (BYTE)MapVirtualKey (VK_VOLUME_UP, 0);

          DWORD dwFlags =
            ( bScancode & 0xE0 ) == 0   ?
              static_cast <DWORD> (0x0) :
              static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);

          keybd_event (VK_VOLUME_UP, bScancode, dwFlags,                   0);
          keybd_event (VK_VOLUME_UP, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
        }
      }

      if (pDevice->state.current.Gamepad.sThumbLY < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*3) && abs (pDevice->state.current.Gamepad.sThumbLX) < (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*2))
      {
        static DWORD dwLastActivation = 0;

        if (dwLastActivation < timeGetTime () - dwRepeatRate)
        {   dwLastActivation = timeGetTime ();
          BYTE bScancode =
            (BYTE)MapVirtualKey (VK_VOLUME_DOWN, 0);

          DWORD dwFlags =
            ( bScancode & 0xE0 ) == 0   ?
              static_cast <DWORD> (0x0) :
              static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);

          keybd_event (VK_VOLUME_DOWN, bScancode, dwFlags,                   0);
          keybd_event (VK_VOLUME_DOWN, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
        }
      }

      if (pDevice->state.current.Gamepad.sThumbLX > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*3) && abs (pDevice->state.current.Gamepad.sThumbLY) < (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*2))
      {
        static DWORD dwLastActivation = 0;

        if (dwLastActivation < timeGetTime () - dwRepeatRate)
        {   dwLastActivation = timeGetTime ();
          BYTE bScancode =
            (BYTE)MapVirtualKey (VK_MEDIA_NEXT_TRACK, 0);

          DWORD dwFlags =
            ( bScancode & 0xE0 ) == 0   ?
              static_cast <DWORD> (0x0) :
              static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);

          keybd_event (VK_MEDIA_NEXT_TRACK, bScancode, dwFlags,                   0);
          keybd_event (VK_MEDIA_NEXT_TRACK, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
        }
      }

      if (pDevice->state.current.Gamepad.sThumbLX < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*3) && abs (pDevice->state.current.Gamepad.sThumbLY) < (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE*2))
      {
        static DWORD dwLastActivation = 0;

        if (dwLastActivation < timeGetTime () - dwRepeatRate)
        {   dwLastActivation = timeGetTime ();
          BYTE bScancode =
            (BYTE)MapVirtualKey (VK_MEDIA_PREV_TRACK, 0);

          DWORD dwFlags =
            ( bScancode & 0xE0 ) == 0   ?
              static_cast <DWORD> (0x0) :
              static_cast <DWORD> (KEYEVENTF_EXTENDEDKEY);

          keybd_event (VK_MEDIA_PREV_TRACK, bScancode, dwFlags,                   0);
          keybd_event (VK_MEDIA_PREV_TRACK, bScancode, dwFlags | KEYEVENTF_KEYUP, 0);
        }
      }
    }
#endif

    //if (pDevice->buttons.size () >= 14)
    //  pDevice->buttons [13].state = pData->ButtonPad    != 0;
    //if (pDevice->buttons.size () >= 15)
    //{
    //  pDevice->buttons [14].state = pData->ButtonMute   != 0;
    //  pDevice->buttons [15].state = pData->ButtonLeftFunction  != 0;
    //  pDevice->buttons [16].state = pData->ButtonRightFunction != 0;
    //  pDevice->buttons [17].state = pData->ButtonLeftPaddle    != 0;
    //  pDevice->buttons [18].state = pData->ButtonRightPaddle   != 0;
    //}

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
          L"HID Input Error (ReadFile - DualSense [USB])",
            MB_OK
      );
  }
  
  return false;
}

bool
SK_DualSense_GetInputReportBt (void *pGenericDev)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)pGenericDev;

  ZeroMemory ( pDevice->input_report.data (),
               pDevice->input_report.size () );

  BYTE* report = pDevice->input_report.data ();

  // HID Input Report 0x31 (Bluetooth)
  report [0] = 0x31;

  DWORD dwBytesRead = 0;

  if (ReadFile (pDevice->hDeviceFile, report, (DWORD)pDevice->input_report.size (), &dwBytesRead, nullptr))
  {
    pDevice->state.current.Gamepad = { };

    BYTE* pInputRaw =
      (BYTE *)&report [0];

    SK_HID_DualSense_GetStateData *pData =
      (SK_HID_DualSense_GetStateData *)&pInputRaw [2];

    bool bSimple  = false;
    bool bNewData = false;

    // We're in simplified mode...
    if (pInputRaw [0] == 0x1)
    {
      bSimple = true;
    }
    
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

      if (pData->ButtonCreate   != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
      if (pData->ButtonOptions  != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_START;

      if (pData->ButtonHome     != 0) pDevice->state.current.Gamepad.wButtons |= XINPUT_GAMEPAD_GUIDE;

      float LX    = pDevice->state.current.Gamepad.sThumbLX;
      float LY    = pDevice->state.current.Gamepad.sThumbLY;
      float normL = sqrtf ( LX*LX + LY*LY );

      float RX    = pDevice->state.current.Gamepad.sThumbRX;
      float RY    = pDevice->state.current.Gamepad.sThumbRY;
      float normR = sqrtf ( RX*RX + RY*RY );

      if ( pData->DeviceTimeStamp == 0 ||
           ! SK_XInput_UpdatePolledDataAndTimestamp (
                  pDevice, (
                  pDevice->state.current.Gamepad.wButtons     !=                                    0 ||
                  pDevice->state.current.Gamepad.wButtons     != pDevice->state.prev.Gamepad.wButtons ||
                                                         normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  * 2 ||
                                                         normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 2 ||
                  pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2 ||
                  pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD    * 2
                           ), bNewData              )
         )
      {
        return false;
      }

#if 0
      // Common to DualSense and DualShock4, but no representation in XInput
      pData->ButtonPad           != 0;

      // New DualSense button
      pData->ButtonMute          != 0;

      // New DualSense Edge buttons
      pData->ButtonLeftFunction  != 0;
      pData->ButtonRightFunction != 0;
      pData->ButtonLeftPaddle    != 0;
      pData->ButtonRightPaddle   != 0;
#endif
    }

    else
    {
      SK_HID_DualSense_GetSimpleStateDataBt *pSimpleData =
        (SK_HID_DualSense_GetSimpleStateDataBt *)&pInputRaw [1];

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
                                                            normL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE   ||
                                                            normR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE  ||
                     pDevice->state.current.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD     ||
                     pDevice->state.current.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD
                              ), bNewData          )
         )
      {
        return false;
      }
    }

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
          L"HID Input Error (ReadFile - DualSense [Bt])",
            MB_OK
      );
  }
  
  return false;
}