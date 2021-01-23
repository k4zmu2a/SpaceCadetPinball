#include "pch.h"
#include "midi.h"

#include "pinball.h"

tagMCI_OPEN_PARMSA midi::mci_open_info;
char midi::midi_device_type[28];
HWND midi::midi_notify_hwnd;
int midi::midi_seq1_open, midi::midi_seq1_playing;

MCIERROR midi::play_pb_theme(int flag)
{
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
	MCIERROR result = 0;
	if (midi_seq1_playing)
		result = mciSendCommandA(mci_open_info.wDeviceID, MCI_STOP, 0, 0);
	return result;
}

int midi::music_init(HWND hwnd)
{
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
	if (midi_seq1_open)
		mciSendCommandA(mci_open_info.wDeviceID, MCI_CLOSE, 0, 0);
	midi_seq1_open = 0;
}
