#include "../Headers/Math.h"


namespace BAMEWARE::MATH
{
	void VectorAngle(const Vector3DF& forward, Vector3DF& angle)
	{
		if (forward[0] == 0.f && forward[1] == 0.f)
		{
			angle[1] = 0.f;
			if (forward[2] > 0.f)
				angle[0] = 270.f;
			else
				angle[0] = 90.f;
		}
		else
		{
			angle[1] = (atan2(forward[1], forward[0]) * 180.f / M_PI_F);
			if (angle[1] < 0.f)
				angle[1] += 360.f;

			angle[0] = (atan2(-forward[2], sqrtf(forward[0] * forward[0] + forward[1] * forward[1])) * 180.f / M_PI_F);
			if (angle[0] < 0.f)
				angle[0] += 360.f;
		}

		angle[2] = 0.f;
	}

	void VectorAngle(const Vector2DF& forward, float& angle)
	{
		angle = RAD2DEG(std::atan2(forward[1], forward[0])) + 90.f;
	}

	void AngleVector(const Vector3DF& angle, Vector3DF& forward)
	{
		const auto SinCos = [](const float radians, float* sine, float* cosine) -> void
		{
			*sine = sin(radians);
			*cosine = cos(radians);
		};

		float sr, sp, sy, cr, cp, cy;
		SinCos(DEG2RAD(angle[0]), &sp, &cp);
		SinCos(DEG2RAD(angle[1]), &sy, &cy);
		SinCos(DEG2RAD(angle[2]), &sr, &cr);

		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	void AngleVector(float angle, Vector2DF& forward)
	{
		angle = DEG2RAD(angle);
		forward[0] = sin(angle), forward[1] = -cos(angle);
	}

	Vector3DF CalcAngle(const Vector3DF& start, const Vector3DF& end)
	{
		Vector3DF ret;
		VectorAngle(end - start, ret);
		return ret;
	}

	float CalcAngle(const Vector2DF& start, const Vector2DF& end)
	{
		float angle;
		VectorAngle(end - start, angle);
		return angle;
	}

	float DotProduct(const float* a, const float* b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	float DotProduct(const Vector3DF& a, const Vector3DF& b)
	{
		return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	}

	void VectorTransform(const float* in1, const Matrix3x4& in2, float* out)
	{
		out[0] = DotProduct(in1, in2[0]) + in2[0][3];
		out[1] = DotProduct(in1, in2[1]) + in2[1][3];
		out[2] = DotProduct(in1, in2[2]) + in2[2][3];
	}

	void VectorTransform(const Vector3DF& in1, const Matrix3x4& in2, Vector3DF& out)
	{
		VectorTransform(&const_cast<Vector3DF&>(in1)[0], in2, &out[0]);
	}

	float NormalizePitch(float pitch)
	{
		while (pitch > 89.f)
			pitch -= 180.f;
		while (pitch < -89.f)
			pitch += 180.f;

		return pitch;
	}

	float NormalizeYaw(float yaw)
	{
		if (yaw > 180.f)
			yaw -= (round(yaw / 360) * 360.f);
		else if (yaw < -180.f)
			yaw += (round(yaw / 360) * -360.f);
		else
			return yaw;

		while (yaw > 180.f)
			yaw -= 360.f;
		while (yaw < -180.f)
			yaw += 360.f;

		return yaw;
	}

	Vector3DF NormalizeVector(Vector3DF vec)
	{
		vec[0] = NormalizePitch(vec[0]);
		vec[1] = NormalizeYaw(vec[1]);
		vec[2] = 0.f;
		return vec;
	}
}
