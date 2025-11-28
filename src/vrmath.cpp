#include "vrmath.h"

vr::HmdVector3_t operator+(const vr::HmdVector3_t &vec1,
                           const vr::HmdVector3_t &vec2) {
  return {vec1.v[0] + vec2.v[0], vec1.v[1] + vec2.v[1], vec1.v[2] + vec2.v[2]};
}

vr::HmdVector3_t operator-(const vr::HmdVector3_t &vec1,
                           const vr::HmdVector3_t &vec2) {
  return {vec1.v[0] - vec2.v[0], vec1.v[1] - vec2.v[1], vec1.v[2] - vec2.v[2]};
}

// Rotate vector by quaternion
vr::HmdVector3_t operator*(const vr::HmdVector3_t &v,
                           const vr::HmdQuaternion_t &q) {
  vr::HmdQuaternion_t r = {
      q.w * 0.0 - q.x * v.v[0] - q.y * v.v[1] - q.z * v.v[2],
      q.w * v.v[0] + q.x * 0.0 + q.y * v.v[2] - q.z * v.v[1],
      q.w * v.v[1] - q.x * v.v[2] + q.y * 0.0 + q.z * v.v[0],
      q.w * v.v[2] + q.x * v.v[1] - q.y * v.v[0] + q.z * 0.0};

  // FIX: Explicitly cast the double result to float to prevent narrowing errors
  return {static_cast<float>(r.w * -q.x + r.x * q.w - r.y * q.z + r.z * q.y),
          static_cast<float>(r.w * -q.y + r.x * q.z + r.y * q.w - r.z * q.x),
          static_cast<float>(r.w * -q.z - r.x * q.y + r.y * q.x + r.z * q.w)};
}

vr::HmdVector3_t operator*(const vr::HmdVector3_t &vec, float scalar) {
  return {vec.v[0] * scalar, vec.v[1] * scalar, vec.v[2] * scalar};
}

vr::HmdQuaternion_t HmdQuaternion_FromMatrix(const vr::HmdMatrix34_t &matrix) {
  vr::HmdQuaternion_t q{};
  float trace = matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2];
  if (trace > 0) {
    float s = 0.5f / sqrtf(trace + 1.0f);
    q.w = 0.25f / s;
    q.x = (matrix.m[2][1] - matrix.m[1][2]) * s;
    q.y = (matrix.m[0][2] - matrix.m[2][0]) * s;
    q.z = (matrix.m[1][0] - matrix.m[0][1]) * s;
  } else {
    if (matrix.m[0][0] > matrix.m[1][1] && matrix.m[0][0] > matrix.m[2][2]) {
      float s =
          2.0f * sqrtf(1.0f + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2]);
      q.w = (matrix.m[2][1] - matrix.m[1][2]) / s;
      q.x = 0.25f * s;
      q.y = (matrix.m[0][1] + matrix.m[1][0]) / s;
      q.z = (matrix.m[0][2] + matrix.m[2][0]) / s;
    } else if (matrix.m[1][1] > matrix.m[2][2]) {
      float s =
          2.0f * sqrtf(1.0f + matrix.m[1][1] - matrix.m[0][0] - matrix.m[2][2]);
      q.w = (matrix.m[0][2] - matrix.m[2][0]) / s;
      q.x = (matrix.m[0][1] + matrix.m[1][0]) / s;
      q.y = 0.25f * s;
      q.z = (matrix.m[1][2] + matrix.m[2][1]) / s;
    } else {
      float s =
          2.0f * sqrtf(1.0f + matrix.m[2][2] - matrix.m[0][0] - matrix.m[1][1]);
      q.w = (matrix.m[1][0] - matrix.m[0][1]) / s;
      q.x = (matrix.m[0][2] + matrix.m[2][0]) / s;
      q.y = (matrix.m[1][2] + matrix.m[2][1]) / s;
      q.z = 0.25f * s;
    }
  }
  return q;
}

vr::HmdVector3_t HmdVector3_From34Matrix(const vr::HmdMatrix34_t &matrix) {
  return {matrix.m[0][3], matrix.m[1][3], matrix.m[2][3]};
}
