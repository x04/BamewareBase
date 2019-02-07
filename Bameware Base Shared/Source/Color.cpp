#include "../Headers/Color.h"

#include "../Headers/Utils.h"


namespace BAMEWARE
{
	/* ColorRGBA */
	ColorRGBA::ColorRGBA() { SetAll(uint8_t(0)); }

	ColorRGBA::ColorRGBA(const int r, const int g, const int b, const int a)
	{
		At(0) = r;
		At(1) = g;
		At(2) = b;
		At(3) = a;
	}

	ColorRGBA::ColorRGBA(const ColorHSBA& hsba)
	{
		const float hue = UTILS::Clamp(hsba.At(0), 0.f, 1.f);
		const float saturation = UTILS::Clamp(hsba.At(1), 0.f, 1.f);
		const float brightness = UTILS::Clamp(hsba.At(2), 0.f, 1.f);

		const float h = (hue == 1.f) ? 0.f : (hue * 6.f);
		const float f = h - int(h);
		const float p = brightness * (1.f - saturation);
		const float q = brightness * (1.f - saturation * f);
		const float t = brightness * (1.f - (saturation * (1.f - f)));

		if (h < 1.f)
			*this = ColorRGBA(int(brightness * 255), int(t * 255), int(p * 255), hsba.GetAlpha());
		else if (h < 2.f)
			*this = ColorRGBA(int(q * 255), int(brightness * 255), int(p * 255), hsba.GetAlpha());
		else if (h < 3.f)
			*this = ColorRGBA(int(p * 255), int(brightness * 255), int(t * 255), hsba.GetAlpha());
		else if (h < 4)
			*this = ColorRGBA(int(p * 255), int(q * 255), int(brightness * 255), hsba.GetAlpha());
		else if (h < 5)
			*this = ColorRGBA(int(t * 255), int(p * 255), int(brightness * 255), hsba.GetAlpha());
		else
			*this = ColorRGBA(int(brightness * 255), int(p * 255), int(q * 255), hsba.GetAlpha());
	}

	ColorRGBA ColorRGBA::RANDOM_COLOR(const int alpha)
	{
		return ColorRGBA(int(UTILS::RandomNumber(0.f, 255.f)), int(UTILS::RandomNumber(0.f, 255.f)),
		                 int(UTILS::RandomNumber(0.f, 255.f)), alpha);
	}

	ColorRGBA ColorRGBA::BLACK(const int alpha) { return ColorRGBA(0, 0, 0, alpha); }
	ColorRGBA ColorRGBA::WHITE(const int alpha) { return ColorRGBA(255, 255, 255, alpha); }
	ColorRGBA ColorRGBA::RED(const int alpha) { return ColorRGBA(255, 0, 0, alpha); }
	ColorRGBA ColorRGBA::GREEN(const int alpha) { return ColorRGBA(0, 255, 0, alpha); }
	ColorRGBA ColorRGBA::BLUE(const int alpha) { return ColorRGBA(0, 0, 255, alpha); }
	ColorRGBA ColorRGBA::PINK(const int alpha) { return ColorRGBA(255, 0, 150, alpha); }
	ColorRGBA ColorRGBA::MANGO(const int alpha) { return ColorRGBA(255, 181, 111, alpha); }


	/* ColorHSB */
	ColorHSBA::ColorHSBA()
	{
		SetAll(0.f);
		m_alpha = 0;
	}

	ColorHSBA::ColorHSBA(const float hue, const float saturation, const float brightness, const int alpha)
	{
		At(0) = hue;
		At(1) = saturation;
		At(2) = brightness;
		m_alpha = alpha;
	}

	ColorHSBA::ColorHSBA(const ColorRGBA& rgba)
	{
		const float R = rgba[0] / 255.f;
		const float G = rgba[1] / 255.f;
		const float B = rgba[2] / 255.f;

		const float mx = UTILS::Max(R, UTILS::Max(G, B));
		const float mn = UTILS::Min(R, UTILS::Min(G, B));

		const float delta = mx - mn;

		/// hue
		{
			if (mx == mn)
				At(0) = 0.f;
			else
			{
				if (mx == R)
					At(0) = (G - B) / delta;
				else if (mx == G)
					At(0) = 2.f + (B - R) / delta;
				else
					At(0) = 4.f + (R - G) / delta;

				At(0) *= 60.f;
				if (At(0) < 0.f)
					At(0) += 360.f;

				At(0) /= 360.f;
			}
		}
		/// saturation
		{
			if (mx == 0.f)
				At(1) = delta;
			else
				At(1) = delta / mx;
		}

		At(2) = UTILS::Max(R, UTILS::Max(G, B));
		m_alpha = rgba.At(3);
	}

	/// usage: Print(std::cout)
	void ColorHSBA::Print(std::ostream& stream) const
	{
		stream << At(0);
		for (size_t i = 1; i < 3; i++)
			stream << ", " << At(i);
		stream << ", " << int(m_alpha) << std::endl;
	}
}
