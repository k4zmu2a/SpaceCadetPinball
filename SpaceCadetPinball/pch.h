// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

// TODO: add headers that you want to pre-compile here
#include <cstdio>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <type_traits> /*For control template*/
#include <chrono>
//#include <iostream>
//#include <iomanip>
//#include <cstdlib>
#include <vector>
#include <algorithm>

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <SDL_mixer.h>

//https://github.com/ocornut/imgui 7b913db1ce9dd2fd98e5790aa59974dd4496be3b
#include "imgui.h"
#include "imgui_impl_sdl.h"
//https://github.com/Tyyppi77/imgui_sdl 01deb04b102b6a1c15c7fdec1977a2c96a885e6f
#include "imgui_sdl.h"

typedef unsigned long  DWORD;
typedef char* LPSTR;
typedef const char* LPCSTR;

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

/*Use (void) to silent unused warnings.*/
#define assertm(exp, msg) assert(((void)msg, exp))

/*Sound uses PlaySound*/
#undef PlaySound


inline size_t pgm_save(int width, int height, char* data, FILE* outfile)
{
	size_t n = 0;
	n += fprintf(outfile, "P5\n%d %d\n%d\n", width, height, 0xFF);
	n += fwrite(data, 1, width * height, outfile);
	return n;
}

#endif //PCH_H
