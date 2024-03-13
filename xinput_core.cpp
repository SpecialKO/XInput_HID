#include "pch.h"

#pragma once
#include <windef.h>
#include <Dbt.h>
#include <atlbase.h>
#include <mmsystem.h>

#pragma comment (lib, "winmm.lib")

bool _GetInputReportStub (void*) { return false; }

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

BOOL SK_XInput_SlotStatus [4] =
 { TRUE, TRUE, TRUE, TRUE };

void SK_HID_SetupCompatibleControllers (void);

void
SK_XInput_NotifyDeviceArrival (void)
{
  static bool s_Once = false;
  static HWND hWndDeviceListener = 0;

  if (std::exchange (s_Once, true))
    return;

  CreateThread (nullptr, 0x0, [](LPVOID/*user*/)->
  DWORD
  {
    //HANDLE hNotify =
    //  (HANDLE)user;

    HANDLE* phWaitObjects = nullptr;//[] = {
      //hNotify, __SK_DLL_TeardownEvent
    //};

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

                DEV_BROADCAST_DEVICEINTERFACE_W *pDev =
                  (DEV_BROADCAST_DEVICEINTERFACE_W *)pDevHdr;

                if (IsEqualGUID (pDev->dbcc_classguid, GUID_DEVINTERFACE_HID))
                {
                  bool playstation = false;

                  wchar_t    wszFileName [MAX_PATH];
                  wcsncpy_s (wszFileName, MAX_PATH, pDev->dbcc_name, _TRUNCATE);

                  CHandle hDeviceFile (
                      CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                                 FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                                   nullptr, OPEN_EXISTING, 0x0, nullptr )
                                      );

                  HIDD_ATTRIBUTES hidAttribs      = {                      };
                                  hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

                  if ((intptr_t)hDeviceFile.m_h > 0)
                  {
                    HidD_GetAttributes (hDeviceFile.m_h, &hidAttribs);

                    playstation |= ( hidAttribs.VendorID == 0x54c );
                    playstation |= ( hidAttribs.VendorID == 0x57e );

                    hDeviceFile.Close ();
                  }

                  else
                  {
                    // On device removal, all we can do is go by the filename...
                    playstation |= wcsstr (wszFileName, L"054c") != nullptr;
                    playstation |= wcsstr (wszFileName, L"057e") != nullptr;
                  }

                  if (arrival)
                  {
                    SK_XInput_SlotStatus [0] = true;
                    SK_XInput_SlotStatus [1] = true;
                    SK_XInput_SlotStatus [2] = true;
                    SK_XInput_SlotStatus [3] = true;

                    if (playstation)
                    {
                      bool has_existing = false;

                      for ( auto& controller : hid_devices )
                      {
                        if (! _wcsicmp (controller.wszDevicePath, wszFileName))
                        {
                          if (controller.bConnected || (intptr_t)controller.hDeviceFile > 0)
                          {
                            controller.disconnect ();
                          }

                          if ((intptr_t)hDeviceFile.m_h > 0)
                                        hDeviceFile.Close ();

                          hDeviceFile.m_h =
                            CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                                       FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                                         nullptr, OPEN_EXISTING, 0x0, nullptr );

                          if ((intptr_t)hDeviceFile.m_h > 0)
                          {
                            //if (HidD_GetPreparsedData (controller.hDeviceFile, &controller.pPreparsedData))
                            {
                              controller.bWireless =   //Bluetooth_Base_UUID
                                StrStrIW (wszFileName, L"{00001124-0000-1000-8000-00805f9b34fb}");
                              controller.reconnect (hDeviceFile.Detach ());

                              has_existing = true;

                              //if (        controller.hReconnectEvent != nullptr)
                              //  SetEvent (controller.hReconnectEvent);
                            }
                          }
                          break;
                        }
                      }

                      if (! has_existing)
                      {
                        SK_HID_SetupCompatibleControllers ();

                        ////hid_device_file_s controller;
                        ////
                        ////controller.devinfo.pid = hidAttribs.ProductID;
                        ////controller.devinfo.vid = hidAttribs.VendorID;
                        ////
                        ////controller.bWireless =
                        ////  StrStrIW (
                        ////    controller.wszDevicePath, //Bluetooth_Base_UUID
                        ////            L"{00001124-0000-1000-8000-00805f9b34fb}"
                        ////  );
                        ////
                        ////////controller.bDualSense =
                        ////////  (controller.pid == 0x0DF2) ||
                        ////////  (controller.pid == 0x0CE6);
                        ////////
                        ////////controller.bDualShock4 =
                        ////////  (controller.pid == 0x05c4) ||
                        ////////  (controller.pid == 0x09CC) ||
                        ////////  (controller.pid == 0x0BA0);
                        ////////
                        ////////controller.bDualShock3 =
                        ////////  (controller.pid == 0x0268);
                        ////
                      //////if (! (controller.bDualSense || controller.bDualShock4 || controller.bDualShock3))
                      //////{
                      //////  if (controller.vid == SK_HID_VID_SONY)
                      //////  {
                      //////    SK_LOGi0 (L"SONY Controller with Unknown PID ignored: %ws", wszFileName);
                      //////  }
                      //////
                      //////  return
                      //////    DefWindowProcW (hwnd, message, wParam, lParam);
                      //////}
                        ////
                        ////wcsncpy_s (controller.wszDevicePath, MAX_PATH,
                        ////                      wszFileName,   _TRUNCATE);
                        ////
                        ////controller.hDeviceFile =
                        ////  CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                        ////                             FILE_SHARE_READ   | FILE_SHARE_WRITE,
                        ////                               nullptr, OPEN_EXISTING, 0x0, nullptr );
                        ////
                        ////if (controller.hDeviceFile != INVALID_HANDLE_VALUE)
                        ////{
                        ////  controller.bConnected = true;
                        ////
                        ////////if (HidD_GetPreparsedData (controller.hDeviceFile, &controller.pPreparsedData))
                        //////{
                        //////  HIDP_CAPS                                   caps = { };
                        //////    HidP_GetCaps (controller.pPreparsedData, &caps);
                        //////
                        //////  controller.input_report.resize  (caps.InputReportByteLength);
                        //////  controller.output_report.resize (caps.OutputReportByteLength);
                        //////
                        //////  std::vector <HIDP_BUTTON_CAPS>
                        //////    buttonCapsArray;
                        //////    buttonCapsArray.resize (caps.NumberInputButtonCaps);
                        //////
                        //////  std::vector <HIDP_VALUE_CAPS>
                        //////    valueCapsArray;
                        //////    valueCapsArray.resize (caps.NumberInputValueCaps);
                        //////
                        //////  USHORT num_caps =
                        //////    caps.NumberInputButtonCaps;
                        //////
                        //////  if (num_caps > 2)
                        //////  {
                        //////    SK_LOGi0 (
                        //////      L"PlayStation Controller has too many button sets (%d);"
                        //////      L" will ignore Device=%ws", num_caps, wszFileName
                        //////    );
                        //////
                        //////    return
                        //////      DefWindowProcW (hwnd, message, wParam, lParam);
                        //////  }
                        //////
                        //////  if ( HIDP_STATUS_SUCCESS ==
                        //////    SK_HidP_GetButtonCaps ( HidP_Input,
                        //////                              buttonCapsArray.data (), &num_caps,
                        //////                                controller.pPreparsedData ) )
                        //////  {
                        //////    for (UINT i = 0 ; i < num_caps ; ++i)
                        //////    {
                        //////      // Face Buttons
                        //////      if (buttonCapsArray [i].IsRange)
                        //////      {
                        //////        controller.button_report_id =
                        //////          buttonCapsArray [i].ReportID;
                        //////        controller.button_usage_min =
                        //////          buttonCapsArray [i].Range.UsageMin;
                        //////        controller.button_usage_max =
                        //////          buttonCapsArray [i].Range.UsageMax;
                        //////
                        //////        controller.buttons.resize (
                        //////          static_cast <size_t> (
                        //////            controller.button_usage_max -
                        //////            controller.button_usage_min + 1
                        //////          )
                        //////        );
                        //////      }
                        //////    }
                        //////
                        //////    USHORT value_caps_count =
                        //////      sk::narrow_cast <USHORT> (valueCapsArray.size ());
                        //////
                        //////    if ( HIDP_STATUS_SUCCESS ==
                        //////           SK_HidP_GetValueCaps ( HidP_Input, valueCapsArray.data (),
                        //////                                             &value_caps_count,
                        //////                                              controller.pPreparsedData ) )
                        //////    {
                        //////      controller.value_caps.resize (value_caps_count);
                        //////
                        //////      for ( int idx = 0; idx < value_caps_count; ++idx )
                        //////      {
                        //////        controller.value_caps [idx] = valueCapsArray [idx];
                        //////      }
                        //////    }
                        //////
                        //////    // We need a contiguous array to read-back the set buttons,
                        //////    //   rather than allocating it dynamically, do it once and reuse.
                        //////    controller.button_usages.resize (controller.buttons.size ());
                        //////
                        //////    USAGE idx = 0;
                        //////
                        //////    for ( auto& button : controller.buttons )
                        //////    {
                        //////      button.UsagePage = buttonCapsArray [0].UsagePage;
                        //////      button.Usage     = controller.button_usage_min + idx++;
                        //////      button.state     = false;
                        //////    }
                        //////  }
                        //////}
                        ////
                        ////  controller.bConnected = true;
                        ////
                        ////  hid_devices.push_back (controller);
                        ////  //SK_HID_PlayStationControllers.push_back (controller);
                        ////
                        ////  //SK_ImGui_Warning (L"PlayStation Controller Connected");
                        ////}
                      }
                    }
                  }

                  else
                  {
                    //for (  auto event : SK_HID_DeviceRemovalEvents  )
                    //  SetEvent (event);

                    {
                      for ( auto& controller : hid_devices )
                      {
                        if (! _wcsicmp (controller.wszDevicePath, wszFileName))
                        {
                          controller.disconnect ();

                        //controller.reset_rgb  = false;

                          //if (controller.pPreparsedData != nullptr)
                          //    HidD_FreePreparsedData (
                          //      std::exchange (controller.pPreparsedData, nullptr)
                          //    );

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

      hWndDeviceListener =
        (HWND)CreateWindowEx ( 0, L"SK_HID_Listener",    NULL, 0,
                               0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );

      DEV_BROADCAST_DEVICEINTERFACE_W
      NotificationFilter                 = { };
      NotificationFilter.dbcc_size       = sizeof (DEV_BROADCAST_DEVICEINTERFACE_W);
      NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      NotificationFilter.dbcc_classguid  = GUID_DEVINTERFACE_HID;

      // It's technically unnecessary to register this, but not a bad idea
      HDEVNOTIFY hDevNotify =
        RegisterDeviceNotificationW (hWndDeviceListener, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);

      do
      {
        auto MessagePump = [&] (void) ->
        void
        {
          MSG                   msg = { };
          while (PeekMessageW (&msg, hWndDeviceListener, 0, 0, PM_REMOVE | PM_NOYIELD) > 0)
          {
            TranslateMessage (&msg);
            DispatchMessageW (&msg);
          }
        };

        dwWaitStatus =
          MsgWaitForMultipleObjects (0, phWaitObjects, FALSE, INFINITE, QS_ALLINPUT);

        if (dwWaitStatus == (WAIT_OBJECT_0))
        {
          MessagePump ();
        }
      } while (true);//dwWaitStatus != ShutdownEvent);

      UnregisterDeviceNotification (hDevNotify);
      DestroyWindow                (hWndDeviceListener);
      UnregisterClassW             (wnd_class.lpszClassName, wnd_class.hInstance);
    }

  //SK_CloseHandle (SK_XInputHot_NotifyEvent);
  //                SK_XInputHot_NotifyEvent = 0;

    return 0;
  }, (LPVOID)nullptr, 0x0, nullptr);//SK_XInputHot_NotifyEvent
}

void SK_HID_SetupCompatibleControllers (void)
{
  SK_XInput_NotifyDeviceArrival ();

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
          if (existing.bConnected && StrStrIW (existing.wszDevicePath, wszFileName))
          {
            bSkip = true;
            break;
          }
        }

        if (bSkip)
          continue;

        HANDLE hDeviceFile (
               CreateFileW ( wszFileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                          FILE_SHARE_READ   | FILE_SHARE_WRITE,
                                            nullptr, OPEN_EXISTING, /*FILE_FLAG_WRITE_THROUGH  |
                                                                    FILE_ATTRIBUTE_TEMPORARY |
                                                                    FILE_FLAG_OVERLAPPED*/0x0, nullptr )
                           );

        if ((intptr_t)hDeviceFile <= 0)
        {
          continue;
        }

        HIDD_ATTRIBUTES hidAttribs      = {                      };
                        hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

        HidD_GetAttributes (hDeviceFile, &hidAttribs);
  
        bool bSONY = 
          hidAttribs.VendorID == 0x54c;

        bool bNintendo =
          hidAttribs.VendorID == 0x57e;

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
              controller.get_input_report =
                controller.bWireless ? SK_DualSense_GetInputReportBt :
                                       SK_DualSense_GetInputReportUSB;
              break;

            // DualShock 4
            case 0x05C4:
            case 0x09CC:
            case 0x0BA0:
              controller.get_input_report =
                controller.bWireless ? SK_DualShock4_GetInputReportBt :
                                       SK_DualShock4_GetInputReportUSB;
              break;

            // DualShock 3
            case 0x0268:
              break;

            default:
              CloseHandle (std::exchange (hDeviceFile, INVALID_HANDLE_VALUE));
              continue;
              break;
          }
  
          wcsncpy_s (controller.wszDevicePath, MAX_PATH,
                                wszFileName,   _TRUNCATE);

          if ( (intptr_t)controller.hDeviceFile > 0 ||
               (intptr_t)hDeviceFile            > 0 )
          {
            auto iter =
              hid_devices.push_back (controller);

            iter->reconnect (hDeviceFile);
          }
        }

        if (bNintendo)
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
            // Switch Pro
            case 0x2009:
              controller.get_input_report =
                controller.bWireless ? SK_SwitchPro_GetInputReportBt :
                                       SK_SwitchPro_GetInputReportUSB;
              break;

            default:
              CloseHandle (hDeviceFile);
              continue;
              break;
          }
  
          wcsncpy_s (controller.wszDevicePath, MAX_PATH,
                                wszFileName,   _TRUNCATE);

          if ( (intptr_t)controller.hDeviceFile > 0 ||
               (intptr_t)hDeviceFile            > 0 )
          {
            auto iter =
              hid_devices.push_back (controller);

            iter->reconnect (hDeviceFile);
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

    SK_HID_SetupCompatibleControllers ();
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

  dwState = SK_XInput_SlotStatus [dwUserIndex] ?
    _XInputGetState (dwUserIndex, pState)      : ERROR_DEVICE_NOT_CONNECTED;

  if (SK_XInput_SlotStatus [dwUserIndex] && dwState == ERROR_DEVICE_NOT_CONNECTED)
      SK_XInput_SlotStatus [dwUserIndex] = false;

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    bool bSuccess = false;

    *pState = {};

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          pState->Gamepad.bLeftTrigger  = controller.state.current.Gamepad.bLeftTrigger;
          pState->Gamepad.bRightTrigger = controller.state.current.Gamepad.bRightTrigger;
          pState->Gamepad.sThumbLX      = controller.state.current.Gamepad.sThumbLX;
          pState->Gamepad.sThumbLY      = controller.state.current.Gamepad.sThumbLY;
          pState->Gamepad.sThumbRX      = controller.state.current.Gamepad.sThumbRX;
          pState->Gamepad.sThumbRY      = controller.state.current.Gamepad.sThumbRY;
          pState->Gamepad.wButtons      = controller.state.current.Gamepad.wButtons;
          pState->dwPacketNumber        = controller.state.current.dwPacketNumber;
        }

        bSuccess = true;
      }
    }

    if (bSuccess)
      return ERROR_SUCCESS;
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

  DWORD dwState = SK_XInput_SlotStatus [dwUserIndex] ?
    _XInputGetStateEx (dwUserIndex, pState)          : ERROR_DEVICE_NOT_CONNECTED;

  if (SK_XInput_SlotStatus [dwUserIndex] && dwState == ERROR_DEVICE_NOT_CONNECTED)
      SK_XInput_SlotStatus [dwUserIndex] = false;

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    bool bSuccess = false;

    *pState = {};

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          pState->Gamepad.bLeftTrigger  = controller.state.current.Gamepad.bLeftTrigger;
          pState->Gamepad.bRightTrigger = controller.state.current.Gamepad.bRightTrigger;
          pState->Gamepad.sThumbLX      = controller.state.current.Gamepad.sThumbLX;
          pState->Gamepad.sThumbLY      = controller.state.current.Gamepad.sThumbLY;
          pState->Gamepad.sThumbRX      = controller.state.current.Gamepad.sThumbRX;
          pState->Gamepad.sThumbRY      = controller.state.current.Gamepad.sThumbRY;
          pState->Gamepad.wButtons      = controller.state.current.Gamepad.wButtons;
          pState->dwPacketNumber        = controller.state.current.dwPacketNumber;
        }

        bSuccess = true;
      }
    }

    if (bSuccess)
      return ERROR_SUCCESS;
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

  DWORD dwResult = SK_XInput_SlotStatus [dwUserIndex]            ?
    _XInputGetCapabilities (dwUserIndex, dwFlags, pCapabilities) : ERROR_DEVICE_NOT_CONNECTED;

  if (SK_XInput_SlotStatus [dwUserIndex] && dwResult == ERROR_DEVICE_NOT_CONNECTED)
      SK_XInput_SlotStatus [dwUserIndex] = false;

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
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

  DWORD dwResult = SK_XInput_SlotStatus [dwUserIndex]                            ?
    _XInputGetCapabilitiesEx (dwReserved, dwUserIndex, dwFlags, pCapabilitiesEx) : ERROR_DEVICE_NOT_CONNECTED;

  if (SK_XInput_SlotStatus [dwUserIndex] && dwResult == ERROR_DEVICE_NOT_CONNECTED)
      SK_XInput_SlotStatus [dwUserIndex] = false;

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
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

bool
SK_XInput_UpdatePolledDataAndTimestamp (hid_device_file_s *pDevice, bool bActive, bool& bNewData)
{
  const DWORD
    dwTimeInMs =
      timeGetTime (),
    dwTimeoutInMs = (1000 * config.dwIdleTimeoutInSeconds);

  if (bActive)
  {
    pDevice->state.prev =
    pDevice->state.current;
    pDevice->state.current.dwPacketNumber++;

    pDevice->state.input_timestamp = dwTimeInMs;

    bNewData = true;
  }

  else if ( pDevice->state.input_timestamp != 0  &&
              config.dwIdleTimeoutInSeconds > 30 &&
                             dwTimeoutInMs < dwTimeInMs ) // Ignore invalid timeout values
  {
    if (pDevice->state.input_timestamp < (dwTimeInMs - dwTimeoutInMs))
    {
      if (SK_Bluetooth_PowerOffGamepad (pDevice))
      {
        return false;
      }
    }
  }

  return true;
}