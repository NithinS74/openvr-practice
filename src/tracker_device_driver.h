//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <array>
#include <string>

// Phase 3: Windows Networking Headers
// These must often be included before other Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>

#include "openvr_driver.h"
#include <atomic>
#include <thread>
#include <mutex> // Phase 3: Added for std::mutex

enum MyComponent
{
	MyComponent_a_touch,
	MyComponent_a_click,

	MyComponent_trigger_value,
	MyComponent_trigger_click,

	MyComponent_MAX
};

//-----------------------------------------------------------------------------
// Purpose: Represents a single tracked device in the system.
// What this device actually is (controller, hmd) depends on the
// properties you set within the device (see implementation of Activate)
//-----------------------------------------------------------------------------
class MyTrackerDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	MyTrackerDeviceDriver( unsigned int my_tracker_id );

	vr::EVRInitError Activate( uint32_t unObjectId ) override;

	void EnterStandby() override;

	void *GetComponent( const char *pchComponentNameAndVersion ) override;

	void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	vr::DriverPose_t GetPose() override;

	void Deactivate() override;

	// ----- Functions we declare ourselves below -----

	const std::string &MyGetSerialNumber();

	void MyRunFrame();
	void MyProcessEvent( const vr::VREvent_t &vrevent );

	// Phase 3: Replaced MyPoseUpdateThread with the network receiver loop
	// (We will implement this function in Phase 4)
	void ReceiveLoop();

private:
	unsigned int my_tracker_id_;

	std::atomic< vr::TrackedDeviceIndex_t > my_device_index_;

	std::string my_device_model_number_;
	std::string my_device_serial_number_;

	std::array< vr::VRInputComponentHandle_t, MyComponent_MAX > input_handles_;

	std::atomic< bool > is_active_;

	// Phase 3: New Member Variables
	// ------------------------------------------------------------------------
	SOCKET udpSocket;                     // To hold the network connection
	std::thread receiverThread;           // To listen for data in the background
	std::atomic<bool> isRunning;          // To safely control the thread lifecycle
	vr::HmdQuaternion_t currentRotation;  // To store the latest received orientation
	std::mutex dataMutex;                 // To protect access to currentRotation
	// ------------------------------------------------------------------------
};
