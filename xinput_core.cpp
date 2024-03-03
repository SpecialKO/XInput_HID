#include "pch.h"

#pragma once
#include <windef.h>

bool _GetInputReportStub (void* pDevice) { return false; }

concurrency::concurrent_vector <hid_device_file_s> hid_devices;

extern HMODULE hModRealXInput14;

XInputGetState_pfn                  _XInputGetState                  = nullptr;
XInputGetStateEx_pfn                _XInputGetStateEx                = nullptr;
XInputSetState_pfn                  _XInputSetState                  = nullptr;
XInputGetBatteryInformation_pfn     _XInputGetBatteryInformation     = nullptr;
XInputGetCapabilities_pfn           _XInputGetCapabilities           = nullptr;
XInputGetCapabilitiesEx_pfn         _XInputGetCapabilitiesEx         = nullptr;
XInputPowerOff_pfn                  _XInputPowerOff                  = nullptr;
XInputEnable_pfn                    _XInputEnable                    = nullptr;
XInputGetKeystroke_pfn              _XInputGetKeystroke              = nullptr;
XInputGetAudioDeviceIds_pfn         _XInputGetAudioDeviceIds         = nullptr;
XInputGetDSoundAudioDeviceGuids_pfn _XInputGetDSoundAudioDeviceGuids = nullptr;

void SK_HID_SetupPlayStationControllers (void)
{
  HDEVINFO hid_device_set = 
    SetupDiGetClassDevsW (&GUID_DEVINTERFACE_HID, nullptr, nullptr, DIGCF_DEVICEINTERFACE |
                                                                    DIGCF_PRESENT);
  
  if (hid_device_set != INVALID_HANDLE_VALUE)
  {
    SP_DEVINFO_DATA devInfoData = {
      .cbSize = sizeof (SP_DEVINFO_DATA)
    };
  
    SP_DEVICE_INTERFACE_DATA devInterfaceData = {
      .cbSize = sizeof (SP_DEVICE_INTERFACE_DATA)
    };
  
    for (                                  DWORD dwDevIdx = 0            ;
          SetupDiEnumDeviceInfo (hid_device_set, dwDevIdx, &devInfoData) ;
                                               ++dwDevIdx )
    {
      devInfoData.cbSize      = sizeof (SP_DEVINFO_DATA);
      devInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
  
      if (! SetupDiEnumDeviceInterfaces ( hid_device_set, nullptr, &GUID_DEVINTERFACE_HID,
                                                dwDevIdx, &devInterfaceData) )
      {
        continue;
      }
  
      static wchar_t devInterfaceDetailData [MAX_PATH + 2];
  
      ULONG ulMinimumSize = 0;
  
      SetupDiGetDeviceInterfaceDetailW (
        hid_device_set, &devInterfaceData, nullptr,
          0, &ulMinimumSize, nullptr );
  
      if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
        continue;
  
      if (ulMinimumSize > sizeof (wchar_t) * (MAX_PATH + 2))
        continue;
  
      SP_DEVICE_INTERFACE_DETAIL_DATA *pDevInterfaceDetailData =
        (SP_DEVICE_INTERFACE_DETAIL_DATA *)devInterfaceDetailData;
  
      pDevInterfaceDetailData->cbSize =
        sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
  
      if ( SetupDiGetDeviceInterfaceDetailW (
             hid_device_set, &devInterfaceData, pDevInterfaceDetailData,
               ulMinimumSize, &ulMinimumSize, nullptr ) )
      {
        wchar_t *wszFileName =
          pDevInterfaceDetailData->DevicePath;

        bool bSkip = false;

        for ( auto& existing : hid_devices )
        {
          if (StrStrIW (existing.wszDevicePath, wszFileName))
          {
            bSkip = true;
            break;
          }
        }

        HANDLE hDeviceFile (
               CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                          FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                            nullptr, OPEN_EXISTING, /*FILE_FLAG_WRITE_THROUGH  |
                                                                    FILE_ATTRIBUTE_TEMPORARY |
                                                                    FILE_FLAG_OVERLAPPED*/0x0, nullptr )
                           );

        if (hDeviceFile == INVALID_HANDLE_VALUE)
        {
          continue;
        }

        HIDD_ATTRIBUTES hidAttribs      = {                      };
                        hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

        HidD_GetAttributes (hDeviceFile, &hidAttribs);
  
        bool bSONY = 
          hidAttribs.VendorID == 0x54c;

        if (bSONY)
        {
          hid_device_file_s controller;

          controller.devinfo.pid = hidAttribs.ProductID;
          controller.devinfo.vid = hidAttribs.VendorID;

          controller.bWireless =
            StrStrIW (
              wszFileName, //Bluetooth_Base_UUID
                           L"{00001124-0000-1000-8000-00805f9b34fb}"
            );

          switch (controller.devinfo.pid)
          {
            // DualSense
            case 0x0DF2:
            case 0x0CE6:
              extern bool SK_DualSense_GetInputReport (void *pGenericDev);
              controller.get_input_report = SK_DualSense_GetInputReport;
              break;

            // DualShock 4
            case 0x05C4:
            case 0x09CC:
            case 0x0BA0:
              break;

            // DualShock 3
            case 0x0268:
              break;

            default:
              CloseHandle (hDeviceFile);
              continue;
              break;
          }
  
          wcsncpy_s (controller.wszDevicePath, MAX_PATH,
                                wszFileName,   _TRUNCATE);

          controller.hDeviceFile =
                     hDeviceFile;
  
          if (controller.hDeviceFile != INVALID_HANDLE_VALUE)
          {
            controller.bConnected = true;
  
            hid_devices.push_back (controller);
          }
        }
      }
    }
  
    SetupDiDestroyDeviceInfoList (hid_device_set);
  }
}



DWORD
WINAPI
XInput_HID_InitThread (LPVOID)
{
  if (hModRealXInput14 != nullptr)
    return 0;

  HMODULE hMod =
    LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS );

  if (hMod != nullptr)
  {
    _XInputGetState                  = (XInputGetState_pfn)                 GetProcAddress (hMod, "XInputGetState");
    _XInputGetAudioDeviceIds         = (XInputGetAudioDeviceIds_pfn)        GetProcAddress (hMod, "XInputGetAudioDeviceIds");
    _XInputGetDSoundAudioDeviceGuids = (XInputGetDSoundAudioDeviceGuids_pfn)GetProcAddress (hMod, "XInputGetDSoundAudioDeviceGuids");
    _XInputSetState                  = (XInputSetState_pfn)                 GetProcAddress (hMod, "XInputSetState");
    _XInputGetBatteryInformation     = (XInputGetBatteryInformation_pfn)    GetProcAddress (hMod, "XInputGetBatteryInformation");
    _XInputGetCapabilities           = (XInputGetCapabilities_pfn)          GetProcAddress (hMod, "XInputGetCapabilities");
    _XInputEnable                    = (XInputEnable_pfn)                   GetProcAddress (hMod, "XInputEnable");
    _XInputGetKeystroke              = (XInputGetKeystroke_pfn)             GetProcAddress (hMod, "XInputGetKeystroke");

    _XInputGetStateEx                = (XInputGetStateEx_pfn)               GetProcAddress (hMod, XINPUT_GETSTATEEX_ORDINAL);
    _XInputGetCapabilitiesEx         = (XInputGetCapabilitiesEx_pfn)        GetProcAddress (hMod, XINPUT_GETCAPABILITIES_EX_ORDINAL);
    _XInputPowerOff                  = (XInputPowerOff_pfn)                 GetProcAddress (hMod, XINPUT_POWEROFF_ORDINAL);

    SK_HID_SetupPlayStationControllers ();
  }

  else
    hMod = (HMODULE)-1;

  hModRealXInput14 = hMod;

  return 0;
}


DWORD
WINAPI
XInputGetState (DWORD dwUserIndex, XINPUT_STATE *pState)
{
  XInput_HID_InitThread (nullptr);

  while (hModRealXInput14 == nullptr)
    ;

  DWORD dwState;

  dwState =
    _XInputGetState (dwUserIndex, pState);

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    if (hid_devices.empty ())
    {
      SK_HID_SetupPlayStationControllers ();
    }

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));

          return ERROR_SUCCESS;
        }

        //return ERROR_SUCCESS;
      }
    }
  }

  return dwState;
}

DWORD
WINAPI
XInputGetStateEx (DWORD dwUserIndex, XINPUT_STATE_EX *pState)
{
  XInput_HID_InitThread (nullptr);

  while (hModRealXInput14 == nullptr)
    ;

  DWORD dwState =
    _XInputGetStateEx (dwUserIndex, pState);

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));

          return ERROR_SUCCESS;
        }

        return ERROR_SUCCESS;
      }
    }
  }

  return dwState;
}

DWORD
WINAPI
XInputSetState (
  _In_    DWORD             dwUserIndex,
  _Inout_ XINPUT_VIBRATION *pVibration )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputSetState (dwUserIndex, pVibration);
}

DWORD
WINAPI
XInputGetCapabilities (
  _In_  DWORD                dwUserIndex,
  _In_  DWORD                dwFlags,
  _Out_ XINPUT_CAPABILITIES *pCapabilities )
{
  XInput_HID_InitThread (nullptr);

  while (hModRealXInput14 == nullptr)
    ;

  DWORD dwResult =
    _XInputGetCapabilities (dwUserIndex, dwFlags, pCapabilities);

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    if (hid_devices.empty ())
    {
      SK_HID_SetupPlayStationControllers ();
    }

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          //memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));
          return ERROR_SUCCESS;
        }

        return ERROR_SUCCESS;
      }
    }
  }

  return dwResult;
}

DWORD
WINAPI
XInputGetCapabilitiesEx (
  _In_  DWORD                   dwReserved,
  _In_  DWORD                   dwUserIndex,
  _In_  DWORD                   dwFlags,
  _Out_ XINPUT_CAPABILITIES_EX *pCapabilitiesEx )
{
  XInput_HID_InitThread (nullptr);

  while (hModRealXInput14 == nullptr)
    ;

  DWORD dwResult =
    _XInputGetCapabilitiesEx (dwReserved, dwUserIndex, dwFlags, pCapabilitiesEx);

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    if (hid_devices.empty ())
    {
      SK_HID_SetupPlayStationControllers ();
    }

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          //memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));
          return ERROR_SUCCESS;
        }

        return ERROR_SUCCESS;
      }
    }
  }

  return dwResult;
}

DWORD
WINAPI
XInputGetBatteryInformation (
  _In_  DWORD                       dwUserIndex,
  _In_  BYTE                        devType,
  _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputGetBatteryInformation (dwUserIndex, devType, pBatteryInformation);
}

DWORD
WINAPI
XInputGetKeystroke (
  DWORD             dwUserIndex,
  DWORD             dwReserved,
  PXINPUT_KEYSTROKE pKeystroke )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputGetKeystroke (dwUserIndex, dwReserved, pKeystroke);
}

DWORD
WINAPI
XInputGetAudioDeviceIds (
  DWORD  dwUserIndex,
  LPWSTR pRenderDeviceId,
  UINT   *pRenderCount,
  LPWSTR pCaptureDeviceId,
  UINT   *pCaptureCount )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputGetAudioDeviceIds (dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount);
}

DWORD
WINAPI
XInputGetDSoundAudioDeviceGuids (
  DWORD dwUserIndex,
  GUID  *pDSoundRenderGuid,
  GUID  *pDSoundCaptureGuid )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputGetDSoundAudioDeviceGuids (dwUserIndex, pDSoundRenderGuid, pDSoundCaptureGuid);
}

DWORD
WINAPI
XInputPowerOff (
  _In_ DWORD dwUserIndex )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputPowerOff (dwUserIndex);
}

void
WINAPI
XInputEnable (
  _In_ BOOL enable )
{
  while (hModRealXInput14 == nullptr)
    ;

  return
    _XInputEnable (enable);
}















#if 0
extern HANDLE __SK_DLL_TeardownEvent;
HWND            SK_hWndDeviceListener;

void
SK_XInput_NotifyDeviceArrival (void)
{
  SK_RunOnce (
  {
    //SK_XInputHot_NotifyEvent =
    //          SK_CreateEvent (nullptr, TRUE, TRUE, nullptr);

    //SK_XInputHot_ReconnectThread =
    //SK_Thread_CreateEx ([](LPVOID user)->
      CreateThread (nullptr, 0x0,
      [](LPVOID user)->
      DWORD
      {
        HANDLE hNotify =
          (HANDLE)user;

        HANDLE phWaitObjects [2] = {
          hNotify, __SK_DLL_TeardownEvent
        };

        static constexpr DWORD ArrivalEvent  = ( WAIT_OBJECT_0     );
        static constexpr DWORD ShutdownEvent = ( WAIT_OBJECT_0 + 1 );

        auto SK_HID_DeviceNotifyProc =
      [] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
      -> LRESULT
        {
          switch (message)
          {
            case WM_DEVICECHANGE:
            {
              switch (wParam)
              {
                case DBT_DEVICEARRIVAL:
                case DBT_DEVICEREMOVECOMPLETE:
                {
                  DEV_BROADCAST_HDR* pDevHdr =
                    (DEV_BROADCAST_HDR *)lParam;

                  if (pDevHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                  {
                    const bool arrival =
                      (wParam == DBT_DEVICEARRIVAL);

                    SK_ReleaseAssert (
                      pDevHdr->dbch_size >= sizeof (DEV_BROADCAST_DEVICEINTERFACE_W)
                    );

                    DEV_BROADCAST_DEVICEINTERFACE_W *pDev =
                      (DEV_BROADCAST_DEVICEINTERFACE_W *)pDevHdr;

                    if (IsEqualGUID (pDev->dbcc_classguid, GUID_DEVINTERFACE_HID) ||
                        IsEqualGUID (pDev->dbcc_classguid, GUID_XUSB_INTERFACE_CLASS))
                    {
                      SK_LOG0 ( ( L" Device %s:\t%s",  arrival ? L"Arrival"
                                                               : L"Removal",
                                                       pDev->dbcc_name ),
                                  __SK_SUBSYSTEM__ );

                      bool playstation = false;
                      bool xinput      = IsEqualGUID (pDev->dbcc_classguid, GUID_XUSB_INTERFACE_CLASS);

                      wchar_t    wszFileName [MAX_PATH];
                      wcsncpy_s (wszFileName, MAX_PATH, pDev->dbcc_name, _TRUNCATE);

                      SK_AutoHandle hDeviceFile (
                          SK_CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                                        FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                                          nullptr, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH  |
                                                                                  FILE_ATTRIBUTE_TEMPORARY |
                                                                                  FILE_FLAG_OVERLAPPED, nullptr )
                                                );
                      
                      HIDD_ATTRIBUTES hidAttribs      = {                      };
                                      hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

                      if (hDeviceFile.isValid ())
                      {
                        HidD_GetAttributes (hDeviceFile.m_h, &hidAttribs);

                        playstation |= ( hidAttribs.VendorID == SK_HID_VID_SONY );

                        hDeviceFile.Close ();
                      }

                      else
                      {
                        // On device removal, all we can do is go by the filename...
                        playstation |= wcsstr (wszFileName, L"054c") != nullptr;
                      }

                      xinput |= wcsstr (wszFileName, L"IG_") != nullptr;

                      if (arrival)
                      {
                        for (  auto event : SK_HID_DeviceArrivalEvents  )
                          SetEvent (event);

                        // XInput devices contain IG_...
                        if ( xinput &&
                             wcsstr (pDev->dbcc_name, LR"(\kbd)") == nullptr )
                             // Ignore XInputGetKeystroke
                        {
                          XINPUT_CAPABILITIES caps = { };

                          // Determine all connected XInput controllers and only
                          //   refresh those that need it...
                          for ( int i = 0 ; i < XUSER_MAX_COUNT ; ++i )
                          {
                            if ( ERROR_SUCCESS ==
                                   SK_XInput_GetCapabilities (i, XINPUT_DEVTYPE_GAMEPAD, &caps) )
                            {
                              SK_XInput_Refresh        (i);
                              SK_XInput_PollController (i);

                              if ((intptr_t)SK_XInputCold_DecommisionEvent > 0)
                                  SetEvent (SK_XInputCold_DecommisionEvent);
                            }
                          }
                        }

                        else if (playstation)
                        {
                          bool has_existing = false;

                          for ( auto& controller : SK_HID_PlayStationControllers )
                          {
                            if (! _wcsicmp (controller.wszDevicePath, wszFileName))
                            {
                              // We missed a device removal event if this is true
                              SK_ReleaseAssert (controller.bConnected == false);

                              controller.hDeviceFile =
                                SK_CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                                              FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                                                nullptr, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH  |
                                                                                        FILE_ATTRIBUTE_TEMPORARY |
                                                                                        FILE_FLAG_OVERLAPPED, nullptr );

                              if (controller.hDeviceFile != INVALID_HANDLE_VALUE)
                              {
                                if (HidD_GetPreparsedData (controller.hDeviceFile, &controller.pPreparsedData))
                                {
                                  controller.bConnected = true;
                                  controller.bBluetooth =  //Bluetooth_Base_UUID
                                    StrStrIW (wszFileName, L"{00001124-0000-1000-8000-00805f9b34fb}");

                                  has_existing = true;

                                  if (        controller.hReconnectEvent != nullptr)
                                    SetEvent (controller.hReconnectEvent);
                                }
                              }
                              break;
                            }
                          }

                          if (! has_existing)
                          {
                            SK_HID_PlayStationDevice controller;

                            controller.pid = hidAttribs.ProductID;
                            controller.vid = hidAttribs.VendorID;

                            controller.bBluetooth =
                              StrStrIW (
                                controller.wszDevicePath, //Bluetooth_Base_UUID
                                        L"{00001124-0000-1000-8000-00805f9b34fb}"
                              );

                            controller.bDualSense =
                              (controller.pid == 0x0DF2) ||
                              (controller.pid == 0x0CE6);

                            controller.bDualShock4 =
                              (controller.pid == 0x05c4) ||
                              (controller.pid == 0x09CC) ||
                              (controller.pid == 0x0BA0);

                            controller.bDualShock3 =
                              (controller.pid == 0x0268);

                            if (! (controller.bDualSense || controller.bDualShock4 || controller.bDualShock3))
                            {
                              if (controller.vid == SK_HID_VID_SONY)
                              {
                                //SK_LOGi0 (L"SONY Controller with Unknown PID ignored: %ws", wszFileName);
                              }

                              return
                                DefWindowProcW (hwnd, message, wParam, lParam);
                            }

                            wcsncpy_s (controller.wszDevicePath, MAX_PATH,
                                                  wszFileName,   _TRUNCATE);

                            controller.hDeviceFile =
                              SK_CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                                            FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                                              nullptr, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH  |
                                                                                      FILE_ATTRIBUTE_TEMPORARY |
                                                                                      FILE_FLAG_OVERLAPPED, nullptr );

                            if (controller.hDeviceFile != INVALID_HANDLE_VALUE)
                            {
                              controller.bConnected = true;

                              if (SK_HidD_GetPreparsedData (controller.hDeviceFile, &controller.pPreparsedData))
                              {
                                HIDP_CAPS                                      caps = { };
                                  SK_HidP_GetCaps (controller.pPreparsedData, &caps);

                                controller.input_report.resize  (caps.InputReportByteLength);
                                controller.output_report.resize (caps.OutputReportByteLength);

                                std::vector <HIDP_BUTTON_CAPS>
                                  buttonCapsArray;
                                  buttonCapsArray.resize (caps.NumberInputButtonCaps);

                                std::vector <HIDP_VALUE_CAPS>
                                  valueCapsArray;
                                  valueCapsArray.resize (caps.NumberInputValueCaps);

                                USHORT num_caps =
                                  caps.NumberInputButtonCaps;

                                if (num_caps > 2)
                                {
                                  SK_LOGi0 (
                                    L"PlayStation Controller has too many button sets (%d);"
                                    L" will ignore Device=%ws", num_caps, wszFileName
                                  );

                                  return
                                    DefWindowProcW (hwnd, message, wParam, lParam);
                                }

                                if ( HIDP_STATUS_SUCCESS ==
                                  SK_HidP_GetButtonCaps ( HidP_Input,
                                                            buttonCapsArray.data (), &num_caps,
                                                              controller.pPreparsedData ) )
                                {
                                  for (UINT i = 0 ; i < num_caps ; ++i)
                                  {
                                    // Face Buttons
                                    if (buttonCapsArray [i].IsRange)
                                    {
                                      controller.button_report_id =
                                        buttonCapsArray [i].ReportID;
                                      controller.button_usage_min =
                                        buttonCapsArray [i].Range.UsageMin;
                                      controller.button_usage_max =
                                        buttonCapsArray [i].Range.UsageMax;

                                      controller.buttons.resize (
                                        static_cast <size_t> (
                                          controller.button_usage_max -
                                          controller.button_usage_min + 1
                                        )
                                      );
                                    }
                                  }

                                  USHORT value_caps_count =
                                    sk::narrow_cast <USHORT> (valueCapsArray.size ());

                                  if ( HIDP_STATUS_SUCCESS ==
                                         SK_HidP_GetValueCaps ( HidP_Input, valueCapsArray.data (),
                                                                           &value_caps_count,
                                                                            controller.pPreparsedData ) )
                                  {
                                    controller.value_caps.resize (value_caps_count);

                                    for ( int idx = 0; idx < value_caps_count; ++idx )
                                    {
                                      controller.value_caps [idx] = valueCapsArray [idx];
                                    }
                                  }

                                  // We need a contiguous array to read-back the set buttons,
                                  //   rather than allocating it dynamically, do it once and reuse.
                                  controller.button_usages.resize (controller.buttons.size ());

                                  USAGE idx = 0;

                                  for ( auto& button : controller.buttons )
                                  {
                                    button.UsagePage = buttonCapsArray [0].UsagePage;
                                    button.Usage     = controller.button_usage_min + idx++;
                                    button.state     = false;
                                  }
                                }
                              }

                              controller.bConnected = true;

                              SK_HID_PlayStationControllers.push_back (controller);

                              //SK_ImGui_Warning (L"PlayStation Controller Connected");
                            }
                          }
                        }
                      }

                      else
                      {
                        for (  auto event : SK_HID_DeviceRemovalEvents  )
                          SetEvent (event);

                        if ( xinput &&
                             wcsstr (pDev->dbcc_name, LR"(\kbd)") == nullptr )
                             // Ignore XInputGetKeystroke
                        {
                          // We really have no idea what controller this is, so refresh them all
                          SetEvent (SK_XInputHot_NotifyEvent);

                          if ((intptr_t)SK_XInputCold_DecommisionEvent > 0)
                              SetEvent (SK_XInputCold_DecommisionEvent);
                        }

                        else// if (playstation)
                        {
                          for ( auto& controller : SK_HID_PlayStationControllers )
                          {
                            if (! _wcsicmp (controller.wszDevicePath, wszFileName))
                            {
                              controller.bConnected = false;
                              controller.reset_rgb  = false;

/*
                              SK_ImGui_CreateNotification (
                                "HID.GamepadReetached", SK_ImGui_Toast::Info,
                                SK_FormatString (
                                  "%ws" ICON_FA_GAMEPAD "\tHas Left the Building!",
                                   controller.wszDevicePath ).c_str (),
                                        "HID Compliant Gamepad is MIssing In Action!",
                                                  16666, SK_ImGui_Toast::UseDuration |
                                                         SK_ImGui_Toast::ShowCaption |
                                                         SK_ImGui_Toast::ShowTitle   |
                                                         SK_ImGui_Toast::ShowNewest
                              );
*/

                              if (                (intptr_t)controller.hDeviceFile > 0)
                                CloseHandle (std::exchange (controller.hDeviceFile,  nullptr));

                              if (controller.pPreparsedData != nullptr)
                                  SK_HidD_FreePreparsedData (
                                    std::exchange (controller.pPreparsedData, nullptr)
                                  );

                              if (        controller.hDisconnectEvent != nullptr)
                                SetEvent (controller.hDisconnectEvent);

                              //SK_ImGui_Warning (L"PlayStation Controller Disconnected");

                              break;
                            }
                          }
                        }
                      }
                    }
                  }
                } break;

                default: // Don't care
                  break;
              }

              return 0;
            } break;

            default: // Don't care
              break;
          };

          return
            DefWindowProcW (hwnd, message, wParam, lParam);
        };

        WNDCLASSEXW
          wnd_class               = {                       };
          wnd_class.hInstance     = GetModuleHandle (nullptr);
          wnd_class.lpszClassName = L"SK_HID_Listener";
          wnd_class.lpfnWndProc   = SK_HID_DeviceNotifyProc;
          wnd_class.cbSize        = sizeof (WNDCLASSEXW);

        if (RegisterClassEx (&wnd_class))
        {
          DWORD dwWaitStatus = WAIT_OBJECT_0;

          SK_hWndDeviceListener =
            (HWND)CreateWindowEx ( 0, L"SK_HID_Listener",    NULL, 0,
                                   0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );

          // It's technically unnecessary to register this, but not a bad idea
          HDEVNOTIFY hDevNotify =
            SK_RegisterDeviceNotification (SK_hWndDeviceListener);

          do
          {
            auto MessagePump = [&] (void) ->
            void
            {
              MSG                      msg = { };
              while (SK_PeekMessageW (&msg, SK_hWndDeviceListener, 0, 0, PM_REMOVE | PM_NOYIELD) > 0)
              {
                SK_TranslateMessage (&msg);
                SK_DispatchMessageW (&msg);
              }
            };

            dwWaitStatus =
              MsgWaitForMultipleObjects (2, phWaitObjects, FALSE, INFINITE, QS_ALLINPUT);

            // Event is created in signaled state to queue a refresh in case of
            //   late inject
            if (dwWaitStatus == ArrivalEvent)
            {
              static ULONGLONG ullLastFrame = 0;

              // Do at most once per-frame, then pick up residuals next frame
              if ( std::exchange (ullLastFrame, SK_GetFramesDrawn ()) <
                                                SK_GetFramesDrawn () )
              {
                SK_XInput_RefreshControllers (                        );
                ResetEvent                   (SK_XInputHot_NotifyEvent);
              }

              else
              {
                dwWaitStatus =
                  MsgWaitForMultipleObjects (0, nullptr, FALSE, 3UL, QS_ALLINPUT);

                if (std::exchange (dwWaitStatus, WAIT_OBJECT_0 + 2) !=
                                                 WAIT_OBJECT_0)
                                   dwWaitStatus  = ArrivalEvent;
              }
            }

            if (dwWaitStatus == (WAIT_OBJECT_0 + 2))
            {
              MessagePump ();
            }
          } while (dwWaitStatus != ShutdownEvent);

          UnregisterDeviceNotification (hDevNotify);
          DestroyWindow                (SK_hWndDeviceListener);
          UnregisterClassW             (wnd_class.lpszClassName, wnd_class.hInstance);
        }

        else if (config.system.log_level > 0)
          SK_ReleaseAssert (! L"Failed to register Window Class!");

        CloseHandle (SK_XInputHot_NotifyEvent);
                     SK_XInputHot_NotifyEvent = 0;

        SK_Thread_CloseSelf ();

        return 0;
      }, nullptr, 0x0, nullptr)
    );
  });
}
#endif