#include "pch.h"
#include "loader.h"
#include "memory.h"
#include "GroupData.h"
#include "pb.h"
#include "pinball.h"
#include "Sound.h"
#include "zdrv.h"


errorMsg loader::loader_errors[] =
{
	errorMsg{0, "Bad Handle"},
	errorMsg{1, "No Type Field"},
	errorMsg{2, "No Attributes Field"},
	errorMsg{3, "Wrong Type: MATERIAL Expected"},
	errorMsg{4, "Wrong Type: KICKER Expected"},
	errorMsg{5, "Wrong Type: AN_OBJECT Expected"},
	errorMsg{6, "Wrong Type: A_STATE Expected"},
	errorMsg{7, "STATES (re)defined in a state"},
	errorMsg{9, "Unrecognized Attribute"},
	errorMsg{0x0A, "Unrecognized float Attribute"},
	errorMsg{0x0B, "No float Attributes Field"},
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
DatFile* loader::loader_table;
DatFile* loader::sound_record_table;
soundListStruct loader::sound_list[65];

int loader::error(int errorCode, int captionCode)
{
	auto curCode = loader_errors;
	const char *errorText = nullptr, *errorCaption = nullptr;
	auto index = 0;
	while (curCode->Code >= 0)
	{
		if (errorCode == curCode->Code)
			errorText = curCode->Message;
		if (captionCode == curCode->Code)
			errorCaption = curCode->Message;
		curCode++;
		index++;
	}

	if (!errorText)
		errorText = loader_errors[index].Message;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, errorCaption, errorText, nullptr);
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

void loader::loadfrom(DatFile* datFile)
{
	loader_table = datFile;
	sound_record_table = loader_table;

	for (auto groupIndex = 0; groupIndex < datFile->Groups.size(); ++groupIndex)
	{
		auto value = reinterpret_cast<int16_t*>(datFile->field(groupIndex, FieldTypes::ShortValue));
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
	for (int index = 1; index < sound_count; ++index)
	{
		Sound::FreeSound(sound_list[index].WavePtr);
		sound_list[index].Loaded = 0;
		sound_list[index].WavePtr = nullptr;
	}

	sound_count = 1;
}

int loader::get_sound_id(int groupIndex)
{
	int16_t soundIndex = 1;
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
		WaveHeader wavHeader{};

		int soundGroupId = sound_list[soundIndex].GroupIndex;
		sound_list[soundIndex].Duration = 0.0;
		if (soundGroupId > 0 && !pinball::quickFlag)
		{
			auto value = reinterpret_cast<int16_t*>(loader_table->field(soundGroupId,
			                                                            FieldTypes::ShortValue));
			if (value && *value == 202)
			{
				std::string fileName = loader_table->field(soundGroupId, FieldTypes::String);

				// File name is in lower case, while game data is in upper case.				
				std::transform(fileName.begin(), fileName.end(), fileName.begin(),
				               [](unsigned char c) { return std::toupper(c); });
				if (pb::FullTiltMode)
				{
					// FT sounds are in SOUND subfolder
					fileName.insert(0, 1, PathSeparator);
					fileName.insert(0, "SOUND");
				}

				auto filePath = pinball::make_path_name(fileName);
				auto file = fopen(filePath.c_str(), "rb");
				if (file)
				{
					fread(&wavHeader, 1, sizeof wavHeader, file);
					fclose(file);
				}

				auto sampleCount = wavHeader.data_size / (wavHeader.channels * (wavHeader.bits_per_sample / 8.0));
				sound_list[soundIndex].Duration = static_cast<float>(sampleCount / wavHeader.sample_rate);
				sound_list[soundIndex].WavePtr = Sound::LoadWaveFile(filePath);
			}
		}
	}

	++sound_list[soundIndex].Loaded;
	return soundIndex;
}


int loader::query_handle(LPCSTR lpString)
{
	return loader_table->record_labeled(lpString);
}

short loader::query_visual_states(int groupIndex)
{
	short result;
	if (groupIndex < 0)
		return error(0, 17);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortArray));
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

	return loader_table->field(groupIndex, FieldTypes::GroupName);
}

int16_t* loader::query_iattribute(int groupIndex, int firstValue, int* arraySize)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return nullptr;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto shortArr = reinterpret_cast<int16_t*>(loader_table->field_nth(groupIndex,
		                                                                   FieldTypes::ShortArray, skipIndex));
		if (!shortArr)
			break;
		if (*shortArr == firstValue)
		{
			*arraySize = loader_table->field_size(groupIndex, FieldTypes::ShortArray) / 2 - 1;
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
		auto floatArr = reinterpret_cast<float*>(loader_table->field_nth(stateId, FieldTypes::FloatArray,
		                                                                 skipIndex));
		if (!floatArr)
			break;
		if (static_cast<int16_t>(floor(*floatArr)) == firstValue)
			return floatArr + 1;
	}

	error(13, 22);
	return nullptr;
}

float loader::query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue, float defVal)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return NAN;
	}

	int stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
	{
		error(16, 22);
		return NAN;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto floatArr = reinterpret_cast<float*>(loader_table->field_nth(stateId,
		                                                                 FieldTypes::FloatArray, skipIndex));
		if (!floatArr)
			break;
		if (static_cast<int16_t>(floor(*floatArr)) == firstValue)
			return floatArr[1];
	}

	if (!isnan(defVal))
		return defVal;
	error(13, 22);
	return NAN;
}

int loader::material(int groupIndex, visualStruct* visual)
{
	if (groupIndex < 0)
		return error(0, 21);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 21);
	if (*shortArr != 300)
		return error(3, 21);
	auto floatArr = reinterpret_cast<float*>(loader_table->field(groupIndex, FieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 21);

	int floatArrLength = loader_table->field_size(groupIndex, FieldTypes::FloatArray) / 4;
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
	Sound::PlaySound(sound_list[soundIndex].WavePtr, 0, 7, 0, 0);
	return sound_list[soundIndex].Duration;
}

int loader::state_id(int groupIndex, int groupIndexOffset)
{
	auto visualState = query_visual_states(groupIndex);
	if (visualState <= 0)
		return error(12, 24);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 200)
		return error(5, 24);
	if (groupIndexOffset > visualState)
		return error(12, 24);
	if (!groupIndexOffset)
		return groupIndex;

	groupIndex += groupIndexOffset;
	shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
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
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 20);
	if (*shortArr != 400)
		return error(4, 20);
	auto floatArr = reinterpret_cast<float*>(loader_table->field(groupIndex, FieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 20);
	int floatArrLength = loader_table->field_size(groupIndex, FieldTypes::FloatArray) / 4;
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

	visual->Bitmap = loader_table->GetBitmap(stateId);
	visual->ZMap = loader_table->GetZMap(stateId);

	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(stateId, FieldTypes::ShortArray));
	if (shortArr)
	{
		unsigned int shortArrSize = loader_table->field_size(stateId, FieldTypes::ShortArray);
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
	auto floatArr = reinterpret_cast<float*>(loader_table->field(stateId, FieldTypes::FloatArray));
	if (!floatArr)
		return 0;
	if (*floatArr != 600.0f)
		return 0;

	visual->FloatArrCount = loader_table->field_size(stateId, FieldTypes::FloatArray) / 4 / 2 - 2;
	auto floatVal = static_cast<int>(floor(floatArr[1]) - 1.0f);
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
