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

class midi
{
public:
	static MCIERROR play_pb_theme(int flag);
	static MCIERROR music_stop();
	static int music_init(HWND hwnd);
	static MCIERROR restart_midi_seq(LPARAM param);
	static void music_shutdown();
private:	
	static Mix_Music* currentMidi;

	static objlist_class<midi_struct>* TrackList;
	static midi_struct *track1, *track2, *track3, *active_track, *active_track2;
	static int some_flag1;
	static int music_init_ft(HWND hwnd);
	static void music_shutdown_ft();
	static midi_struct* load_track(LPCSTR fileName);
	static int load_file(midi_struct** midi_res, void* filePtrOrPath, int fileSizeP, int flags);
	static int read_file(midi_struct* midi, riff_header* filePtr, unsigned int fileSize);
	static int play_ft(midi_struct* midi);
	static int stop_ft();
	static int unload_track(midi_struct* midi);
	static int stream_open(midi_struct* midi, char flags);
	static int stream_close(midi_struct* midi);
	static void CALLBACK midi_callback(HMIDIOUT hmo, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
	                                 DWORD_PTR dwParam2);
};
