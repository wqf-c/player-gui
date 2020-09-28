#include "PlayerLivePage.h"

PlayerLivePage::PlayerLivePage( wxWindow* parent )
:
LivePage( parent )
{
	timer = new RenderTimer(videoPanel);
	controller.setFrameVec(&frameVec);
	controller.setLock(&mtx);

	auto CallBack = [&](CallBackType type, int time) {
		Callback(type, time);
	};
	controller.setUiCallBack(CallBack);

	wxListItem col0;
	col0.SetId(0); // id必须设置，代表第0列
	col0.SetText(wxT("#"));
	//col0.SetWidth(50);
	col0.SetAlign(wxLIST_FORMAT_CENTER); // 此列的文本居中显示
	videoList->InsertColumn(0, col0); // 插入列

	// 添加第二列的表头
	wxListItem col1;
	col1.SetId(1);
	col1.SetWidth(190);
	col1.SetText(wxT("文件名"));
	videoList->InsertColumn(1, col1);
	Connect(wxEVT_CUSTOM_UI_COMMAND,
		wxCommandEventHandler(PlayerLivePage::Callback));
}


void PlayerLivePage::Callback(CallBackType type, int time)
{
	wxCommandEvent evtcustom(wxEVT_CUSTOM_UI_COMMAND);
	evtcustom.SetInt(type);
	evtcustom.SetTimestamp(time);
	wxPostEvent(this, evtcustom);
}

void PlayerLivePage::Callback(wxCommandEvent &evt) {
	int eve = evt.GetInt();
	int time = evt.GetTimestamp()/1000;
	switch (eve)
	{
	case VIDEODURATION:
		processSlide->SetMax(time);
		break;
	case PLAYPROGRESS:
		processSlide->SetValue(time);
		break;
	default:
		break;
	}
	
}


void PlayerLivePage::processSlideOnSlider( wxCommandEvent& event )
{
// TODO: Implement processSlideOnSlider
	int current = processSlide->GetValue();
	controller.seekVideo(current);
}

void PlayerLivePage::videoSelect( wxListEvent& event )
{
// TODO: Implement videoSelect
	unique_lock<mutex> lock{ mtx };
	for (auto iter = frameVec.begin(); iter != frameVec.end(); ++iter) {
		av_frame_free(&(*iter));
	}
	frameVec.clear();
	lock.unlock();
	wxString fileName = fileCache.at(event.m_itemIndex);
	controller.setStopFlag(true);
	Sleep(300);
	timer->start();
	controller.start(fileName.ToStdString());
}

void PlayerLivePage::chooseFolder( wxCommandEvent& event )
{
// TODO: Implement chooseFolder
	fileCache.clear();
	wxDirDialog dirDialog(this, wxT("Choose a folder"));

	if (dirDialog.ShowModal() != wxID_OK)
	{
		return;
	}
	size_t n = 0;
	wxArrayString files;
	wxString path = dirDialog.GetPath();
	wxDir dir(path);
	if (dir.IsOpened())
	{
		wxString filter = wxT("*.*");//文件过滤
		n = dir.GetAllFiles(path, &files, filter, wxDIR_DEFAULT);
	}
	int total = 0; // 获取当前元素数量
	for (size_t i = 0; i < n; i++)
	{
		if (files[i].EndsWith("avi") || files[i].EndsWith("mp4") || files[i].EndsWith("flv") || files[i].EndsWith("mkv")) {
			long indexItem = videoList->InsertItem(total, wxString::FromDouble(total + 1));
			videoList->SetItem(indexItem, 1, files[i]);
			fileCache[indexItem] = files[i];
		}
		total++;
	}
}


void PlayerLivePage::paintEvent(wxPaintEvent &evt) 
{
	wxPaintDC  dc(videoPanel);
	renderPic(&dc);
}

void PlayerLivePage::renderPic(wxDC *dc) 
{
	std::unique_lock<std::mutex> locker(mtx);
	if (!frameVec.empty()) {
		auto frameRGB = frameVec.back();
		locker.unlock();
		unsigned char *data = frameRGB->data[0];
		wxSize size = wxSize(frameRGB->width, frameRGB->height);

		wxImage *image = new wxImage(size, data);
		wxBitmap *img = new wxBitmap(*image);

		dc->DrawBitmap(*img, 0, 0);
		delete img;
	}
	else {
		locker.unlock();
		printf("frameVec is null");
	}
}


