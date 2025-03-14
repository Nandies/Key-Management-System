#ifndef WINDOWS_COMPATIBILITY_FIX_H
#define WINDOWS_COMPATIBILITY_FIX_H

// This file contains workarounds for common Windows-specific issues

// Fix for Visual Studio string conversion issues
#ifdef _MSC_VER
#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif
#endif

// Fix for Windows.h including min/max macros that conflict with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Ensure secure CRT functions are used
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Add common Windows headers
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>

// Workaround for C4996 warnings about deprecated functions
// Only include these if needed
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif
#endif

#endif // WINDOWS_COMPATIBILITY_FIX_H