#include "pch.h"
#include "midi.h"

#include "pb.h"
#include "pinball.h"

HWND midi::midi_notify_hwnd;
objlist_class<BaseMidi>* midi::TrackList;
BaseMidi* midi::track1, * midi::track2, * midi::track3;
MidiTracks midi::ActiveTrack, midi::NextTrack;
bool midi::IsPlaying;

void midi::music_play()
{
	if (!IsPlaying)
	{
		IsPlaying = true;
		play_track(NextTrack, true);
		NextTrack = MidiTracks::None;
	}
}

void midi::music_stop()
{
	if (IsPlaying)
	{
		IsPlaying = false;
		NextTrack = ActiveTrack;
		StopPlayback();
	}
}

int midi::music_init(HWND hwnd)
{
	TrackList = new objlist_class<BaseMidi>(0, 1);
	midi_notify_hwnd = hwnd;
	ActiveTrack = NextTrack = MidiTracks::None;
	track1 = track2 = track3 = nullptr;
	if (!pb::FullTiltMode)
	{
		track1 = LoadTrack("PINBALL.MID");
	}
	else 
	{
		track1 = LoadTrack("TABA1.MDS");
		track2 = LoadTrack("TABA2.MDS");
		track3 = LoadTrack("TABA3.MDS");
	}

	return track1 != nullptr;
}

void midi::restart_midi_seq(LPARAM param)
{
	auto midi = TrackToMidi(ActiveTrack);
	if (midi != nullptr)
		midi->Play();
}

void midi::music_shutdown()
{
	music_stop();

	while (TrackList->GetCount()) {
		auto midi = TrackList->Get(0);
		TrackList->Delete(midi);
		delete midi;
	}

	delete TrackList;
}

void midi::StopPlayback()
{
	auto midi = TrackToMidi(ActiveTrack);
	if (midi != nullptr) 
	{
		midi->Stop();
		ActiveTrack = MidiTracks::None;
	}
}

BaseMidi* midi::LoadTrack(LPCSTR fileName)
{
	BaseMidi* track;
	if (pb::FullTiltMode)
	{
		track = new MdsMidi(fileName);
	}
	else 
	{
		track = new MciMidi(fileName, midi_notify_hwnd);
	}
	if (track->IsOpen) 
	{
		TrackList->Add(track);
		return track;
	}

	delete track;
	return nullptr;
}

BaseMidi* midi::TrackToMidi(MidiTracks track)
{
	BaseMidi* midi;
	switch (track)
	{
	default:
	case MidiTracks::None:
		midi = nullptr;
		break;
	case MidiTracks::Track1:
		midi = track1;
		break;
	case MidiTracks::Track2:
		midi = track2;
		break;
	case MidiTracks::Track3:
		midi = track3;
		break;
	}
	return midi;
}

bool midi::play_track(MidiTracks track, bool replay)
{
	auto midi = TrackToMidi(track);
	if (!midi || (!replay && ActiveTrack == track))
		return false;

	StopPlayback();

	if (!IsPlaying)
	{
		NextTrack = track;
		return false;
	}

	midi->Play();
	ActiveTrack = track;
	return true;
}

#pragma region MdsMidi
MdsMidi::MdsMidi(LPCSTR fileName)
{
	char filePath[256];
	char fileName2[256];

	lstrcpyA(fileName2, "sound\\");
	lstrcatA(fileName2, fileName);
	pinball::make_path_name(filePath, fileName2, 254u);
	IsOpen = load_file(midi, filePath, 0, 1) == 0;
}

MdsMidi::~MdsMidi()
{
	if (midi.Magic == mmioFOURCC('M', 'D', 'S', 'I'))
	{
		if (midi.StreamHandle)
			stream_close(midi);
		if (midi.DataPtr1)
		{
			memory::free(midi.DataPtr1);
		}
		midi.Magic = mmioFOURCC('d', 'a', 't', 'a');
	}
}

void MdsMidi::Play()
{
	Stop();
	stream_open(midi, 1);
}

void MdsMidi::Stop()
{
	stream_close(midi);
}

int MdsMidi::stream_close(midi_struct& midi)
{
	int returnCode;

	if (midi.Magic != mmioFOURCC('M', 'D', 'S', 'I'))
		return 6;
	if (!midi.StreamHandle)
		return 7;
	midi.SomeFlag2 |= 1u;
	if (midiOutReset(reinterpret_cast<HMIDIOUT>(midi.StreamHandle)))
	{
		returnCode = 5;
		midi.SomeFlag2 &= ~1;
	}
	else
	{
		midihdr_tag* blockPtr = midi.DataPtr1;
		for (int i = midi.BlockCount; i; --i)
		{
			midiOutUnprepareHeader(reinterpret_cast<HMIDIOUT>(midi.StreamHandle), blockPtr, sizeof(MIDIHDR));
			blockPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&blockPtr[1]) + blockPtr->dwBufferLength);
		}
		midiStreamClose(midi.StreamHandle);
		returnCode = 0;
		midi.StreamHandle = nullptr;
		midi.SomeFlag2 = 0;
	}
	return returnCode;
}

int MdsMidi::stream_open(midi_struct& midi, char flags)
{
	auto returnCode = 0;
	if (midi.Magic != mmioFOURCC('M', 'D', 'S', 'I'))
		return 6;

	UINT puDeviceID = -1;
	auto steamOpenedFg = !midi.StreamHandle;
	MIDIPROPTIMEDIV propdata{ 8, midi.DwTimeFormat };
	if (steamOpenedFg &&
		!midiStreamOpen(&midi.StreamHandle, &puDeviceID, 1u, reinterpret_cast<DWORD_PTR>(midi_callback), 0,
			CALLBACK_FUNCTION) &&
		!midiStreamProperty(midi.StreamHandle, reinterpret_cast<LPBYTE>(&propdata), MIDIPROP_TIMEDIV | MIDIPROP_SET))
	{
		midihdr_tag* blockPtr = midi.DataPtr1;
		for (auto blockIndex = midi.BlockCount; blockIndex; blockIndex--)
		{
			if (midiOutPrepareHeader(reinterpret_cast<HMIDIOUT>(midi.StreamHandle), blockPtr, sizeof(MIDIHDR)) ||
				midiStreamOut(midi.StreamHandle, blockPtr, sizeof(MIDIHDR)))
			{
				returnCode = 5;
				break;
			}

			++midi.PreparedBlocksCount;
			blockPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&blockPtr[1]) + blockPtr->dwBufferLength);
		}
	}

	if (!returnCode)
	{
		if (!steamOpenedFg && (midi.SomeFlag2 & 4) == 0)
			return 7;

		midi.SomeFlag2 &= ~2;
		if ((flags & 1) != 0)
			midi.SomeFlag2 |= 2;
		midi.SomeFlag2 &= ~4;
		if (midiStreamRestart(midi.StreamHandle))
			returnCode = 5;
	}

	if (returnCode && steamOpenedFg)
	{
		if (midi.StreamHandle)
			stream_close(midi);
	}
	return returnCode;
}

int MdsMidi::load_file(midi_struct& midi, void* filePtrOrPath, int fileSizeP, int flags)
{
	int returnCode;
	unsigned int fileSize;
	HANDLE mapHandle = nullptr;
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	int fileFlag = 0;

	do
	{
		if ((flags & 3) == 0 || (flags & 3) == 3)
		{
			returnCode = 4;
			break;
		}

		midi.Magic = mmioFOURCC('M', 'D', 'S', 'I');
		midi.StreamHandle = nullptr;
		midi.PreparedBlocksCount = 0;

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
	} while (false);

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

int MdsMidi::read_file(midi_struct& midi, riff_header* filePtr, unsigned fileSize)
{
	auto returnCode = 0;

	do
	{
		midi.DataPtr1 = nullptr;
		if (fileSize < 12)
		{
			returnCode = 3;
			break;
		}
		if (filePtr->Riff != mmioFOURCC('R', 'I', 'F', 'F'))
		{
			returnCode = 3;
			break;
		}
		if (filePtr->Mids != mmioFOURCC('M', 'I', 'D', 'S'))
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
		if (filePtr->Fmt != mmioFOURCC('f', 'm', 't', ' '))
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

		midi.DwTimeFormat = filePtr->dwTimeFormat;
		midi.CbMaxBuffer = filePtr->cbMaxBuffer;
		midi.DwFlagsFormat = filePtr->dwFlags;
		auto blocksSize = fileSize - 20 - filePtr->FmtSize;
		if (blocksSize < 8)
		{
			returnCode = 3;
			break;
		}

		auto dataChunk = reinterpret_cast<riff_data*>(reinterpret_cast<char*>(&filePtr->dwTimeFormat) + filePtr->FmtSize
			);
		if (dataChunk->Data != mmioFOURCC('d', 'a', 't', 'a'))
		{
			returnCode = 3;
			break;
		}
		if (dataChunk->DataSize > blocksSize || dataChunk->DataSize < 4)
		{
			returnCode = 3;
			break;
		}

		midi.BlockCount = dataChunk->BlocksPerChunk;
		midi.DataPtr1 = reinterpret_cast<midihdr_tag*>(memory::allocate(
			dataChunk->BlocksPerChunk * (midi.CbMaxBuffer + sizeof(midihdr_tag))));
		if (!midi.DataPtr1)
		{
			returnCode = 1;
			break;
		}
		if (!midi.BlockCount)
		{
			returnCode = 3;
			break;
		}

		auto blocksSizeIndex = blocksSize - 12;
		auto srcPtr = dataChunk->Blocks;
		auto* dstPtr = midi.DataPtr1;
		for (auto blockIndex = midi.BlockCount; blockIndex; blockIndex--)
		{
			dstPtr->lpData = reinterpret_cast<LPSTR>(&dstPtr[1]);
			dstPtr->dwBufferLength = midi.CbMaxBuffer;
			dstPtr->dwFlags = 0;
			dstPtr->dwUser = reinterpret_cast<DWORD_PTR>(&midi);
			dstPtr->lpNext = nullptr;
			if (blocksSizeIndex < 8)
			{
				returnCode = 3;
				break;
			}

			auto blockSize = srcPtr->CbBuffer;
			if (blockSize > midi.CbMaxBuffer || blockSize > blocksSizeIndex - 8)
			{
				returnCode = 3;
				break;
			}

			if ((midi.DwFlagsFormat & 1) != 0)
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
			dstPtr = reinterpret_cast<midihdr_tag*>(reinterpret_cast<char*>(&dstPtr[1]) + midi.CbMaxBuffer);
		}
	} while (false);

	if (returnCode && midi.DataPtr1)
	{
		memory::free(midi.DataPtr1);
	}
	return returnCode;
}

void MdsMidi::midi_callback(HMIDIOUT hmo, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
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

#pragma endregion

#pragma region MciMidi

MciMidi::MciMidi(LPCSTR fileName, HWND notifyHwnd)
{
	char filePath[256];
	MCI_OPEN_PARMS mci_open_info{};
	midi_notify_hwnd = notifyHwnd;

	pinball::make_path_name(filePath, fileName, 254u);
	mci_open_info.lpstrElementName = filePath;
	auto result = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_NOTIFY_SUPERSEDED, (DWORD_PTR)&mci_open_info);
	IsOpen = result == 0;
	DeviceId = mci_open_info.wDeviceID;
}

MciMidi::~MciMidi()
{
	if (IsOpen)
		mciSendCommandA(DeviceId, MCI_CLOSE, 0, 0);
	IsOpen = false;
}

void MciMidi::Play()
{
	MCI_PLAY_PARMS playParams{ (DWORD_PTR)midi_notify_hwnd, 0, 0 };

	Stop();

	if (IsOpen)
	{
		auto result = mciSendCommandA(DeviceId, MCI_PLAY, MCI_FROM | MCI_NOTIFY, (DWORD_PTR)&playParams);
		IsPlaying = result == 0;
	}
}

void MciMidi::Stop()
{
	if (IsPlaying)
		mciSendCommandA(DeviceId, MCI_STOP, 0, 0);
	IsPlaying = false;
}

#pragma endregion
