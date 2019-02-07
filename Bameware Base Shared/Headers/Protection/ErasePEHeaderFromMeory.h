#pragma once

#include <Windows.h>

namespace BAMEWARE::PROTECTION::ANTIDUMP
{
	void ErasePEHeaderFromMemory(HMODULE module);
}