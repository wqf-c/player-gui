#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include<iostream>

using namespace std;

 // ���ݴ��ھ����̨��ͼ�����BMPλͼ�ļ�������ʾ��picture �ؼ���
void GetScreenBmp(HWND hwnd, CHAR* path)
{
	int width, height, left, top;
	RECT rect = { 0 };
	HDC pDC;// ԴDC
	//�ж��ǲ��Ǵ��ھ������ǵĻ�����ʹ��GetDC����ȡDC ��Ȼ��ͼ���Ǻ���
	if (hwnd == ::GetDesktopWindow())
	{
		pDC = CreateDCA("DISPLAY", NULL, NULL, NULL);
	}
	else
	{
		pDC = ::GetDC(hwnd);//��ȡ��ĻDC(0Ϊȫ���������Ϊ����)
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
	int BitPerPixel = ::GetDeviceCaps(pDC, BITSPIXEL);//�����ɫģʽ
	
	HDC memDC;//�ڴ�DC
	memDC = ::CreateCompatibleDC(pDC);
	HBITMAP memBitmap, oldmemBitmap;//��������Ļ���ݵ�bitmap
	memBitmap = ::CreateCompatibleBitmap(pDC, width, height);
	oldmemBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);//��memBitmapѡ���ڴ�DC
	if (hwnd == ::GetDesktopWindow())
	{
		BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//ͼ���ȸ߶Ⱥͽ�ȡλ��
	}
	else
	{
		bool bret = ::PrintWindow(hwnd, memDC, PW_CLIENTONLY);
		if (!bret)
		{
			BitBlt(memDC, 0, 0, width, height, pDC, left, top, SRCCOPY);//ͼ���ȸ߶Ⱥͽ�ȡλ��
		}
	}
	//���´��뱣��memDC�е�λͼ���ļ�
	BITMAP bmp;
	::GetObject(memBitmap, sizeof(BITMAP), &bmp);;//���λͼ��Ϣ
	FILE *fp;
	fopen_s(&fp, path, "w+b");//ͼƬ����·���ͷ�ʽ

	BITMAPINFOHEADER bih = { 0 };//λͼ��Ϣͷ
	bih.biBitCount = bmp.bmBitsPixel;//ÿ�������ֽڴ�С
	bih.biCompression = BI_RGB;
	bih.biHeight = bmp.bmHeight;//�߶�
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//ͼ�����ݴ�С
	bih.biWidth = bmp.bmWidth;//���

	BITMAPFILEHEADER bfh = { 0 };//λͼ�ļ�ͷ
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//��λͼ���ݵ�ƫ����
	bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//�ļ��ܵĴ�С
	bfh.bfType = (WORD)0x4d42;

	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//д��λͼ�ļ�ͷ
	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//д��λͼ��Ϣͷ
	byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//�����ڴ汣��λͼ����
	GetDIBits(memDC, (HBITMAP)memBitmap, 0, height, p,
		(LPBITMAPINFO)&bih, DIB_RGB_COLORS);//��ȡλͼ����
	fwrite(p, 1, bmp.bmWidthBytes * bmp.bmHeight, fp);//д��λͼ����
	delete[] p;
	fclose(fp);
	//HWND sBitHwnd = GetDlgItem(g_Hwnd, IDC_STATIC_IMG);
	///*�����ڴ��е�λͼ��� ��ԭԭ�����ڴ�DCλͼ��� ����ֱ���� memBitmap�Ҳ��Ժ����ǲ��в�֪��Ϊʲô*/
	//HBITMAP oleImage = (HBITMAP)::SelectObject(memDC, oldmemBitmap);
	//oleImage = (HBITMAP)SendMessage(sBitHwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)oleImage);
#if 0
	/*���ַ���Ҳ�ܰ�λͼ��ʾ��picture �ؼ���*/
	HDC bitDc = NULL;
	bitDc = ::GetDC(sBitHwnd);
	BitBlt(bitDc, 0, 0, bmp.bmWidth, bmp.bmHeight, memDC, 0, 0, SRCCOPY); //�ڴ�DCӳ�䵽��ĻDC
	ReleaseDC(sBitHwnd, bitDc);
	/*�����Ҫ��λͼת��*/
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
		hdc = ::GetDC(hWnd);//��ȡ��ĻDC(0Ϊȫ���������Ϊ����)
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
	// ͼƬ����������
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
		//��ȡ���ڱ���
		::GetWindowText(hWnd1, sBuf, 256);
		if (strlen(sBuf) > 0) {
			if (strcmp(sBuf, "�ĵ�")) {
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
	//�õ����洰��
	HWND hd = GetDesktopWindow();

	//�õ���Ļ�ϵ�һ���Ӵ���
	hd = GetWindow(hd, GW_CHILD);
	char s[200] = { 0 };

	//ѭ���õ����е��Ӵ���
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