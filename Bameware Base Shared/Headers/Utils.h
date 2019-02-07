#pragma once

#include <Windows.h>
#include <Wbemcli.h>
#include <cstdint>
#include <string>
#include <vector>


namespace BAMEWARE::UTILS
{
	template<class T>
	constexpr const T& Clamp(const T& v, const T& lo, const T& hi)
	{
		return (v >= lo && v <= hi) ? v : (v < lo ? lo : hi);
	}

	template<class T>
	constexpr const T& Min(const T& x, const T& y)
	{
		return (x > y) ? y : x;
	}

	template<class T>
	constexpr const T& Max(const T& x, const T& y)
	{
		return (x < y) ? y : x;
	}

	/// usage: FroceCompileTime<uint32_t, FNVHash("test")>()
	template <typename T, T Value>
	constexpr T ForceCompileTime()
	{
		return Value;
	}

	/// compile time fnv-a hash
	constexpr uint32_t FNVHash(const char* const data, const uint32_t value = 2166136261)
	{
		return (data[0] == '\0') ? value : (FNVHash(&data[1], (value * 16777619) ^ data[0]));
	}

	void AllocateConsole(const char* console_title);
	float RandomNumber(float min, float max);
	std::string GetLastErrorAsString();
}
