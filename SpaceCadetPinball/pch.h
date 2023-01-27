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
#include <unordered_map>
#include <initializer_list>
//#include <array>

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <SDL_mixer.h>

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer v2.0.2
// Older versions of SDL_mixer did not have SDL_MIXER_VERSION_ATLEAST
constexpr int MIX_INIT_MID_Proxy =
#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2, 0, 2)
	MIX_INIT_MID;
#else
	MIX_INIT_FLUIDSYNTH;
#endif

//https://github.com/ocornut/imgui 59b63defe5421642fb0cdcfd1fa850fc85a13791 + patches
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"

// imgui_impl_sdlrenderer is faster and more accurate, but it requires newer SDL.
#if SDL_VERSION_ATLEAST(2, 0, 17)
#include "imgui_impl_sdlrenderer.h"
constexpr const char* ImGuiRender = "HW";
inline void ImGui_Render_Init(SDL_Renderer* renderer) { ImGui_ImplSDLRenderer_Init(renderer); }
inline void ImGui_Render_Shutdown() { ImGui_ImplSDLRenderer_Shutdown(); }
inline void ImGui_Render_NewFrame() { ImGui_ImplSDLRenderer_NewFrame(); }
inline void ImGui_Render_RenderDrawData(ImDrawData* draw_data) { ImGui_ImplSDLRenderer_RenderDrawData(draw_data); }
#else
//https://github.com/Tyyppi77/imgui_sdl 01deb04b102b6a1c15c7fdec1977a2c96a885e6f + patches
#include "imgui_sdl.h"
constexpr const char* ImGuiRender = "SW";
inline void ImGui_Render_Init(SDL_Renderer* renderer) { ImGuiSDL::Initialize(renderer, 0, 0); }
inline void ImGui_Render_Shutdown() { ImGuiSDL::Deinitialize(); }
inline void ImGui_Render_NewFrame() { }
inline void ImGui_Render_RenderDrawData(ImDrawData* draw_data) { ImGuiSDL::Render(draw_data); }
#endif


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

template <typename T>
constexpr int Sign(T val)
{
	return (T(0) < val) - (val < T(0));
}

template <typename T>
const T& Clamp(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
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

constexpr float Pi = 3.14159265358979323846f;

#endif //PCH_H
