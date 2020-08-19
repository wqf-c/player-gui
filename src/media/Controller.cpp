#include"Controller.h"

Controller::Controller()
{
	avdevice_register_all();
	 
}

void Controller::start(string fileName) {
	out_chn_layout = AV_CH_LAYOUT_STEREO;  //通道布局 输出双声道
	out_sample_fmt = AV_SAMPLE_FMT_S16; //声音格式
	out_sample_rate = 44100;   //采样率
	width = 640;
	height = 480;
	unique_lock<mutex> locker{ mux };
	stopFlag = false;
	for (auto iter = videoQue.begin(); iter != videoQue.end(); ++iter) {
		av_packet_free(&(*iter));
	}
	for (auto iter = audioQue.begin(); iter != audioQue.end(); ++iter) {
		av_packet_free(&(*iter));
	}
	videoQue.clear();
	audioQue.clear();
	locker.unlock();
	videoTime = 0;
	audioTime = 0;
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
	out_sample_rate = audioStream->codecpar->sample_rate;
	
	videoDecoder.setCodec(videoStream->codecpar);
	audioDecoder.setCodec(audioStream->codecpar);
	audio_convert_ctx = swr_alloc();
	audio_convert_ctx = swr_alloc_set_opts(audio_convert_ctx,
		out_chn_layout,
		out_sample_fmt,
		out_sample_rate,
		audioDecoder.getChannelLayout(),                                
		audioDecoder.getSampleFmt(),              
		audioDecoder.getSampleRate(),             
		0,
		NULL);

	int ret = swr_init(audio_convert_ctx);
	if (ret != 0) {
		printf("swr_init failed\n");
	}
	else {
		printf("swr_init success\n");
	}

	if (out_buffer != NULL) {
		av_free(out_buffer);
		out_buffer = NULL;
	}
	out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 32) * sizeof(uint8_t));
	img_convert_ctx = sws_getContext(videoDecoder.getWidth(), videoDecoder.getHeight(),
		videoDecoder.getPixFmt() , width, height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	if (audioPlayer != NULL) {
		delete audioPlayer;
	}
	audioPlayer = new AudioPlayer("", out_sample_rate);
	int duration = pFormatCtx->duration / AV_TIME_BASE;//求出总时长
	callBack(VIDEODURATION, duration*1000);
	thread grab{ &Controller::grabPkt, this };
	thread video{ &Controller::videoThreadFunc, this };
	thread audio{ &Controller::audioThreadFunc, this };
	audio.detach();
	grab.detach();
	video.detach();
	
}

void Controller::grabPkt() {
	while (!stopFlag)
	{
		unique_lock<mutex> lock{ mux };
		if (videoQue.size() >= 3 && audioQue.size() >= 3) {
			cond.wait(lock, [&]() {return videoQue.size() < 3 || audioQue.size() < 3; });
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
			audioQue.push_back(inputPkt);
		}
		if (inputPkt->stream_index == videoIndex) {
			videoQue.push_back(inputPkt);
		}
		lock.unlock();
	}
}

void Controller::videoThreadFunc() {
	vector<AVFrame *> vec{};
	int sleep = 1000 / getVideoFrameRate(videoStream);
	int time = 0;
	while (!stopFlag)
	{
		int delay = sleep;
		if (faster == -1) {
			delay = sleep / 2;
		}
		if (faster == 1) {
			delay = sleep * 2;
		}
		Sleep(delay);
		unique_lock<mutex> lock{ mux };
		if (videoQue.size() >= 3) {
			AVPacket *pkt = videoQue.front();
			videoQue.pop_front();
			lock.unlock();
			cond.notify_one();
			AVFrame * inputFrame = videoDecoder.decode( pkt);
			av_packet_free(&pkt);
			if (inputFrame != NULL && locker != NULL && frameRGBvec != NULL) {
				videoTime = inputFrame->pts * av_q2d(videoStream->time_base) * 1000;
				if (time == 0) time = videoTime;
				if (videoTime - time > 500) {
					time = videoTime;
					callBack(PLAYPROGRESS, time);
				}
				if (videoTime < audioTime && audioTime - videoTime > 30) {
					faster = -1;
				}
				else if(audioTime < videoTime && videoTime - audioTime > 30){
					faster = 1;
				}
				else {
					faster = 0;
				}
				AVFrame *frameRGB = av_frame_alloc();
				av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer,
					AV_PIX_FMT_RGB24, width, height, 32);
				frameRGB->format = AV_PIX_FMT_RGB24;
				frameRGB->width = width;
				frameRGB->height = height;
				sws_scale(img_convert_ctx, (const uint8_t* const*)inputFrame->data,
					inputFrame->linesize, 0, videoDecoder.getHeight(), frameRGB->data, frameRGB->linesize);
				av_frame_free(&inputFrame);
				if (frameRGB) {
					locker->lock();
					if (frameRGBvec->size() == 0) {
						frameRGBvec->push_back(frameRGB);
					}
					else {
						AVFrame *f = frameRGBvec->back();
						frameRGBvec->pop_back();
						frameRGBvec->push_back(frameRGB);
						av_frame_free(&f);
					}
					locker->unlock();
				}
			}
		}
		else {
			lock.unlock();
		}
		
	}
}

void Controller::audioThreadFunc() {
	vector<AVFrame *> vec{};
	int sleep = 40;
	out_sample_rate = audioStream->codecpar->sample_rate;
	int i = 0;
	uint8_t *audio_buf = (uint8_t *)malloc(MAX_AUDIO_FRAME_SIZE * sizeof(uint8_t));
	while (!stopFlag)
	{
		
		Sleep(sleep);
		unique_lock<mutex> lock{ mux };
		if (audioQue.size() >= 3) {
			AVPacket *pkt = audioQue.front();
			audioQue.pop_front();
			lock.unlock();
			cond.notify_one();
			
			AVFrame *inputFrame = audioDecoder.decode( pkt);
			
			if (inputFrame != NULL) {
				memset(audio_buf, 0, MAX_AUDIO_FRAME_SIZE);
				sleep = (inputFrame->nb_samples * 1000) / out_sample_rate;
				audioTime = inputFrame->pts * av_q2d(audioStream->time_base) * 1000;
				int outSamples = swr_convert(audio_convert_ctx, (uint8_t**)&audio_buf, MAX_AUDIO_FRAME_SIZE,
					(const uint8_t**)&inputFrame->data[0], inputFrame->nb_samples);
				int outDataSize = 
					av_samples_get_buffer_size(NULL, 2, outSamples, out_sample_fmt, 1);
				audioPlayer->workData(audio_buf, outDataSize);
				av_frame_free(&inputFrame);
			}
				
			
			av_packet_free(&pkt);
			
		}
		else {
			lock.unlock();
		}
	}
	free(audio_buf);
}

void Controller::setStopFlag(bool flag) {
	stopFlag = flag;
}

int Controller::getVideoFrameRate(AVStream * inputVideoStream) {
	int frameRate = 40;
	if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0)
	{
		frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
	}
	else if (inputVideoStream != nullptr && inputVideoStream->r_frame_rate.den > 0)
	{

		frameRate = inputVideoStream->r_frame_rate.num / inputVideoStream->r_frame_rate.den;
	}
	cout << "frame rate:" << frameRate << endl;
	return frameRate;
}

int Controller::getAudioFrameRate(AVStream *inputAudioStream, int nb_samples) {
	return nb_samples * 1000 / inputAudioStream->codecpar->sample_rate;
}

Controller::~Controller()
{
	if (pFormatCtx != NULL) {
		avformat_free_context(pFormatCtx);
	}
}