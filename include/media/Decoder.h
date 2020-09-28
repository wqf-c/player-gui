#pragma once
#include<iostream>
#include<vector>
extern "C"
{
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
#include<libavutil/time.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include "libavutil/opt.h"
#include "libavutil/time.h"
}

using std::vector;

class Decoder
{
public:
	Decoder();
	void setCodec(AVCodecParameters *codecpar);
	AVFrame * decode(AVPacket *pkt);
	int64_t getChannelLayout() { return codecCtx->channel_layout; }
	AVSampleFormat getSampleFmt() { return codecCtx->sample_fmt; }
	AVPixelFormat getPixFmt() { return codecCtx->pix_fmt; }
	int getSampleRate() { return codecCtx->sample_rate; }
	int getHeight() { return codecCtx->height; }
	int getWidth() { return codecCtx->width; }
	~Decoder();

public:
	AVCodec *codec = NULL;
	AVCodecContext *codecCtx = NULL;
};



