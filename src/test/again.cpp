#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include<iostream>
#include<string>

using namespace std;

int getWindows(HWND &hwnd)
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
			if (!strcmp(s, "gzhb.pptx - PowerPoint"))
			{

				cout << "find !!!!" << endl;
				hwnd = hd;
				/*cout<<s<<endl;*/
				//SetWindowText(hd,"My Windows");
				return 0;
			}
			cout << s << endl;
		}


		hd = GetNextWindow(hd, GW_HWNDNEXT);
	}
	return 0;
}

void drawMouse(HWND& hwnd, HDC& dest_hdc, HDC& source_hdc, RECT& clip_rect) {
	CURSORINFO ci = { 0 };
	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci)) {
		HCURSOR icon = CopyCursor(ci.hCursor);
		ICONINFO info;
		POINT pos;
		int horzres = GetDeviceCaps(source_hdc, HORZRES);
		int vertres = GetDeviceCaps(source_hdc, VERTRES);
		int desktophorzres = GetDeviceCaps(source_hdc, DESKTOPHORZRES);
		int desktopvertres = GetDeviceCaps(source_hdc, DESKTOPVERTRES);
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
			cout << "Could not get icon info" << endl;
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
				cout << "Couldn't get window rectangle" << endl;
				goto icon_error;
			}
		}
		else {
			//that would keep the correct location of mouse with hidpi screens
			pos.x = ci.ptScreenPos.x * desktophorzres / horzres - clip_rect.left - info.xHotspot;
			pos.y = ci.ptScreenPos.y * desktopvertres / vertres - clip_rect.top - info.yHotspot;
		}

		cout << "x:" << pos.x << "  y:" << pos.y << endl;
		if (pos.x >= 0 && pos.x <= clip_rect.right - clip_rect.left &&
			pos.y >= 0 && pos.y <= clip_rect.bottom - clip_rect.top) {
			
			if (!DrawIcon(dest_hdc, pos.x, pos.y, icon))
				cout << "Couldn't draw icon" << endl;
		}
		else {
			cout << "....." << endl;
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
		cout << "Couldn't get cursor info" << endl;
	}
}

void shotCutScreen(HWND hwnd, string path) {
	
	//ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	//SetForegroundWindow(hwnd);
	/*if (IsIconic(hwnd)) {
		ShowWindow(hwnd, SW_SHOWNORMAL);
	}*/
		
	HDC source_hdc = NULL;
	HDC dest_hdc = NULL;

	int bpp;
	int horzres;
	int vertres;
	int desktophorzres;
	int desktopvertres;
	RECT virtual_rect;
	RECT clip_rect;

	BITMAPINFO bmi;
	HBITMAP hbmp = NULL;

	void *buffer = NULL;

	source_hdc = GetDC(hwnd);
	if (!source_hdc) {
		cout << "can not find windows device context" << endl;
		return;
	}
	//获取系统颜色位数
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

	clip_rect.left = virtual_rect.left;
	clip_rect.top = virtual_rect.top;
	clip_rect.right = virtual_rect.right;
	clip_rect.bottom = virtual_rect.bottom;

	cout << "capture area as " << clip_rect.right - clip_rect.left << "x" << clip_rect.bottom - clip_rect.top << "x" << bpp
		<< " at(" << clip_rect.left << "," << clip_rect.top << ")" << endl;

	dest_hdc = CreateCompatibleDC(source_hdc);
	if (!dest_hdc) {
		cout << "Screen DC CreateCompatibleDC" << endl;
		goto error;
	}
	
	/* Create a DIB and select it into the dest_hdc */
	/*bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
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
		cout << "Creating DIB Section fail" << endl;
		goto error;
	}

	if (!SelectObject(dest_hdc, hbmp)) {
		cout << "SelectObject fail" << endl;
		goto error;
	}
	GetObject(hbmp, sizeof(BITMAP), &bmp);*/

	HBITMAP hBmpScreen = CreateCompatibleBitmap(source_hdc, clip_rect.right - clip_rect.left, clip_rect.bottom - clip_rect.top);
	HBITMAP holdbmp = (HBITMAP)SelectObject(dest_hdc, hBmpScreen);

	BITMAP bm;
	GetObject(hBmpScreen, sizeof(bm), &bm);

	BITMAPINFOHEADER bi = { 0 };
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bm.bmWidth;
	bi.biHeight = bm.bmHeight;
	bi.biPlanes = bm.bmPlanes;
	bi.biBitCount = bm.bmBitsPixel;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = bm.bmHeight * bm.bmWidthBytes;
	// 图片的像素数据
	char *buf = new char[bi.biSizeImage];
	
	BitBlt(dest_hdc, 0, 0, clip_rect.right - clip_rect.left, clip_rect.bottom - clip_rect.top, source_hdc, clip_rect.left, clip_rect.top, SRCCOPY);
	drawMouse(hwnd, dest_hdc, source_hdc, clip_rect);
	GetDIBits(dest_hdc, hBmpScreen, 0L, (DWORD)clip_rect.bottom - clip_rect.top, buf, (LPBITMAPINFO)&bi, (DWORD)DIB_RGB_COLORS);

	BITMAPFILEHEADER bfh = { 0 };
	bfh.bfType = ((WORD)('M' << 8) | 'B');
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	HANDLE hFile = CreateFile(path.c_str(), GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD dwWrite;
	WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWrite, NULL);
	WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWrite, NULL);
	WriteFile(hFile, buf, bi.biSizeImage, &dwWrite, NULL);
	CloseHandle(hFile);
	delete[] buf;
	hBmpScreen = (HBITMAP)SelectObject(dest_hdc, holdbmp);


error:
	if (source_hdc)
		ReleaseDC(hwnd, source_hdc);
	if (dest_hdc)
		DeleteDC(dest_hdc);
	if (hbmp)
		DeleteObject(hbmp);
	if (source_hdc)
		DeleteDC(source_hdc);

}

void main() {
	HWND hwnd = NULL;
	getWindows(hwnd);
	if (hwnd == NULL) {
		cout << "not find windows" << endl;
		return;
	}
	int index = 0;
	char name[256] = { 0 };
	while (true)
	{
		Sleep(1000);
		sprintf_s(name, 256, "D://%d.bmp", index);
		index++;
		printf("shooting %s\n", name);
		shotCutScreen(hwnd, name);
	}
	system("pause");
}