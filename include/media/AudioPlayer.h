#pragma once

#include<iostream>
#include<string>
#include<list>
#include<thread>
#ifdef WIN32
#include<windows.h>
#endif // WIN32
#ifdef _WIN32
#include "al.h"
#include "alc.h"
#else
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#endif

using std::cout;
using std::endl;
using std::string;
using std::list;

#define DECODE_DATA_SIZE 8192
#define MAX_CACHE 5
#define PLAY_AUDIO_DATA_SIZE 512
class AudioPlayer {
public:
	void workData(uint8_t *data, int len);
	//void play(uint8_t *data, int len, bool useSsrc, uint32_t ssrc, uint32_t timeStamp);
	//void setDecoder(MediaDecoder *_decoder);
	AudioPlayer(string playerName = "", int pFreq = 22050);
	~AudioPlayer();

private:


	string playerDeviceName;
	ALCcontext *playerContext;
	ALCdevice *playerDevice;

	int freq;
	//typedef unsigned int ALuint;
	ALuint playerSource;
	ALuint playerBuffer[MAX_CACHE];
	uint8_t audioBuffer[PLAY_AUDIO_DATA_SIZE];
	int bufferIndex;
	std::list<ALuint> playerBufferQueue;
	bool checkALError();
	bool isPlaying();
	ALint playState();
	void resume();
	void pause();
	void stop();
	void recycle();
	void makeALCurrent();

};
