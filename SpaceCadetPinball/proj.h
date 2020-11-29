#pragma once
#include "maths.h"

struct vector_type4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct mat4_row_major
{
	vector_type4 Row0;
	vector_type4 Row1;
	vector_type4 Row2;
	vector_type4 Row3;
};


class proj
{
public:
	static void init(float* mat4x3, float d, float centerX, float centerY);
	static void matrix_vector_multiply(mat4_row_major* mat, vector_type* vec, vector_type* dstVec);
	static float z_distance(vector_type* vec);
	static void xform_to_2d(vector_type* vec, int* dst);
	static void recenter(float centerX, float centerY);
private:
	static mat4_row_major matrix;
	static float d_, centerx, centery;
};
