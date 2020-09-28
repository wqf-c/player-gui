#include <string>
#include <iostream>
#include <wx/wx.h>
#include <vector>
#include <mutex>
#include <list>
#include <Windows.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include <libavutil/time.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}

using std::string;
using std::cout;
using std::cin;
using std::endl;

void start(std::list<AVFrame*>* list_, std::mutex* mtx_);

string AnsiToUTF81(const char *_ansi, int _ansi_len);

std::string TCHAR2STRING(TCHAR *STR);

class RenderTimerTest : public wxTimer {
public:
	wxPanel *panel;
	explicit RenderTimerTest(wxPanel* page) {
		this->panel = page;
	}
	void Notify() override {
		panel->Refresh(false);
	}
	void start() {
		wxTimer::Start(40);
	}
};

class Page : public wxFrame {
public:

	Page(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(500, 300), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL) : wxFrame(parent, id, title, pos, size, style) {
		this->SetSizeHints(wxDefaultSize, wxDefaultSize);
		wxBoxSizer* bSizer6;
		bSizer6 = new wxBoxSizer(wxHORIZONTAL);

		camera_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		wxBoxSizer* bSizer12;
		bSizer12 = new wxBoxSizer(wxHORIZONTAL);


		camera_panel->SetSizer(bSizer12);
		camera_panel->Layout();
		bSizer12->Fit(camera_panel);
		bSizer6->Add(camera_panel, 1, wxALL | wxEXPAND, 5);

		camera_panel->Connect(wxEVT_PAINT, wxPaintEventHandler(Page::paintEvent), NULL, this);
	}

protected:

	virtual void paintEvent(wxPaintEvent& event) { event.Skip(); }
	wxPanel* camera_panel;


};

class TestPage : public Page {
public:
	TestPage(wxWindow *parent) : Page(parent) {
		timer = new RenderTimerTest(camera_panel);
		timer->start();
	}

	void paintEvent(wxPaintEvent &evt) {
		wxPaintDC dc(camera_panel);

		if (!frameList->empty()) {
			if (!mtx) {
				cout << "mxt is nullptr";
				return;
			}
			std::unique_lock<std::mutex> locker(*mtx);
			if (!frameList->empty()) {
				auto frameRGB = frameList->front();

				unsigned char *data = frameRGB->data[0];
				wxSize size = wxSize(frameRGB->width, frameRGB->height);

				wxImage* image = new wxImage(size, data);

				wxBitmap img(*image);

				dc.DrawBitmap(img, wxPoint(40, 40));
			}
		}

	}

	void setList(std::list<AVFrame*>* list_) {
		frameList = list_;
	}

	void setMutex(std::mutex* mtx_) {
		mtx = mtx_;
	}

private:
	RenderTimerTest* timer;

	std::list<AVFrame*>* frameList;

	std::mutex* mtx = nullptr;

};


class APP : public wxApp {
public:
	virtual bool OnInit() {
		auto mtx = new std::mutex();
		auto list = new std::list<AVFrame*>();

		std::thread grab{ start, list, mtx };
		grab.detach();

		auto* frame = new TestPage(nullptr);
		frame->setList(list);
		frame->setMutex(mtx);
		frame->Show(true);
		return true;
	}
};

void start(std::list<AVFrame*>* list_, std::mutex* mtx_) {
	avdevice_register_all();
	av_register_all();

	AVCodec					*decodeCamera;
	AVCodecContext			*decodeCameraContext;
	AVFormatContext			*c_inputContext;
	AVStream				*inputCameraStream;
	AVCodecID				m_camera_codec_id;
	std::string				desktopDeviceName;
	int						desktopIndex;
	AVCodec					*decodeDesktop;
	AVCodecContext			*decodeDesktopContext;
	AVFormatContext			*d_inputContext;
	AVStream				*inputDesktopStream;
	AVCodecID				m_desktop_codec_id;
	int						frameRate;
	bool					stopFlag;


	//string deviceName = "desktop";
	string deviceName = "title=gzhb.pptx - PowerPoint";
	char utf8[1024];

	deviceName = AnsiToUTF81(deviceName.c_str(), deviceName.length());

	AVInputFormat *ifmt = av_find_input_format("gdigrab");
	AVDictionary *format_opts = nullptr;
	AVFormatContext	*inputCtx = avformat_alloc_context();

	bool flag = true;

	int videoIndex = -1;
	AVStream *inputStream = nullptr;


	AVCodec *decode = nullptr;
	AVCodecContext *decodeCtx = nullptr;
	AVFrame *frameRGB = nullptr;
	uint8_t* out_buffer = nullptr;
	SwsContext* video_convert_ctx = nullptr;


	//打开设备
	int ret = avformat_open_input(&inputCtx, deviceName.c_str(), ifmt, &format_opts);
	if (ret < 0) {
		cout << "open device error" << endl;
		flag = false;
		goto _END;
	}

	ret = avformat_find_stream_info(inputCtx, nullptr);
	if (ret < 0)
	{
		//cout << "查找视频流信息失败:" << ret << endl;
		flag = false;
		goto _END;
	}

	cout << "open video input stream success" << endl;


	for (int i = 0; i < inputCtx->nb_streams; ++i) {
		if (inputCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
			inputStream = inputCtx->streams[i];
			break;
		}
	}

	if (videoIndex < 0) {
	//	cout << "摄像头流索引查找失败" << endl;
		flag = false;
		goto _END;
	}

	av_dump_format(inputCtx, 0, deviceName.c_str(), 0);


	//解码器
	decode = avcodec_find_decoder(inputStream->codecpar->codec_id);
	if (!decode) {
		cout << "decodec find error" << endl;
		flag = false;
		goto _END;
	}

	decodeCtx = avcodec_alloc_context3(NULL);
	if (!decodeCtx) {
		//cout << "输入流解码器上下文分配内存失败" << endl;
		flag = false;
		goto _END;
	}

	ret = avcodec_parameters_to_context(decodeCtx, inputStream->codecpar);
	if (ret < 0) {
		cout << "拷贝输入流解码器上下文参数失败:" << ret << endl;
		flag = false;
		goto _END;
	}

	ret = avcodec_open2(decodeCtx, decode, NULL);
	if (ret < 0) {
		cout << "打开输入流解码器失败:" << ret << endl;
		flag = false;
		goto _END;
	}

	out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, 640, 480, 32) * sizeof(uint8_t));

	video_convert_ctx = sws_getContext(decodeCtx->width, decodeCtx->height,
		decodeCtx->pix_fmt, 640, 480, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);


	//抓流

	while (1) {
		AVPacket *inputPkt = av_packet_alloc();
		AVFrame *inputFrame = av_frame_alloc();
		av_init_packet(inputPkt);

		ret = av_read_frame(inputCtx, inputPkt);

		if (ret < 0) {
			cout << "读取摄像头图像失败:" << ret << endl;
			goto _END;
		}
		ret = avcodec_send_packet(decodeCtx, inputPkt);
		if (ret != 0 && ret != AVERROR(EAGAIN)) {
			cout << "avcodec_send_packet fail:" << ret << endl;
			goto _END;
		}

		ret = avcodec_receive_frame(decodeCtx, inputFrame);
		if (ret != 0 && ret != AVERROR(EAGAIN)) {
			cout << "avcodec_receive_frame fail:" << ret << endl;
			goto _END;
		}

		frameRGB = av_frame_alloc();
		av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer,
			AV_PIX_FMT_RGB24, 640, 480, 32);
		frameRGB->format = AV_PIX_FMT_RGB24;
		frameRGB->width = 640;
		frameRGB->height = 480;

		sws_scale(video_convert_ctx, (const uint8_t* const*)inputFrame->data,
			inputFrame->linesize, 0, decodeCtx->height, frameRGB->data, frameRGB->linesize);

		if (frameRGB) {
			//draw
			//cout << "draw" << endl;
			std::unique_lock<std::mutex> locker(*mtx_);
			list_->push_back(frameRGB);
		}

		if (inputFrame) {
			av_frame_free(&inputFrame);
		}
		if (inputPkt) {
			av_packet_free(&inputPkt);
		}
	}

_END:
	if (!flag) {
		cout << "open device error" << endl;
	}
	system("pause");
}

#define getWindow 0

int main() {
	if (getWindow) {
		HWND hd = GetDesktopWindow();
		hd = GetWindow(hd, GW_CHILD);
		TCHAR s[200] = { 0 };
		//cout << sizeof(s) << endl;
		while (hd) {
			if (IsWindowVisible(hd)) {
				memset(s, 0, 400);
				GetWindowText(hd, s, 200);
				//string str(s);
				//cout << str << endl;
				auto str = TCHAR2STRING(s);
				//printf("%ws\n", s);
				if (str.length() > 0) {
					cout << str << endl;
				}

				//cout << str << "=====" << str.length() << endl;
			}

			hd = GetNextWindow(hd, GW_HWNDNEXT);
		}
		system("pause");
	}
	else {
		auto app = new APP();
		wxApp::SetInstance(app);
		return wxEntry();
	}
}

string AnsiToUTF81(const char *_ansi, int _ansi_len)
{
#ifdef _WIN32

	std::string str_utf8 = "";
	wchar_t* pUnicode = NULL;
	BYTE * pUtfData = NULL;
	do
	{

		int unicodeNeed = MultiByteToWideChar(CP_ACP, 0, _ansi, _ansi_len, NULL, 0);

		pUnicode = new wchar_t[unicodeNeed + 1];
		memset(pUnicode, 0, (unicodeNeed + 1) * sizeof(wchar_t));

		int unicodeDone = MultiByteToWideChar(CP_ACP, 0, _ansi, _ansi_len, (LPWSTR)pUnicode, unicodeNeed);


		if (unicodeDone != unicodeNeed)
		{
			break;
		}

		int utfNeed = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, unicodeDone, (char *)pUtfData, 0, NULL, NULL);


		pUtfData = new BYTE[utfNeed + 1];
		memset(pUtfData, 0, utfNeed + 1);

		int utfDone = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, unicodeDone, (char *)pUtfData, utfNeed, NULL, NULL);


		if (utfNeed != utfDone)
		{
			break;
		}
		str_utf8.assign((char *)pUtfData);
	} while (false);

	if (pUnicode)
	{
		delete[] pUnicode;
	}
	if (pUtfData)
	{
		delete[] pUtfData;
	}

	return str_utf8;

#else
	//mac下中文没有编码问题
	return _ansi;
#endif
}


std::string TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	delete chRtn;
	return str;
}