#pragma once

#include <math.h>
#include <openvr_driver.h>

// Vector operations
vr::HmdVector3_t operator+(const vr::HmdVector3_t &vec1,
                           const vr::HmdVector3_t &vec2);
vr::HmdVector3_t operator-(const vr::HmdVector3_t &vec1,
                           const vr::HmdVector3_t &vec2);
vr::HmdVector3_t operator*(const vr::HmdVector3_t &vec,
                           const vr::HmdQuaternion_t &q);
vr::HmdVector3_t operator*(const vr::HmdVector3_t &vec, float scalar);

// Conversions
vr::HmdQuaternion_t HmdQuaternion_FromMatrix(const vr::HmdMatrix34_t &matrix);
vr::HmdVector3_t HmdVector3_From34Matrix(const vr::HmdMatrix34_t &matrix);
