#include "pch.h"

#pragma once
#include <windef.h>

bool _GetInputReportStub (void* pDevice) { return false; }

concurrency::concurrent_vector <hid_device_file_s> hid_devices;



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









using XInputGetDSoundAudioDeviceGuids_pfn = DWORD (WINAPI *)(DWORD,GUID*,GUID*);
using XInputGetAudioDeviceIds_pfn = DWORD (WINAPI *)(DWORD,LPWSTR,UINT*,LPWSTR,UINT*);

DWORD
WINAPI
XInputGetState (DWORD dwUserIndex, XINPUT_STATE *pState)
{
  extern volatile LONG s_Init;
  while (! ReadAcquire (&s_Init))
    ;

  DWORD dwState;

  static XInputGetState_pfn _XInputGetState =
        (XInputGetState_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputGetState" );

  dwState =
    _XInputGetState (dwUserIndex, pState);

  if (dwState == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          //static auto x = MessageBox (0, L"Test", L"Test", MB_OK);
          memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));
          return ERROR_SUCCESS;
        }
      }
    }
  }

  return dwState;
}

DWORD
WINAPI
XInputGetStateEx (DWORD dwUserIndex, XINPUT_STATE_EX *pState)
{
  static XInputGetStateEx_pfn _XInputGetStateEx =
        (XInputGetStateEx_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                               XINPUT_GETSTATEEX_ORDINAL );

  return
    _XInputGetStateEx (dwUserIndex, pState);
}

DWORD
WINAPI
XInputSetState (
  _In_    DWORD             dwUserIndex,
  _Inout_ XINPUT_VIBRATION *pVibration )
{
  static XInputSetState_pfn _XInputSetState =
        (XInputSetState_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                            "XInputSetState" );

  DWORD dwState =
    _XInputSetState (dwUserIndex, pVibration);

  return dwState;
}

DWORD
WINAPI
XInputGetCapabilities (
  _In_  DWORD                dwUserIndex,
  _In_  DWORD                dwFlags,
  _Out_ XINPUT_CAPABILITIES *pCapabilities )
{
  extern volatile LONG s_Init;
  while (! ReadAcquire (&s_Init))
    ;

  static XInputGetCapabilities_pfn _XInputGetCapabilities =
        (XInputGetCapabilities_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                   "XInputGetCapabilities" );

  DWORD dwResult =
    _XInputGetCapabilities (dwUserIndex, dwFlags, pCapabilities);

  if (dwResult == ERROR_DEVICE_NOT_CONNECTED && dwUserIndex == 0)
  {
    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.GetInputReport ())
        {
          //memcpy (pState, &controller.state.current, sizeof (XINPUT_STATE));
          //return ERROR_SUCCESS;
        }
        return ERROR_SUCCESS;
      }
    }
  }

  return ERROR_SUCCESS;

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
  static XInputGetCapabilitiesEx_pfn _XInputGetCapabilitiesEx =
        (XInputGetCapabilitiesEx_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                      XINPUT_GETCAPABILITIES_EX_ORDINAL );

  return
    _XInputGetCapabilitiesEx (dwReserved, dwUserIndex, dwFlags, pCapabilitiesEx);
}

DWORD
WINAPI
XInputGetBatteryInformation (
  _In_  DWORD                       dwUserIndex,
  _In_  BYTE                        devType,
  _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation )
{
  static XInputGetBatteryInformation_pfn _XInputGetBatteryInformation =
        (XInputGetBatteryInformation_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                         "XInputGetBatteryInformation" );

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
  static XInputGetKeystroke_pfn _XInputGetKeystroke =
        (XInputGetKeystroke_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                "XInputGetKeystroke" );

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
  static XInputGetAudioDeviceIds_pfn _XInputGetAudioDeviceIds =
        (XInputGetAudioDeviceIds_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS),
                                     "XInputGetAudioDeviceIds");

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
  static XInputGetDSoundAudioDeviceGuids_pfn _XInputGetDSoundAudioDeviceGuids =
        (XInputGetDSoundAudioDeviceGuids_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                                             "XInputGetDSoundAudioDeviceGuids" );

  return
    _XInputGetDSoundAudioDeviceGuids (dwUserIndex, pDSoundRenderGuid, pDSoundCaptureGuid);
}

DWORD
WINAPI
XInputPowerOff (
  _In_ DWORD dwUserIndex )
{
  static XInputPowerOff_pfn _XInputPowerOff =
        (XInputPowerOff_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                             XINPUT_POWEROFF_ORDINAL );

  return
    _XInputPowerOff (dwUserIndex);
}

void
WINAPI
XInputEnable (
  _In_ BOOL enable )
{
  static XInputEnable_pfn _XInputEnable =
        (XInputEnable_pfn)GetProcAddress (
          LoadLibraryExW ( config.wszPathToSystemXInput1_4,
                  nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 |
                           LOAD_LIBRARY_SAFE_CURRENT_DIRS ),
                          "XInputEnable" );

  return
    _XInputEnable (enable);
}