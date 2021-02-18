#include "pch.h"
#include "proj.h"

mat4_row_major proj::matrix;
float proj::d_, proj::centerx, proj::centery;

void proj::init(float* mat4x3, float d, float centerX, float centerY)
{
	/*for (auto colIndex = 0; colIndex < 4; ++colIndex)
	{
		// Todo: out of bounds read from mat4x3?
		for (int rowIndex = colIndex, i = 4; i > 0; rowIndex += 4, --i)
		{
			((float*)&matrix)[rowIndex] = mat4x3[rowIndex];
		}
	}*/
	memcpy(&matrix, mat4x3, sizeof(float) * 4 * 3);

	matrix.Row3.X = 0.0;
	matrix.Row3.Y = 0.0;
	matrix.Row3.Z = 0.0;
	matrix.Row3.W = 1.0;

	d_ = d;
	centerx = centerX;
	centery = centerY;
}

void proj::matrix_vector_multiply(mat4_row_major* mat, vector_type* vec, vector_type* dstVec)
{
	const float x = vec->X, y = vec->Y, z = vec->Z;
	dstVec->X = z * mat->Row0.Z + y * mat->Row0.Y + x * mat->Row0.X + mat->Row0.W;
	dstVec->Y = z * mat->Row1.Z + y * mat->Row1.Y + x * mat->Row1.X + mat->Row1.W;
	dstVec->Z = z * mat->Row2.Z + y * mat->Row2.Y + x * mat->Row2.X + mat->Row2.W;
}

float proj::z_distance(vector_type* vec)
{
	vector_type dstVec{};
	matrix_vector_multiply(&matrix, vec, &dstVec);
	return maths::magnitude(&dstVec);
}

void proj::xform_to_2d(vector_type* vec, int* dst)
{
	float projCoef;
	vector_type dstVec2{};

	matrix_vector_multiply(&matrix, vec, &dstVec2);
	if (dstVec2.Z == 0.0f)
		projCoef = 999999.88f;
	else
		projCoef = d_ / dstVec2.Z;
	dst[0] = static_cast<int>(dstVec2.X * projCoef + centerx);
	dst[1] = static_cast<int>(dstVec2.Y * projCoef + centery);
}

void proj::recenter(float centerX, float centerY)
{
	centerx = centerX;
	centery = centerY;
}
