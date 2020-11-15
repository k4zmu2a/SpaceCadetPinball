#pragma once


struct __declspec(align(4)) rectangle_type
{
	int XPosition;
	int YPosition;
	int Width;
	int Height;
};

class maths
{
public:
	static void enclosing_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
	static int rectangle_clip(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
	static int overlapping_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect);
};
