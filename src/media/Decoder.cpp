
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
	if (avcodec_open2(codecCtx, codec, NULL) != 0) {
		printf("codec open fail\n");
		return;
	}
}

AVFrame * Decoder::decode(AVPacket *pkt) {
	int ret = avcodec_send_packet(codecCtx, pkt);
	if (ret < 0) {
		printf("Error submitting the packet to the decoder\n");
		return NULL;
	}

	/* read all the output frames (in general there may be any number of them */

	AVFrame *d_frame = av_frame_alloc();
	ret = avcodec_receive_frame(codecCtx, d_frame);
	if (ret == 0) {
		return d_frame;
	}
	else {
		printf("Error submitting the packet to the decoder 22:%d \n", ret);
		return NULL;
	}
		
}

Decoder::~Decoder()
{
	if (codecCtx != NULL) {
		avcodec_free_context(&codecCtx);
	}
}