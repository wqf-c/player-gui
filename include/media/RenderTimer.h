#pragma once

#include <wx/sizer.h>
#include <wx/wx.h>
#include <wx/timer.h>
#include <string>
#include <vector>

class RenderTimer : public wxTimer {

public:
	bool isTimeRender = false;
	wxPanel *panel;
	explicit RenderTimer(wxPanel* page);
	void Notify() override;
	void start();
};