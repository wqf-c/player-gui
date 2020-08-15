#include"Controller.h"

Controller::Controller()
{

}

void Controller::start(string fileName) {
	for (auto iter = videoVec.begin(); iter != videoVec.end(); ++iter) {
		av_packet_free(&(*iter));
	}
	for (auto iter = audioVec.begin(); iter != audioVec.end(); ++iter) {
		av_packet_free(&(*iter));
	}
	videoVec.clear();
	audioVec.clear();
	if (pFormatCtx != NULL) {
		avformat_free_context(pFormatCtx);
	}
	if (audio_convert_ctx != NULL) {
		swr_free(&audio_convert_ctx);
		audio_convert_ctx = NULL;
	}
	if (img_convert_ctx != NULL) {
		sws_freeContext(img_convert_ctx);
		img_convert_ctx = NULL;

	}

	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, fileName.c_str(), NULL, NULL) != 0) {
		printf("could not find video\n");
		return ;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("could not find stream info\n");
		return;
	}

	av_dump_format(pFormatCtx, 0, fileName.c_str(), 0);
	for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = pFormatCtx->streams[i];
			audioIndex = i;
		}
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = pFormatCtx->streams[i];
			videoIndex = i;
		}
	}
	if (videoIndex == -1 || audioIndex == -1) {
		printf("not find video or audio index\n");
		return;
	}
	
	videoDecoder.setCodec(videoStream->codecpar);
	audioDecoder.setCodec(audioStream->codecpar);
	audio_convert_ctx = swr_alloc();
	audio_convert_ctx = swr_alloc_set_opts(NULL,
		AV_CH_LAYOUT_STEREO,                                /*out*/
		//av_get_default_channel_layout(spec.channels),
		AV_SAMPLE_FMT_S16,                              /*out*/
		//out_sample_rate,                             /*out*/
		audioStream->codecpar->sample_rate,
		audioDecoder.getChannelLayout(),                                  /*in*/
		audioDecoder.getSampleFmt(),               /*in*/
		audioDecoder.getSampleRate(),               /*in*/
		0,
		NULL);

	swr_init(audio_convert_ctx);
	if (out_buffer != NULL) {
		av_free(out_buffer);
		out_buffer = NULL;
	}
	out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoDecoder.getWidth(), videoDecoder.getHeight(), 32) * sizeof(uint8_t));
	img_convert_ctx = sws_getContext(videoDecoder.getWidth(), videoDecoder.getHeight(),
		videoDecoder.getPixFmt() , videoDecoder.getWidth(), videoDecoder.getHeight(), AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

}

void Controller::grabPkt() {
	while (true)
	{
		unique_lock<mutex> lock{ mux };
		if (videoVec.size() >= 3 && audioVec.size() >= 3) {
			cond.wait(lock, [&]() {return videoVec.size() < 3 || audioVec.size() <= 3; });
		}
		AVPacket *inputPkt = av_packet_alloc();
		av_init_packet(inputPkt);
		int ret = av_read_frame(pFormatCtx, inputPkt);
		if (ret < 0) {
			printf("read frame fail\n");
			lock.unlock();
			return;
		}
		if (inputPkt->stream_index == audioIndex) {
			audioVec.push_back(inputPkt);
		}
		if (inputPkt->stream_index == videoIndex) {
			videoVec.push_back(inputPkt);
		}
		lock.unlock();
	}
}

void Controller::setStopFlag(bool flag) {
	stopFlag = flag;
}

int Controller::getFrameRate(AVStream * inputVideoStream) {
	int frameRate = 40;
	if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0)
	{
		frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
	}
	else if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0)
	{

		frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
	}
	return frameRate;
}

Controller::~Controller()
{
	if (pFormatCtx != NULL) {
		avformat_free_context(pFormatCtx);
	}
}