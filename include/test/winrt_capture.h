#pragma once

#define WIN32_LEAN_AND_MEAN
#include<iostream>
#include <windows.h>
#include<winrt_capture.h>
#include<WinRTBase.h>
#include<dxgi.h>
#include<Unknwnbase.h>
#include <winrt/windows.graphics.directx.direct3d11.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	//bool winrt_capture_supported();
	bool winrt_capture_cursor_toggle_supported();
	struct winrt_capture *winrt_capture_init(bool cursor, HWND window,
		bool client_area);
	void winrt_capture_free(struct winrt_capture *capture);

	bool winrt_capture_supported(const struct winrt_capture *capture);
	void winrt_capture_show_cursor(struct winrt_capture *capture, bool visible);
	uint32_t winrt_capture_width(const struct winrt_capture *capture);
	uint32_t winrt_capture_height(const struct winrt_capture *capture);

	void winrt_capture_thread_start();
	void winrt_capture_thread_stop();

#ifdef __cplusplus
}
#endif
