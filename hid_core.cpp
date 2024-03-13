#include "pch.h"

#pragma once
#include <windef.h>
#include <Dbt.h>
#include <atlbase.h>

DWORD
SK_HID_InputReportThread (LPVOID user)
{
  hid_device_file_s* pDevice =
    (hid_device_file_s *)user;

  static constexpr DWORD _PollInput       = WAIT_OBJECT_0;
  static constexpr DWORD _ShutdownInput   = WAIT_OBJECT_0 + 1;
  static constexpr DWORD _DisconnectInput = WAIT_OBJECT_0 + 2;
  static constexpr DWORD _ReconnectInput  = WAIT_OBJECT_0 + 3;

  HANDLE hOperationalEvents [] = {
    pDevice->threads.input.hDataRequest,
    pDevice->threads.events.hShutdown,
    pDevice->threads.events.hDisconnect,
    pDevice->threads.events.hReconnect,
  };

  DWORD  dwWaitState  = WAIT_FAILED;
  while (dwWaitState != _ShutdownInput)
  {
    dwWaitState =
      WaitForMultipleObjects ( _countof (hOperationalEvents),
                                         hOperationalEvents, FALSE, INFINITE );

    // Disconnect
    if (dwWaitState == _DisconnectInput)
    {
      ResetEvent (pDevice->threads.input.hDataRequest);
      continue;
    }

    // Reconnect
    if (dwWaitState == _ReconnectInput)
    {
      SetEvent (pDevice->threads.input.hDataRequest);
      continue;
    }

    // Input Report Waiting
    if (dwWaitState == _PollInput)
    {
      if (pDevice->GetInputReport ())
      {      
        if ( _PollInput ==
               WaitForSingleObject (pDevice->threads.input.hDataRequest, 0) )
        {
          SleepEx (2, TRUE);
        }
      }
    }
  }

  CloseHandle (
    std::exchange (pDevice->threads.input.hThread, INVALID_HANDLE_VALUE)
  );

  return 0;
}

void
SK_HID_SpawnInputReportThread (hid_device_file_s* pDevice)
{
  //static volatile LONG               s_InitOnce    =   FALSE;
  //if (! InterlockedCompareExchange (&s_InitOnce, TRUE, FALSE))
  {
    if (pDevice->threads.events.hShutdown == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hShutdown    = CreateEvent (nullptr, TRUE, FALSE, nullptr);

    if (pDevice->threads.events.hDisconnect == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hDisconnect  = CreateEvent (nullptr, FALSE, FALSE, nullptr);

    if (pDevice->threads.events.hReconnect == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hReconnect   = CreateEvent (nullptr, FALSE, FALSE, nullptr);
  }

  if (pDevice->threads.input.hDataRequest == INVALID_HANDLE_VALUE)
  {   pDevice->threads.input.hDataRequest  = CreateEvent  ( nullptr, TRUE, TRUE, nullptr );
      pDevice->threads.input.hThread       = CreateThread ( nullptr, 0x0,
                                                            SK_HID_InputReportThread,
                                                              (LPVOID)pDevice, 0x0, nullptr );
  }
}

void
SK_HID_SpawnOutputReportThread (hid_device_file_s* pDevice)
{
  //static volatile LONG               s_InitOnce    =   FALSE;
  //if (! InterlockedCompareExchange (&s_InitOnce, TRUE, FALSE))
  {
    if (pDevice->threads.events.hShutdown == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hShutdown    = CreateEvent (nullptr, TRUE, FALSE, nullptr);

    if (pDevice->threads.events.hDisconnect == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hDisconnect  = CreateEvent (nullptr, FALSE, FALSE, nullptr);

    if (pDevice->threads.events.hReconnect == INVALID_HANDLE_VALUE)
        pDevice->threads.events.hReconnect   = CreateEvent (nullptr, FALSE, FALSE, nullptr);
  }

#if 0
  if (pDevice->threads.output.hDataRequest == INVALID_HANDLE_VALUE)
  {   pDevice->threads.output.hDataRequest  = CreateEvent  ( nullptr, TRUE, TRUE, nullptr );
      pDevice->threads.output.hThread       = CreateThread ( nullptr, 0x0,
                                                               SK_HID_OutputReportThread,
                                                                 (LPVOID)pDevice, 0x0, nullptr );
  }
#endif
}

bool
SK_HID_PauseReportThreads (hid_device_file_s* pDevice)
{
  if (pDevice->threads.input.hThread      == INVALID_HANDLE_VALUE &&
      pDevice->threads.output.hThread     == INVALID_HANDLE_VALUE) return false;
  if (pDevice->threads.events.hDisconnect == INVALID_HANDLE_VALUE) return false;

  if (pDevice->threads.input.hDataRequest != INVALID_HANDLE_VALUE)
    ResetEvent (pDevice->threads.input.hDataRequest);

  if (pDevice->threads.output.hDataRequest != INVALID_HANDLE_VALUE)
    ResetEvent (pDevice->threads.output.hDataRequest);

  return
    SetEvent (pDevice->threads.events.hDisconnect);
}

bool
SK_HID_ResumeReportThreads (hid_device_file_s* pDevice)
{
  // Implicitly spawn the thread if necessary
  if (pDevice->threads.input.hThread == INVALID_HANDLE_VALUE)
  {
    SK_HID_SpawnInputReportThread (pDevice);
  }

  if (pDevice->threads.output.hThread == INVALID_HANDLE_VALUE)
  {
    SK_HID_SpawnOutputReportThread (pDevice);
  }

  if (pDevice->threads.input.hThread     == INVALID_HANDLE_VALUE) return false;
  if (pDevice->threads.events.hReconnect == INVALID_HANDLE_VALUE) return false;

  if (pDevice->threads.input.hDataRequest != INVALID_HANDLE_VALUE)
    SetEvent (pDevice->threads.input.hDataRequest);

  if (pDevice->threads.output.hDataRequest != INVALID_HANDLE_VALUE)
    SetEvent (pDevice->threads.output.hDataRequest);

  return
    SetEvent (pDevice->threads.events.hReconnect);
}

bool
SK_HID_ShutdownReportThreads (hid_device_file_s* pDevice)
{
  if (pDevice->threads.input.hThread    == INVALID_HANDLE_VALUE &&
      pDevice->threads.output.hThread   == INVALID_HANDLE_VALUE) return false;
  if (pDevice->threads.events.hShutdown == INVALID_HANDLE_VALUE) return false;

  return
    SetEvent (pDevice->threads.events.hShutdown);
}


bool
hid_device_file_s::disconnect (void)
{
  bool bWasConnected = bConnected;

  bConnected  = false;
  state       = {   };

  if (                (intptr_t)hDeviceFile > 0)
    CloseHandle (std::exchange (hDeviceFile, INVALID_HANDLE_VALUE));
  else                          hDeviceFile= INVALID_HANDLE_VALUE;

  SK_HID_PauseReportThreads (this);

  return bWasConnected;
}

bool
hid_device_file_s::reconnect (HANDLE hNewDeviceFile)
{
  if ((intptr_t)hNewDeviceFile <= 0 && (intptr_t)hDeviceFile <= 0)
    return false;

  if ((intptr_t)hDeviceFile <= 0)
                hDeviceFile = hNewDeviceFile;

  bConnected            = true;
  state.current         = { };
  state.prev            = { };
  state.input_timestamp = timeGetTime ();

  if (threads.input.hThread == INVALID_HANDLE_VALUE)
    SK_HID_SpawnInputReportThread (this);
  else
    SK_HID_ResumeReportThreads    (this);

  // TODO: Handle OutputReportThread

  return bConnected;
}