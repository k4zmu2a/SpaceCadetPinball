#include "pch.h"
#include "loader.h"

#include "partman.h"
#include "pinball.h"


/*_loader_errors  dd 0, offset aBadHandle, 1, offset aNoTypeField, 2, offset aNoAttributesFi

			 dd 0Bh, offset aNoFloatAttribu, 3, offset aWrongTypeMater; "Unknown" ...
			 dd 4, offset aWrongTypeKicke, 5, offset aWrongTypeAnObj, 6
			   dd offset aWrongTypeAStat, 7, offset aStatesReDefine, 9, offset aUnrecognizedAt
			   dd 0Ah, offset aUnrecognizedFl, 0Dh, offset aFloatAttribute
			   dd 0Ch, offset aStateIndexOutO, 0Fh, offset aLoaderMaterial_0
			   dd 0Eh, offset aLoaderKickerRe, 10h, offset aLoaderStateIdR
			   dd 8, offset aWallsDoesnTMat, 11h, offset aLoaderQueryVis_0
			   dd 12h, offset aLoaderQueryVis, 15h, offset aLoaderMaterial
			   dd 14h, offset aLoaderKicker, 16h, offset aLoaderQueryAtt
			   dd 17h, offset aLoaderQueryIat, 13h, offset aLoaderQueryNam
			   dd 18h, offset aLoaderStateId, 19h, offset aLoaderGetSound
			   dd 1Ah, offset aSoundReference, 0FFFFFFFFh, offset aUnknown*/


errorMsg loader::loader_errors[] = {
	errorMsg{0, "Bad Handle"},
	errorMsg{1, "No Type Field"},
	errorMsg{2, "No Attributes Field"},
	errorMsg{0x0B, "No float Attributes Field"},
	errorMsg{3, "Wrong Type: MATERIAL Expected"},
	errorMsg{4, "Wrong Type: KICKER Expected"},
	errorMsg{5, "Wrong Type: AN_OBJECT Expected"},
	errorMsg{6, "Wrong Type: A_STATE Expected"},
	errorMsg{7, "STATES (re)defined in a state"},
	errorMsg{9, "Unrecognized Attribute"},
	errorMsg{0x0A, "Unrecognized float Attribute"},
	errorMsg{0x0D, "float Attribute not found"},
	errorMsg{0x0C, "state_index out of range"},
	errorMsg{0x0F, "loader_material() reports failure"},
	errorMsg{0x0E, "loader_kicker() reports failure"},
	errorMsg{0x10, "loader_state_id() reports failure"},
	errorMsg{0x8, "# walls doesn't match data size"},
	errorMsg{0x11, "loader_query_visual_states()"},
	errorMsg{0x12, "loader_query_visual()"},
	errorMsg{0x15, "loader_material()"},
	errorMsg{0x14, "loader_kicker()"},
	errorMsg{0x16, "loader_query_attribute()"},
	errorMsg{0x17, "loader_query_iattribute()"},
	errorMsg{0x13, "loader_query_name()"},
	errorMsg{0x18, "loader_state_id()"},
	errorMsg{0x19, "loader_get_sound_id()"},
	errorMsg{0x1A, "sound reference is not A_SOUND record"},
	errorMsg{-1, "Unknown"},
};

int loader::sound_count = 1;
int loader::loader_sound_count;
datFileStruct* loader::loader_table;
datFileStruct* loader::sound_record_table;
soundListStruct loader::sound_list[65];

int loader::error(int errorCode, int captionCode)
{
	int curCode = loader_errors[0].Code;
	const char *errorText = nullptr, *errorCaption = nullptr;
	int index = 0, index2 = 0;
	if (loader_errors[0].Code >= 0)
		do
		{
			if (errorCode == curCode)
				errorText = loader_errors[index2].Message;
			if (captionCode == curCode)
				errorCaption = loader_errors[index2].Message;
			index2 = ++index;
			curCode = loader_errors[index].Code;
		}
		while (curCode >= 0);
	if (!errorText)
		errorText = loader_errors[index].Message;
	MessageBoxA(nullptr, errorText, errorCaption, 0x2000u);
	return -1;
}

void loader::default_vsi(int* arr)
{
	arr[13] = 0;
	arr[5] = 1369940824;
	arr[12] = 0;
	arr[0] = 1064514355;
	arr[1] = 1058642330;
	arr[2] = 0;
	arr[4] = 0;
	arr[16] = 0;
	arr[17] = 0;
	arr[15] = 0;
	arr[14] = 0;
}

void loader::loadfrom(datFileStruct* datFile)
{
	int groupIndex = 0;
	loader_table = datFile;
	sound_record_table = loader_table;
	int soundIndex = sound_count;
	if (datFile->NumberOfGroups > 0)
	{
		do
		{
			__int16* value = (__int16*)partman::field(datFile, groupIndex, ShortValue);
			if (value && *value == 202)
			{
				soundIndex = sound_count;
				if (sound_count < 65)
				{
					sound_list[soundIndex].WavePtr = nullptr;
					sound_list[soundIndex].GroupIndex = groupIndex;
					sound_count = ++soundIndex;
				}
			}
			++groupIndex;
		}
		while (groupIndex < datFile->NumberOfGroups);
	}
	loader_sound_count = soundIndex;
}

void loader::unload()
{
	int index = 1;
	if (sound_count > 1)
	{
		soundListStruct* soundListPtr = &sound_list[1];
		do
		{
			//Sound_FreeSound(soundListPtr->WavePtr);
			++index;
			++soundListPtr;
		}
		while (index < sound_count);
	}
	if (sound_list[index].PtrToSmth)
		memoryfree(sound_list[index].PtrToSmth);
	sound_count = 1;
}

int loader::get_sound_id(int groupIndex)
{
	int result;
	__int16 soundIndex = 1;
	if (sound_count <= 1)
	{
		error(25, 26);
		result = -1;
	}
	else
	{
		while (sound_list[soundIndex].GroupIndex != groupIndex)
		{
			++soundIndex;
			if (soundIndex >= sound_count)
			{
				error(25, 26);
				return -1;
			}
		}
		if (!sound_list[soundIndex].Loaded && !sound_list[soundIndex].WavePtr)
		{
			int soundGroupId = sound_list[soundIndex].GroupIndex;
			sound_list[soundIndex].Volume = 0.0;
			if (soundGroupId > 0 && !pinball::quickFlag)
			{
				__int16* value = (__int16*)partman::field(loader_table, soundGroupId, ShortValue);
				if (value && *value == 202)
				{
					const CHAR* fileName = partman::field(loader_table, soundGroupId, String);
					HFILE hFile = _lopen(fileName, 0);
					sound_list[soundIndex].Volume = static_cast<double>(_llseek(hFile, 0, 2)) * 0.0000909090909090909;
					_lclose(hFile);
					//sound_list[soundIndex4].WavePtr = Sound_LoadWaveFile(lpName);
				}
			}
		}
		++sound_list[soundIndex].Loaded;
		result = soundIndex;
	}
	return result;
}


int loader::query_handle(LPCSTR lpString)
{
	return partman::record_labeled(loader_table, lpString);
}

short loader::query_visual_states(int groupIndex)
{
	short result;
	if (groupIndex < 0)
		return error(0, 17);
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, ShortArray);
	if (shortArr && *shortArr == 100)
		result = shortArr[1];
	else
		result = 1;
	return result;
}

char* loader::query_name(int groupIndex)
{
	if (groupIndex >= 0)
		return partman::field(loader_table, groupIndex, GroupName);
	error(0, 19);
	return nullptr;
}

__int16* loader::query_iattribute(int groupIndex, int firstValue, int* arraySize)
{
	__int16* result;
	__int16 skipIndex = 0;
	if (groupIndex >= 0)
	{
		while (true)
		{
			__int16* shortArr = (__int16*)partman::field_nth(loader_table, groupIndex, ShortArray, skipIndex);
			if (!shortArr)
				break;
			if (*shortArr == firstValue)
			{
				*arraySize = partman::field_size(loader_table, groupIndex, ShortArray) / 2 - 1;
				return shortArr + 1;
			}
			++skipIndex;
		}
		error(2, 23);
		*arraySize = 0;
		result = nullptr;
	}
	else
	{
		error(0, 22);
		result = nullptr;
	}
	return result;
}

float* loader::query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue)
{
	float* result;
	__int16 skipIndex = 0;
	if (groupIndex >= 0)
	{
		int groupIndexSum = state_id(groupIndex, groupIndexOffset);
		if (groupIndexSum >= 0)
		{
			while (true)
			{
				float* floatArr = (float*)partman::field_nth(loader_table, groupIndexSum, FloatArray, skipIndex);
				if (!floatArr)
					break;
				if (static_cast<__int16>(static_cast<__int64>(floor(*floatArr))) == firstValue)
					return floatArr + 1;
				++skipIndex;
			}
			error(13, 22);
			result = nullptr;
		}
		else
		{
			error(16, 22);
			result = nullptr;
		}
	}
	else
	{
		error(0, 22);
		result = nullptr;
	}
	return result;
}


double loader::play_sound(int soundIndex)
{
	if (soundIndex <= 0)
		return 0.0;
	//Sound_PlaySound(sound_list[soundIndex].WavePtr, 0, 7, 5, 0);
	return sound_list[soundIndex].Volume;
}

int loader::state_id(int groupIndex, int groupIndexOffset)
{
	int result;
	int groupIndex2 = groupIndex;
	__int16 visualState = query_visual_states(groupIndex);
	if (visualState <= 0)
		return error(12, 24);
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, ShortValue);
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 200)
		return error(5, 24);
	if (groupIndexOffset > visualState)
		return error(12, 24);
	if (!groupIndexOffset)
		return groupIndex2;

	groupIndex2 = groupIndexOffset + groupIndex;
	shortArr = (__int16*)partman::field(loader_table, groupIndexOffset + groupIndex, ShortValue);
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 201)
		result = error(6, 24);
	else
		result = groupIndex2;
	return result;
}
