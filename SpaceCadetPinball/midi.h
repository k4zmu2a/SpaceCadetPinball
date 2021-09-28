#pragma once
#include "objlist_class.h"

#ifdef MUSIC_TSF
#include "tml.h"
#include "tsf.h"
#endif

constexpr uint32_t SwapByteOrderInt(uint32_t val)
{
	return (val >> 24) |
		((val << 8) & 0x00FF0000) |
		((val >> 8) & 0x0000FF00) |
		(val << 24);
}

constexpr uint16_t SwapByteOrderShort(uint16_t val)
{
	return static_cast<uint16_t>((val >> 8) | (val << 8));
}

#pragma pack(push)
#pragma pack(1)
struct riff_block
{
	DWORD TkStart;
	DWORD CbBuffer;
	char AData[4];
};

struct riff_data
{
	DWORD Data;
	DWORD DataSize;
	DWORD BlocksPerChunk;
	riff_block Blocks[1];
};

struct riff_header
{
	DWORD Riff;
	DWORD FileSize;
	DWORD Mids;
	DWORD Fmt;
	DWORD FmtSize;
	DWORD dwTimeFormat;
	DWORD cbMaxBuffer;
	DWORD dwFlags;
	riff_data Data;
};

struct midi_event
{
	DWORD iTicks;
	DWORD iEvent;
};

struct midi_header
{
	explicit midi_header(uint16_t tickdiv)
		: tickdiv(tickdiv)
	{
	}

	const char MThd[4]{ 'M','T','h','d' };
	const uint32_t chunklen = SwapByteOrderInt(6);
	const int16_t format = SwapByteOrderShort(0);
	const uint16_t ntracks = SwapByteOrderShort(1);
	uint16_t tickdiv;
};

struct midi_track
{
	explicit midi_track(uint32_t chunklen)
		: chunklen(chunklen)
	{
	}

	const char MTrk[4]{ 'M','T','r','k' };
	uint32_t chunklen;
};

static_assert(sizeof(riff_block) == 0xC, "Wrong size of riff_block");
static_assert(sizeof(riff_data) == 0x18, "Wrong size of riff_data");
static_assert(sizeof(riff_header) == 0x38, "Wrong size of riff_header");
static_assert(sizeof(midi_event) == 8, "Wrong size of midi_event2");
static_assert(sizeof(midi_header) == 14, "Wrong size of midi_header");
static_assert(sizeof(midi_track) == 8, "Wrong size of midi_track");

#pragma pack(pop)

struct midi_song 
{
	bool valid;
#ifdef MUSIC_SDL
	Mix_Music* handle;
#elif defined(MUSIC_TSF)
	tml_message* start;
#else
	void* dummy;
#endif
};

class midi
{
public:
	static int play_pb_theme(int flag);
	static int music_stop();
	static int music_init();
	static void music_shutdown();
private:
	static midi_song currentMidi;
#ifdef MUSIC_TSF
	static tml_message* currentMessage;
	static void sdl_audio_callback(void* data, Uint8 *stream, int len);
#endif

	static std::vector<midi_song> TrackList;
	static midi_song track1, track2, track3, active_track, active_track2;
	static int some_flag1;
	static int music_init_ft();
	static void music_shutdown_ft();
	static midi_song load_track(std::string fileName);
	static int play_ft(midi_song* midi);
	static int stop_ft();
	static std::vector<uint8_t>* MdsToMidi(std::string file);
};
