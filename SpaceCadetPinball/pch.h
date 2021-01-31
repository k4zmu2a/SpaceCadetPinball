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
#include <Windows.h>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <CommCtrl.h>
#include <htmlhelp.h>
#include <type_traits> /*For control template*/
//#include <cstdlib>

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
