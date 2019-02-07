#pragma once

#include "Vector.h"


namespace BAMEWARE
{
	class ColorHSBA;

	class ColorRGBA : public Vector<uint8_t, 4>
	{
	public:
		ColorRGBA();
		ColorRGBA(int r, int g, int b, int a = 255);
		ColorRGBA(const ColorHSBA& hsba);

	public:
		static Vector4DF GetFloatVec(const ColorRGBA& col) { return Vector4DF({ col[0] / 255.f, col[1] / 255.f, col[2] / 255.f, col[3] / 255.f }); }
		Vector4DF GetFloatVec() const { return GetFloatVec(*this); }

		static ColorRGBA RANDOM_COLOR(int alpha = 255);
		static ColorRGBA BLACK(int alpha = 255);
		static ColorRGBA WHITE(int alpha = 255);
		static ColorRGBA RED(int alpha = 255);
		static ColorRGBA GREEN(int alpha = 255);
		static ColorRGBA BLUE(int alpha = 255);
		static ColorRGBA PINK(int alpha = 255);
		static ColorRGBA MANGO(int alpha = 255);
	};

	class ColorHSBA : public Vector<float, 3>
	{
	public:
		ColorHSBA();
		ColorHSBA(float hue, float saturation, float brightness, int alpha = 255);
		ColorHSBA(const ColorRGBA& rgba);

	public:
		/// usage: Print(std::cout)
		void Print(std::ostream& stream) const;

		uint8_t GetAlpha() const { return m_alpha; }

	private:
		uint8_t m_alpha;
	};
	
}