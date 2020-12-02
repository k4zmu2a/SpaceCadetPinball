#pragma once
class midi
{
public:
	static MCIERROR play_pb_theme(int flag);
	static MCIERROR music_stop();
	static int music_init(HWND hwnd);
	static MCIERROR restart_midi_seq(int param);
};
