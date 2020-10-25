#pragma once


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
	float Volume;
	char* PtrToSmth;
};


class loader
{
public:
	static int error(int errorCode, int captionCode);
	static void default_vsi(int* arr);
	static void loadfrom(datFileStruct* datFile);
	static void unload();
	static int get_sound_id(int groupIndex);
	static int query_handle(LPCSTR lpString);
	static short query_visual_states(int groupIndex);
	static char* query_name(int groupIndex);
	static double play_sound(int soundIndex);
	static __int16* query_iattribute(int groupIndex, int firstValue, int* arraySize);
	static float* query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue);
	static int state_id(int groupIndex, int groupIndexOffset);
private:
	static errorMsg loader_errors[];
	static datFileStruct* loader_table;
	static datFileStruct* sound_record_table;
	static int sound_count;
	static int loader_sound_count;
	static soundListStruct sound_list[65];
};
