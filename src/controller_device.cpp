
#include "controller_device.h"

class ControllerDevice : public vr::ITrackedDeviceServerDriver {
private:
    vr::ETrackedControllerRole role_;
    vr::TrackedDeviceIndex_t device_id_;
public:
    ControllerDevice(vr::ETrackedControllerRole role):role_(role),device_id_(vr::k_unTrackedDeviceIndexInvalid){}

    // Inherited via ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate(uint32_t unObjectId) override{
    vr::VRDriverLog()->Log("Controller activated")
    device_id_=unObjectId
    return vr::EVRInitError_None;
  }
    virtual void Deactivate() override{}
    virtual void EnterStandby() override{}
    virtual void* GetComponent(const char* pchComponentNameAndVersion) override{return nullptr}
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override{
    if(unResponseBufferSize>=1) pchResponseBuffer[0]=0;
  }
    virtual vr::DriverPose_t GetPose() override{
    return vr::DriverPose_t();
  }

};

