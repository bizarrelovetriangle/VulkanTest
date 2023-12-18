#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <memory>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <malloc.h>
#include <locale.h>
#include <math.h>
#include <vulkan/vulkan.hpp>


// Windows SDK
#define _WIN32_WINNT 0x0501     // _WIN32_WINNT_WINXP
#include <SDKDDKVer.h>

// Windows API
#define WIN32_LEAN_AND_MEAN
#include <windows.h>