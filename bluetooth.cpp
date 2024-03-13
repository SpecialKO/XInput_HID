#include "pch.h"

bool
SK_Bluetooth_PowerOffGamepad (hid_device_file_s* pDevice)
{
  if (! pDevice->bWireless)
    return false;

  if (! pDevice->bConnected)
    return false;

  // The serial number is actually the Bluetooth MAC address; handy...
  wchar_t wszSerialNumber [32] = { };
  
  DWORD dwBytesReturned = 0;
  
  DeviceIoControl (
    pDevice->hDeviceFile, IOCTL_HID_GET_SERIALNUMBER_STRING, 0, 0,
                            wszSerialNumber, 64,
                           &dwBytesReturned, nullptr );

  ULONGLONG                           ullHWAddr = 0x0;
  swscanf (wszSerialNumber, L"%llx", &ullHWAddr);

  BLUETOOTH_FIND_RADIO_PARAMS findParams =
    { .dwSize = sizeof (BLUETOOTH_FIND_RADIO_PARAMS) };

  HANDLE hBtRadio    = INVALID_HANDLE_VALUE;
  HANDLE hFindRadios =
    BluetoothFindFirstRadio (&findParams, &hBtRadio);

  BOOL   success  = FALSE;
  while (success == FALSE && hBtRadio != 0)
  {
    success =
      DeviceIoControl (
        hBtRadio, IOCTL_BTH_DISCONNECT_DEVICE, &ullHWAddr, 8,
                                               nullptr,    0,
                             &dwBytesReturned, nullptr );

    if (success)
    {
      pDevice->disconnect ();
    }

    CloseHandle (hBtRadio);

    if (! success)
      if (! BluetoothFindNextRadio (hFindRadios, &hBtRadio))
        hBtRadio = 0;
  }

  BluetoothFindRadioClose (hFindRadios);

  return success != FALSE;
}