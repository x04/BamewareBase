#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "Vector.h"
#include "Matrix.h"


#define M_PI_F (static_cast<float>(M_PI))
#define RAD2DEG(x) (static_cast<float>(x) * (180.f / M_PI_F))
#define DEG2RAD(x) (static_cast<float>(x) * (M_PI_F / 180.f))


namespace BAMEWARE::MATH
{
	void VectorAngle(const Vector3DF& forward, Vector3DF& angle);
	void VectorAngle(const Vector2DF& forward, float& angle);
	void AngleVector(const Vector3DF& angle, Vector3DF& forward);
	void AngleVector(float angle, Vector2DF& forward);
	Vector3DF CalcAngle(const Vector3DF& start, const Vector3DF& end);
	float CalcAngle(const Vector2DF& start, const Vector2DF& end);

	float DotProduct(const float *a, const float *b);
	float DotProduct(const Vector3DF& a, const Vector3DF& b);
	void VectorTransform(const float *in1, const Matrix3x4& in2, float *out);
	void VectorTransform(const Vector3DF& in1, const Matrix3x4& in2, Vector3DF& out);

	float NormalizePitch(float pitch);
	float NormalizeYaw(float yaw);
	Vector3DF NormalizeVector(Vector3DF vec);
}