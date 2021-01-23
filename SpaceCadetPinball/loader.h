#pragma once
#include "gdrv.h"
#include "maths.h"
#include "zdrv.h"


struct datFileStruct;

struct errorMsg
{
	int Code;
	const char* Message;
};

struct soundListStruct
{
	char* WavePtr;
	int GroupIndex;
	int Loaded;
	float Duration;
	char* PtrToSmth;
};

struct __declspec(align(4)) visualKickerStruct
{
	float Threshold;
	float Boost;
	float ThrowBallMult;
	vector_type ThrowBallAcceleration;	
	float ThrowBallAngleMult;
	int HardHitSoundId;
};


struct __declspec(align(4)) visualStruct
{
	float Smoothness;
	float Elasticity;
	int FloatArrCount;
	float* FloatArr;
	int SoftHitSoundId;
	visualKickerStruct Kicker;
	int Flag;
	int SoundIndex4;
	int SoundIndex3;
	gdrv_bitmap8* Bitmap;
	zmap_header_type* ZMap;
};


class loader
{
public:
	static int error(int errorCode, int captionCode);
	static void default_vsi(visualStruct* visual);
	static int get_sound_id(int groupIndex);
	static void unload();
	static void loadfrom(datFileStruct* datFile);
	static int query_handle(LPCSTR lpString);
	static short query_visual_states(int groupIndex);
	static int material(int groupIndex, visualStruct* visual);
	static int kicker(int groupIndex, visualKickerStruct* kicker);
	static int state_id(int groupIndex, int groupIndexOffset);
	static int query_visual(int groupIndex, int groupIndexOffset, visualStruct* visual);
	static char* query_name(int groupIndex);
	static float* query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue);
	static __int16* query_iattribute(int groupIndex, int firstValue, int* arraySize);
	static float play_sound(int soundIndex);
	static datFileStruct* loader_table;
private:
	static errorMsg loader_errors[];	
	static datFileStruct* sound_record_table;
	static int sound_count;
	static int loader_sound_count;
	static soundListStruct sound_list[65];
};
