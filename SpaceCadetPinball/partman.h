#pragma once

enum class datFieldTypes : __int16
{
	ShortValue = 0,
	//, does not have the 32bits size value, but a 16bits value(see above).
	Bitmap8bit = 1,
	//             8 bpp bitmap
	Unknown2 = 2,
	GroupName = 3,
	//             Group name
	Unknown4 = 4,
	Palette = 5,
	//             Palette(1 color is 1 DWORD, only present 1 time in PINBALL.DAT ,with a data size of 1024 bytes for 256 colors.Some colors are 0 ,because their indexes are reserved by Windows.)
	Unknown6 = 6,
	Unknown7 = 7,
	Unknown8 = 8,
	String = 9,
	//             String(content)
	ShortArray = 10,
	//             Array of 16bits integer values
	FloatArray = 11,
	//             Array of 32bits floating point values(collision box, ...)
	Bitmap16bit = 12,
	//             16 bpp bitmap(Heightmap ? )
};

#pragma pack(push)
#pragma pack(1)
struct datFileHeader
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

struct datEntryData
{
	datFieldTypes EntryType;
	int FieldSize;
	char* Buffer;
};

struct datGroupData
{
	__int16 EntryCount;
	datEntryData Entries[1];
};

struct datFileStruct
{
	unsigned short NumberOfGroups;
	char* Description;
	datGroupData** GroupData;
};

#pragma pack(push)
#pragma pack(1)
struct dat8BitBmpHeader
{
	char Unknown1;
	__int16 Width;
	__int16 Height;
	__int16 XPosition;
	__int16 YPosition;
	int Size;
	char Unknown2;
};

#pragma pack(pop)
static_assert(sizeof(dat8BitBmpHeader) == 14, "Wrong size of dat8BitBmpHeader");


//typedef const char* LPCSTR;
class partman
{
public:
	static datFileStruct* load_records(LPCSTR lpFileName);
	static void unload_records(datFileStruct* datFile);
	static char* field_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN);
	static char* field(datFileStruct* datFile, int groupIndex, datFieldTypes entryType);
	static int field_size_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN);
	static int field_size(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType);
	static int record_labeled(datFileStruct* datFile, LPCSTR targetGroupName);
	static char* field_labeled(datFileStruct* datFile, LPCSTR lpString, datFieldTypes fieldType);

	static int make_path_name(LPSTR lpFilename, LPCSTR lpString2, int nSize = 0x12Cu);

private:
	static short _field_size[];
	static char _lread_char(HFILE hFile);
	static int _lread_long(HFILE hFile);
};
