#include "pch.h"
#include "maths.h"


void maths::enclosing_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int xPos1 = rect1->XPosition;
	int yPos1 = rect1->YPosition;
	int width1 = rect1->Width;
	int height1 = rect1->Height;
	int xPos2 = rect2->XPosition;
	bool rect2XPosLessRect1 = rect2->XPosition < rect1->XPosition;
	int yPos2 = rect2->YPosition;
	int width2 = rect2->Width;
	int height2 = rect2->Height;
	int xPos2_2 = rect2->XPosition;
	if (rect2XPosLessRect1)
	{
		width1 += xPos1 - xPos2;
		xPos1 = xPos2;
	}
	if (yPos2 < yPos1)
	{
		height1 += yPos1 - yPos2;
		yPos1 = yPos2;
	}
	if (width2 + xPos2 > xPos1 + width1)
		width1 = xPos2_2 + width2 - xPos1;
	int height1_2 = height1;
	if (height2 + yPos2 > height1 + yPos1)
		height1_2 = yPos2 + height2 - yPos1;
	dstRect->YPosition = yPos1;
	dstRect->Height = height1_2;
	dstRect->XPosition = xPos1;
	dstRect->Width = width1;
}


int maths::rectangle_clip(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int xPos1 = rect1->XPosition;
	int yPos1 = rect1->YPosition;
	int height1 = rect1->Height;
	int xRight2 = rect2->XPosition + rect2->Width;
	int width1 = rect1->Width;
	int yRight2 = rect2->YPosition + rect2->Height;
	if (xPos1 + width1 < rect2->XPosition)
		return 0;
	if (xPos1 >= xRight2)
		return 0;
	int yPos2 = yPos1;
	if (yPos1 + height1 < rect2->YPosition || yPos1 >= yRight2)
		return 0;
	if (xPos1 < rect2->XPosition)
	{
		width1 += xPos1 - rect2->XPosition;
		xPos1 = rect2->XPosition;
	}
	if (xPos1 + width1 > xRight2)
		width1 = xRight2 - xPos1;
	int height2 = height1;
	if (yPos1 < rect2->YPosition)
	{
		height2 = yPos1 - rect2->YPosition + height1;
		yPos2 = rect2->YPosition;
	}
	if (height2 + yPos2 > yRight2)
		height2 = yRight2 - yPos2;
	if (!width1 || !height2)
		return 0;
	if (dstRect)
	{
		dstRect->XPosition = xPos1;
		dstRect->YPosition = yPos2;
		dstRect->Width = width1;
		dstRect->Height = height2;
	}
	return 1;
}


int maths::overlapping_box(rectangle_type* rect1, rectangle_type* rect2, rectangle_type* dstRect)
{
	int v3; // esi
	int v4; // edi
	int v5; // esi
	int v6; // esi
	int v7; // edi

	if (rect1->XPosition >= rect2->XPosition)
	{
		dstRect->XPosition = rect2->XPosition;
		v3 = rect1->Width - rect2->XPosition;
		v4 = rect1->XPosition;
	}
	else
	{
		dstRect->XPosition = rect1->XPosition;
		v3 = rect2->Width - rect1->XPosition;
		v4 = rect2->XPosition;
	}
	dstRect->Width = v3 + v4 + 1;
	v5 = rect1->YPosition;
	if (v5 >= rect2->YPosition)
	{
		dstRect->YPosition = rect2->YPosition;
		v6 = rect1->Height - rect2->YPosition;
		v7 = rect1->YPosition;
	}
	else
	{
		dstRect->YPosition = v5;
		v6 = rect2->Height - rect1->YPosition;
		v7 = rect2->YPosition;
	}
	dstRect->Height = v6 + v7 + 1;
	return dstRect->Width <= rect2->Width + rect1->Width && dstRect->Height <= rect2->Height + rect1->Height;
}
