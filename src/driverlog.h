#pragma once

#include <openvr_driver.h>
#include <string>

extern void DriverLog(const char *pchFormat, ...);
extern void DriverLogVR(const char *pchFormat, ...);

extern bool InitDriverLog(vr::IVRDriverLog *pDriverLog);
extern void CleanupDriverLog();
