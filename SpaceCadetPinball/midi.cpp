#include "pch.h"
#include "midi.h"


#include "pb.h"
#include "pinball.h"

Mix_Music* midi::currentMidi;

#define FOURCC(a,b,c,d) ( (uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )

int ToVariableLen(uint32_t value, uint32_t& dst)
{
	auto count = 1;
	dst = value & 0x7F;

	while ((value >>= 7))
	{
		dst <<= 8;
		dst |= ((value & 0x7F) | 0x80);
		count++;
	}

	return count;
}

int midi::play_pb_theme(int flag)
{
	if (pb::FullTiltMode)
	{
		return play_ft(track1);
	}

	int result = 0;
	music_stop();
	if (currentMidi)
		result = Mix_PlayMusic(currentMidi, -1);

	return result;
}

int midi::music_stop()
{
	if (pb::FullTiltMode)
	{
		return stop_ft();
	}

	return Mix_HaltMusic();
}

int midi::music_init()
{
	if (pb::FullTiltMode)
	{
		return music_init_ft();
	}

	currentMidi = Mix_LoadMUS(pinball::get_rc_string(156, 0));
	return currentMidi != nullptr;
}

void midi::music_shutdown()
{
	if (pb::FullTiltMode)
	{
		music_shutdown_ft();
		return;
	}

	Mix_FreeMusic(currentMidi);
}


objlist_class<Mix_Music>* midi::TrackList;
Mix_Music *midi::track1, *midi::track2, *midi::track3, *midi::active_track, *midi::active_track2;
int midi::some_flag1;

int midi::music_init_ft()
{
	active_track = nullptr;
	TrackList = new objlist_class<Mix_Music>(0, 1);

	track1 = load_track("taba1");
	track2 = load_track("taba2");
	track3 = load_track("taba3");
	if (!track2)
		track2 = track1;
	if (!track3)
		track3 = track1;
	return 1;
}

void midi::music_shutdown_ft()
{
	if (active_track)
		Mix_HaltMusic();
	while (TrackList->GetCount())
	{
		auto midi = TrackList->Get(0);
		Mix_FreeMusic(midi);
		TrackList->Delete(midi);
	}
	active_track = nullptr;
	delete TrackList;
}

Mix_Music* midi::load_track(LPCSTR fileName)
{
	char filePath[256];
	char fileName2[256];

	strcpy_s(fileName2, "sound\\");
	strcat_s(fileName2, fileName);
	pinball::make_path_name(filePath, fileName2, 254u);
	strcat_s(filePath, ".MDS");
	auto midi = MdsToMidi(filePath);
	if (!midi)
		return nullptr;

	auto rw = SDL_RWFromMem(midi->data(), static_cast<int>(midi->size()));
	auto audio = Mix_LoadMUS_RW(rw, 0);
	SDL_FreeRW(rw);
	delete midi;
	if (!audio)
		return nullptr;

	// Dump converted MIDI file
	/*FILE* fileHandle;
	strcpy_s(fileName2, fileName);
	strcat_s(fileName2, ".midi");
	fopen_s(&fileHandle, fileName2, "wb");
	fwrite(midi->data(), 1, midi->size(), fileHandle);
	fclose(fileHandle);*/
	
	TrackList->Add(audio);
	return audio;
}

int midi::play_ft(Mix_Music* midi)
{
	int result;

	stop_ft();
	if (!midi)
		return 0;
	if (some_flag1)
	{
		active_track2 = midi;
		return 0;
	}
	if (Mix_PlayMusic(midi, -1))
	{
		active_track = nullptr;
		result = 0;
	}
	else
	{
		active_track = midi;
		result = 1;
	}
	return result;
}

int midi::stop_ft()
{
	int returnCode = 0;
	if (active_track)
		returnCode = Mix_HaltMusic();
	active_track = nullptr;
	return returnCode;
}

/// <summary>
/// SDL_mixed does not support MIDS. To support FT music, a conversion to MIDI is required.
/// </summary>
/// <param name="file">Path to .MDS file</param>
/// <returns>Vector that contains MIDI file</returns>
std::vector<uint8_t>* midi::MdsToMidi(char* file)
{
	FILE* fileHandle;
	if (fopen_s(&fileHandle, file, "rb"))
		return nullptr;

	fseek(fileHandle, 0, SEEK_END);
	auto fileSize = static_cast<uint32_t>(ftell(fileHandle));
	auto filePtr = reinterpret_cast<riff_header*>(memory::allocate(fileSize));
	fseek(fileHandle, 0, SEEK_SET);
	fread(filePtr, 1, fileSize, fileHandle);
	fclose(fileHandle);

	int returnCode = 0;
	std::vector<uint8_t>* midiOut = nullptr;
	do
	{
		if (fileSize < 12)
		{
			returnCode = 3;
			break;
		}
		if (filePtr->Riff != FOURCC('R', 'I', 'F', 'F') ||
			filePtr->Mids != FOURCC('M', 'I', 'D', 'S') ||
			filePtr->Fmt != FOURCC('f', 'm', 't', ' '))
		{
			returnCode = 3;
			break;
		}
		if (filePtr->FileSize > fileSize - 8)
		{
			returnCode = 3;
			break;
		}
		if (fileSize - 12 < 8)
		{
			returnCode = 3;
			break;
		}
		if (filePtr->FmtSize < 12 || filePtr->FmtSize > fileSize - 12)
		{
			returnCode = 3;
			break;
		}

		auto streamIdUsed = filePtr->dwFlags == 0;
		auto dataChunk = reinterpret_cast<riff_data*>(reinterpret_cast<char*>(&filePtr->dwTimeFormat) + filePtr->
			FmtSize);
		if (dataChunk->Data != FOURCC('d', 'a', 't', 'a'))
		{
			returnCode = 3;
			break;
		}
		if (dataChunk->DataSize < 4)
		{
			returnCode = 3;
			break;
		}

		auto srcPtr = dataChunk->Blocks;
		std::vector<midi_event_x2> midiEvents{};
		for (auto blockIndex = dataChunk->BlocksPerChunk; blockIndex; blockIndex--)
		{
			auto eventSizeInt = streamIdUsed ? 3 : 2;
			auto eventCount = srcPtr->CbBuffer / (4 * eventSizeInt);

			auto currentTicks = srcPtr->TkStart;
			auto srcPtr2 = reinterpret_cast<uint32_t*>(srcPtr->AData);
			for (auto i = 0u; i < eventCount; i++)
			{
				currentTicks += srcPtr2[0];
				auto event = streamIdUsed ? srcPtr2[2] : srcPtr2[1];
				midiEvents.push_back({currentTicks, event});
				srcPtr2 += eventSizeInt;
			}

			srcPtr = reinterpret_cast<riff_block*>(&srcPtr->AData[srcPtr->CbBuffer]);
		}

		// MIDS events can be out of order in the file
		std::sort(midiEvents.begin(), midiEvents.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.iTicks < rhs.iTicks;
		});

		// MThd chunk
		std::vector<uint8_t>& midiBytes = *new std::vector<uint8_t>();
		midiOut = &midiBytes;
		midi_header header(SwapByteOrderShort(static_cast<uint16_t>(filePtr->dwTimeFormat)));
		auto headerData = reinterpret_cast<const uint8_t*>(&header);
		midiBytes.insert(midiBytes.end(), headerData, headerData + sizeof header);

		// MTrk chunk
		midi_track track(7);
		auto trackData = reinterpret_cast<const uint8_t*>(&track);
		midiBytes.insert(midiBytes.end(), trackData, trackData + sizeof track);
		auto lengthPos = midiBytes.size() - 4;

		auto prevTime = 0u;
		for (const auto& event : midiEvents)
		{
			assertm(event.iTicks >= prevTime, "MIDS events: negative delta-time");
			uint32_t delta = event.iTicks - prevTime;
			prevTime = event.iTicks;

			// Delta time is in variable quantity, Big Endian
			uint32_t deltaVarLen;
			auto count = ToVariableLen(delta, deltaVarLen);
			deltaVarLen = SwapByteOrderInt(deltaVarLen);
			auto deltaData = reinterpret_cast<const uint8_t*>(&deltaVarLen) + 4 - count;
			midiBytes.insert(midiBytes.end(), deltaData, deltaData + count);

			switch (event.iEvent >> 24)
			{
			case 0:
				{
					// Type 0 - MIDI short message. 3 bytes: xx p1 p2 00, where xx - message, p* - parameters
					// Some of the messages have only one parameter
					auto msgMask = (event.iEvent) & 0xF0;
					auto shortMsg = (msgMask == 0xC0 || msgMask == 0xD0);
					auto eventData = reinterpret_cast<const uint8_t*>(&event.iEvent);
					midiBytes.insert(midiBytes.end(), eventData, eventData + (shortMsg ? 2 : 3));
					break;
				}
			case 1:
				{
					// Type 1 - tempo change, 3 bytes: xx xx xx 01
					// Meta message, set tempo, 3 bytes payload
					const uint8_t metaSetTempo[] = {0xFF, 0x51, 0x03};
					midiBytes.insert(midiBytes.end(), metaSetTempo, metaSetTempo + 3);

					auto eventBE = SwapByteOrderInt(event.iEvent);
					auto eventData = reinterpret_cast<const uint8_t*>(&eventBE) + 1;
					midiBytes.insert(midiBytes.end(), eventData, eventData + 3);
					break;
				}
			default:
				assertm(0, "MIDS events: uknown event");
				break;
			}
		}

		// Meta message, end of track, 0 bytes payload
		const uint8_t metaEndTrack[] = {0x00, 0xFF, 0x2f, 0x00};
		midiBytes.insert(midiBytes.end(), metaEndTrack, metaEndTrack + 4);

		// Set final MTrk size
		*(uint32_t*)&midiBytes[lengthPos] = SwapByteOrderInt((uint32_t)midiBytes.size() - sizeof header - sizeof track);
	}
	while (false);

	if (filePtr)
		memory::free(filePtr);
	if (returnCode && midiOut)
		delete midiOut;
	return midiOut;
}
