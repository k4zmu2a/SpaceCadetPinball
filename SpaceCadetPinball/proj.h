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
	static void matrix_vector_multiply(mat4_row_major* mat, vector3* vec, vector3* dstVec);
	static float z_distance(vector3* vec);
	static void xform_to_2d(vector3* vec, int* dst);
	static void recenter(float centerX, float centerY);
private:
	static mat4_row_major matrix;
	static float d_, centerx, centery;
};
