#include "pch.h"
#include "midi.h"


#include "pb.h"
#include "pinball.h"

#ifndef TSF_RENDER_EFFECTSAMPLEBLOCK
#define TSF_RENDER_EFFECTSAMPLEBLOCK 64
#endif

midi_song midi::currentMidi = {false};

#ifdef MUSIC_TSF
tml_message* midi::currentMessage = nullptr;
static float midiTime = 0.0f;
static float sampPerSec = 1000.0 / 22050.0;
static tsf* tsfSynth = nullptr;

void midi::sdl_audio_callback(void* data, Uint8 *stream, int len)
{
	memset(stream, 0, len);
		
	if (tsfSynth == nullptr) {
		return;
	}

	int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
	{
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		for (midiTime += SampleBlock * sampPerSec; midi::currentMessage && midiTime >= midi::currentMessage->time; )
		{
			switch (midi::currentMessage->type)
			{
				case TML_PROGRAM_CHANGE:
					tsf_channel_set_presetnumber(tsfSynth, midi::currentMessage->channel, midi::currentMessage->program, (midi::currentMessage->channel == 9));
					tsf_channel_midi_control(tsfSynth, midi::currentMessage->channel, TML_ALL_NOTES_OFF, 0);
					break;
				case TML_NOTE_ON:
					tsf_channel_note_on(tsfSynth, midi::currentMessage->channel, midi::currentMessage->key, midi::currentMessage->velocity / 127.0f);
					break;
				case TML_NOTE_OFF:
					tsf_channel_note_off(tsfSynth, midi::currentMessage->channel, midi::currentMessage->key);
					break;
				case TML_PITCH_BEND:
					tsf_channel_set_pitchwheel(tsfSynth, midi::currentMessage->channel, midi::currentMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE:
					tsf_channel_midi_control(tsfSynth, midi::currentMessage->channel, midi::currentMessage->control, midi::currentMessage->control_value);
					break;
			}

			if (midi::currentMessage->next == nullptr) {
				midiTime = 0.0f;
				midi::currentMessage = midi::currentMidi.start;
			} else {
				midi::currentMessage = midi::currentMessage->next;
			}
		}

		// Render the block of audio samples in float format
		tsf_render_short(tsfSynth, (short*)stream, SampleBlock, 0);
	}
}
#endif

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

int midi::play_pb_theme(int flag)
{
	if (pb::FullTiltMode)
	{
		return play_ft(&track1);
	}

#ifdef MUSIC_SDL
	int result = 0;
	music_stop();
	if (currentMidi.valid)
		result = Mix_PlayMusic(currentMidi.handle, -1);

	return result;
#elif defined(MUSIC_TSF)
	int result = 0;
	if (currentMidi.valid) {
		currentMessage = currentMidi.start;
		midiTime = 0.0f;
		// Mix_HookMusic(midi::sdl_audio_callback, nullptr);
		result = 1;
	}

	return result;
#else
	return 0;
#endif
}

int midi::music_stop()
{
	if (pb::FullTiltMode)
	{
		return stop_ft();
	}

#ifdef MUSIC_SDL
	return Mix_HaltMusic();
#else
	return 0;
#endif
}

#ifdef MUSIC_TSF
extern unsigned char gm_sf2[];
extern unsigned int gm_sf2_len;
#endif

int midi::music_init()
{
	if (pb::FullTiltMode)
	{
		return music_init_ft();
	}

#if defined(MUSIC_SDL)
	// File name is in lower case, while game data is in upper case.
	std::string fileName = pinball::get_rc_string(156, 0);
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](unsigned char c) { return std::toupper(c); });
	auto midiPath = pinball::make_path_name(fileName);
	auto song = Mix_LoadMUS(midiPath.c_str());
	if (song) {
		currentMidi = {true, song};
	} else {
		currentMidi = {false};
	}

	return currentMidi.valid;
#elif defined(MUSIC_TSF)
	currentMessage = nullptr;
	currentMidi = {false};
	
	tsfSynth = tsf_load_memory(gm_sf2, (int)gm_sf2_len);

	int sampleRate;
	if (Mix_QuerySpec(&sampleRate, nullptr, nullptr)) {
		tsf_set_output(tsfSynth, TSF_STEREO_INTERLEAVED, sampleRate, 0.0f);
		sampPerSec = 1000.0f / float(sampleRate);
	}

	auto fileName = std::string(pinball::get_rc_string(156, 0));
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](unsigned char c) { return std::toupper(c); });
	auto filePath = pinball::make_path_name(fileName);

	auto midi = tml_load_filename(filePath.c_str());
	if (midi != nullptr) {
		currentMidi = {true, midi};
	}

	Mix_HookMusic(midi::sdl_audio_callback, nullptr);
	return currentMidi.valid;
#else
	return 1;
#endif
}

void midi::music_shutdown()
{
	if (pb::FullTiltMode)
	{
		music_shutdown_ft();
		return;
	}

#ifdef MUSIC_SDL
	Mix_FreeMusic(currentMidi.handle);
#endif
}


std::vector<midi_song> midi::TrackList;
midi_song midi::track1, midi::track2, midi::track3, midi::active_track, midi::active_track2;
int midi::some_flag1;

int midi::music_init_ft()
{
	active_track = {false};
	//TrackList = new objlist_class<midi_song>(0, 1);
	TrackList.clear();

	track1 = load_track("taba1");
	track2 = load_track("taba2");
	track3 = load_track("taba3");
	if (!track2.valid)
		track2 = track1;
	if (!track3.valid)
		track3 = track1;
	return 1;
}

void midi::music_shutdown_ft()
{
#ifdef MUSIC_SDL
	if (active_track.valid)
		Mix_HaltMusic();

	for (auto& track : TrackList) {
		if (track.valid) Mix_FreeMusic(track.handle);
	}

	TrackList.clear();

	active_track = {false};
#elif defined(MUSIC_TSF)
	if (active_track.valid) {
		tsf_note_off_all(tsfSynth);
		active_track = {false, nullptr};
		currentMessage = nullptr;
		midiTime = 0.0f;
	}

	for (auto& track : TrackList) {
		//if (track.valid) tml_free(track.handle);
	}

	TrackList.clear();
#endif
}

midi_song midi::load_track(std::string fileName)
{
	auto origFile = fileName;

	// File name is in lower case, while game data is in upper case.				
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](unsigned char c) { return std::toupper(c); });
	if (pb::FullTiltMode)
	{
		// FT sounds are in SOUND subfolder
		fileName.insert(0, 1, PathSeparator);
		fileName.insert(0, "SOUND");
	}
	fileName += ".MDS";

	auto filePath = pinball::make_path_name(fileName);
	auto midi = MdsToMidi(filePath);
	if (!midi)
		return {false};

	// Dump converted MIDI file
	/*origFile += ".midi";
	FILE* fileHandle = fopen(origFile.c_str(), "wb");
	fwrite(midi->data(), 1, midi->size(), fileHandle);
	fclose(fileHandle);*/

#ifdef MUSIC_SDL
	auto rw = SDL_RWFromMem(midi->data(), static_cast<int>(midi->size()));
	auto audio = Mix_LoadMUS_RW(rw, 1); // This call seems to leak memory no matter what.
	
#elif defined(MUSIC_TSF)
	auto audio = tml_load_memory(midi->data(), static_cast<int>(midi->size()));
#else
	void* audio = nullptr;
#endif
	delete midi;
	if (!audio)
		return {false};
	
	midi_song song = {true, audio};
	TrackList.push_back(song);

	return song;
}

int midi::play_ft(midi_song* midi)
{
	int result = 0;

	stop_ft();
	if (!midi || !midi->valid)
		return 0;

	if (some_flag1)
	{
		active_track2 = *midi;
		return 0;
	}

#ifdef MUSIC_SDL
	if (Mix_PlayMusic(midi->handle, -1))
	{
		active_track = {false, nullptr};
		result = 0;
	}
	else
	{
		active_track = *midi;
		result = 1;
	}
#elif defined(MUSIC_TSF)
	active_track = *midi;
	result = 1;
#endif

	return result;
}

int midi::stop_ft()
{
	int returnCode = 0;
	
#ifdef MUSIC_SDL
	if (active_track.valid)
		returnCode = Mix_HaltMusic();

	active_track.valid = false;
	active_track.handle = nullptr;
#elif defined(MUSIC_TSF)
	// Mix_HookMusic(nullptr, nullptr);
	tsf_note_off_all(tsfSynth);
	active_track = {false, nullptr};
	currentMessage = nullptr;
	midiTime = 0.0f;
#endif

	return returnCode;
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
		auto lengthBE = SwapByteOrderInt(static_cast<uint32_t>(midiBytes.size()) - sizeof header - sizeof track);
		auto lengthData = reinterpret_cast<const uint8_t*>(&lengthBE);
		std::copy_n(lengthData, 4, midiBytes.begin() + lengthPos);
	}
	while (false);

	if (filePtr)
		memory::free(filePtr);
	if (returnCode && midiOut)
		delete midiOut;
	return midiOut;
}
