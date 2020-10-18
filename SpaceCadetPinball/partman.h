#pragma once

#pragma pack(push)
#pragma pack(1)
struct  datFileHeader
{
	char FileSignature[21];
	char AppName[50];
	char Description[100];
	int FileSize;
	unsigned short NumberOfGroups;
	int SizeOfBody;
	unsigned short Unknown;
};
#pragma pack(pop)
static_assert(sizeof(datFileHeader) == 183, "Wrong size of datFileHeader");

struct datFileStruct
{
	unsigned short NumberOfGroups;
	char* Description;
	char** GroupData;
};



enum datFieldTypes
{
	ShortValue = 0,//, does not have the 32bits size value, but a 16bits value(see above).
	Bitmap8bit = 1,//             8 bpp bitmap
	Unknown2 = 2,
	GroupName = 3,//             Group name
	Unknown4 = 4,
	Palette = 5,//             Palette(1 color is 1 DWORD, only present 1 time in PINBALL.DAT ,with a data size of 1024 bytes for 256 colors.Some colors are 0 ,because their indexes are reserved by Windows.)
	Unknown6 = 6,
	Unknown7 = 7,
	Unknown8 = 8,
	String = 9,//             String(content)
	ShortArray = 10,//             Array of 16bits integer values
	FloatArray = 11,//             Array of 32bits floating point values(collision box, ...)
	Bitmap16bit = 12,//             16 bpp bitmap(Heightmap ? )
};

//typedef const char* LPCSTR;
class partman
{
public:
	static datFileStruct* load_records(LPCSTR lpFileName);

	static int make_path_name(LPSTR lpFilename, LPCSTR lpString2, int nSize = 0x12Cu);
	
private:
	static short _field_size[];
};




