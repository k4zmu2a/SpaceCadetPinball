#include "pch.h"
#include "midi.h"


#include "pb.h"
#include "pinball.h"


objlist_class<Mix_Music>* midi::LoadedTracks;
Mix_Music *midi::track1, *midi::track2, *midi::track3, *midi::active_track, *midi::NextTrack;
bool midi::SetNextTrackFlag;

constexpr uint32_t FOURCC(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	return static_cast<uint32_t>((d << 24) | (c << 16) | (b << 8) | a);
}

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

int midi::play_pb_theme()
{
	// Todo: add support for tracks 2 and 3
	return play_track(track1);
}

int midi::music_stop()
{
	if (active_track)
	{
		active_track = nullptr;
		Mix_HaltMusic();
	}

	return true;
}

int midi::music_init()
{
	active_track = nullptr;
	LoadedTracks = new objlist_class<Mix_Music>(0, 1);

	if (pb::FullTiltMode)
	{
		track1 = load_track("TABA1.MDS", true);
		track2 = load_track("TABA2.MDS", true);
		track3 = load_track("TABA3.MDS", true);
	}
	else
	{
		// 3DPB has only one music track. PINBALL2.MID is a bitmap font, in the same format as PB_MSGFT.bin
		track1 = load_track("PINBALL.MID", false);
	}

	if (!track2)
		track2 = track1;
	if (!track3)
		track3 = track1;
	return track1 != nullptr;
}

void midi::music_shutdown()
{
	if (active_track)
		Mix_HaltMusic();

	while (LoadedTracks->GetCount())
	{
		auto midi = LoadedTracks->Get(0);
		Mix_FreeMusic(midi);
		LoadedTracks->Delete(midi);
	}
	active_track = nullptr;
	delete LoadedTracks;
}

Mix_Music* midi::load_track(std::string fileName, bool isMds)
{
	Mix_Music* audio;
	auto origFile = fileName;

	if (pb::FullTiltMode)
	{
		// FT sounds are in SOUND subfolder
		fileName.insert(0, 1, PathSeparator);
		fileName.insert(0, "SOUND");
	}

	auto filePath = pinball::make_path_name(fileName);
	if (isMds)
	{
		auto midi = MdsToMidi(filePath);
		if (!midi)
			return nullptr;

		// Dump converted MIDI file
		/*origFile += ".midi";
		FILE* fileHandle = fopen(origFile.c_str(), "wb");
		fwrite(midi->data(), 1, midi->size(), fileHandle);
		fclose(fileHandle);*/

		auto rw = SDL_RWFromMem(midi->data(), static_cast<int>(midi->size()));
		audio = Mix_LoadMUS_RW(rw, 1); // This call seems to leak memory no matter what.
		delete midi;
	}
	else
	{
		audio = Mix_LoadMUS(filePath.c_str());
	}

	if (!audio)
		return nullptr;

	LoadedTracks->Add(audio);
	return audio;
}

bool midi::play_track(Mix_Music* midi)
{
	music_stop();
	if (!midi)
		return false;

	if (SetNextTrackFlag)
	{
		NextTrack = midi;
		SetNextTrackFlag = false;
		return true;
	}

	if (Mix_PlayMusic(midi, -1))
	{
		active_track = nullptr;
		return false;
	}

	active_track = midi;
	return true;
}


/// <summary>
/// SDL_mixed does not support MIDS. To support FT music, a conversion to MIDI is required.
/// </summary>
/// <param name="file">Path to .MDS file</param>
/// <returns>Vector that contains MIDI file</returns>
std::vector<uint8_t>* midi::MdsToMidi(std::string file)
{
	auto fileHandle = fopen(file.c_str(), "rb");
	if (!fileHandle)
		return nullptr;

	fseek(fileHandle, 0, SEEK_END);
	auto fileSize = static_cast<uint32_t>(ftell(fileHandle));
	auto fileBuf = reinterpret_cast<riff_header*>(new uint8_t [fileSize]);
	fseek(fileHandle, 0, SEEK_SET);
	fread(fileBuf, 1, fileSize, fileHandle);
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
		if (fileBuf->Riff != FOURCC('R', 'I', 'F', 'F') ||
			fileBuf->Mids != FOURCC('M', 'I', 'D', 'S') ||
			fileBuf->Fmt != FOURCC('f', 'm', 't', ' '))
		{
			returnCode = 3;
			break;
		}
		if (fileBuf->FileSize > fileSize - 8)
		{
			returnCode = 3;
			break;
		}
		if (fileSize - 12 < 8)
		{
			returnCode = 3;
			break;
		}
		if (fileBuf->FmtSize < 12 || fileBuf->FmtSize > fileSize - 12)
		{
			returnCode = 3;
			break;
		}

		auto streamIdUsed = fileBuf->dwFlags == 0;
		auto dataChunk = reinterpret_cast<riff_data*>(reinterpret_cast<char*>(&fileBuf->dwTimeFormat) + fileBuf->
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
		std::vector<midi_event> midiEvents{};
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
		std::sort(midiEvents.begin(), midiEvents.end(), [](const midi_event& lhs, const midi_event& rhs)
		{
			return lhs.iTicks < rhs.iTicks;
		});

		// MThd chunk
		std::vector<uint8_t>& midiBytes = *new std::vector<uint8_t>();
		midiOut = &midiBytes;
		midi_header header(SwapByteOrderShort(static_cast<uint16_t>(fileBuf->dwTimeFormat)));
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
		auto lengthBE = SwapByteOrderInt(static_cast<uint32_t>(midiBytes.size()) - sizeof header - sizeof track);
		auto lengthData = reinterpret_cast<const uint8_t*>(&lengthBE);
		std::copy_n(lengthData, 4, midiBytes.begin() + lengthPos);
	}
	while (false);

	delete[] fileBuf;
	if (returnCode && midiOut)
		delete midiOut;
	return midiOut;
}
