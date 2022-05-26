#pragma once
#include "objlist_class.h"

struct midi_struct
{
	DWORD Magic;
	DWORD DwTimeFormat;
	DWORD CbMaxBuffer;
	DWORD DwFlagsFormat;
	midihdr_tag* DataPtr1;
	HMIDISTRM StreamHandle;
	int SomeFlag2;
	int BlockCount;
	int PreparedBlocksCount;
};

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

static_assert(sizeof(riff_block) == 0xC, "Wrong size of riff_block");
static_assert(sizeof(riff_data) == 0x18, "Wrong size of riff_data");
static_assert(sizeof(riff_header) == 0x38, "Wrong size of riff_header");
#pragma pack(pop)

class BaseMidi
{
public:
	bool IsOpen = false, IsPlaying = false;

	virtual ~BaseMidi() = default;
	virtual void Play() = 0;
	virtual void Stop() = 0;
};

class MdsMidi : public BaseMidi
{
public:
	MdsMidi(LPCSTR fileName);
	~MdsMidi() override;
	void Play() override;
	void Stop() override;
private:
	midi_struct midi{};

	static int stream_open(midi_struct& midi, char flags);
	static int stream_close(midi_struct& midi);
	static int load_file(midi_struct& midi, void* filePtrOrPath, int fileSizeP, int flags);
	static int read_file(midi_struct& midi, riff_header* filePtr, unsigned int fileSize);
	static void CALLBACK midi_callback(HMIDIOUT hmo, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
		DWORD_PTR dwParam2);
};

class MciMidi : public BaseMidi
{
public:
	MciMidi(LPCSTR fileName, HWND notifyHwnd);
	~MciMidi() override;
	void Play() override;
	void Stop() override;
private:
	MCIDEVICEID DeviceId{};
	HWND midi_notify_hwnd;
};

enum class MidiTracks
{
	None,
	Track1,
	Track2,
	Track3
};


class midi
{
public:
	static void music_play();
	static void music_stop();
	static int music_init(HWND hwnd);
	static void restart_midi_seq(LPARAM param);
	static void music_shutdown();
	static bool play_track(MidiTracks track, bool replay);
private:
	static HWND midi_notify_hwnd;
	static objlist_class<BaseMidi>* TrackList;
	static BaseMidi* track1, * track2, * track3;
	static MidiTracks ActiveTrack, NextTrack;
	static bool IsPlaying;

	static void StopPlayback();
	static BaseMidi* LoadTrack(LPCSTR fileName);
	static BaseMidi* TrackToMidi(MidiTracks track);
};
