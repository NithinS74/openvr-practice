//============ Copyright (c) Valve Corporation, All rights reserved.
//============
#pragma once

#include <array>
#include <string>

// Networking Headers - Must often come first on Windows
#include <winsock2.h>
#include <ws2tcpip.h>

#include "openvr_driver.h"
#include <atomic>
#include <mutex> // Added for std::mutex
#include <thread>

enum MyComponent {
  MyComponent_grip_click, // Fixed typo from previous steps
  MyComponent_trigger_value,
  MyComponent_trigger_click,
  MyComponent_MAX
};

//-----------------------------------------------------------------------------
// Purpose: Represents a single tracked device in the system.
//-----------------------------------------------------------------------------
class MyTrackerDeviceDriver : public vr::ITrackedDeviceServerDriver {
public:
  MyTrackerDeviceDriver(unsigned int my_tracker_id);
  ~MyTrackerDeviceDriver() = default; // Destructor

  vr::EVRInitError Activate(uint32_t unObjectId) override;
  void Deactivate() override;
  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;
  void DebugRequest(const char *pchRequest, char *pchResponseBuffer,
                    uint32_t unResponseBufferSize) override;
  vr::DriverPose_t GetPose() override;

  // ----- Functions we declare ourselves -----
  const std::string &MyGetSerialNumber();
  void MyRunFrame();
  void MyProcessEvent(const vr::VREvent_t &vrevent);

  // This was missing in your previous header, causing the "undeclared
  // identifier" error
  void ReceiveLoop();

private:
  unsigned int my_tracker_id_;
  std::atomic<vr::TrackedDeviceIndex_t> my_device_index_;

  std::string my_device_model_number_;
  std::string my_device_serial_number_;

  std::array<vr::VRInputComponentHandle_t, MyComponent_MAX> input_handles_;

  // Tracks if the driver has been Activated() by SteamVR
  std::atomic<bool> is_active_;

  // --- Networking & Threading Members (These were missing!) ---
  SOCKET udpSocket;
  std::thread receiverThread;
  std::atomic<bool> isRunning;
  vr::HmdQuaternion_t currentRotation;
  std::mutex dataMutex;
};
