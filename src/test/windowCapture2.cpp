#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include<iostream>

using namespace std;

 // 根据窗口句柄后台截图保存成BMP位图文件并且显示到picture 控件上
void GetScreenBmp(HWND hwnd, CHAR* path)
{
	int width, height, left, top;
	RECT rect = { 0 };
	HDC pDC;// 源DC
	//判断是不是窗口句柄如果是的话不能使用GetDC来获取DC 不然截图会是黑屏
	if (hwnd == ::GetDesktopWindow())
	{
		pDC = CreateDCA("DISPLAY", NULL, NULL, NULL);
	}
	else
	{
		pDC = ::GetDC(hwnd);//获取屏幕DC(0为全屏，句柄则为窗口)
	}
	if (hwnd == NULL)
	{
		width = GetDeviceCaps(pDC, HORZRES);
		height = GetDeviceCaps(pDC, VERTRES);
	}
	else
	{
		GetWindowRect(hwnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		cout << "width:" << width << "   height:" << height << endl;
	}
	left = rect.left;
	top = rect.top;
	int BitPerPixel = ::GetDeviceCaps(pDC, BITSPIXEL);//获得颜色模式
	
	HDC memDC;//内存DC
	memDC = ::CreateCompatibleDC(pDC);
	HBITMAP memBitmap, oldmemBitmap;//建立和屏幕兼容的bitmap
	memBitmap = ::CreateCompatibleBitmap(pDC, width, height);
	oldmemBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);//将memBitmap选入内存DC
	if (hwnd == ::GetDesktopWindow())
	{
		BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//图像宽度高度和截取位置
	}
	else
	{
		bool bret = ::PrintWindow(hwnd, memDC, PW_CLIENTONLY);
		if (!bret)
		{
			BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//图像宽度高度和截取位置
		}
	}
	//以下代码保存memDC中的位图到文件
	BITMAP bmp;
	::GetObject(memBitmap, sizeof(BITMAP), &bmp);;//获得位图信息
	FILE *fp;
	fopen_s(&fp, path, "w+b");//图片保存路径和方式

	BITMAPINFOHEADER bih = { 0 };//位图信息头
	bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
	bih.biCompression = BI_RGB;
	bih.biHeight = bmp.bmHeight;//高度
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
	bih.biWidth = bmp.bmWidth;//宽度

	BITMAPFILEHEADER bfh = { 0 };//位图文件头
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
	bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
	bfh.bfType = (WORD)0x4d42;

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//写入位图文件头
	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//写入位图信息头
	byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//申请内存保存位图数据
	GetDIBits(memDC, (HBITMAP)memBitmap, 0, height, p,
		(LPBITMAPINFO)&bih, DIB_RGB_COLORS);//获取位图数据
	fwrite(p, 1, bmp.bmWidthBytes * bmp.bmHeight, fp);//写入位图数据
	delete[] p;
	fclose(fp);
	//HWND sBitHwnd = GetDlgItem(g_Hwnd, IDC_STATIC_IMG);
	///*返回内存中的位图句柄 还原原来的内存DC位图句柄 不能直接用 memBitmap我测试好像是不行不知道为什么*/
	//HBITMAP oleImage = (HBITMAP)::SelectObject(memDC, oldmemBitmap);
	//oleImage = (HBITMAP)SendMessage(sBitHwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)oleImage);
#if 0
	/*这种方法也能把位图显示到picture 控件上*/
	HDC bitDc = NULL;
	bitDc = ::GetDC(sBitHwnd);
	BitBlt(bitDc, 0, 0, bmp.bmWidth, bmp.bmHeight, memDC, 0, 0, SRCCOPY); //内存DC映射到屏幕DC
	ReleaseDC(sBitHwnd, bitDc);
	/*如果需要把位图转换*/
	/*
	CImage image;
	image.Create(nWidth, nHeight, nBitPerPixel);
	BitBlt(image.GetDC(), 0, 0, nWidth, nHeight, hdcSrc, 0, 0, SRCCOPY);
	::ReleaseDC(NULL, hdcSrc);
	image.ReleaseDC();
	image.Save(path, Gdiplus::ImageFormatPNG);//ImageFormatJPEG
	*/
#endif
	DeleteObject(memBitmap);
	//DeleteObject(oleImage);
	DeleteDC(memDC);
	ReleaseDC(hwnd, pDC);

}

void ShootScreen(const char* filename, HWND hWnd)
{
	HDC hdc;
	if (hWnd == ::GetDesktopWindow())
	{
		hdc = CreateDCA("DISPLAY", NULL, NULL, NULL);
	}
	else
	{
		hdc = ::GetDC(hWnd);//获取屏幕DC(0为全屏，句柄则为窗口)
	}
	int32_t ScrWidth = 0, ScrHeight = 0;
	RECT rect = { 0 };
	if (hWnd == NULL)
	{
		ScrWidth = GetDeviceCaps(hdc, HORZRES);
		ScrHeight = GetDeviceCaps(hdc, VERTRES);
	}
	else
	{
		GetWindowRect(hWnd, &rect);
		ScrWidth = rect.right - rect.left;
		ScrHeight = rect.bottom - rect.top;
	}
	HDC hmdc = CreateCompatibleDC(hdc);

	HBITMAP hBmpScreen = CreateCompatibleBitmap(hdc, ScrWidth, ScrHeight);
	HBITMAP holdbmp = (HBITMAP)SelectObject(hmdc, hBmpScreen);

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
	BitBlt(hmdc, 0, 0, ScrWidth, ScrHeight, hdc, rect.left, rect.top, SRCCOPY);
	GetDIBits(hmdc, hBmpScreen, 0L, (DWORD)ScrHeight, buf, (LPBITMAPINFO)&bi, (DWORD)DIB_RGB_COLORS);

	BITMAPFILEHEADER bfh = { 0 };
	bfh.bfType = ((WORD)('M' << 8) | 'B');
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD dwWrite;
	WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWrite, NULL);
	WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWrite, NULL);
	WriteFile(hFile, buf, bi.biSizeImage, &dwWrite, NULL);
	CloseHandle(hFile);
	hBmpScreen = (HBITMAP)SelectObject(hmdc, holdbmp);
}

HWND hWnd2 = NULL;

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
			if (strcmp(s,"Drivers"))
			{
				cout << "find !!!!" << endl;
				hWnd2 = hd;
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


int32_t main()
{
	getWindows();
	//EnumWindows(EnumWindowsProc, NULL);
	////hWnd2 = FindWindow(NULL, "Drivers");
	//Sleep(100);
	//cout << (hWnd2 == NULL) << endl;
	char name[256] = { 0 };
	for (int32_t i = 0; i < 20; ++i)
	{
		sprintf_s(name, 256, "D://%d.bmp", i);
		printf("shooting %s\n", name);
		//ShootScreen(name, hWnd2);
		if (hWnd2 == NULL) return 0;
		//ShootScreen(name, hWnd2);
		GetScreenBmp(hWnd2, name);
		Sleep(1000);
	}
	system("pause");
	return 0;
}