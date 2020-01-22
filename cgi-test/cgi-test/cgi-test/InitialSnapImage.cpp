#include "stdafx.h"
#include <windows.h>
#include <stdio.h>



#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define MAX_CHRROME_TABS 30

typedef struct TAG_Point
{
	int x, y;
}TYPE_Point;

typedef struct TAG_Rect
{
	TYPE_Point UpperLeft;
	TYPE_Point LowerRight;
}TYPE_Rect;

TYPE_Rect CropArea = {
	{ 0 , 96 },
	{ 1919, 1032}
};

extern char site[1000];
extern char filename[260];

int cropSize = (CropArea.LowerRight.x - CropArea.UpperLeft.x) * (CropArea.LowerRight.y - CropArea.UpperLeft.y);


int TakeScreenShot(unsigned int id);



unsigned int pBuf[SCREEN_SIZE];
unsigned int pCroppedBuf[SCREEN_SIZE];
unsigned int pLastCroppedBuf[SCREEN_SIZE];

int CompareImages(unsigned char *src1, unsigned char *src2, int size)
{
	int i;
	int c1, c2;
	for (i = 0; i < size; i++)
	{
		c1 = src1[i];
		c2 = src2[i];
		if (abs(c1 - c2) > 10)
		{
			return 1;
		}
	}
	return 0;
}

int TakeScreenShot(unsigned int id)
{
	static HDC hdc = GetDC(NULL);

	const char* exec_chrome_command = "ExecChrome.bat";

	FILE *fp;

	fp = fopen("ExecChrome.bat","wt");
	fprintf(fp, "echo off\n\"C:/Program Files (x86)/Google/Chrome/Application/chrome.exe\" --start-maximized ");
	fprintf(fp, "%s", site);
	fclose(fp);
		
	WinExec(exec_chrome_command, SW_SHOW);

	return 0;

	Sleep(3000);
	
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	HBITMAP h;

	int res = 0;

	Sleep(3000);

	OpenClipboard(NULL);
	h = (HBITMAP)GetClipboardData(CF_BITMAP);
	CloseClipboard();
	
	HANDLE fp_StreamData = INVALID_HANDLE_VALUE;
	
	
	BITMAPINFO bmpInfo;
	BITMAPINFO bmpCroppedInfo;
	BITMAPFILEHEADER bmpCroppedFileHeader;
			
	
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	
	GetDIBits(hdc, h, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
	
	if (bmpInfo.bmiHeader.biSizeImage <= 0)
	{
		bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth*abs(bmpInfo.bmiHeader.biHeight)*(bmpInfo.bmiHeader.biBitCount + 7) / 8;
	}
		
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	GetDIBits(hdc, h, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);
			
	//if (hdc)ReleaseDC(NULL, hdc);
	
	bmpCroppedInfo.bmiHeader.biWidth = (CropArea.LowerRight.x - CropArea.UpperLeft.x);
	bmpCroppedInfo.bmiHeader.biHeight = (CropArea.LowerRight.y - CropArea.UpperLeft.y);
		
	int i;
	
	for (i = (CropArea.LowerRight.y - CropArea.UpperLeft.y) - 1; i >= 0; i--)
	{
		memcpy(&pCroppedBuf[i * bmpCroppedInfo.bmiHeader.biWidth],
			&pBuf[(i + bmpInfo.bmiHeader.biHeight - CropArea.LowerRight.y) 
				* (bmpInfo.bmiHeader.biWidth) + CropArea.UpperLeft.x], 
					(CropArea.LowerRight.x - CropArea.UpperLeft.x) * sizeof(int));
	}
	
	
	
	memcpy(&bmpCroppedInfo, &bmpInfo, sizeof(BITMAPINFO));
	
	
	bmpCroppedInfo.bmiHeader.biWidth = (CropArea.LowerRight.x - CropArea.UpperLeft.x);
	bmpCroppedInfo.bmiHeader.biHeight = (CropArea.LowerRight.y - CropArea.UpperLeft.y);
	//bmpCroppedInfo.bmiHeader.biSizeImage = bmpCroppedInfo.bmiHeader.biWidth*abs(bmpCroppedInfo.bmiHeader.biHeight)*(bmpCroppedInfo.bmiHeader.biBitCount + 7) / 8;
	bmpCroppedInfo.bmiHeader.biSizeImage = bmpCroppedInfo.bmiHeader.biWidth*(bmpCroppedInfo.bmiHeader.biHeight)*(bmpCroppedInfo.bmiHeader.biBitCount) / 8;
	bmpCroppedFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpCroppedInfo.bmiHeader.biSizeImage;

	bmpCroppedFileHeader.bfReserved1 = 0;
	bmpCroppedFileHeader.bfReserved2 = 0;
	bmpCroppedFileHeader.bfType = 'MB';
	bmpCroppedFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	sprintf(filename, "C:/AviWork/www/%08X_InitialSnap.bmp", id);

	fp_StreamData = CreateFile((char *)filename,                // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		OPEN_ALWAYS,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);// no attr. template
	if ((fp_StreamData) == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Unable to Create Bitmap File", "Error", MB_OK | MB_ICONERROR);
		return res;
	}
		
	DWORD dwBytesWritten;
	WriteFile(
		fp_StreamData,           // open file handle
		&bmpCroppedFileHeader,      // start of data to write
		sizeof(bmpCroppedFileHeader),  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);
		
	WriteFile(
		fp_StreamData,           // open file handle
		&bmpCroppedInfo.bmiHeader,      // start of data to write
		sizeof(bmpCroppedInfo.bmiHeader),  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);
		
	WriteFile(
		fp_StreamData,           // open file handle
		pCroppedBuf,      // start of data to write
		bmpCroppedInfo.bmiHeader.biSizeImage,  // number of bytes to write
		&dwBytesWritten, // number of bytes that were written
		NULL);

	if (fp_StreamData != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fp_StreamData);
	}

	
		

	return res;
	
}
