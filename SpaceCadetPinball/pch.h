// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H


// GCC does not have *_s functions
#define _CRT_SECURE_NO_WARNINGS

// TODO: add headers that you want to pre-compile here
#include <cstdio>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <type_traits> /*For control template*/
#include <chrono>
#include <iostream>
//#include <iomanip>
//#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <thread>
#include <map>
//#include <array>

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <SDL_mixer.h>

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer v2.0.2
constexpr int MIX_INIT_MID_Proxy =
#if SDL_MIXER_PATCHLEVEL >= 2
	MIX_INIT_MID;
#else
	MIX_INIT_FLUIDSYNTH;
#endif

//https://github.com/ocornut/imgui 7b913db1ce9dd2fd98e5790aa59974dd4496be3b
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
//https://github.com/Tyyppi77/imgui_sdl 01deb04b102b6a1c15c7fdec1977a2c96a885e6f
#include "imgui_sdl.h"

typedef char* LPSTR;
typedef const char* LPCSTR;

constexpr char PathSeparator =
#ifdef _WIN32
'\\';
#else
'/';
#endif


/*Use (void) to silent unused warnings.*/
#define assertm(exp, msg) assert(((void)msg, exp))


inline size_t pgm_save(int width, int height, char* data, FILE* outfile)
{
	size_t n = 0;
	n += fprintf(outfile, "P5\n%d %d\n%d\n", width, height, 0xFF);
	n += fwrite(data, 1, width * height, outfile);
	return n;
}

inline float RandFloat()
{
	return static_cast<float>(std::rand() / static_cast<double>(RAND_MAX));
}

template <typename T> constexpr
int Sign(T val)
{
	return (T(0) < val) - (val < T(0));
}

// UTF-8 path adapter for fopen on Windows, implemented in SpaceCadetPinball.cpp
#ifdef _WIN32
extern FILE* fopenu(const char* path, const char* opt);
#else
inline FILE* fopenu(const char* path, const char* opt)
{
	return fopen(path, opt);
}
#endif

// Platform specific data paths not found in SDL
constexpr const char* PlatformDataPaths[2] = 
{
	#ifdef _WIN32
	nullptr
	#else
	"/usr/local/share/SpaceCadetPinball/",
	"/usr/share/SpaceCadetPinball/"
	#endif
};

#endif //PCH_H
