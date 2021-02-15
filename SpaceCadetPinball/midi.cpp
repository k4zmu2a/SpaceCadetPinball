#include "pch.h"
#include "midi.h"


#include "pb.h"
#include "pinball.h"

tagMCI_OPEN_PARMSA midi::mci_open_info;
char midi::midi_device_type[28];
HWND midi::midi_notify_hwnd;
int midi::midi_seq1_open, midi::midi_seq1_playing;

MCIERROR midi::play_pb_theme(int flag)
{
	if (pb::FullTiltMode)
	{
		return play_ft(track1);
	}

	MCI_PLAY_PARMS playParams;
	MCIERROR result = 0;

	music_stop();
	playParams.dwFrom = 0;
	playParams.dwCallback = (DWORD_PTR)midi_notify_hwnd;
	if (!flag && midi_seq1_open)
	{
		result = mciSendCommandA(mci_open_info.wDeviceID, MCI_PLAY, MCI_FROM | MCI_NOTIFY, (DWORD_PTR)&playParams);
		midi_seq1_playing = result == 0;
	}
	return result;
}

MCIERROR midi::music_stop()
{
	if (pb::FullTiltMode)
	{
		return stop_ft();
	}

	MCIERROR result = 0;
	if (midi_seq1_playing)
		result = mciSendCommandA(mci_open_info.wDeviceID, MCI_STOP, 0, 0);
	return result;
}

int midi::music_init(HWND hwnd)
{
	if (pb::FullTiltMode)
	{
		return music_init_ft(hwnd);
	}

	mci_open_info.wDeviceID = 0;
	midi_notify_hwnd = hwnd;
	lstrcpyA(midi_device_type, pinball::get_rc_string(156, 0));
	mci_open_info.lpstrElementName = nullptr;
	mci_open_info.lpstrDeviceType = midi_device_type;
	auto result = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_NOTIFY_SUPERSEDED, (DWORD_PTR)&mci_open_info);
	midi_seq1_open = result == 0;
	return midi_seq1_open;
}

MCIERROR midi::restart_midi_seq(int param)
{
	if (pb::FullTiltMode)
	{
		return play_ft(active_track);
	}

	MCI_PLAY_PARMS playParams;
	MCIERROR result = 0;

	playParams.dwFrom = 0;
	playParams.dwCallback = (DWORD_PTR)midi_notify_hwnd;
	if (midi_seq1_playing)
		result = mciSendCommandA(mci_open_info.wDeviceID, MCI_PLAY, MCI_FROM | MCI_NOTIFY, (DWORD_PTR)&playParams);
	return result;
}

void midi::music_shutdown()
{
	if (pb::FullTiltMode)
	{
		music_shutdown_ft();
		return;
	}

	if (midi_seq1_open)
		mciSendCommandA(mci_open_info.wDeviceID, MCI_CLOSE, 0, 0);
	midi_seq1_open = 0;
}


objlist_class<midi_struct>* midi::TrackList;
midi_struct *midi::track1, *midi::track2, *midi::track3, *midi::active_track, *midi::active_track2;
int midi::some_flag1;

int midi::music_init_ft(HWND hwnd)
{
	midi_notify_hwnd = hwnd;
	active_track = nullptr;
	TrackList = new objlist_class<midi_struct>(0, 1);

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
		stream_close(active_track);
	while (TrackList->GetCount())
	{
		midi_struct* midi = TrackList->Get(0);
		unload_track(midi);
		TrackList->Delete(midi);
	}
	active_track = nullptr;
	delete TrackList;
}

midi_struct* midi::load_track(LPCSTR fileName)
{
	midi_struct* midi;
	char filePath[256];
	char fileName2[256];

	lstrcpyA(fileName2, "sound\\");
	lstrcatA(fileName2, fileName);
	pinball::make_path_name(filePath, fileName2, 254u);
	lstrcatA(filePath, ".MDS");
	if (load_file(&midi, filePath, 0, 1))
		return nullptr;

	if (midi)
		TrackList->Add(midi);
	return midi;
}

int midi::load_file(midi_struct** midi_res, void* filePtrOrPath, int fileSizeP, int flags)
{
	int returnCode;
	unsigned int fileSize;
	HANDLE mapHandle = nullptr;
	midi_struct* midi = nullptr;
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	int fileFlag = 0;

	do
	{
		if ((flags & 3) == 0 || (flags & 3) == 3)
		{
			returnCode = 4;
			break;
		}

		midi = static_cast<midi_struct*>(LocalAlloc(0x40u, sizeof(midi_struct)));
		if (!midi)
		{
			returnCode = 1;
			break;
		}
		midi->Magic = 'ISDM';
		midi->StreamHandle = nullptr;
		midi->PreparedBlocksCount = 0;

		if ((flags & 2) != 0)
		{
			fileSize = fileSizeP;
		}
		else
		{
			fileFlag = 1;
			fileHandle = CreateFileA(static_cast<LPCSTR>(filePtrOrPath), GENERIC_READ, 1u, nullptr, OPEN_EXISTING,
			                         0x80u, nullptr);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				returnCode = 2;
				break;
			}

			fileSize = GetFileSize(fileHandle, nullptr);
			mapHandle = CreateFileMappingA(fileHandle, nullptr, 2u, 0, 0, nullptr);
			if (!mapHandle)
			{
				returnCode = 2;
				break;
			}

			filePtrOrPath = MapViewOfFile(mapHandle, 4u, 0, 0, 0);
			if (!filePtrOrPath)
			{
				returnCode = 2;
				break;
			}
		}
		returnCode = read_file(midi, static_cast<riff_header*>(filePtrOrPath), fileSize);
	}
	while (false);


	if (returnCode)
	{
		if (midi)
			LocalFree(midi);
	}
	else
	{
		*midi_res = midi;
	}

	if (fileFlag)
	{
		if (filePtrOrPath)
			UnmapViewOfFile(filePtrOrPath);
		if (mapHandle)
			CloseHandle(mapHandle);
		if (fileHandle != INVALID_HANDLE_VALUE)
			CloseHandle(fileHandle);
	}
	return returnCode;
}

int midi::read_file(midi_struct* midi, riff_header* filePtr, unsigned fileSize)
{
	auto returnCode = 0;

	do
	{
		midi->DataPtr1 = nullptr;
		if (fileSize < 12)
		{
			returnCode = 3;
			break;
		}
		if (filePtr->Riff != 'FFIR')
		{
			returnCode = 3;
			break;
		}
		if (filePtr->Mids != 'SDIM')
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
		if (filePtr->Fmt != ' tmf')
		{
			returnCode = 3;
			break;
		}
		if (filePtr->FmtSize > fileSize - 12)
		{
			returnCode = 3;
			break;
		}
		if (filePtr->FmtSize < 12)
		{
			returnCode = 3;
			break;
		}

		midi->DwTimeFormat = filePtr->dwTimeFormat;
		midi->CbMaxBuffer = filePtr->cbMaxBuffer;
		midi->DwFlagsFormat = filePtr->dwFlags;
		auto blocksSize = fileSize - 20 - filePtr->FmtSize;
		if (blocksSize < 8)
		{
			returnCode = 3;
			break;
		}

		auto dataChunk = reinterpret_cast<riff_data*>(reinterpret_cast<char*>(&filePtr->dwTimeFormat) + filePtr->FmtSize
		);
		if (dataChunk->Data != 'atad')
		{
			returnCode = 3;
			break;
		}
		if (dataChunk->DataSize > blocksSize || dataChunk->DataSize < 4)
		{
			returnCode = 3;
			break;
		}

		midi->BlockCount = dataChunk->BlocksPerChunk;
		midi->DataPtr1 = static_cast<midihdr_tag*>(GlobalLock(GlobalAlloc(
			GMEM_DDESHARE | GMEM_MOVEABLE, dataChunk->BlocksPerChunk * (midi->CbMaxBuffer + sizeof(midihdr_tag)))));
		if (!midi->DataPtr1)
		{
			returnCode = 1;
			break;
		}
		if (!midi->BlockCount)
		{
			returnCode = 3;
			break;
		}

		auto blocksSizeIndex = blocksSize - 12;
		auto srcPtr = dataChunk->Blocks;
		auto* dstPtr = midi->DataPtr1;
		for (auto blockIndex = midi->BlockCount; blockIndex; blockIndex--)
		{
			dstPtr->lpData = reinterpret_cast<LPSTR>(&dstPtr[1]);
			dstPtr->dwBufferLength = midi->CbMaxBuffer;
			dstPtr->dwFlags = 0;
			dstPtr->dwUser = reinterpret_cast<DWORD_PTR>(midi);
			dstPtr->lpNext = nullptr;
			if (blocksSizeIndex < 8)
			{
				returnCode = 3;
				break;
			}

			auto blockSize = srcPtr->CbBuffer;
			if (blockSize > midi->CbMaxBuffer || blockSize > blocksSizeIndex - 8)
			{
				returnCode = 3;
				break;
			}

			if ((midi->DwFlagsFormat & 1) != 0)
			{
				/*Not used in FT, some kind of compression*/
				assertm(false, "Unimplemented code reached");
				/*int a1[16]; 
				a1[0] = (int)blockDataPtr;
				a1[2] = blockSize;
				a1[1] = blockSize;
				if (!sub_4031A0(a1, dataPtr))
				{
					returnCode = 3;	break;
				}*/
			}
			else
			{
				dstPtr->dwBytesRecorded = blockSize;
				memcpy(dstPtr->lpData, srcPtr->AData, blockSize);
			}
			blocksSizeIndex -= blockSize + 8;
			srcPtr = reinterpret_cast<riff_block*>(&srcPtr->AData[blockSize]);
			dstPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&dstPtr[1]) + midi->CbMaxBuffer);
		}
	}
	while (false);

	if (returnCode && midi->DataPtr1)
	{
		GlobalUnlock(GlobalHandle(midi->DataPtr1));
		GlobalFree(GlobalHandle(midi->DataPtr1));
	}
	return returnCode;
}

int midi::play_ft(midi_struct* midi)
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
	if (stream_open(midi, 1))
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
		returnCode = stream_close(active_track);
	active_track = nullptr;
	return returnCode;
}

int midi::unload_track(midi_struct* midi)
{
	if (midi->Magic != 'ISDM')
		return 6;
	if (midi->StreamHandle)
		stream_close(midi);
	if (midi->DataPtr1)
	{
		GlobalUnlock(GlobalHandle(midi->DataPtr1));
		GlobalFree(GlobalHandle(midi->DataPtr1));
	}
	midi->Magic = 'atad';
	LocalFree(midi);
	return 0;
}

int midi::stream_open(midi_struct* midi, char flags)
{
	auto returnCode = 0;
	if (midi->Magic != 'ISDM')
		return 6;

	UINT puDeviceID = -1;
	auto steamOpenedFg = !midi->StreamHandle;
	MIDIPROPTIMEDIV propdata{8, midi->DwTimeFormat};
	if (steamOpenedFg &&
		!midiStreamOpen(&midi->StreamHandle, &puDeviceID, 1u, reinterpret_cast<DWORD_PTR>(midi_callback), 0,
		                CALLBACK_FUNCTION) &&
		!midiStreamProperty(midi->StreamHandle, reinterpret_cast<LPBYTE>(&propdata),MIDIPROP_TIMEDIV | MIDIPROP_SET))
	{
		midihdr_tag* blockPtr = midi->DataPtr1;
		for (auto blockIndex = midi->BlockCount; blockIndex; blockIndex--)
		{
			if (midiOutPrepareHeader(reinterpret_cast<HMIDIOUT>(midi->StreamHandle), blockPtr, sizeof(MIDIHDR)) ||
				midiStreamOut(midi->StreamHandle, blockPtr, sizeof(MIDIHDR)))
			{
				returnCode = 5;
				break;
			}

			++midi->PreparedBlocksCount;
			blockPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&blockPtr[1]) + blockPtr->dwBufferLength);
		}
	}

	if (!returnCode)
	{
		if (!steamOpenedFg && (midi->SomeFlag2 & 4) == 0)
			return 7;

		midi->SomeFlag2 &= ~2;
		if ((flags & 1) != 0)
			midi->SomeFlag2 |= 2;
		midi->SomeFlag2 &= ~4;
		if (midiStreamRestart(midi->StreamHandle))
			returnCode = 5;
	}

	if (returnCode && steamOpenedFg)
	{
		if (midi->StreamHandle)
			stream_close(midi);
	}
	return returnCode;
}

int midi::stream_close(midi_struct* midi)
{
	int returnCode;

	if (midi->Magic != 'ISDM')
		return 6;
	if (!midi->StreamHandle)
		return 7;
	midi->SomeFlag2 |= 1u;
	if (midiOutReset(reinterpret_cast<HMIDIOUT>(midi->StreamHandle)))
	{
		returnCode = 5;
		midi->SomeFlag2 &= ~1;
	}
	else
	{
		midihdr_tag* blockPtr = midi->DataPtr1;
		for (int i = midi->BlockCount; i; --i)
		{
			midiOutUnprepareHeader(reinterpret_cast<HMIDIOUT>(midi->StreamHandle), blockPtr, sizeof(MIDIHDR));
			blockPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&blockPtr[1]) + blockPtr->dwBufferLength);
		}
		midiStreamClose(midi->StreamHandle);
		returnCode = 0;
		midi->StreamHandle = nullptr;
		midi->SomeFlag2 = 0;
	}
	return returnCode;
}

void midi::midi_callback(HMIDIOUT hmo, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (wMsg == 969)
	{
		auto mhdr = reinterpret_cast<LPMIDIHDR>(dwParam1);
		auto midi = reinterpret_cast<midi_struct*>(mhdr->dwUser);
		if ((midi->SomeFlag2 & 2) == 0 || (midi->SomeFlag2 & 1) != 0 || midiStreamOut(
			midi->StreamHandle, mhdr, sizeof(MIDIHDR)))
			--midi->PreparedBlocksCount;
	}
}
