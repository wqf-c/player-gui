#pragma once
extern "C"
{
#include "SDL2/SDL.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include<libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}
#include<iostream>
#include<string>
#include<mutex>
#include<thread>
#include<condition_variable>
#include<Windows.h>
#include<deque>
#include"Decoder.h"
#include"AudioPlayer.h"

using std::string;
using std::mutex;
using std::thread;
using std::condition_variable;
using std::unique_lock;
using std::deque;

#define MAX_AUDIO_FRAME_SIZE 192000

enum CallBackType
{
	VIDEODURATION,
	PLAYPROGRESS,
	VIDEOEND
};

typedef std::function<void(CallBackType, int)> UI_CALLBACK;

class Controller
{
public:
	Controller();
	void start(string fileName);
	void setStopFlag(bool flag);
	void grabPkt();
	static int getAudioFrameRate(AVStream *inputAudioStream, int nb_samples);
	void videoThreadFunc();
	void audioThreadFunc();
	static int getVideoFrameRate(AVStream * inputVideoStream);
	void setFrameVec(vector<AVFrame *> *frameVec) { frameRGBvec = frameVec; }
	void setLock(mutex *_lock) { locker = _lock; }
	void setUiCallBack(UI_CALLBACK _callBack) { callBack = _callBack; }
	void pause();
	void reStart();
	void seekVideo(long ms);
	~Controller();

private:
	bool					stopFlag = false;
	AVFormatContext			*pFormatCtx = NULL;
	AVStream				*audioStream = NULL;
	int						audioIndex = -1;
	AVStream				*videoStream = NULL;
	int						videoIndex = -1;
	Decoder					audioDecoder;
	Decoder					videoDecoder;
	struct SwrContext		*audio_convert_ctx;
	struct SwsContext		*img_convert_ctx;
	deque<AVPacket *>		videoQue{};
	deque<AVPacket *>		audioQue{};
	uint8_t					*out_buffer = NULL;
	condition_variable		cond;
	mutex					mux;
	AudioPlayer				*audioPlayer = NULL;
	vector<AVFrame *>		*frameRGBvec{};
	mutex					*locker;
	mutex                   ctxMux;
	int						videoTime;
	int						audioTime;
	int					faster = 0;
	uint64_t out_chn_layout = AV_CH_LAYOUT_STEREO;  //通道布局 输出双声道
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; //声音格式
	int out_sample_rate = 44100;   //采样率
	int width = 640;
	int height = 480;
	UI_CALLBACK callBack;
	bool noMoreFrame = false;
};

