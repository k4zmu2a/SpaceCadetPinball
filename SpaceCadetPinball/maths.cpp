#include "pch.h"
#include "maths.h"


void maths::enclosing_box(visual_rect* rect1, visual_rect* rect2, visual_rect* dstRect)
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