///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "LivePage.h"

///////////////////////////////////////////////////////////////////////////

LivePage::LivePage(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer(wxVERTICAL);

	videoPanel = new wxPanel(m_panel1, wxID_ANY, wxDefaultPosition, wxSize(640, 480), wxTAB_TRAVERSAL);
	videoPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	bSizer7->Add(videoPanel, 2, wxEXPAND | wxALL, 5);

	m_panel5 = new wxPanel(m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_panel5->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
	m_panel5->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxVERTICAL);

	processSlide = new wxSlider(m_panel5, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxSize(630, 25), wxSL_HORIZONTAL | wxSL_LABELS);
	processSlide->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTIONTEXT));
	processSlide->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	bSizer8->Add(processSlide, 0, wxALL, 5);

	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer(wxHORIZONTAL);

	m_bpButton3 = new wxBitmapButton(m_panel5, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	bSizer71->Add(m_bpButton3, 0, wxALIGN_BOTTOM | wxALL, 5);

	m_bpButton4 = new wxBitmapButton(m_panel5, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	bSizer71->Add(m_bpButton4, 0, wxALIGN_BOTTOM | wxALL, 5);

	m_bpButton5 = new wxBitmapButton(m_panel5, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	bSizer71->Add(m_bpButton5, 0, wxALIGN_BOTTOM | wxALL, 5);


	bSizer8->Add(bSizer71, 1, wxALIGN_CENTER, 5);


	m_panel5->SetSizer(bSizer8);
	m_panel5->Layout();
	bSizer8->Fit(m_panel5);
	bSizer7->Add(m_panel5, 1, wxEXPAND | wxALL, 5);


	bSizer2->Add(bSizer7, 1, wxEXPAND, 5);

	m_panel3 = new wxPanel(m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVEBORDER));

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxVERTICAL);

	videoList = new wxListCtrl(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES | wxLC_REPORT | wxLC_VRULES);
	videoList->Enable(true);

	bSizer3->Add(videoList, 5, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	chooseFolderBtn = new wxButton(m_panel3, wxID_ANY, wxT("选择文件夹"), wxDefaultPosition, wxDefaultSize, 0);
	chooseFolderBtn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVEBORDER));

	bSizer5->Add(chooseFolderBtn, 1, wxALIGN_CENTER | wxALL, 0);


	bSizer3->Add(bSizer5, 1, wxEXPAND, 5);


	m_panel3->SetSizer(bSizer3);
	m_panel3->Layout();
	bSizer3->Fit(m_panel3);
	bSizer2->Add(m_panel3, 1, wxEXPAND | wxALL, 5);


	m_panel1->SetSizer(bSizer2);
	m_panel1->Layout();
	bSizer2->Fit(m_panel1);
	bSizer1->Add(m_panel1, 1, wxEXPAND | wxALL, 5);


	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events
	videoPanel->Connect(wxEVT_PAINT, wxPaintEventHandler(LivePage::paintEvent), NULL, this);
	processSlide->Connect(wxEVT_SLIDER, wxCommandEventHandler(LivePage::processSlideOnSlider), NULL, this);
	m_bpButton3->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::lastVideo), NULL, this);
	m_bpButton4->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::playBtnClick), NULL, this);
	m_bpButton5->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::nextVideo), NULL, this);
	videoList->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LivePage::videoSelect), NULL, this);
	chooseFolderBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::chooseFolder), NULL, this);
}

LivePage::~LivePage()
{
	// Disconnect Events
	videoPanel->Disconnect(wxEVT_PAINT, wxPaintEventHandler(LivePage::paintEvent), NULL, this);
	processSlide->Disconnect(wxEVT_SLIDER, wxCommandEventHandler(LivePage::processSlideOnSlider), NULL, this);
	m_bpButton3->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::lastVideo), NULL, this);
	m_bpButton4->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::playBtnClick), NULL, this);
	m_bpButton5->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::nextVideo), NULL, this);
	videoList->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LivePage::videoSelect), NULL, this);
	chooseFolderBtn->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LivePage::chooseFolder), NULL, this);

}


