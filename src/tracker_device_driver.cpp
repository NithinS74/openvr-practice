//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "tracker_device_driver.h"
#include "driverlog.h"
#include "vrmath.h"
#include "common.h" // Phase 4: Include the shared protocol definition

#include <cstdio>

// Phase 4: Link against WS2_32.lib (strictly handled by CMake, but good practice in MSVC)
#pragma comment(lib, "ws2_32.lib")

// Settings definitions
static const char *my_tracker_main_settings_section = "driver_simpletrackers";
static const char *my_tracker_settings_key_model_number = "mytracker_model_number";

MyTrackerDeviceDriver::MyTrackerDeviceDriver( unsigned int my_tracker_id )
{
	my_tracker_id_ = my_tracker_id;
	
	// Phase 4: Initialize thread control and socket
	is_active_ = false;
	isRunning = false;
	udpSocket = INVALID_SOCKET;

	// Initialize a neutral rotation identity
	currentRotation.w = 1.0f;
	currentRotation.x = 0.0f;
	currentRotation.y = 0.0f;
	currentRotation.z = 0.0f;

	// Retrieve model number from settings
	char model_number[ 1024 ];
	vr::VRSettings()->GetString(
		my_tracker_main_settings_section, my_tracker_settings_key_model_number, model_number, sizeof( model_number ) );
	my_device_model_number_ = model_number;

	// Generate a unique serial number
	my_device_serial_number_ = my_device_model_number_ + std::to_string( my_tracker_id );

	DriverLog( "My Controller Model Number: %s", my_device_model_number_.c_str() );
	DriverLog( "My Controller Serial Number: %s", my_device_serial_number_.c_str() );
}

vr::EVRInitError MyTrackerDeviceDriver::Activate( uint32_t unObjectId )
{
	is_active_ = true;
	my_device_index_ = unObjectId;

	// Get the property container
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer( my_device_index_ );

	// Set standard properties
	vr::VRProperties()->SetStringProperty( container, vr::Prop_ModelNumber_String, my_device_model_number_.c_str() );
	vr::VRProperties()->SetStringProperty( container, vr::Prop_InputProfilePath_String, "{simpletrackers}/input/mytracker_profile.json" );

	// Set up input components (buttons/triggers)
	vr::VRDriverInput()->CreateBooleanComponent( container, "/input/grip/click", &input_handles_[MyComponent_grip_click] );
	vr::VRDriverInput()->CreateScalarComponent( container, "/input/trigger/value",
		&input_handles_[ MyComponent_trigger_value ], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided );
	vr::VRDriverInput()->CreateBooleanComponent(
		container, "/input/trigger/click", &input_handles_[ MyComponent_trigger_click ] );

	// -------------------------------------------------------------------------
	// Phase 4: Network Initialization
	// -------------------------------------------------------------------------
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
	{
		DriverLog( "Failed to initialize Winsock. Error Code : %d", WSAGetLastError() );
		return vr::VRInitError_Driver_Failed;
	}

	// Create UDP socket
	if ( ( udpSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET )
	{
		DriverLog( "Could not create socket : %d", WSAGetLastError() );
		return vr::VRInitError_Driver_Failed;
	}

	// Prepare the sockaddr_in structure
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	server.sin_port = htons( 8080 );     // Port 8080

	// Bind
	if ( bind( udpSocket, ( struct sockaddr * )&server, sizeof( server ) ) == SOCKET_ERROR )
	{
		DriverLog( "Bind failed with error code : %d", WSAGetLastError() );
		return vr::VRInitError_Driver_Failed;
	}

	DriverLog( "UDP Socket initialized on port 8080" );

	// Start the receiver thread
	isRunning = true;
	receiverThread = std::thread( &MyTrackerDeviceDriver::ReceiveLoop, this );

	return vr::VRInitError_None;
}

// Phase 4: The Network Receiver Loop
void MyTrackerDeviceDriver::ReceiveLoop()
{
	IMUPacket packet;
	struct sockaddr_in si_other;
	int slen = sizeof( si_other );

	DriverLog( "Starting ReceiveLoop..." );

	while ( isRunning )
	{
		// Waiting for data (blocking call)
		int recv_len = recvfrom( udpSocket, (char *)&packet, sizeof( IMUPacket ), 0, ( struct sockaddr * )&si_other, &slen );

		if ( recv_len == SOCKET_ERROR )
		{
			// If socket is closed during Deactivate, this error is expected.
			if ( isRunning )
			{
				DriverLog( "recvfrom() failed with error code : %d", WSAGetLastError() );
			}
			break;
		}

		if ( recv_len == sizeof( IMUPacket ) )
		{
			std::lock_guard<std::mutex> lock( dataMutex );

			// -----------------------------------------------------------------
			// Coordinate Conversion: IMU (Z-Up) -> OpenVR (Y-Up)
			// -----------------------------------------------------------------
			// Standard mapping:
			// OpenVR X = IMU X
			// OpenVR Y = IMU Z
			// OpenVR Z = -IMU Y
			// -----------------------------------------------------------------
			currentRotation.w = packet.w;
			currentRotation.x = packet.x;
			currentRotation.y = packet.z;  // Swap Y and Z
			currentRotation.z = -packet.y; // Invert new Z
		}

		// Notify SteamVR that we have a new pose
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated( my_device_index_, GetPose(), sizeof( vr::DriverPose_t ) );
	}
	
	DriverLog( "ReceiveLoop finished." );
}

// Phase 4: GetPose uses the received data
vr::DriverPose_t MyTrackerDeviceDriver::GetPose()
{
	vr::DriverPose_t pose = { 0 };

	// Always valid
	pose.poseIsValid = true;
	pose.deviceIsConnected = true;
	pose.result = vr::TrackingResult_Running_OK;

	// Set World<->Driver transform to identity
	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;

	// Position: Fixed in space (e.g., at floor or waist level)
	// Since we only have an IMU, we cannot track position changes (drift is too high).
	pose.vecPosition[ 0 ] = 0.0f;
	pose.vecPosition[ 1 ] = 1.0f; // 1 meter up (approx waist height)
	pose.vecPosition[ 2 ] = 0.0f;

	// Rotation: Read from our thread-safe variable
	{
		std::lock_guard<std::mutex> lock( dataMutex );
		pose.qRotation = currentRotation;
	}

	return pose;
}

void MyTrackerDeviceDriver::Deactivate()
{
	// Phase 4: Clean shutdown
	isRunning = false;

	// Closing the socket forces recvfrom() to unblock and fail, exiting the thread
	if ( udpSocket != INVALID_SOCKET )
	{
		closesocket( udpSocket );
		udpSocket = INVALID_SOCKET;
	}

	if ( receiverThread.joinable() )
	{
		receiverThread.join();
	}

	WSACleanup();

	is_active_ = false;
	my_device_index_ = vr::k_unTrackedDeviceIndexInvalid;
}

void *MyTrackerDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
	return nullptr;
}

void MyTrackerDeviceDriver::DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
	if ( unResponseBufferSize >= 1 )
		pchResponseBuffer[ 0 ] = 0;
}

void MyTrackerDeviceDriver::EnterStandby()
{
	DriverLog( "Tracker has been put into standby" );
}

void MyTrackerDeviceDriver::MyRunFrame()
{
  vr::VRDriverLog()->Log("hear beat!");
}

void MyTrackerDeviceDriver::MyProcessEvent( const vr::VREvent_t &vrevent )
{
}

const std::string &MyTrackerDeviceDriver::MyGetSerialNumber()
{
	return my_device_serial_number_;
}
