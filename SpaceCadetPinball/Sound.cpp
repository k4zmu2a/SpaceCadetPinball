#include "options.h"
#include "pch.h"
#include "Sound.h"

int Sound::num_channels;
bool Sound::enabled_flag = false;
int* Sound::TimeStamps = nullptr;
int Sound::Volume = MIX_MAX_VOLUME;

bool Sound::Init(int channels, bool enableFlag, int volume)
{
	Volume = volume;
	Mix_Init(MIX_INIT_MID_Proxy);
	auto result = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
	SetChannels(channels);
	Enable(enableFlag);
	return !result;
}

void Sound::Enable(bool enableFlag)
{
	enabled_flag = enableFlag;
	if (!enableFlag)
		Mix_HaltChannel(-1);
}

void Sound::Activate()
{
	Mix_Resume(-1);
}

void Sound::Deactivate()
{
	Mix_Pause(-1);
}

void Sound::Close()
{
	delete[] TimeStamps;
	TimeStamps = nullptr;
	Mix_CloseAudio();
	Mix_Quit();
}

void Sound::PlaySound(Mix_Chunk* wavePtr, int time, TPinballComponent *soundSource, const char* info)
{
	if (wavePtr && enabled_flag)
	{
		if (Mix_Playing(-1) == num_channels)
		{
			auto oldestChannel = std::min_element(TimeStamps, TimeStamps + num_channels) - TimeStamps;
			Mix_HaltChannel(oldestChannel);
		}

		auto channel = Mix_PlayChannel(-1, wavePtr, 0);
		if (channel != -1) {
			TimeStamps[channel] = time;
			if (options::Options.SoundStereo) {
				/* Think 3D sound positioning, where:
				 *   - x goes from 0 to 1, left to right on the screen,
				 *   - y goes from 0 to 1, top to bottom on the screen,
				 *   - z goes from 0 to infinity, from table-level to the sky.
				 *
				 * We position the listener at the bottom center of the table,
				 * at 0.5 height, so roughly a table half-length.  Coords of
				 * the listener are thus {0.5, 1.0, 0.5}.
				 *
				 * We use basic trigonometry to calculate the angle and distance
				 * from a sound source to the listener.
				 *
				 * Mix_SetPosition expects an angle in (Sint16)degrees, where
				 * 0 degrees is in front, 90 degrees is to the right, and so on.
				 * Mix_SetPosition expects a (Uint8)distance from 0 (near) to 255 (far).
				 */

				/* Get the sound source position. */
				vector2 coordinates;
				/* Some sounds are unpositioned; for that case the caller sends
				 * a NULL pointer as a soundSource; in those cases we position
				 * the sound at the center top of the table.
				 */
				if (!soundSource) {
					coordinates.X = 0.5f;
					coordinates.Y = 0.0f;
				}
				else {
					coordinates = soundSource->get_coordinates();
				};

				/* Player position. */
				auto pX = 0.5f;
				auto pY = 1.0f;
				auto pZ = 0.5f;

				/* Calculate lengths of three sides of a triangle.
				 * ptos (Player-to-sound):  distance from listener to the sound source,
				 * ptom (player-to-middle): distance from listener to the sound source
				 *                          when the latter is repositioned to the
				 *                          X center,
				 * stom (sound-to-middle):  distance from ptos to ptom.
				 */
				auto ptos = sqrt(((coordinates.X - pX) * (coordinates.X - pX)) + ((coordinates.Y - pY) * (coordinates.Y - pY)) + (pZ * pZ));
				auto ptom = sqrt(((coordinates.Y - pY) * (coordinates.Y - pY)) + (pZ * pZ));
				auto stom = fabs(coordinates.X - 0.5);

				/* Calculate the angle using the law of cosines and acos().
				 * That will return an angle in radians, e.g. in the [0,PI] range;
				 * we remap to [0,180], and cast to an integer.
				 */
				Sint16 angle = (Sint16)(acos(((stom * stom) - (ptos * ptos) - (ptom * ptom)) / (-2.0f * ptos * ptom)) * 180.0f / IM_PI);

				/* Because we are using distances to calculate the angle,
				 * we now have no clue if the sound is to the right or the
				 * left.  If the sound is to the right, the current value
				 * is good, but to the left, we need substract it from 360.
				 */
				if (coordinates.X < 0.5) {
					angle = (360 - angle);
				}

				/* Distance from listener to the ball (ptos) is roughly
				 * in the [0.5,1.55] range; remap to 50-155 by multiplying
				 * by 100 and cast to an integer. */
				Uint8 distance = (Uint8)(100.0f * ptos);
				Mix_SetPosition(channel, angle, distance);

				/* Output position of each sound emitted so we can verify
				 * the sanity of the implementation.
				 */
				/*
				printf("X: %3.3f Y: %3.3f Angle: %3d Distance: %3d, Object: %s\n",
					coordinates.X,
					coordinates.Y,
					angle,
					distance,
					info
				);
				*/
			}
		}
	}
}

Mix_Chunk* Sound::LoadWaveFile(const std::string& lpName)
{
	auto wavFile = fopenu(lpName.c_str(), "r");
	if (!wavFile)
		return nullptr;
	fclose(wavFile);

	return Mix_LoadWAV(lpName.c_str());
}

void Sound::FreeSound(Mix_Chunk* wave)
{
	if (wave)
		Mix_FreeChunk(wave);
}

void Sound::SetChannels(int channels)
{
	if (channels <= 0)
		channels = 8;

	num_channels = channels;
	delete[] TimeStamps;
	TimeStamps = new int[num_channels]();
	Mix_AllocateChannels(num_channels);
	SetVolume(Volume);
}

void Sound::SetVolume(int volume)
{
	Volume = volume;
	Mix_Volume(-1, volume);
}
