#pragma once
#include "gdrv.h"
#include "maths.h"
#include "zdrv.h"


struct DatFile;

struct errorMsg
{
	int Code;
	const char* Message;
};

struct soundListStruct
{
	Mix_Chunk* WavePtr;
	int GroupIndex;
	int Loaded;
	float Duration;
};

struct visualKickerStruct
{
	float Threshold;
	float Boost;
	float ThrowBallMult;
	vector_type ThrowBallAcceleration;
	float ThrowBallAngleMult;
	int HardHitSoundId;
};


struct visualStruct
{
	float Smoothness;
	float Elasticity;
	int FloatArrCount;
	float* FloatArr;
	int SoftHitSoundId;
	visualKickerStruct Kicker;
	int CollisionGroup;
	int SoundIndex4;
	int SoundIndex3;
	gdrv_bitmap8* Bitmap;
	zmap_header_type* ZMap;
};

#pragma pack(push)
#pragma pack(1)
// WAVE file header format
struct WaveHeader
{
	unsigned char riff[4]; // RIFF string

	unsigned int overall_size; // overall size of file in bytes

	unsigned char wave[4]; // WAVE string

	unsigned char fmt_chunk_marker[4]; // fmt string with trailing null char

	unsigned int length_of_fmt; // length of the format data

	unsigned short format_type; // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law

	unsigned short  channels; // no.of channels

	unsigned int sample_rate; // sampling rate (blocks per second)

	unsigned int byterate; // SampleRate * NumChannels * BitsPerSample/8

	unsigned short block_align; // NumChannels * BitsPerSample/8

	unsigned short bits_per_sample; // bits per sample, 8- 8bits, 16- 16 bits etc

	unsigned char data_chunk_header[4]; // DATA string or FLLR string

	unsigned int data_size; // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};
#pragma pack(pop)
static_assert(sizeof(WaveHeader) == 44, "Wrong size of WaveHeader");


class loader
{
public:
	static int error(int errorCode, int captionCode);
	static void default_vsi(visualStruct* visual);
	static int get_sound_id(int groupIndex);
	static void unload();
	static void loadfrom(DatFile* datFile);
	static int query_handle(LPCSTR lpString);
	static short query_visual_states(int groupIndex);
	static int material(int groupIndex, visualStruct* visual);
	static int kicker(int groupIndex, visualKickerStruct* kicker);
	static int state_id(int groupIndex, int groupIndexOffset);
	static int query_visual(int groupIndex, int groupIndexOffset, visualStruct* visual);
	static char* query_name(int groupIndex);
	static float* query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue);
	static float query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue, float defVal);
	static int16_t* query_iattribute(int groupIndex, int firstValue, int* arraySize);
	static float play_sound(int soundIndex);
	static DatFile* loader_table;
private:
	static errorMsg loader_errors[];
	static DatFile* sound_record_table;
	static int sound_count;
	static int loader_sound_count;
	static soundListStruct sound_list[65];
};
