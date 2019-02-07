#pragma once


namespace BAMEWARE::CRYPTOGRAPHY
{
	/// will break if both x and y are zero
	template <typename T>
	constexpr T IgnoreZeroMultiply(const T& x, const T& y)
	{
		if (x == T(0))
			return y;
		if (y == T(0))
			return x;

		return x * y;
	}

	/// usage example: CompileTimeRandom<int32_t, 10, 100>().value
	template <typename T, T factor = 1, T offset = 0>
	struct CompileTimeRandom
	{
		static constexpr T value = offset +
			T((__TIME__[0] * 10) + (__TIME__[1] * 50) + (__TIME__[3] * 100) + (__TIME__[4] * 500) + (__TIME__[6] * 1000) + (__TIME__[7] * 10000) +
			(__TIME__[7] * factor) + (__TIME__[6] * factor * 2) + (__TIME__[4] * factor * 3) + (__TIME__[3] * factor * 4) + (__TIME__[1] * factor * 5) + (__TIME__[0] * factor * 6) +
				(IgnoreZeroMultiply<T>(__TIME__[7], __TIME__[6]) + IgnoreZeroMultiply<T>(__TIME__[4], __TIME__[3]) + IgnoreZeroMultiply<T>(__TIME__[1], __TIME__[0])));
	};
}