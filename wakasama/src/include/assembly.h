#pragma once

#include <windows.h>

inline void shld(DWORD &a, const DWORD &b, const DWORD count) {
  a = (a << count) | (b >> (32 - count));
}