
#include "controller_device.h"

ControllerDevice::ControllerDevice(vr::ETrackedControllerRole role) : role_(role), device_id_(vr::k_unTrackedDeviceIndexInvalid) {};

vr::EVRInitError ControllerDevice::Activate(uint32_t unObjectId) {
    vr::VRDriverLog()->Log("ControllerDevice::Activate");
    
    device_id_ = unObjectId;
    
    return vr::VRInitError_None;
}

void ControllerDevice::Deactivate() {
}

void ControllerDevice::EnterStandby() {
}

void* ControllerDevice::GetComponent(const char* pchComponentNameAndVersion) {
    return nullptr;
}

void ControllerDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
    if(unResponseBufferSize >= 1)
        pchResponseBuffer[0] = 0;
}

vr::DriverPose_t ControllerDevice::GetPose() {
    return vr::DriverPose_t();
}

