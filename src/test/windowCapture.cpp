#include<iostream>
#include <windows.h>
#include<string.h>
#include"Decoder.h"

using namespace std;

struct Gdigrab {
public:
	const AVClass *avClass;   /**< Class for private options */

	int        frame_size;  /**< Size in bytes of the frame pixel data */
	int        header_size; /**< Size in bytes of the DIB header */
	AVRational time_base;   /**< Time base */
	int64_t    time_frame;  /**< Current time */

	int        draw_mouse;  /**< Draw mouse cursor (private option) */
	int        show_region; /**< Draw border (private option) */
	AVRational framerate;   /**< Capture framerate (private option) */
	int        width;       /**< Width of the grab frame (private option) */
	int        height;      /**< Height of the grab frame (private option) */
	int        offset_x;    /**< Capture x offset (private option) */
	int        offset_y;    /**< Capture y offset (private option) */

	HWND       hwnd;        /**< Handle of the window for the grab */
	HDC        source_hdc;  /**< Source device context */
	HDC        dest_hdc;    /**< Destination, source-compatible DC */
	BITMAPINFO bmi;         /**< Information describing DIB format */
	HBITMAP    hbmp;        /**< Information on the bitmap captured */
	void      *buffer;      /**< The buffer containing the bitmap image data */
	RECT       clip_rect;   /**< The subarea of the screen or window to clip */

	HWND       region_hwnd; /**< Handle of the region border window */

	int cursor_error_printed;
};

#define WIN32_API_ERROR(str)                                            \
	cout << "error:" << GetLastError() << endl;

#define REGION_WND_BORDER 3

Gdigrab* gdigrab = new Gdigrab();

struct SwsContext		*img_convert_ctx;

AVCodecParameters *codecpar = new  AVCodecParameters();

HWND hWnd2 = NULL;


/**
 * Callback to handle Windows messages for the region outline window.
 *
 * In particular, this handles painting the frame rectangle.
 *
 * @param hwnd The region outline window handle.
 * @param msg The Windows message.
 * @param wparam First Windows message parameter.
 * @param lparam Second Windows message parameter.
 * @return 0 success, !0 failure
 */
static LRESULT CALLBACK
gdigrab_region_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	switch (msg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		GetClientRect(hwnd, &rect);
		FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		rect.left++; rect.top++; rect.right--; rect.bottom--;
		FrameRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

		rect.left++; rect.top++; rect.right--; rect.bottom--;
		FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		EndPaint(hwnd, &ps);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

/**
 * Initialize the region outline window.
 *
 * @param s1 The format context.
 * @param gdigrab gdigrab context.
 * @return 0 success, !0 failure
 */
static int
gdigrab_region_wnd_init(struct Gdigrab *gdigrab)
{
	HWND hwnd;
	RECT rect = gdigrab->clip_rect;
	HRGN region = NULL;
	HRGN region_interior = NULL;

	DWORD style = WS_POPUP | WS_VISIBLE;
	DWORD ex = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT;

	rect.left -= REGION_WND_BORDER; rect.top -= REGION_WND_BORDER;
	rect.right += REGION_WND_BORDER; rect.bottom += REGION_WND_BORDER;

	AdjustWindowRectEx(&rect, style, FALSE, ex);

	// Create a window with no owner; use WC_DIALOG instead of writing a custom
	// window class
	hwnd = CreateWindowEx(ex, WC_DIALOG, NULL, style, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, NULL, NULL);
	if (!hwnd) {
		WIN32_API_ERROR("Could not create region display window");
		goto error;
	}

	// Set the window shape to only include the border area
	GetClientRect(hwnd, &rect);
	region = CreateRectRgn(0, 0,
		rect.right - rect.left, rect.bottom - rect.top);
	region_interior = CreateRectRgn(REGION_WND_BORDER, REGION_WND_BORDER,
		rect.right - rect.left - REGION_WND_BORDER,
		rect.bottom - rect.top - REGION_WND_BORDER);
	CombineRgn(region, region, region_interior, RGN_DIFF);
	if (!SetWindowRgn(hwnd, region, FALSE)) {
		WIN32_API_ERROR("Could not set window region");
		goto error;
	}
	// The "region" memory is now owned by the window
	region = NULL;
	DeleteObject(region_interior);

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)gdigrab_region_wnd_proc);

	ShowWindow(hwnd, SW_SHOW);

	gdigrab->region_hwnd = hwnd;

	return 0;

error:
	if (region)
		DeleteObject(region);
	if (region_interior)
		DeleteObject(region_interior);
	if (hwnd)
		DestroyWindow(hwnd);
	return 1;
}

/**
 * Initializes the gdi grab device demuxer (public device demuxer API).
 *
 * @param s1 Context from avformat core
 * @return AVERROR_IO error, 0 success
 */
static int
gdigrab_read_header(HWND hwnd)
{	
	HDC source_hdc = NULL;
	HDC dest_hdc = NULL;
	BITMAPINFO bmi;
	HBITMAP hbmp = NULL;
	void *buffer = NULL;

	const char *name = NULL;
	

	int bpp;
	int horzres;
	int vertres;
	int desktophorzres;
	int desktopvertres;
	RECT virtual_rect;
	RECT clip_rect;
	BITMAP bmp;
	int ret;

	if (hwnd) {
		if (gdigrab->show_region) {
			cout << "Can't show region when grabbing a window." << endl;
			gdigrab->show_region = 0;
		}
	}
	else {
		hwnd = NULL;
	}

	/* This will get the device context for the selected window, or if
	 * none, the primary screen */
	source_hdc = GetDC(hwnd);
	if (!source_hdc) {
		WIN32_API_ERROR("Couldn't get window device context");
		
		goto error;
	}
	bpp = GetDeviceCaps(source_hdc, BITSPIXEL);

	horzres = GetDeviceCaps(source_hdc, HORZRES);
	vertres = GetDeviceCaps(source_hdc, VERTRES);
	desktophorzres = GetDeviceCaps(source_hdc, DESKTOPHORZRES);
	desktopvertres = GetDeviceCaps(source_hdc, DESKTOPVERTRES);

	if (hwnd) {
		GetClientRect(hwnd, &virtual_rect);
		/* window -- get the right height and width for scaling DPI */
		virtual_rect.left = virtual_rect.left   * desktophorzres / horzres;
		virtual_rect.right = virtual_rect.right  * desktophorzres / horzres;
		virtual_rect.top = virtual_rect.top    * desktopvertres / vertres;
		virtual_rect.bottom = virtual_rect.bottom * desktopvertres / vertres;
	}
	else {
		/* desktop -- get the right height and width for scaling DPI */
		virtual_rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
		virtual_rect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
		virtual_rect.right = (virtual_rect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN)) * desktophorzres / horzres;
		virtual_rect.bottom = (virtual_rect.top + GetSystemMetrics(SM_CYVIRTUALSCREEN)) * desktopvertres / vertres;
	}

	/* If no width or height set, use full screen/window area */
	if (!gdigrab->width || !gdigrab->height) {
		clip_rect.left = virtual_rect.left;
		clip_rect.top = virtual_rect.top;
		clip_rect.right = virtual_rect.right;
		clip_rect.bottom = virtual_rect.bottom;
	}
	else {
		clip_rect.left = gdigrab->offset_x;
		clip_rect.top = gdigrab->offset_y;
		clip_rect.right = gdigrab->width + gdigrab->offset_x;
		clip_rect.bottom = gdigrab->height + gdigrab->offset_y;
	}

	if (clip_rect.left < virtual_rect.left ||
		clip_rect.top < virtual_rect.top ||
		clip_rect.right > virtual_rect.right ||
		clip_rect.bottom > virtual_rect.bottom) {
		printf(
			"Capture area (%d,%d),(%d,%d) extends outside window area (%d,%d),(%d,%d)",
			clip_rect.left, clip_rect.top,
			clip_rect.right, clip_rect.bottom,
			virtual_rect.left, virtual_rect.top,
			virtual_rect.right, virtual_rect.bottom);
		
		goto error;
	}


	if (hwnd) {
		printf(
			"Found window %s, capturing %dx%dx%i at (%d,%d)\n",
			"windows",
			clip_rect.right - clip_rect.left,
			clip_rect.bottom - clip_rect.top,
			bpp, clip_rect.left, clip_rect.top);
	}
	else {
		printf(
			"Capturing whole desktop as %dx%dx%i at (%d,%d)\n",
			clip_rect.right - clip_rect.left,
			clip_rect.bottom - clip_rect.top,
			bpp, clip_rect.left, clip_rect.top);
	}

	if (clip_rect.right - clip_rect.left <= 0 ||
		clip_rect.bottom - clip_rect.top <= 0 || bpp % 8) {
		cout << "Invalid properties, aborting" << endl;
		
		goto error;
	}

	dest_hdc = CreateCompatibleDC(source_hdc);
	if (!dest_hdc) {
		WIN32_API_ERROR("Screen DC CreateCompatibleDC");
		
		goto error;
	}

	/* Create a DIB and select it into the dest_hdc */
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = clip_rect.right - clip_rect.left;
	bmi.bmiHeader.biHeight = -(clip_rect.bottom - clip_rect.top);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = bpp;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	hbmp = CreateDIBSection(dest_hdc, &bmi, DIB_RGB_COLORS,
		&buffer, NULL, 0);
	if (!hbmp) {
		WIN32_API_ERROR("Creating DIB Section");
		
		goto error;
	}

	if (!SelectObject(dest_hdc, hbmp)) {
		WIN32_API_ERROR("SelectObject");
		
		goto error;
	}

	/* Get info from the bitmap */
	GetObject(hbmp, sizeof(BITMAP), &bmp);


	gdigrab->frame_size = bmp.bmWidthBytes * bmp.bmHeight * bmp.bmPlanes;
	gdigrab->header_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
		(bpp <= 8 ? (1 << bpp) : 0) * sizeof(RGBQUAD) /* palette size */;
	gdigrab->time_base = av_inv_q(gdigrab->framerate);
	gdigrab->time_frame = av_gettime() / av_q2d(gdigrab->time_base);

	gdigrab->hwnd = hwnd;
	gdigrab->source_hdc = source_hdc;
	gdigrab->dest_hdc = dest_hdc;
	gdigrab->hbmp = hbmp;
	gdigrab->bmi = bmi;
	gdigrab->buffer = buffer;
	gdigrab->clip_rect = clip_rect;

	gdigrab->cursor_error_printed = 0;

	if (gdigrab->show_region) {
		if (gdigrab_region_wnd_init(gdigrab)) {
			
			goto error;
		}
	}

	codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	codecpar->codec_id = AV_CODEC_ID_BMP;
	codecpar->bit_rate = (gdigrab->header_size + gdigrab->frame_size) * 1 / av_q2d(gdigrab->time_base) * 8;

	return 0;

error:
	if (source_hdc)
		ReleaseDC(hwnd, source_hdc);
	if (dest_hdc)
		DeleteDC(dest_hdc);
	if (hbmp)
		DeleteObject(hbmp);
	if (source_hdc)
		DeleteDC(source_hdc);
	return ret;
}

/**
 * Paints a mouse pointer in a Win32 image.
 *
 * @param s1 Context of the log information
 * @param s  Current grad structure
 */
static void paint_mouse_pointer(struct Gdigrab *gdigrab)
{
	CURSORINFO ci = { 0 };

#define CURSOR_ERROR(str)                 \
    if (!gdigrab->cursor_error_printed) {       \
        WIN32_API_ERROR(str);             \
        gdigrab->cursor_error_printed = 1;      \
    }

	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci)) {
		HCURSOR icon = CopyCursor(ci.hCursor);
		ICONINFO info;
		POINT pos;
		RECT clip_rect = gdigrab->clip_rect;
		HWND hwnd = gdigrab->hwnd;
		int horzres = GetDeviceCaps(gdigrab->source_hdc, HORZRES);
		int vertres = GetDeviceCaps(gdigrab->source_hdc, VERTRES);
		int desktophorzres = GetDeviceCaps(gdigrab->source_hdc, DESKTOPHORZRES);
		int desktopvertres = GetDeviceCaps(gdigrab->source_hdc, DESKTOPVERTRES);
		info.hbmMask = NULL;
		info.hbmColor = NULL;

		if (ci.flags != CURSOR_SHOWING)
			return;

		if (!icon) {
			/* Use the standard arrow cursor as a fallback.
			 * You'll probably only hit this in Wine, which can't fetch
			 * the current system cursor. */
			icon = CopyCursor(LoadCursor(NULL, IDC_ARROW));
		}

		if (!GetIconInfo(icon, &info)) {
			CURSOR_ERROR("Could not get icon info");
			goto icon_error;
		}

		if (hwnd) {
			RECT rect;

			if (GetWindowRect(hwnd, &rect)) {
				pos.x = ci.ptScreenPos.x - clip_rect.left - info.xHotspot - rect.left;
				pos.y = ci.ptScreenPos.y - clip_rect.top - info.yHotspot - rect.top;

				//that would keep the correct location of mouse with hidpi screens
				pos.x = pos.x * desktophorzres / horzres;
				pos.y = pos.y * desktopvertres / vertres;
			}
			else {
				CURSOR_ERROR("Couldn't get window rectangle");
				goto icon_error;
			}
		}
		else {
			//that would keep the correct location of mouse with hidpi screens
			pos.x = ci.ptScreenPos.x * desktophorzres / horzres - clip_rect.left - info.xHotspot;
			pos.y = ci.ptScreenPos.y * desktopvertres / vertres - clip_rect.top - info.yHotspot;
		}

		printf("Cursor pos (%d,%d) -> (%d,%d)\n",
			ci.ptScreenPos.x, ci.ptScreenPos.y, pos.x, pos.y);

		if (pos.x >= 0 && pos.x <= clip_rect.right - clip_rect.left &&
			pos.y >= 0 && pos.y <= clip_rect.bottom - clip_rect.top) {
			if (!DrawIcon(gdigrab->dest_hdc, pos.x, pos.y, icon))
				CURSOR_ERROR("Couldn't draw icon");
		}

	icon_error:
		if (info.hbmMask)
			DeleteObject(info.hbmMask);
		if (info.hbmColor)
			DeleteObject(info.hbmColor);
		if (icon)
			DestroyCursor(icon);
	}
	else {
		CURSOR_ERROR("Couldn't get cursor info");
	}
}

/**
 * Process the Windows message queue.
 *
 * This is important to prevent Windows from thinking the window has become
 * unresponsive. As well, things like WM_PAINT (to actually draw the window
 * contents) are handled from the message queue context.
 *
 * @param s1 The format context.
 * @param gdigrab gdigrab context.
 */
static void
gdigrab_region_wnd_update(Gdigrab *gdigrab)
{
	HWND hwnd = gdigrab->region_hwnd;
	MSG msg;

	while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		DispatchMessage(&msg);
	}
}

/**
 * Grabs a frame from gdi (public device demuxer API).
 *
 * @param s1 Context from avformat core
 * @param pkt Packet holding the grabbed frame
 * @return frame size in bytes
 */
static int gdigrab_read_packet(AVPacket *pkt, char *filename)
{
	struct gdigrab *gdigrab = s1->priv_data;

	HDC        dest_hdc = gdigrab->dest_hdc;
	HDC        source_hdc = gdigrab->source_hdc;
	RECT       clip_rect = gdigrab->clip_rect;
	AVRational time_base = gdigrab->time_base;
	int64_t    time_frame = gdigrab->time_frame;

	BITMAPFILEHEADER bfh;
	int file_size = gdigrab->header_size + gdigrab->frame_size;

	int64_t curtime, delay;

	/* Calculate the time of the next frame */
	time_frame += INT64_C(1000000);

	/* Run Window message processing queue */
	if (gdigrab->show_region)
		gdigrab_region_wnd_update(s1, gdigrab);

	/* wait based on the frame rate */
	for (;;) {
		curtime = av_gettime();
		delay = time_frame * av_q2d(time_base) - curtime;
		if (delay <= 0) {
			if (delay < INT64_C(-1000000) * av_q2d(time_base)) {
				time_frame += INT64_C(1000000);
			}
			break;
		}
		if (s1->flags & AVFMT_FLAG_NONBLOCK) {
			return AVERROR(EAGAIN);
		}
		else {
			av_usleep(delay);
		}
	}

	if (av_new_packet(pkt, file_size) < 0)
		return AVERROR(ENOMEM);
	pkt->pts = curtime;

	/* Blit screen grab */
	if (!BitBlt(dest_hdc, 0, 0,
		clip_rect.right - clip_rect.left,
		clip_rect.bottom - clip_rect.top,
		source_hdc,
		clip_rect.left, clip_rect.top, SRCCOPY | CAPTUREBLT)) {
		WIN32_API_ERROR("Failed to capture image");
		return AVERROR(EIO);
	}
	if (gdigrab->draw_mouse)
		paint_mouse_pointer(s1, gdigrab);

	/* Copy bits to packet data */

	bfh.bfType = 0x4d42; /* "BM" in little-endian */
	bfh.bfSize = file_size;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = gdigrab->header_size;

	memcpy(pkt->data, &bfh, sizeof(bfh));

	memcpy(pkt->data + sizeof(bfh), &gdigrab->bmi.bmiHeader, sizeof(gdigrab->bmi.bmiHeader));

	if (gdigrab->bmi.bmiHeader.biBitCount <= 8)
		GetDIBColorTable(dest_hdc, 0, 1 << gdigrab->bmi.bmiHeader.biBitCount,
		(RGBQUAD *)(pkt->data + sizeof(bfh) + sizeof(gdigrab->bmi.bmiHeader)));

	memcpy(pkt->data + gdigrab->header_size, gdigrab->buffer, gdigrab->frame_size);

	gdigrab->time_frame = time_frame;

	return gdigrab->header_size + gdigrab->frame_size;
}



BOOL CALLBACK EnumWindowsProc(HWND hWnd1, LPARAM lParam)
{
	if (::GetWindowLong(hWnd1, GWL_STYLE) & WS_VISIBLE)
	{
		char sBuf[256];
		//获取窗口标题
		::GetWindowText(hWnd1, sBuf, 256);
		if (strlen(sBuf) > 0) {
			if (strcmp(sBuf, "文档")) {
				cout << "find wd" << endl;
				hWnd2 = hWnd1;
				GetDC(hWnd1);
				return FALSE;
			}
			
		}
	}
	return TRUE;
}

///现在我们需要做的是让SaveFrame函数能把RGB信息定稿到一个PPM格式的文件中。
///我们将生成一个简单的PPM格式文件，请相信，它是可以工作的。
void SaveFrame(AVFrame *pFrame, int width, int height, int index)
{

	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "D://frame%d.ppm", index);
	pFile = fopen(szFilename, "wb");

	if (pFile == NULL)
		return;

	// Write header
	fprintf(pFile, "P6%d %d255", width, height);

	// Write pixel data
	for (y = 0; y < height; y++)
	{
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
	}

	// Close file
	fclose(pFile);

}

int getWindows()
{
	//得到桌面窗口
	HWND hd = GetDesktopWindow();

	//得到屏幕上第一个子窗口
	hd = GetWindow(hd, GW_CHILD);
	char s[200] = { 0 };

	//循环得到所有的子窗口
	while (hd != NULL)
	{
		memset(s, 0, 200);
		if (IsWindowVisible(hd)) {
			GetWindowText(hd, s, 200);
			if (strcmp(s, "Drivers"))
			{
				cout << "find !!!!" << endl;
				hWnd2 = hd;
				/*cout<<s<<endl;*/
				//SetWindowText(hd, "My Windows");
				return 0;
			}
			cout << s << endl;
		}


		hd = GetNextWindow(hd, GW_HWNDNEXT);
	}
	return 0;
}

void main() {
	//EnumWindows(EnumWindowsProc, NULL);
	getWindows();
	gdigrab_read_header(hWnd2);
	if (hWnd2 == NULL) return;
	Decoder decoder;
	decoder.setCodec(codecpar);
	char szFilename[32];
	int index = 0;
	while (true)
	{
		int index = 0;
		index++;
		sprintf(szFilename, "D://frame%d.bmp", index);
		Sleep(1000);
		AVPacket *pkt = av_packet_alloc();
		
		//av_init_packet(pkt);
		gdigrab_read_packet(pkt, szFilename);
		//AVFrame *frame = decoder.decode(pkt);
		
		//SaveFrame(frame, width, height, index);
		cout << "get pkt" << endl;
		av_packet_free(&pkt);
		//av_frame_free(&frame);
	}
	system("pause");
}
