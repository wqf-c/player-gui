#include "PlayerLivePage.h"

PlayerLivePage::PlayerLivePage( wxWindow* parent )
:
LivePage( parent )
{
	timer = new RenderTimer(videoPanel);
	controller.setFrameVec(&frameVec);
	controller.setLock(&mtx);
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
}

void PlayerLivePage::paintEvent(wxPaintEvent &evt) {
	wxPaintDC dc(videoPanel);
	renderPic(&dc);
}

void PlayerLivePage::renderPic(wxDC *dc) {
	std::lock_guard<std::mutex> locker(mtx);
	if (!frameVec.empty()) {
		auto frameRGB = frameVec.back();

		unsigned char *data = frameRGB->data[0];
		wxSize size = wxSize(frameRGB->width, frameRGB->height);
		//wxSize size = wxSize(640, 480);

		wxImage *image = new wxImage(size, data);
		if (frameRGB->width > 640) {//固定画布的显示范围，否则超出了屏幕了
//            image->Scale(640,640*frameRGB->height/frameRGB->width);
		}
		wxBitmap *img = new wxBitmap(*image);

		dc->DrawBitmap(*img, wxPoint(40, 40));

		//wxMemoryDC memDC;
		//wxRect rect = this->GetRect();
		//wxBitmap bitmap(rect.GetSize());
		//memDC.SelectObject(bitmap);

		/* 开始绘图 */
//        memDC.SetBackground(*wxRED_BRUSH);
//        memDC.Clear();

		//memDC.DrawBitmap(*img, 40, 40);
		//dc->Blit(wxPoint(40, 40), vGrabber->getImageSize(), &memDC, wxPoint(40, 40));

		//delete img;
	}
	else {
		printf("frameVec is null");
	}
}

void PlayerLivePage::videoSelect( wxListEvent& event )
{
// TODO: Implement videoSelect
	wxString fileName = fileCache.at(event.m_itemIndex);
	controller.setStopFlag(true);
	Sleep(100);
	timer->start();
	controller.start(fileName.ToStdString());
	//cout << fileName << endl;
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
