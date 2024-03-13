// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS

// add headers that you want to pre-compile here
#include "framework.h"
#include "config.h"

#include <windef.h>
#include <cstdint>
#include <string>

#include <mmsystem.h>
#include <Shlwapi.h>

#include <concurrent_vector.h>
#include <SetupAPI.h>

#include <initguid.h>
#include <hidclass.h>
#include <hidsdi.h>

#include <bluetoothapis.h>
#include <winioctl.h>
#include <bthioctl.h>

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "bthprops.lib")

#include "xinput_defs.h"
#include "hid_device.h"
#include "bluetooth.h"

#endif //PCH_H
