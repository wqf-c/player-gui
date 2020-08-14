#pragma once

#ifndef _WX_PAGE_H_

#define _WX_PAGE_H_
#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <wx/wx.h>


class PlayerApp : public wxApp {
public:
	virtual bool OnInit();
};

#endif //_WX_PAGE_H_