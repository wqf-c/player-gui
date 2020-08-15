
#include<iostream>
#include"Decoder.h"

Decoder::Decoder() {
	
}

void Decoder::setCodec(AVCodecParameters *codecpar) {
	if (codecCtx != NULL) {
		avcodec_free_context(&codecCtx);
	}
	codec = avcodec_find_decoder(codecpar->codec_id);
	if (codec == NULL) {
		printf("open codec fail\n");
		return;
	}
	codecCtx = avcodec_alloc_context3(NULL);
	if (codecCtx == NULL) {
		printf("allow codec context fail\n");
		codec = NULL;
		return;
	}
	if ((avcodec_parameters_to_context(codecCtx, codecpar)) < 0) {
		fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
			av_get_media_type_string(AVMEDIA_TYPE_AUDIO));
		if (codecCtx != NULL) {
			avcodec_free_context(&codecCtx);
		}
	}
}

void Decoder::decode(vector<AVFrame *> &frames, AVPacket *pkt) {
	int ret = avcodec_send_packet(codecCtx, pkt);
	if (ret < 0) {
		printf("Error submitting the packet to the decoder\n");
		return;
	}

	/* read all the output frames (in general there may be any number of them */
	while (ret >= 0) {
		AVFrame *d_frame = av_frame_alloc();
		ret = avcodec_receive_frame(codecCtx, d_frame);
		if (d_frame->nb_samples == 0) {
			av_frame_free(&d_frame);
			return;
		}
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&d_frame);
			return;
		}
		else if (ret < 0) {
			av_frame_free(&d_frame);
			printf("Error during decoding\n");
			return;
		}
		//av_frame_free(&d_frame);
		frames.push_back(d_frame);
	}
}

Decoder::~Decoder()
{
	if (codecCtx != NULL) {
		avcodec_free_context(&codecCtx);
	}
}