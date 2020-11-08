#include "pch.h"
#include "loader.h"
#include "memory.h"
#include "partman.h"
#include "pinball.h"
#include "zdrv.h"


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

void  loader::default_vsi(visualStruct* visual)
{
	visual->Unknown14Flag = 0;
	visual->Kicker.Unknown1F = 8.9999999e10f;
	visual->Kicker.SoundIndex = 0;
	visual->Unknown1F = 0.94999999f;
	visual->Unknown2F = 0.60000002f;
	visual->FloatArrSizeDiv8Sub2 = 0;
	visual->SoundIndex2 = 0;
	visual->Bitmap = 0;
	visual->ZMap = 0;
	visual->SoundIndex3 = 0;
	visual->SoundIndex4 = 0;
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
			__int16* value = (__int16*)partman::field(datFile, groupIndex, datFieldTypes::ShortValue);
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
		memory::free(sound_list[index].PtrToSmth);
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
				__int16* value = (__int16*)partman::field(loader_table, soundGroupId, datFieldTypes::ShortValue);
				if (value && *value == 202)
				{
					const CHAR* fileName = partman::field(loader_table, soundGroupId, datFieldTypes::String);
					HFILE hFile = _lopen(fileName, 0);
					sound_list[soundIndex].Volume = (float)((double)(_llseek(hFile, 0, 2)) * 0.0000909090909090909);
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
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, datFieldTypes::ShortArray);
	if (shortArr && *shortArr == 100)
		result = shortArr[1];
	else
		result = 1;
	return result;
}

char* loader::query_name(int groupIndex)
{
	if (groupIndex >= 0)
		return partman::field(loader_table, groupIndex, datFieldTypes::GroupName);
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
			__int16* shortArr = (__int16*)partman::field_nth(loader_table, groupIndex, datFieldTypes::ShortArray, skipIndex);
			if (!shortArr)
				break;
			if (*shortArr == firstValue)
			{
				*arraySize = partman::field_size(loader_table, groupIndex, datFieldTypes::ShortArray) / 2 - 1;
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
				float* floatArr = (float*)partman::field_nth(loader_table, groupIndexSum, datFieldTypes::FloatArray, skipIndex);
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

int loader::material(int groupIndex, visualStruct* visual)
{
	if (groupIndex < 0)
		return error(0, 21);
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, datFieldTypes::ShortValue);
	if (!shortArr)
		return error(1, 21);
	if (*shortArr != 300)
		return error(3, 21);
	float* floatArr = (float*)partman::field(loader_table, groupIndex, datFieldTypes::FloatArray);
	if (!floatArr)
		return error(11, 21);
	int index = 0;
	int floatArrLength = partman::field_size(loader_table, groupIndex, datFieldTypes::FloatArray) >> 2;
	if (floatArrLength > 0)
	{
		do
		{
			float* nextFloatVal = floatArr + 1;
			switch (static_cast<int>(floor(*floatArr)))
			{
			case 301:
				visual->Unknown1F = *nextFloatVal;
				break;
			case 302:
				visual->Unknown2F = *nextFloatVal;
				break;
			case 304:
				visual->SoundIndex2 = get_sound_id((int)floor(*nextFloatVal));
				break;
			default:
				return error(9, 21);
			}
			floatArr = nextFloatVal + 1;
			index += 2;
		}
		while (index < floatArrLength);
	}
	return 0;
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
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, datFieldTypes::ShortValue);
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 200)
		return error(5, 24);
	if (groupIndexOffset > visualState)
		return error(12, 24);
	if (!groupIndexOffset)
		return groupIndex2;

	groupIndex2 = groupIndexOffset + groupIndex;
	shortArr = (__int16*)partman::field(loader_table, groupIndexOffset + groupIndex, datFieldTypes::ShortValue);
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 201)
		result = error(6, 24);
	else
		result = groupIndex2;
	return result;
}

int loader::kicker(int groupIndex, visualKickerStruct* kicker)
{
	if (groupIndex < 0)
		return error(0, 20);
	__int16* shortArr = (__int16*)partman::field(loader_table, groupIndex, datFieldTypes::ShortValue);
	if (!shortArr)
		return error(1, 20);
	if (*shortArr != 400)
		return error(4, 20);
	float* floatArr = (float*)partman::field(loader_table, groupIndex, datFieldTypes::FloatArray);
	if (!floatArr)
		return error(11, 20);
	int floatArrLength = partman::field_size(loader_table, groupIndex, datFieldTypes::FloatArray) >> 2;
	int index = 0;
	if (floatArrLength <= 0)
		return 0;
	while (index < floatArrLength)
	{
		int floorVal = static_cast<int>(floor(*floatArr++));
		switch (floorVal)
		{
		case 401:
			kicker->Unknown1F = *floatArr;
			break;
		case 402:
			kicker->Unknown2F = *floatArr;
			break;
		case 403:
			kicker->Unknown3F = *floatArr;
			break;
		case 404:
			kicker->Unknown4F = *floatArr++;
			kicker->Unknown5F = *floatArr++;
			kicker->Unknown6F = *floatArr++;
			index += 4;
			break;
		case 405:
			kicker->Unknown7F = *floatArr;
			break;
		case 406:
			kicker->SoundIndex = get_sound_id(static_cast<int>(floor(*floatArr)));
			break;
		default:
			return error(10, 20);
		}
		if (floorVal != 404)
		{
			floatArr++;
			index += 2;
		}
	}
	return 0;
}



int  loader::query_visual(int groupIndex, int groupIndexOffset, visualStruct* visual)
{
	visualStruct* visual2; // edi
	int groupIndexSum; // eax
	int groupIndexSum2; // ebx
	zmap_header_type* bitmap16; // eax
	__int16* shortArr; // esi
	unsigned int shortArrSize; // eax
	int index; // ebx
	int shortVal; // ecx
	__int16* nextShortVal; // esi
	int nextIndex; // ebx
	int shortValSub100; // ecx
	int shortValSub300; // ecx
	int shortValSub304; // ecx
	int shortValSub602; // ecx
	int shortValSub1100; // ecx
	int shortValSub1101; // ecx
	float* floatArr; // eax
	float* nextFloatVal; // esi
	__int64 floatVal; // rax
	float* floatArrPtr; // esi
	int groupIndexSum3; // [esp+1Ch] [ebp+8h]
	int shortArrLength; // [esp+24h] [ebp+10h]

	visual2 = visual;
	default_vsi(visual);
	if (groupIndex < 0)
		return error(0, 18);
	groupIndexSum = state_id(groupIndex, groupIndexOffset);
	groupIndexSum2 = groupIndexSum;
	groupIndexSum3 = groupIndexSum;
	if (groupIndexSum < 0)
		return error(16, 18);
	visual->Bitmap = (gdrv_bitmap8*)partman::field(loader_table, groupIndexSum, datFieldTypes::Bitmap8bit);
	bitmap16 = (zmap_header_type*)partman::field(loader_table, groupIndexSum2, datFieldTypes::Bitmap16bit);
	visual->ZMap = bitmap16;
	if (bitmap16)
	{
		bitmap16->BmpBufPtr1 = bitmap16->BmpBuffer;
		visual->ZMap->bmpBufPtr2 = visual->ZMap->BmpBufPtr1;
	}
	shortArr = (__int16*)partman::field(loader_table, groupIndexSum2, datFieldTypes::ShortArray);
	if (shortArr)
	{
		shortArrSize = partman::field_size(loader_table, groupIndexSum2, datFieldTypes::ShortArray);
		index = 0;
		shortArrLength = shortArrSize >> 1;
		if ((__int16)(shortArrSize >> 1) > 0)
		{
			while (1)
			{
				shortVal = *shortArr;
				nextShortVal = shortArr + 1;
				nextIndex = index + 1;
				if (shortVal <= 406)
				{
					if (shortVal == 406)
					{
						visual2->Kicker.SoundIndex = get_sound_id(*nextShortVal);
					}
					else
					{
						shortValSub100 = shortVal - 100;
						if (shortValSub100)
						{
							shortValSub300 = shortValSub100 - 200;
							if (shortValSub300)
							{
								shortValSub304 = shortValSub300 - 4;
								if (shortValSub304)
								{
									if (shortValSub304 != 96)
										return error(9, 18);
									if (kicker(*nextShortVal, &visual2->Kicker))
										return error(14, 18);
								}
								else
								{
									visual2->SoundIndex2 = get_sound_id(*nextShortVal);
								}
							}
							else if (material(*nextShortVal, visual2))
							{
								return error(15, 18);
							}
						}
						else if (groupIndexOffset)
						{
							return error(7, 18);
						}
					}
					goto LABEL_31;
				}
				shortValSub602 = shortVal - 602;
				if (!shortValSub602)
				{
					visual2->Unknown14Flag |= 1 << *nextShortVal;
					goto LABEL_31;
				}
				shortValSub1100 = shortValSub602 - 498;
				if (!shortValSub1100)
					break;
				shortValSub1101 = shortValSub1100 - 1;
				if (!shortValSub1101)
				{
					visual2->SoundIndex3 = get_sound_id(*nextShortVal);
				LABEL_31:
					shortArr = nextShortVal + 1;
					index = nextIndex + 1;
					goto LABEL_32;
				}
				if (shortValSub1101 != 399)
					return error(9, 18);
				shortArr = nextShortVal + 8;
				index = nextIndex + 8;
			LABEL_32:
				if (index >= shortArrLength)
					goto LABEL_33;
			}
			visual2->SoundIndex4 = get_sound_id(*nextShortVal);
			goto LABEL_31;
		}
	}
LABEL_33:
	if (!visual2->Unknown14Flag)
		visual2->Unknown14Flag = 1;
	floatArr = (float*)partman::field(loader_table, groupIndexSum3, datFieldTypes::FloatArray);
	if (!floatArr)
		return 0;
	nextFloatVal = floatArr + 1;
	if (*floatArr != 600.0)
		return 0;
	visual2->FloatArrSizeDiv8Sub2 = (partman::field_size(loader_table, groupIndexSum3, datFieldTypes::FloatArray) >> 2)/ 2- 2;
	floatVal = (__int64)(floor(*nextFloatVal) - 1.0);
	floatArrPtr = nextFloatVal + 1;
	if ((int)floatVal)
	{
		if ((int)floatVal == 1)
		{
			visual2->FloatArrSizeDiv8Sub2 = 2;
		}
		else if ((int)floatVal != visual2->FloatArrSizeDiv8Sub2)
		{
			return error(8, 18);
		}
	}
	else
	{
		visual2->FloatArrSizeDiv8Sub2 = 1;
	}
	visual2->FloatArr = floatArrPtr;
	return 0;
}