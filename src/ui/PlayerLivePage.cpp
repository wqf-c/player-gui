#include "PlayerLivePage.h"

PlayerLivePage::PlayerLivePage( wxWindow* parent )
:
LivePage( parent )
{
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

void PlayerLivePage::videoSelect( wxListEvent& event )
{
// TODO: Implement videoSelect
}

void PlayerLivePage::chooseFolder( wxCommandEvent& event )
{
// TODO: Implement chooseFolder
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
		if (files[i].EndsWith("avi") || files[i].EndsWith("mp4") || files[i].EndsWith("flv")) {
			long indexItem = videoList->InsertItem(total, wxString::FromDouble(total + 1));
			videoList->SetItem(indexItem, 1, files[i]);
		}
		total++;
	}
}
