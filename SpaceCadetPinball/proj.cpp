#include "pch.h"
#include "proj.h"

mat4_row_major proj::matrix;
float proj::d_, proj::centerx, proj::centery;
float proj::zscaler, proj::zmin, proj::zmax;

void proj::init(float* mat4x3, float d, float centerX, float centerY, float zMin, float zScaler)
{
	memcpy(&matrix, mat4x3, sizeof(float) * 4 * 3);

	matrix.Row3.X = 0.0;
	matrix.Row3.Y = 0.0;
	matrix.Row3.Z = 0.0;
	matrix.Row3.W = 1.0;

	d_ = d;
	centerx = centerX;
	centery = centerY;

	zscaler = zScaler;
	zmin = zMin;
	zmax = static_cast<float>(0xffFFffFF) / zScaler + zMin;
}

vector3 proj::matrix_vector_multiply(const mat4_row_major& mat, const vector3& vec)
{
	vector3 dstVec;
	const float x = vec.X, y = vec.Y, z = vec.Z;
	dstVec.X = z * mat.Row0.Z + y * mat.Row0.Y + x * mat.Row0.X + mat.Row0.W;
	dstVec.Y = z * mat.Row1.Z + y * mat.Row1.Y + x * mat.Row1.X + mat.Row1.W;
	dstVec.Z = z * mat.Row2.Z + y * mat.Row2.Y + x * mat.Row2.X + mat.Row2.W;
	return dstVec;
}

float proj::z_distance(const vector3& vec)
{
	auto projVec = matrix_vector_multiply(matrix, vec);
	return maths::magnitude(projVec);
}

vector2i proj::xform_to_2d(const vector2& vec)
{
	vector3 vec3{ vec.X, vec.Y, 0 };
	return xform_to_2d(vec3);
}

vector3 proj::ReverseXForm(const vector2i& vec)
{
	// Pinball perspective projection matrix, the same for all tables resolutions:
	// X: 1.000000      Y: 0.000000      Z: 0.000000      W: 0.000000
	// X: 0.000000      Y: -0.913545     Z: 0.406737      W: 3.791398
	// X: 0.000000      Y: -0.406737     Z: -0.913545     W: 24.675402
	// X: 0.000000      Y: 0.000000      Z: 0.000000      W: 1.000000
	// Let A = -0.913545, B = 0.406737, F = 3.791398, G = 24.675402
	// Then forward projection can be expressed as:
	// x1 = x0
	// y1 = y0 * A + z0 * B + F
	// z1 = -y0 * B + z0 * A + G
	// x2 = x1 / z1 = x0 / z1
	// y2 = y1 / z1
	// z2 = z1 / z1 = 1

	// Reverse projection: find x0, y0, z0 given x2 and y2
	// y0 from y2 and z0, based on substitution in y2 = y1 / z1
	// y0 =  (y2 * (A * z0 + G) - B * z0 - F)/(A + B * y2)
	// x0 from x2, y0 and z0,  based on substitution in x2 = x0 / z1
	// x0 = (x2 * (A * z0 - B * y0 + G)
	// This pair of equations is solvable with fixed z0; most commonly z0 = 0

	// PB projection also includes scaling and offset, this can be undone before the main calculations
	// x2 = x0 * D / z1 + cX
	// x0 = ((x2 - cX) / D) * z1
	const auto A = matrix.Row1.Y, B = matrix.Row1.Z, F = matrix.Row1.W, G = matrix.Row2.W, D = d_;
	const auto x2 = (vec.X - centerx) /D, y2 = (vec.Y - centery)/D, z0 = 0.0f;

	auto y0 = (y2 * (A * z0 + G) - B * z0 - F) / (A + B * y2);
	auto x0 = x2 * (A * z0 - B * y0 + G);
	return {x0, y0, z0};
}

vector2i proj::xform_to_2d(const vector3& vec)
{
	float projCoef;

	auto projVec = matrix_vector_multiply(matrix, vec);
	if (projVec.Z == 0.0f)
		projCoef = 999999.88f;
	else
		projCoef = d_ / projVec.Z;
	return
	{
		static_cast<int>(projVec.X * projCoef + centerx),
		static_cast<int>(projVec.Y * projCoef + centery)
	};
}

void proj::recenter(float centerX, float centerY)
{
	centerx = centerX;
	centery = centerY;
}

uint16_t proj::NormalizeDepth(float depth)
{
	uint16_t result = 0;
	if (depth >= zmin)
	{
		auto depthScaled = (depth - zmin) * zscaler;
		if (depthScaled <= zmax)
			result = static_cast<uint16_t>(depthScaled);
		else
			result = 0xffFF;
	}
	return result;
}
