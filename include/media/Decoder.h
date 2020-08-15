#pragma once
#include<iostream>
#include<vector>
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

using std::vector;

class Decoder
{
public:
	Decoder();
	void setCodec(AVCodecParameters *codecpar);
	void decode(vector<AVFrame *> &frames, AVPacket *pkt);
	int64_t getChannelLayout() { return codecCtx->channel_layout; }
	AVSampleFormat getSampleFmt() { return codecCtx->sample_fmt; }
	AVPixelFormat getPixFmt() { return codecCtx->pix_fmt; }
	int getSampleRate() { return codecCtx->sample_rate; }
	int getHeight() { return codecCtx->height; }
	int getWidth() { return codecCtx->width; }
	~Decoder();

private:
	AVCodec *codec;
	AVCodecContext *codecCtx;
};



