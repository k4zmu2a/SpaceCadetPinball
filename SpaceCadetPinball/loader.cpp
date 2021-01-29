#include "pch.h"
#include "loader.h"
#include "memory.h"
#include "partman.h"
#include "pinball.h"
#include "Sound.h"
#include "zdrv.h"


errorMsg loader::loader_errors[] =
{
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

void loader::default_vsi(visualStruct* visual)
{
	visual->CollisionGroup = 0;
	visual->Kicker.Threshold = 8.9999999e10f;
	visual->Kicker.HardHitSoundId = 0;
	visual->Smoothness = 0.94999999f;
	visual->Elasticity = 0.60000002f;
	visual->FloatArrCount = 0;
	visual->SoftHitSoundId = 0;
	visual->Bitmap = nullptr;
	visual->ZMap = nullptr;
	visual->SoundIndex3 = 0;
	visual->SoundIndex4 = 0;
}

void loader::loadfrom(datFileStruct* datFile)
{
	loader_table = datFile;
	sound_record_table = loader_table;

	for (auto groupIndex = 0; groupIndex < datFile->NumberOfGroups; ++groupIndex)
	{
		auto value = reinterpret_cast<__int16*>(partman::field(datFile, groupIndex, datFieldTypes::ShortValue));
		if (value && *value == 202)
		{
			if (sound_count < 65)
			{
				sound_list[sound_count].WavePtr = nullptr;
				sound_list[sound_count].GroupIndex = groupIndex;
				sound_count++;
			}
		}
	}
	loader_sound_count = sound_count;
}

void loader::unload()
{
	int index;
	for (index = 1; index < sound_count; ++index)
		Sound::FreeSound(sound_list[index].WavePtr);

	if (sound_list[index].PtrToSmth)
		memory::free(sound_list[index].PtrToSmth);
	sound_count = 1;
}

int loader::get_sound_id(int groupIndex)
{
	__int16 soundIndex = 1;
	if (sound_count <= 1)
	{
		error(25, 26);
		return -1;
	}

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
		sound_list[soundIndex].Duration = 0.0;
		if (soundGroupId > 0 && !pinball::quickFlag)
		{
			auto value = reinterpret_cast<__int16*>(partman::field(loader_table, soundGroupId,
			                                                       datFieldTypes::ShortValue));
			if (value && *value == 202)
			{
				auto fileName = partman::field(loader_table, soundGroupId, datFieldTypes::String);
				HFILE hFile = _lopen(fileName, 0);
				sound_list[soundIndex].Duration = static_cast<float>(static_cast<double>(_llseek(hFile, 0, SEEK_END)) *
					0.0000909090909090909);
				_lclose(hFile);
				sound_list[soundIndex].WavePtr = Sound::LoadWaveFile(fileName);
			}
		}
	}

	++sound_list[soundIndex].Loaded;
	return soundIndex;
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
	auto shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, groupIndex, datFieldTypes::ShortArray));
	if (shortArr && *shortArr == 100)
		result = shortArr[1];
	else
		result = 1;
	return result;
}

char* loader::query_name(int groupIndex)
{
	if (groupIndex < 0)
	{
		error(0, 19);
		return nullptr;
	}

	return partman::field(loader_table, groupIndex, datFieldTypes::GroupName);
}

__int16* loader::query_iattribute(int groupIndex, int firstValue, int* arraySize)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return nullptr;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto shortArr = reinterpret_cast<__int16*>(partman::field_nth(loader_table, groupIndex,
		                                                              datFieldTypes::ShortArray, skipIndex));
		if (!shortArr)
			break;
		if (*shortArr == firstValue)
		{
			*arraySize = partman::field_size(loader_table, groupIndex, datFieldTypes::ShortArray) / 2 - 1;
			return shortArr + 1;
		}
	}

	error(2, 23);
	*arraySize = 0;
	return nullptr;
}

float* loader::query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return nullptr;
	}

	int stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
	{
		error(16, 22);
		return nullptr;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto floatArr = reinterpret_cast<float*>(partman::field_nth(loader_table, stateId, datFieldTypes::FloatArray,
		                                                            skipIndex));
		if (!floatArr)
			break;
		if (static_cast<__int16>(floor(*floatArr)) == firstValue)
			return floatArr + 1;
	}

	error(13, 22);
	return nullptr;
}

int loader::material(int groupIndex, visualStruct* visual)
{
	if (groupIndex < 0)
		return error(0, 21);
	auto shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, groupIndex, datFieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 21);
	if (*shortArr != 300)
		return error(3, 21);
	auto floatArr = reinterpret_cast<float*>(partman::field(loader_table, groupIndex, datFieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 21);

	int floatArrLength = partman::field_size(loader_table, groupIndex, datFieldTypes::FloatArray) / 4;
	for (auto index = 0; index < floatArrLength; index += 2)
	{
		switch (static_cast<int>(floor(floatArr[index])))
		{
		case 301:
			visual->Smoothness = floatArr[index + 1];
			break;
		case 302:
			visual->Elasticity = floatArr[index + 1];
			break;
		case 304:
			visual->SoftHitSoundId = get_sound_id(static_cast<int>(floor(floatArr[index + 1])));
			break;
		default:
			return error(9, 21);
		}
	}
	return 0;
}


float loader::play_sound(int soundIndex)
{
	if (soundIndex <= 0)
		return 0.0;
	Sound::PlaySound(sound_list[soundIndex].WavePtr, 0, 7, WMIX_HIPRIORITY | WMIX_CLEARQUEUE, 0);
	return sound_list[soundIndex].Duration;
}

int loader::state_id(int groupIndex, int groupIndexOffset)
{
	auto visualState = query_visual_states(groupIndex);
	if (visualState <= 0)
		return error(12, 24);
	auto shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, groupIndex, datFieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 200)
		return error(5, 24);
	if (groupIndexOffset > visualState)
		return error(12, 24);
	if (!groupIndexOffset)
		return groupIndex;

	groupIndex += groupIndexOffset;
	shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, groupIndex, datFieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 201)
		return error(6, 24);
	return groupIndex;
}

int loader::kicker(int groupIndex, visualKickerStruct* kicker)
{
	if (groupIndex < 0)
		return error(0, 20);
	auto shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, groupIndex, datFieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 20);
	if (*shortArr != 400)
		return error(4, 20);
	auto floatArr = reinterpret_cast<float*>(partman::field(loader_table, groupIndex, datFieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 20);
	int floatArrLength = partman::field_size(loader_table, groupIndex, datFieldTypes::FloatArray) / 4;
	if (floatArrLength <= 0)
		return 0;

	for (auto index = 0; index < floatArrLength;)
	{
		int floorVal = static_cast<int>(floor(*floatArr++));
		switch (floorVal)
		{
		case 401:
			kicker->Threshold = *floatArr;
			break;
		case 402:
			kicker->Boost = *floatArr;
			break;
		case 403:
			kicker->ThrowBallMult = *floatArr;
			break;
		case 404:
			kicker->ThrowBallAcceleration = *reinterpret_cast<vector_type*>(floatArr);
			floatArr += 3;
			index += 4;
			break;
		case 405:
			kicker->ThrowBallAngleMult = *floatArr;
			break;
		case 406:
			kicker->HardHitSoundId = get_sound_id(static_cast<int>(floor(*floatArr)));
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


int loader::query_visual(int groupIndex, int groupIndexOffset, visualStruct* visual)
{
	default_vsi(visual);
	if (groupIndex < 0)
		return error(0, 18);
	auto stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
		return error(16, 18);

	visual->Bitmap = reinterpret_cast<gdrv_bitmap8*>(partman::field(loader_table, stateId, datFieldTypes::Bitmap8bit));
	visual->ZMap = reinterpret_cast<zmap_header_type*>(partman::field(loader_table, stateId, datFieldTypes::Bitmap16bit)
	);
	if (visual->ZMap)
	{
		visual->ZMap->ZPtr1 = visual->ZMap->ZBuffer;
		visual->ZMap->ZPtr2 = visual->ZMap->ZPtr1;
	}

	auto shortArr = reinterpret_cast<__int16*>(partman::field(loader_table, stateId, datFieldTypes::ShortArray));
	if (shortArr)
	{
		unsigned int shortArrSize = partman::field_size(loader_table, stateId, datFieldTypes::ShortArray);
		for (auto index = 0u; index < shortArrSize / 2;)
		{
			switch (shortArr[0])
			{
			case 100:
				if (groupIndexOffset)
					return error(7, 18);
				break;
			case 300:
				if (material(shortArr[1], visual))
					return error(15, 18);
				break;
			case 304:
				visual->SoftHitSoundId = get_sound_id(shortArr[1]);
				break;
			case 400:
				if (kicker(shortArr[1], &visual->Kicker))
					return error(14, 18);
				break;
			case 406:
				visual->Kicker.HardHitSoundId = get_sound_id(shortArr[1]);
				break;
			case 602:
				visual->CollisionGroup |= 1 << shortArr[1];
				break;
			case 1100:
				visual->SoundIndex4 = get_sound_id(shortArr[1]);
				break;
			case 1101:
				visual->SoundIndex3 = get_sound_id(shortArr[1]);
				break;
			case 1500:
				shortArr += 7;
				index += 7;
				break;
			default:
				return error(9, 18);
			}
			shortArr += 2;
			index += 2;
		}
	}

	if (!visual->CollisionGroup)
		visual->CollisionGroup = 1;
	auto floatArr = reinterpret_cast<float*>(partman::field(loader_table, stateId, datFieldTypes::FloatArray));
	if (!floatArr)
		return 0;
	if (*floatArr != 600.0)
		return 0;

	visual->FloatArrCount = (partman::field_size(loader_table, stateId, datFieldTypes::FloatArray) / 4) / 2 - 2;
	auto floatVal = static_cast<int>(floor(floatArr[1]) - 1.0);
	switch (floatVal)
	{
	case 0:
		visual->FloatArrCount = 1;
		break;
	case 1:
		visual->FloatArrCount = 2;
		break;
	default:
		if (floatVal != visual->FloatArrCount)
			return error(8, 18);
		break;
	}

	visual->FloatArr = floatArr + 2;
	return 0;
}
