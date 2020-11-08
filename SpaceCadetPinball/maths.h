#pragma once


struct __declspec(align(4)) visual_rect
{
	int XPosition;
	int YPosition;
	int Width;
	int Height;
};

class maths
{
public:
	static void enclosing_box(visual_rect* rect1, visual_rect* rect2, visual_rect* dstRect);
	
};

