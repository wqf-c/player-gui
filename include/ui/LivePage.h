///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/listctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include<wx/msgdlg.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class LivePage
///////////////////////////////////////////////////////////////////////////////
class LivePage : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel1;
		wxPanel* videoPanel;
		wxPanel* m_panel3;
		wxListCtrl* videoList;
		wxButton* chooseFolderBtn;

		// Virtual event handlers, overide them in your derived class
		virtual void videoSelect( wxListEvent& event ) { event.Skip(); }
		virtual void chooseFolder( wxCommandEvent& event ) { event.Skip(); }
		virtual void paintEvent(wxPaintEvent& event) { event.Skip(); }


	public:

		LivePage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("videoplayer-wqf"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 894,516 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~LivePage();

};

