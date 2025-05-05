#include "pch.h"

// Preliminary support for force-disconnecting Steam Input controllers,
//   code is unused but functional...
#ifdef STEAM_INPUT_MANAGEMENT
#include <minwindef.h>
#include <hidsdi.h>
#pragma pack (push,1)
#include <Dbt.h>
#pragma pack (pop)

#include <winnt.h>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <TlHelp32.h>
#include <atlbase.h>
#include <climits>
#include <string>
#include <vector>
#include <typeindex>

#define SK_HID_VID_SONY 0x054c

#pragma pack (push,8)
#ifndef  NT_SUCCESS
# define NT_SUCCESS(Status)           (((NTSTATUS)(Status)) >= 0)
# define STATUS_SUCCESS                ((NTSTATUS)0x00000000L)
# define STATUS_UNSUCCESSFUL           ((NTSTATUS)0xC0000001L)
# define STATUS_INFO_LENGTH_MISMATCH   ((NTSTATUS)0xC0000004L)
# define STATUS_NO_SUCH_FILE           ((NTSTATUS)0xC000000FL)
# define STATUS_ACCESS_DENIED          ((NTSTATUS)0xc0000022L)
# define STATUS_BUFFER_TOO_SMALL       ((NTSTATUS)0xC0000023L)
# define STATUS_ALERTED                ((NTSTATUS)0x00000101L)
# define STATUS_PROCESS_IS_TERMINATING ((NTSTATUS)0xC000010AL)

# define LDR_LOCK_LOADER_LOCK_DISPOSITION_INVALID           0
# define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED     1
# define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED 2

# define LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS   0x00000001
# define LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY          0x00000002

# define LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS 0x00000001
#endif
#define SystemHandleInformationSize 0x400000

enum SKIF_PROCESS_INFORMATION_CLASS
{
    ProcessBasicInformation                     = 0x00, //  0
    ProcessDebugPort                            = 0x07, //  7
    ProcessWow64Information                     = 0x1A, // 26
    ProcessImageFileName                        = 0x1B, // 27
    ProcessBreakOnTermination                   = 0x1D, // 29
    ProcessSubsystemInformation                 = 0x4B  // 75
};

enum SYSTEM_INFORMATION_CLASS
{ SystemBasicInformation                = 0,
  SystemPerformanceInformation          = 2,
  SystemTimeOfDayInformation            = 3,
  SystemProcessInformation              = 5,
  SystemProcessorPerformanceInformation = 8,
  SystemHandleInformation               = 16, // 0x10
  SystemInterruptInformation            = 23,
  SystemExceptionInformation            = 33,
  SystemRegistryQuotaInformation        = 37,
  SystemLookasideInformation            = 45,
  SystemExtendedHandleInformation       = 64, // 0x40
  SystemCodeIntegrityInformation        = 103,
  SystemPolicyInformation               = 134,
};

typedef enum _OBJECT_INFORMATION_CLASS {
  ObjectBasicInformation,
  ObjectNameInformation,
  ObjectTypeInformation,
  ObjectTypesInformation,
  ObjectHandleFlagInformation,
  ObjectSessionInformation,
} OBJECT_INFORMATION_CLASS;

using NtQueryInformationProcess_pfn =
  NTSTATUS (NTAPI *)(
       IN  HANDLE                    Handle,
       IN  SKIF_PROCESS_INFORMATION_CLASS ProcessInformationClass, // PROCESSINFOCLASS 
       OUT PVOID                     ProcessInformation,
       IN  ULONG                     ProcessInformationLength,
       OUT PULONG                    ReturnLength OPTIONAL
  );

typedef struct _SK_SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
  USHORT UniqueProcessId;
  USHORT CreatorBackTraceIndex;
  UCHAR  ObjectTypeIndex;
  UCHAR  HandleAttributes;
  USHORT HandleValue;
  PVOID  Object;
  ULONG  GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO,
*PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SK_SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
  PVOID       Object;
  union
  {
    ULONG_PTR UniqueProcessId;

    struct
    {
#ifdef _WIN64
      DWORD   ProcessId;
      DWORD   ThreadId; // ?? ( What are the upper-bits for anyway ? )
#else
      WORD    ProcessId;
      WORD    ThreadId; // ?? ( What are the upper-bits for anyway ? )
#endif
    };
  };

  union
  {
    ULONG_PTR HandleValue;
    HANDLE    Handle;
  };

  ULONG       GrantedAccess;
  USHORT      CreatorBackTraceIndex;
  USHORT      ObjectTypeIndex;
  ULONG       HandleAttributes;
  ULONG       Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX,
*PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SK_SYSTEM_HANDLE_INFORMATION
{ ULONG                          NumberOfHandles;
  SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles     [1];
} SYSTEM_HANDLE_INFORMATION,
 *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SK_SYSTEM_HANDLE_INFORMATION_EX
{ ULONG_PTR                         NumberOfHandles;
  ULONG_PTR                         Reserved;
  SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles     [1];
} SYSTEM_HANDLE_INFORMATION_EX,
*PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _SK_UNICODE_STRING
{ USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING,
*PUNICODE_STRING;

// MIT: https://github.com/antonioCoco/ConPtyShell/blob/master/ConPtyShell.cs
typedef struct _OBJECT_TYPE_INFORMATION_V2 {
  UNICODE_STRING  TypeName;
  ULONG           TotalNumberOfObjects;
  ULONG           TotalNumberOfHandles;
  ULONG           TotalPagedPoolUsage;
  ULONG           TotalNonPagedPoolUsage;
  ULONG           TotalNamePoolUsage;
  ULONG           TotalHandleTableUsage;
  ULONG           HighWaterNumberOfObjects;
  ULONG           HighWaterNumberOfHandles;
  ULONG           HighWaterPagedPoolUsage;
  ULONG           HighWaterNonPagedPoolUsage;
  ULONG           HighWaterNamePoolUsage;
  ULONG           HighWaterHandleTableUsage;
  ULONG           InvalidAttributes;
  GENERIC_MAPPING GenericMapping;
  ULONG           ValidAccessMask;
  BOOLEAN         SecurityRequired;
  BOOLEAN         MaintainHandleCount;
  UCHAR           TypeIndex;    // Added in V2
  CHAR            ReservedByte; // Added in V2
  ULONG           PoolType;
  ULONG           DefaultPagedPoolCharge;
  ULONG           DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION_V2,
*POBJECT_TYPE_INFORMATION_V2;

typedef struct _SK_OBJECT_NAME_INFORMATION
{ UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION,
*POBJECT_NAME_INFORMATION;

using NtQueryObject_pfn =
  NTSTATUS (NTAPI *)(
       IN  HANDLE                   Handle       OPTIONAL,
       IN  OBJECT_INFORMATION_CLASS ObjectInformationClass,
       OUT PVOID                    ObjectInformation,
       IN  ULONG                    ObjectInformationLength,
       OUT PULONG                   ReturnLength OPTIONAL
  );

using NtQuerySystemInformation_pfn =
  NTSTATUS (NTAPI *)(
        IN  SYSTEM_INFORMATION_CLASS SystemInformationClass,
        OUT PVOID                    SystemInformation,
        IN  ULONG                    SystemInformationLength,
        OUT PULONG                   ReturnLength OPTIONAL
  );

using NtQueryObject_pfn =
  NTSTATUS (NTAPI *)(
       IN  HANDLE                   Handle       OPTIONAL,
       IN  OBJECT_INFORMATION_CLASS ObjectInformationClass,
       OUT PVOID                    ObjectInformation,
       IN  ULONG                    ObjectInformationLength,
       OUT PULONG                   ReturnLength OPTIONAL
  );

using NtDuplicateObject_pfn =
  NTSTATUS (NTAPI *)(
        IN  HANDLE      SourceProcessHandle,
        IN  HANDLE      SourceHandle,
        OUT HANDLE      TargetProcessHandle,
        OUT PHANDLE     TargetHandle,
        IN  ACCESS_MASK DesiredAccess OPTIONAL,
        IN  ULONG       Attributes    OPTIONAL,
        IN  ULONG       Options       OPTIONAL
  );

template <typename _StrT>
struct ProcModule
{
  constexpr ProcModule (const _StrT* name) :
                           mod_name (name) {};

  operator HMODULE (void)
  {
    return (proc_mod != nullptr) ?
            proc_mod             :
            proc_mod = GetModule ();
  }

  const _StrT* mod_name = nullptr;
  HMODULE      proc_mod = nullptr;

private:
  HMODULE GetModule (void)
  {
    LPCVOID GetHandleProc = nullptr;

    if (     typeid (wchar_t) == typeid (_StrT)) GetHandleProc = GetModuleHandleW;
    else if (typeid ( char  ) == typeid (_StrT)) GetHandleProc = GetModuleHandleA;

    return                                                 GetHandleProc != nullptr ?
      reinterpret_cast <HMODULE (WINAPI *)(const _StrT*)> (GetHandleProc)(mod_name) : nullptr;
  }
};

template <typename _T, ProcModule proc_mod>
_T
WINAPI
GetProcAddress (LPCSTR proc_name)
{
  return                 ((HMODULE)static_cast <decltype (proc_mod)>(proc_mod) != nullptr) ?
    (_T)::GetProcAddress ((HMODULE)static_cast <decltype (proc_mod)>(proc_mod), proc_name) : nullptr;
}

// CC BY-SA 3.0: https://stackoverflow.com/a/39104745
typedef struct _OBJECT_TYPES_INFORMATION {
  LONG NumberOfTypes;
} OBJECT_TYPES_INFORMATION,
*POBJECT_TYPES_INFORMATION;

// CC BY-SA 3.0: https://stackoverflow.com/a/39104745
//               https://jadro-windows.cz/download/ntqueryobject.zip
#define ALIGN_DOWN(Length, Type)       (            (ULONG)(Length) & ~(sizeof(Type) - 1))
#define ALIGN_UP(Length, Type)         (ALIGN_DOWN(((ULONG)(Length) +   sizeof(Type) - 1), Type))

class file {
public:
    static const wchar_t arg[];
};
//decltype(file::arg) file::arg = __FILEW__;

#define DeclLibraryProc(proc,mod) static proc##_pfn proc = GetProcAddress <decltype (proc), mod> (#proc)
// CC BY-SA 3.0: https://stackoverflow.com/a/39104745
//               https://jadro-windows.cz/download/ntqueryobject.zip
// Modified to use POBJECT_TYPE_INFORMATION_V2 instead of POBJECT_TYPE_INFORMATION
USHORT GetTypeIndexByName (std::wstring TypeName)
{
  DeclLibraryProc (NtQueryObject, L"NtDll");
  //static NtQueryObject_pfn
  //       NtQueryObject = GetProcAddressEx (NtQueryObject, L"NtDll");
  //      //(NtQueryObject), L"NtDll">(
  //      //"NtQueryObject");

  POBJECT_TYPE_INFORMATION_V2  TypeInfo = NULL;
  POBJECT_TYPES_INFORMATION   TypesInfo = NULL;
  USHORT   ret                          = USHRT_MAX;
  NTSTATUS ntStatTypesInfo;
  ULONG    BufferLength = SystemHandleInformationSize;

  std::vector <uint8_t> types_info_buffer (BufferLength);

  do
  {
    types_info_buffer.resize (BufferLength);
    TypesInfo = (POBJECT_TYPES_INFORMATION)types_info_buffer.data ();

    ntStatTypesInfo =
      NtQueryObject (NULL, ObjectTypesInformation, TypesInfo, BufferLength, &BufferLength);
  } while (ntStatTypesInfo == STATUS_INFO_LENGTH_MISMATCH && BufferLength != 0);

  if (NT_SUCCESS (ntStatTypesInfo) && TypesInfo->NumberOfTypes > 0)
  {
    // Align to first element of the array
    TypeInfo = (POBJECT_TYPE_INFORMATION_V2)((PCHAR)TypesInfo + ALIGN_UP(sizeof(*TypesInfo), ULONG_PTR));
    for (int i = 0; i < TypesInfo->NumberOfTypes; i++)
    {
      //USHORT     _TypeIndex = i + 2;               // OBJECT_TYPE_INFORMATION requires adding 2 to get the proper type index
      USHORT       _TypeIndex = TypeInfo->TypeIndex; // OBJECT_TYPE_INFORMATION_V2 includes it in the struct
      std::wstring _TypeName  = std::wstring(TypeInfo->TypeName.Buffer, TypeInfo->TypeName.Length / sizeof(WCHAR));

      if (TypeName == _TypeName)
      {
        ret = _TypeIndex;
        break;
      }

      // Align to the next element of the array
      TypeInfo = (POBJECT_TYPE_INFORMATION_V2)((PCHAR)(TypeInfo + 1) + ALIGN_UP(TypeInfo->TypeName.MaximumLength, ULONG_PTR));
    }
  }

  return ret;
}
#pragma pack (pop)

PROCESSENTRY32W
FindProcessByName (const wchar_t* wszName)
{
  PROCESSENTRY32W pe32 = { };

  CHandle hProcessSnap (
    CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0)
  );

  if ((intptr_t)hProcessSnap.m_h <= 0)// == INVALID_HANDLE_VALUE)
    return pe32;

  pe32.dwSize = sizeof (PROCESSENTRY32W);

  if (! Process32FirstW (hProcessSnap, &pe32))
  {
    return pe32;
  }

  do
  {
    if (wcsstr (pe32.szExeFile, wszName))
      return pe32;
  } while (Process32NextW (hProcessSnap, &pe32));

  return pe32;
}

#include <initguid.h>
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

void
SK_Input_KillSteamInput (void)
{
  HANDLE hDeviceEnumThread =
  CreateThread (nullptr, 0x0, [](LPVOID)->DWORD
  {
    DeclLibraryProc (NtQuerySystemInformation, L"NtDll");
    DeclLibraryProc (NtDuplicateObject,        L"NtDll");
    DeclLibraryProc (NtQueryObject,            L"NtDll");

    DWORD dwSteamClientPid =
      FindProcessByName (L"steam.exe").th32ProcessID;

    if (! dwSteamClientPid)
    {
      return 0;
    }

    CHandle hSteamProcess (
      dwSteamClientPid != 0 ?
        OpenProcess ( PROCESS_DUP_HANDLE |
                      PROCESS_QUERY_INFORMATION,
                        FALSE,
                          dwSteamClientPid )
                            :
        INVALID_HANDLE_VALUE
    );

    extern concurrency::concurrent_vector <hid_device_file_s> hid_devices;
    std::vector <std::pair <hid_device_file_s*, DWORD>> connected_devices;

    for ( auto& controller : hid_devices )
    {
      if (controller.bConnected)
      {
        if (controller.devinfo.vid == SK_HID_VID_SONY)
        {
          connected_devices.push_back (
            std::make_pair (&controller, controller.state.input_timestamp)
          );
        }
      }
    }

#if 0
    if (! connected_devices.empty ())
    {
      static const wchar_t* wszDevicePath;

      wszDevicePath =
        connected_devices [0].first->wszDevicePath;

      EnumWindows ([](HWND hWnd, LPARAM lParam)->BOOL
      {
        DWORD window_pid = 0;
        DWORD steam_proc = (DWORD)lParam;

        if (GetWindowThreadProcessId (hWnd, &window_pid) != 0)
        {
          if (window_pid == steam_proc)
          {
            std::vector <uint8_t> broadcast_data (
              sizeof (DEV_BROADCAST_DEVICEINTERFACE_W) + 261 * sizeof (wchar_t)
            );

            DEV_BROADCAST_DEVICEINTERFACE_W*
                    broadcast_arrival                  = (DEV_BROADCAST_DEVICEINTERFACE_W *)broadcast_data.data ();
                    broadcast_arrival->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
                    broadcast_arrival->dbcc_classguid  = GUID_DEVINTERFACE_HID;
            memcpy (broadcast_arrival->dbcc_name,                                           wszDevicePath,        wcslen (wszDevicePath) * sizeof (wchar_t));
                    broadcast_arrival->dbcc_size       = (DWORD)sizeof (DEV_BROADCAST_DEVICEINTERFACE_W) + (DWORD)wcslen (wszDevicePath) * sizeof (wchar_t);

            DWORD broadcast_target = BSM_APPLICATIONS;
            BroadcastSystemMessageW ( BSF_IGNORECURRENTTASK | BSF_FORCEIFHUNG |
                                      BSF_NOHANG            | BSF_NOTIMEOUTIFNOTHUNG, &broadcast_target, WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE, (LPARAM)(DEV_BROADCAST_HDR *)broadcast_arrival);
                                                                                       broadcast_target = BSM_APPLICATIONS;
            BroadcastSystemMessageW ( BSF_IGNORECURRENTTASK | BSF_FORCEIFHUNG |
                                      BSF_NOHANG            | BSF_NOTIMEOUTIFNOTHUNG, &broadcast_target, WM_DEVICECHANGE, DBT_DEVNODES_CHANGED,     (LPARAM)(DEV_BROADCAST_HDR *)broadcast_arrival);
                                                                                       broadcast_target = BSM_APPLICATIONS;
            BroadcastSystemMessageW ( BSF_IGNORECURRENTTASK | BSF_FORCEIFHUNG |
                                      BSF_NOHANG            | BSF_NOTIMEOUTIFNOTHUNG, &broadcast_target, WM_DEVICECHANGE, DBT_DEVICEARRIVAL,        (LPARAM)(DEV_BROADCAST_HDR *)broadcast_arrival);

            return FALSE;
          }
        }

        return TRUE;
      }, dwSteamClientPid);
    }
#endif

    static USHORT FileIndex = GetTypeIndexByName (L"File");
    
    std::vector <uint8_t> handle_info_buffer (SystemHandleInformationSize);

    NTSTATUS                        ns           = STATUS_INFO_LENGTH_MISMATCH;
    DWORD                         dwReadBytes    =     0UL;
    PSYSTEM_HANDLE_INFORMATION_EX  pHandleInfoEx = nullptr;

    for ( DWORD dSize   =  SystemHandleInformationSize ;
                dSize  != 0 && !pHandleInfoEx          ;
                dSize <<= 1 )
    {
      handle_info_buffer.resize (static_cast <size_t> (dSize));

      pHandleInfoEx =
        (PSYSTEM_HANDLE_INFORMATION_EX)handle_info_buffer.data ();

      ns =
        NtQuerySystemInformation (
          SystemExtendedHandleInformation,
                                 handle_info_buffer.data (),
            static_cast <ULONG> (handle_info_buffer.size ()),
            &dwReadBytes );

      if (! NT_SUCCESS (ns))
      {
        pHandleInfoEx = nullptr;
        dwReadBytes   = 0;
        
        if (ns != STATUS_INFO_LENGTH_MISMATCH)
          break;
      }
    }
    
    if (NT_SUCCESS (ns) && dwReadBytes != 0 && pHandleInfoEx != nullptr)
    {
      // Go through all handles of the system
      for ( unsigned int i = 0;
                         i < pHandleInfoEx->NumberOfHandles;
                         i++ )
      {
        // We only want files
        if (pHandleInfoEx->Handles [i].ObjectTypeIndex != FileIndex)
          continue;

        // Skip handles that do not belong to Steam
        if (pHandleInfoEx->Handles [i].ProcessId != dwSteamClientPid)
          continue;
    
        HANDLE file =
          pHandleInfoEx->Handles [i].Handle;

        HIDD_ATTRIBUTES hidAttribs      = {                      };
                        hidAttribs.Size = sizeof (HIDD_ATTRIBUTES);

        if (pHandleInfoEx->Handles [i].ProcessId == dwSteamClientPid)
        {
          if ((intptr_t)hSteamProcess.m_h > 0)
          {
            if (DuplicateHandle ( hSteamProcess,     file,
                              GetCurrentProcess (), &file, 0, FALSE, DUPLICATE_SAME_ACCESS ))
            {
              CHandle   hRemoteFile  (file);
              if (HidD_GetAttributes (file, &hidAttribs))
              {
                if (hidAttribs.VendorID == SK_HID_VID_SONY)
                {
                  HANDLE hSayonaraSteamInput;

                  if ( DuplicateHandle ( hSteamProcess,    pHandleInfoEx->Handles [i].Handle,
                                     GetCurrentProcess (), &hSayonaraSteamInput, 0, FALSE, DUPLICATE_CLOSE_SOURCE )) {
                           CloseHandle                     (hSayonaraSteamInput);
                  }
                }
              }
            }
          }
        }

#if 0
        std::wstring handle_name = L"";

        ULONG                 _ObjectNameLen ( MAX_PATH );
        std::vector <uint8_t> pObjectName;

        for ( NTSTATUS ntStat  = STATUS_INFO_LENGTH_MISMATCH ;
                       ntStat == STATUS_INFO_LENGTH_MISMATCH ; )
        {
          pObjectName.resize (
              _ObjectNameLen );

          ntStat =
            NtQueryObject ( file,
                    ObjectNameInformation,
                  pObjectName.data (),
                  _ObjectNameLen,
                  &_ObjectNameLen
            );

          if (NT_SUCCESS (ntStat))
          {
             POBJECT_NAME_INFORMATION _pni =
            (POBJECT_NAME_INFORMATION) pObjectName.data ();

            handle_name = _pni != nullptr ?
                          _pni->Name.Length > 0 ?
                          _pni->Name.Buffer     : L""
                                                : L"";
          }
        }

        SK_LOGi0 (L"File Name=%ws", handle_name.c_str ());
#endif
      }
    }

    return 0;
  }, nullptr, 0x0, nullptr);
}
#endif