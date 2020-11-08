#pragma once

#pragma pack(push, 1)
struct __declspec(align(1)) zmap_header_type
{
	__int16 Width;
	__int16 Height;
	__int16 Stride;
	char* BmpBufPtr1;
	char* bmpBufPtr2;
	char BmpBuffer[1];
};
#pragma pack(pop)

static_assert(sizeof(zmap_header_type) == 15, "Wrong size of zmap_header_type");

class zdrv
{
public:
	
};

