#include "pch.h"

#pragma once
#include <windef.h>
#include <Dbt.h>
#include <atlbase.h>
#include <mmsystem.h>

#include <vector>
#include <algorithm>
#include <format>

#pragma comment (lib, "winmm.lib")

bool _GetInputReportStub (void*) { return false; }

concurrency::concurrent_vector <hid_device_file_s> hid_devices;

extern HMODULE hModRealXInput14;
extern BOOL    bCanUnloadRealXInput;

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
        case WM_POWERBROADCAST:
        {
          switch (wParam)
          {
            case PBT_APMSUSPEND:
              //Put controller to sleep.
              for (auto& device : hid_devices)
              {
                SK_Bluetooth_PowerOffGamepad (&device);
                device.state.input_timestamp = timeGetTime ();
              }

              if (_XInputPowerOff != nullptr)
              {
                for (auto i = 0 ; i < 4 ; ++i)
                {
                  _XInputPowerOff (i);
                }
              }
              break;

            case PBT_APMRESUMEAUTOMATIC:
            case PBT_APMRESUMESUSPEND:
              //Reset idle input timer.
              for (auto& device : hid_devices)
              {
                device.state.input_timestamp = timeGetTime ();
              }
              break;
          }

          return 0;
        } break;

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
                                                         nullptr, OPEN_EXISTING, 0x0,  nullptr );

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

    std::wstring wnd_class_name =
      std::format (L"SK_HID_XInput_Listener_pid{}", GetCurrentProcessId ());

    WNDCLASSEXW
      wnd_class               = {                       };
      wnd_class.hInstance     = GetModuleHandle (nullptr);
      wnd_class.lpszClassName = wnd_class_name.c_str ();
      wnd_class.lpfnWndProc   = SK_HID_DeviceNotifyProc;
      wnd_class.cbSize        = sizeof (WNDCLASSEXW);

    if (RegisterClassEx (&wnd_class))
    {
      DWORD dwWaitStatus = WAIT_OBJECT_0;

      hWndDeviceListener =
        (HWND)CreateWindowEx ( 0, wnd_class_name.c_str (),     NULL, 0,
                               0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );

      DEV_BROADCAST_DEVICEINTERFACE_W
      NotificationFilter                 = { };
      NotificationFilter.dbcc_size       = sizeof (DEV_BROADCAST_DEVICEINTERFACE_W);
      NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      NotificationFilter.dbcc_classguid  = GUID_DEVINTERFACE_HID;

      // It's technically unnecessary to register this, but not a bad idea
      HDEVNOTIFY   hDevNotify =
        RegisterDeviceNotificationW      (hWndDeviceListener, &NotificationFilter,          DEVICE_NOTIFY_WINDOW_HANDLE);
      HPOWERNOTIFY hPowNotify =
        RegisterPowerSettingNotification (hWndDeviceListener, &GUID_SESSION_DISPLAY_STATUS, DEVICE_NOTIFY_WINDOW_HANDLE);

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

      UnregisterDeviceNotification       (hDevNotify);
      UnregisterPowerSettingNotification (hPowNotify);
      DestroyWindow                      (hWndDeviceListener);
      UnregisterClassW                   (wnd_class.lpszClassName, wnd_class.hInstance);
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
                                            nullptr, OPEN_EXISTING, 0x0, nullptr )
                           );

        if ((intptr_t)hDeviceFile <= 0)
        {
          continue;
        }

        HIDD_ATTRIBUTES hidAttribs      = {                      };
                        hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

        PHIDP_PREPARSED_DATA pPreparsedData = nullptr;

        HidD_GetAttributes (hDeviceFile, &hidAttribs);

        if (! HidD_GetPreparsedData (hDeviceFile, &pPreparsedData))
        {
          continue;
        }

//#ifdef DEBUG
//            if (controller.bBluetooth)
//              SK_ImGui_Warning (L"Bluetooth");
//#endif

        HIDP_CAPS                                caps = { };
        HidP_GetCaps (          pPreparsedData, &caps);
        HidD_FreePreparsedData (pPreparsedData);
  
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

          controller.input_report.resize   (caps.InputReportByteLength);
          controller.output_report.resize  (caps.OutputReportByteLength);
          controller.feature_report.resize (caps.FeatureReportByteLength);

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

          controller.input_report.resize   (caps.InputReportByteLength);
          controller.output_report.resize  (caps.OutputReportByteLength);
          controller.feature_report.resize (caps.FeatureReportByteLength);

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


void
SK_XInput_ProbeAllSlotsForControllers (int slot0, int slot1, int slot2, int slot3)
{
  XINPUT_CAPABILITIES test_caps = {};

  bool connected0 =
    _XInputGetCapabilities (0, XINPUT_FLAG_GAMEPAD, &test_caps) == ERROR_SUCCESS;
  bool connected1 =
    _XInputGetCapabilities (1, XINPUT_FLAG_GAMEPAD, &test_caps) == ERROR_SUCCESS;
  bool connected2 =
    _XInputGetCapabilities (2, XINPUT_FLAG_GAMEPAD, &test_caps) == ERROR_SUCCESS;
  bool connected3 =
    _XInputGetCapabilities (3, XINPUT_FLAG_GAMEPAD, &test_caps) == ERROR_SUCCESS;

  SK_XInput_SlotStatus [0] = connected0;
  SK_XInput_SlotStatus [1] = connected1;
  SK_XInput_SlotStatus [2] = connected2;
  SK_XInput_SlotStatus [3] = connected3;
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

  DWORD first_unused_slot = 0;

  if (      SK_XInput_SlotStatus [ 0 ])
  { if (    SK_XInput_SlotStatus [ 1 ])
    { if (  SK_XInput_SlotStatus [ 2 ])
      { if (SK_XInput_SlotStatus [ 3 ])
        {      first_unused_slot = 4; // XXX
        } else first_unused_slot = 3;
      } else   first_unused_slot = 2;
    } else     first_unused_slot = 1;
  } else       first_unused_slot = 0;

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex >= first_unused_slot)
  {
    bool bSuccess = false;

    *pState = {};

    std::vector <std::pair <hid_device_file_s*, DWORD>> connected_devices;

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          connected_devices.push_back (
            std::make_pair (&controller, controller.state.input_timestamp)
          );
        }

        bSuccess = true;
      }
    }

    hid_device_file_s* pNewestDevice = nullptr;

    for ( auto& dev : connected_devices )
    {
      if (pNewestDevice == nullptr)
          pNewestDevice = dev.first;

      if (dev.second > pNewestDevice->state.input_timestamp)
      {
        pNewestDevice = dev.first;
      }
    }

    if (pNewestDevice != nullptr)
    {
      pState->Gamepad.bLeftTrigger  = pNewestDevice->state.current.Gamepad.bLeftTrigger;
      pState->Gamepad.bRightTrigger = pNewestDevice->state.current.Gamepad.bRightTrigger;
      pState->Gamepad.sThumbLX      = pNewestDevice->state.current.Gamepad.sThumbLX;
      pState->Gamepad.sThumbLY      = pNewestDevice->state.current.Gamepad.sThumbLY;
      pState->Gamepad.sThumbRX      = pNewestDevice->state.current.Gamepad.sThumbRX;
      pState->Gamepad.sThumbRY      = pNewestDevice->state.current.Gamepad.sThumbRY;
      pState->Gamepad.wButtons      = pNewestDevice->state.current.Gamepad.wButtons;
      pState->dwPacketNumber        = pNewestDevice->state.current.dwPacketNumber;

      // Give most recently active device a +75 ms advantage,
      //   we may have the same device connected over two
      //     protocols and do not want repeats of the same
      //       input...
      pNewestDevice->state.input_timestamp += 75;
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

  DWORD first_unused_slot = 0;

  if (      SK_XInput_SlotStatus [ 0 ])
  { if (    SK_XInput_SlotStatus [ 1 ])
    { if (  SK_XInput_SlotStatus [ 2 ])
      { if (SK_XInput_SlotStatus [ 3 ])
        {      first_unused_slot = 4; // XXX
        } else first_unused_slot = 3;
      } else   first_unused_slot = 2;
    } else     first_unused_slot = 1;
  } else       first_unused_slot = 0;

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex >= first_unused_slot)
  {
    bool bSuccess = false;

    *pState = {};

    std::vector <std::pair <hid_device_file_s*, DWORD>> connected_devices;

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          connected_devices.push_back (
            std::make_pair (&controller, controller.state.input_timestamp)
          );
        }

        bSuccess = true;
      }
    }

    hid_device_file_s* pNewestDevice = nullptr;

    for ( auto& dev : connected_devices )
    {
      if (pNewestDevice == nullptr)
          pNewestDevice = dev.first;

      if (dev.second > pNewestDevice->state.input_timestamp)
      {
        pNewestDevice = dev.first;
      }
    }

    if (pNewestDevice != nullptr)
    {
      pState->Gamepad.bLeftTrigger  = pNewestDevice->state.current.Gamepad.bLeftTrigger;
      pState->Gamepad.bRightTrigger = pNewestDevice->state.current.Gamepad.bRightTrigger;
      pState->Gamepad.sThumbLX      = pNewestDevice->state.current.Gamepad.sThumbLX;
      pState->Gamepad.sThumbLY      = pNewestDevice->state.current.Gamepad.sThumbLY;
      pState->Gamepad.sThumbRX      = pNewestDevice->state.current.Gamepad.sThumbRX;
      pState->Gamepad.sThumbRY      = pNewestDevice->state.current.Gamepad.sThumbRY;
      pState->Gamepad.wButtons      = pNewestDevice->state.current.Gamepad.wButtons;
      pState->dwPacketNumber        = pNewestDevice->state.current.dwPacketNumber;
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

  DWORD first_unused_slot = 0;

  if (      SK_XInput_SlotStatus [ 0 ])
  { if (    SK_XInput_SlotStatus [ 1 ])
    { if (  SK_XInput_SlotStatus [ 2 ])
      { if (SK_XInput_SlotStatus [ 3 ])
        {      first_unused_slot = 4; // XXX
        } else first_unused_slot = 3;
      } else   first_unused_slot = 2;
    } else     first_unused_slot = 1;
  } else       first_unused_slot = 0;

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex >= first_unused_slot)
  {
    int num_connected = 0;

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected) ++num_connected;
    }

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected && dwUserIndex < first_unused_slot + num_connected)
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

  DWORD first_unused_slot = 0;

  if (      SK_XInput_SlotStatus [ 0 ])
  { if (    SK_XInput_SlotStatus [ 1 ])
    { if (  SK_XInput_SlotStatus [ 2 ])
      { if (SK_XInput_SlotStatus [ 3 ])
        {      first_unused_slot = 4; // XXX
        } else first_unused_slot = 3;
      } else   first_unused_slot = 2;
    } else     first_unused_slot = 1;
  } else       first_unused_slot = 0;

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex >= first_unused_slot)
  {
    int num_connected = 0;

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected) ++num_connected;
    }

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected && dwUserIndex < first_unused_slot + num_connected)
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

using SKX_GetInjectedPIDs_pfn = size_t (__stdcall *)( DWORD* pdwList,
                                                      size_t capacity );

#ifdef _M_IX86
#define SK_BITNESS_A  "32"
#define SK_BITNESS_W L"32"
#else
#define SK_BITNESS_A  "64"
#define SK_BITNESS_W L"64"
#endif

const wchar_t*
SK_Util_GetSKDllName (void)
{
  static const wchar_t
         wszSpecialKDllName [16] = L"SpecialK" SK_BITNESS_W L".dll";
  return wszSpecialKDllName;
}

bool
SK_XInput_UpdatePolledDataAndTimestamp (hid_device_file_s *pDevice, bool bActive, bool& bNewData)
{
  const DWORD
    dwTimeInMs =
      timeGetTime (),
    dwTimeoutInMs = (1000 * config.dwIdleTimeoutInSeconds);

  static const wchar_t*
    wszSpecialKDllName = SK_Util_GetSKDllName ();

  static const bool bHasSpecialKDll =
    PathFileExistsW (wszSpecialKDllName);

  if (pDevice->state.input_timestamp == 0 && (! bActive))
  {   pDevice->state.input_timestamp = dwTimeInMs;
      pDevice->state.last_idle_check = dwTimeInMs;
  }

  if (bActive)
  {
    pDevice->state.prev =
    pDevice->state.current;
    pDevice->state.current.dwPacketNumber++;

    pDevice->state.input_timestamp = dwTimeInMs;
    pDevice->state.last_idle_check = dwTimeInMs;

    bNewData = true;
  }

  else
  {
    bNewData = false;

    if (                  pDevice->bConnected &&
                          pDevice->bWireless  &&
         pDevice->state.input_timestamp !=  0 &&
           config.dwIdleTimeoutInSeconds > 30 &&
                           dwTimeoutInMs < dwTimeInMs ) // Ignore invalid timeout values
    {
      if ( pDevice->state.input_timestamp < (dwTimeInMs - dwTimeoutInMs) &&
           pDevice->state.last_idle_check < (dwTimeInMs - dwTimeoutInMs) )
      {
        bool bIsSpecialKActive = false;

        HMODULE hModSpecialK =
          bHasSpecialKDll ? LoadLibraryW (wszSpecialKDllName)
                          : nullptr;

        if (hModSpecialK != nullptr)
        {
          const auto SKX_GetInjectedPIDs =
                    (SKX_GetInjectedPIDs_pfn)GetProcAddress ( hModSpecialK,
                    "SKX_GetInjectedPIDs" );

          static DWORD                 dwInjectedPIDs [512] = { };
          if ( nullptr       !=   SKX_GetInjectedPIDs )
            bIsSpecialKActive = ( SKX_GetInjectedPIDs (
                                       dwInjectedPIDs, 512) > 0 );

          FreeLibrary (hModSpecialK);
        }

        if (bIsSpecialKActive)
        {
          //
          // SK is actively injected into things, let's test for
          //   idle state again in 1/8th of the idle input period...
          //
          //  This increases the frequency that idle input is tested,
          //    but nothing significant given the typical timeout
          //      (i.e. 7.5 minutes -> 0.9375 minutes).
          //
          pDevice->state.last_idle_check =
            (dwTimeInMs - dwTimeoutInMs) / 8;
          //
          // Controller -is- idle, but SK is injected into a game, so all we can
          //   do is schedule another idle test sooner (8x more often) than usual.
          // 
          //  If SK's injected game exits before the next scheduled test, then we
          //    can finally power-off this gamepad!
          //
        }

        else if (SK_Bluetooth_PowerOffGamepad (pDevice))
        {
          return false;
        }
      }
    }
  }

  return true;
}