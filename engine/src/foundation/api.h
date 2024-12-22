#pragma once

#ifdef foundation_dll_export
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif