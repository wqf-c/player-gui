#include"AudioPlayer.h"


AudioPlayer::AudioPlayer(string playerName, int pFreq) {
	freq = pFreq;
	playerDeviceName = playerName;

	playerSource = 0;
	if (playerName.length() != 0) {
		playerDevice = alcOpenDevice(playerName.c_str());
	}
	else {
		playerDevice = alcOpenDevice(nullptr);
	}

	if (playerDevice == nullptr) {
		printf("open player device fail\n");
		return;
	}
	playerContext = alcCreateContext(playerDevice, nullptr);
	if (playerContext == nullptr) {
		printf("create player context fail\n");
		return;
	}
	alcMakeContextCurrent(playerContext);

	if (checkALError()) {
		return;
	}
	alGenSources(1, &playerSource);
	alGenBuffers(MAX_CACHE, playerBuffer);
	if (checkALError()) {
		return;
	}
	for (auto buf : playerBuffer) {
		playerBufferQueue.push_back(buf);
	}

}

//void MediaPlayer::setDecoder(MediaDecoder *_decoder) {
//	decoder = _decoder;
//	auto decoderCallback = [&](uint8_t *data, int len) {
//		workData(data, len);
//	};
//
//	decoder->setCallback(decoderCallback);
//}

//void MediaPlayer::play(uint8_t *data, int len, bool useSsrc, uint32_t ssrc, uint32_t timeStamp) {
//	D_LOG("recv play data, len:{}", len);
//	//decoder->process(data, len, useSsrc, ssrc, timeStamp);
//	//workData(data, len);
//}

void AudioPlayer::workData(uint8_t *data, int len) {

	//	unique_lock<mutex> locker{ mu };

	recycle();
	if (data != nullptr && !playerBufferQueue.empty()) {
		ALuint buffer = playerBufferQueue.front();
		playerBufferQueue.pop_front();
		alBufferData(buffer, AL_FORMAT_MONO16, data, len, freq);
		alSourceQueueBuffers(playerSource, 1, &buffer);
		bufferIndex = 0;
	}

	resume();

	//todo 播放结束清空该数据对应的ssrc的用户名称
//	locker.unlock();
}

bool AudioPlayer::isPlaying() {
	return playState() == AL_PLAYING;
}

ALint AudioPlayer::playState() {
	ALint playState = 0;
	alGetSourcei(playerSource, AL_SOURCE_STATE, &playState);
	return playState;
}

void AudioPlayer::resume() {
	if (!isPlaying()) {
		ALint bufferQueued;
		alGetSourcei(playerSource, AL_BUFFERS_QUEUED, &bufferQueued);
		if (bufferQueued != 0) {
			alSourcePlay(playerSource);
		}
	}
}

void AudioPlayer::pause() {
	if (isPlaying()) {
		alSourcePause(playerSource);
	}
}

void AudioPlayer::stop() {
	if (isPlaying()) {
		alSourceStop(playerSource);
	}
}

void AudioPlayer::recycle() {
	ALint proceBufNum;
	alGetSourcei(playerSource, AL_BUFFERS_PROCESSED, &proceBufNum);
	if (proceBufNum > 0) {
		ALuint buffers[MAX_CACHE];
		alSourceUnqueueBuffers(playerSource, proceBufNum, buffers);
		for (int i = 0; i < proceBufNum; ++i) {
			playerBufferQueue.push_back(buffers[i]);
		}
	}
}

void AudioPlayer::makeALCurrent() {
	alcMakeContextCurrent(playerContext);
}


bool AudioPlayer::checkALError() {
	int loopCnt = 0;
	for (ALenum error = alGetError(); loopCnt < 32 && error; error = alGetError(), ++loopCnt) {
		const char* pMsg;
		switch (error)
		{
		case AL_INVALID_NAME:
			pMsg = "invalid name";
			break;
		case AL_INVALID_ENUM:
			pMsg = "invalid enum";
			break;
		case AL_INVALID_VALUE:
			pMsg = "invalid value";
			break;
		case AL_INVALID_OPERATION:
			pMsg = "invalid operation";
			break;
		case AL_OUT_OF_MEMORY:
			pMsg = "out of memory";
			break;
		default:
			pMsg = "unknown error";
		}
		printf("alGetError:{%s}", pMsg);
	}
	return loopCnt != 0;
}

AudioPlayer::~AudioPlayer() {
	if (playerContext != nullptr) {
		alDeleteSources(1, &playerSource);
		playerSource = 0;
		alDeleteBuffers(MAX_CACHE, playerBuffer);
		memset(playerBuffer, 0, sizeof(playerBuffer));
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(playerContext);
		playerContext = nullptr;
		alcCloseDevice(playerDevice);
		playerDevice = nullptr;
		playerBufferQueue.clear();
	}
	//if (decoder != nullptr) {
	//	delete decoder;
	//}
}