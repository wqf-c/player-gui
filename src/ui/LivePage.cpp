///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "LivePage.h"

///////////////////////////////////////////////////////////////////////////

LivePage::LivePage( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	videoPanel = new wxPanel( m_panel1, wxID_ANY, wxDefaultPosition, wxSize(640, 480), wxTAB_TRAVERSAL );
	videoPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );

	bSizer2->Add( videoPanel, 2, wxEXPAND | wxALL, 5 );

	m_panel3 = new wxPanel( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel3->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	videoList = new wxListCtrl( m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES | wxLC_REPORT | wxLC_VRULES);
	bSizer3->Add( videoList, 5, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	chooseFolderBtn = new wxButton( m_panel3, wxID_ANY, wxT("选择文件夹"), wxDefaultPosition, wxDefaultSize, 0 );
	chooseFolderBtn->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );

	bSizer5->Add( chooseFolderBtn, 1, wxALIGN_CENTER|wxALL, 0 );


	bSizer3->Add( bSizer5, 1, wxEXPAND, 5 );


	m_panel3->SetSizer( bSizer3 );
	m_panel3->Layout();
	bSizer3->Fit( m_panel3 );
	bSizer2->Add( m_panel3, 1, wxEXPAND | wxALL, 5 );


	m_panel1->SetSizer( bSizer2 );
	m_panel1->Layout();
	bSizer2->Fit( m_panel1 );
	bSizer1->Add( m_panel1, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	videoList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( LivePage::videoSelect ), NULL, this );
	chooseFolderBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( LivePage::chooseFolder ), NULL, this );
	videoPanel->Connect(wxEVT_PAINT, wxPaintEventHandler(LivePage::paintEvent), NULL, this);
}

LivePage::~LivePage()
{
	// Disconnect Events
	videoList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( LivePage::videoSelect ), NULL, this );
	chooseFolderBtn->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( LivePage::chooseFolder ), NULL, this );
	videoPanel->Disconnect(wxEVT_PAINT, wxPaintEventHandler(LivePage::paintEvent), NULL, this);
}
