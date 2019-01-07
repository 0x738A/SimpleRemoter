// ScreenSpy.h: interface for the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_)
#define AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define ALGORITHM_DIFF 1
#define COPY_ALL 1	// ����ȫ����Ļ�����ֿ鿽����added by yuanyuanxiang 2019-1-7��
#include "CursorInfor.h"


class CScreenSpy  
{
public:
	CScreenSpy(ULONG ulbiBitCount);
	virtual ~CScreenSpy();
	ULONG GetBISize(); 
	LPBITMAPINFO GetBIData();
	ULONG  m_ulbiBitCount;
	LPBITMAPINFO m_BitmapInfor_Full;
	ULONG  m_ulFullWidth, m_ulFullHeight;               //��Ļ�ķֱ���
	LPBITMAPINFO ConstructBI(ULONG ulbiBitCount, 
		ULONG ulFullWidth, ULONG ulFullHeight);

	HWND m_hDeskTopWnd;      //��ǰ�������Ĵ��ھ��
	HDC  m_hFullDC;          //Explorer.exe �Ĵ����豸DC
	HDC  m_hFullMemDC;
	HBITMAP	m_BitmapHandle;
	PVOID   m_BitmapData_Full;
	DWORD  m_dwBitBltRop;
	LPVOID GetFirstScreenData();
	ULONG GetFirstScreenLength();
	LPVOID GetNextScreenData(ULONG* ulNextSendLength);
	BYTE* m_RectBuffer;
	ULONG m_RectBufferOffset;
	BYTE   m_bAlgorithm;
	VOID WriteRectBuffer(LPBYTE	szBuffer,ULONG ulLength);
	CCursorInfor m_CursorInfor;
	HDC  m_hDiffMemDC;
	HBITMAP	m_DiffBitmapHandle;
	PVOID   m_DiffBitmapData_Full;
	ULONG CompareBitmap(LPBYTE CompareSourData, LPBYTE CompareDestData, 
		LPBYTE szBuffer, DWORD ulCompareLength);
	VOID ScanScreen(HDC hdcDest, HDC hdcSour, ULONG ulWidth, ULONG ulHeight);
};

#endif // !defined(AFX_SCREENSPY_H__5F74528D_9ABD_404E_84D2_06C96A0615F4__INCLUDED_)
