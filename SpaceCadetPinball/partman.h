#pragma once

struct zmap_header_type;
struct gdrv_bitmap8;


enum class bmp8Flags : unsigned char
{
	RawBmpUnaligned = 1 << 0,
	DibBitmap = 1 << 1,
	Spliced = 1 << 2,
};


#pragma pack(push, 1)
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

struct dat8BitBmpHeader
{
	uint8_t Resolution;
	int16_t Width;
	int16_t Height;
	int16_t XPosition;
	int16_t YPosition;
	int Size;
	bmp8Flags Flags;

	bool IsFlagSet(bmp8Flags flag) const
	{
		return static_cast<char>(Flags) & static_cast<char>(flag);
	}
};

struct dat16BitBmpHeader
{
	int16_t Width;
	int16_t Height;
	int16_t Stride;
	int Unknown0;
	int16_t Unknown1_0;
	int16_t Unknown1_1;
};
#pragma pack(pop)

static_assert(sizeof(dat8BitBmpHeader) == 14, "Wrong size of dat8BitBmpHeader");
static_assert(sizeof(datFileHeader) == 183, "Wrong size of datFileHeader");
static_assert(sizeof(dat16BitBmpHeader) == 14, "Wrong size of zmap_header_type");


class partman
{
public:
	static struct DatFile* load_records(LPCSTR lpFileName, bool fullTiltMode);
private:
	static short _field_size[];
	static char _lread_char(FILE* file);
	static int _lread_long(FILE* file);
};
