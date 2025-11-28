//============ Copyright (c) Valve Corporation, All rights reserved.
//============
#pragma once

#include <array>
#include <atomic>
#include <string>
#include <thread>

// Needed for networking (from previous steps)
#include <winsock2.h>
#include <ws2tcpip.h>

#include "common.h" // Include your shared data struct
#include "openvr_driver.h"

enum MyComponent {
  // FIXED: Added grip_click which was missing and causing the error
  MyComponent_grip_click,

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
  ~MyTrackerDeviceDriver(); // Added destructor for cleanup

  vr::EVRInitError Activate(uint32_t unObjectId) override;
  void Deactivate() override;
  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;
  void DebugRequest(const char *pchRequest, char *pchResponseBuffer,
                    uint32_t unResponseBufferSize) override;
  vr::DriverPose_t GetPose() override;

  // ----- Functions we declare ourselves below -----

  const std::string &MyGetSerialNumber();

  void MyRunFrame();
  void MyProcessEvent(const vr::VREvent_t &vrevent);

  // Networking / Pose Update Logic
  void UpdatePose(vr::HmdQuaternion_t newRotation);

private:
  unsigned int my_tracker_id_;
  std::atomic<vr::TrackedDeviceIndex_t> my_device_index_;

  std::string my_device_model_number_;
  std::string my_device_serial_number_;

  std::array<vr::VRInputComponentHandle_t, MyComponent_MAX> input_handles_;

  // Helper to store the latest rotation received from the provider
  vr::HmdQuaternion_t last_rotation_;
};
