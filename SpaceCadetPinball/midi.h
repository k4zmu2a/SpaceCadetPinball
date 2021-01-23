#pragma once
class midi
{
public:
	static MCIERROR play_pb_theme(int flag);
	static MCIERROR music_stop();
	static int music_init(HWND hwnd);
	static MCIERROR restart_midi_seq(int param);
	static void music_shutdown();

private:
	static tagMCI_OPEN_PARMSA mci_open_info;
	static char midi_device_type[28];
	static HWND midi_notify_hwnd;
	static int midi_seq1_open, midi_seq1_playing;
};
