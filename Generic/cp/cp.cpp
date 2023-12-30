// cp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MD5CheckSumUtil.h"
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#include "vstrmuser.h"


#include "getopt.h"

class BitrateControlor
{
public:
	void init(int nBitrateBps, int nCtrlIntervalMs = 100)
	{
		_nBitrateBps = nBitrateBps;
		_nCtrlIntervalMs = nCtrlIntervalMs;		
	}
	
	void start()
	{
		_dwLastCtrl = _dwStart = GetTickCount();
	}
	
	void control(LONGLONG llProcBytes)
	{
		if (!_nBitrateBps)
			return;
		
		DWORD dwNow = GetTickCount();
		if (dwNow>=_dwLastCtrl && dwNow<_dwLastCtrl + _nCtrlIntervalMs)
		{
			//do nothing
			return;
		}
		
		int nSpent = dwNow - _dwStart;			
		int nShouldSpent = int(llProcBytes*8000/_nBitrateBps); //ms
		
		if (nShouldSpent > nSpent)
		{
			Sleep(nShouldSpent - nSpent);
			_dwLastCtrl = GetTickCount();
		}
		else
		{
			_dwLastCtrl = dwNow;
		}
	}	
	
private:
	
	int			_nBitrateBps;
	int			_nCtrlIntervalMs;
	DWORD		_dwStart;
	DWORD		_dwLastCtrl;
};

class VstreamIO {

public:
	
	VstreamIO(): _hVstrm(INVALID_HANDLE_VALUE), _hdll(NULL), _hFile(INVALID_HANDLE_VALUE) {
	} 

	~VstreamIO() {
		if(isValid()) {
			_classClose(_hVstrm);
			CloseHandle(_hdll);
		}
		if(_hFile) {
			CloseHandle(_hFile);
		}
	}
	
	bool isValid() const {
		return (_hdll != NULL);
	}
	
	bool isOpened() const {
		return (_hFile != INVALID_HANDLE_VALUE);
	}

	bool load() {
		
		_hdll = LoadLibrary("VstrmDLLEx.dll");
		if (!_hdll)
		{
			fprintf(stderr, "(1)failed to load VstrmDll, error is %d", GetLastError());
			return false;
		}
		
		_classOpen = (PFVstrmClassOpenEx)GetProcAddress(_hdll, "VstrmClassOpenEx");
		_classClose = (PFVstrmClassCloseEx)GetProcAddress(_hdll, "VstrmClassCloseEx");
		
		_createFile = (PFVstrmCreateFile)GetProcAddress(_hdll, "VstrmCreateFile");
		_readFile = (PFVstrmReadFile)GetProcAddress(_hdll, "VstrmReadFile");
		_closeHandle = (PFVstrmCloseHandle)GetProcAddress(_hdll, "VstrmCloseHandle");
		
	
		_getLastError = (PFVstrmGetLastError)GetProcAddress(_hdll, "VstrmGetLastError");
		_getErrorText = (PFVstrmClassGetErrorText)GetProcAddress(_hdll, "VstrmClassGetErrorText");

	
		VSTATUS status = _classOpen(&_hVstrm);
		if (status != VSTRM_SUCCESS) 
		{
			fprintf(stderr, "(1)failed to open vstream class");
			return false;
		}
	
		return true;
	}

	bool openFile(const char* szFile, int nOpenFlag)
	{
		_hFile = _createFile(_hVstrm, 
			szFile, 
			GENERIC_READ, 
			FILE_SHARE_READ, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL);
		if (INVALID_HANDLE_VALUE == _hFile)
		{
			fprintf(stderr, "(1)failed to open file [%s]", szFile);
			return false;
		}
		
		return true;
	}

	int readFile(char* pPtr, int nReadLen)
	{
		DWORD dwRet = 0;
		if (!_readFile(_hVstrm, _hFile, pPtr, nReadLen, &dwRet, 0))
		{		
			int nStatus = _getLastError();
			if (nStatus == 0x00000026)
			{
				return dwRet;
			}
			
			char szBuf[512];
			szBuf[0] = '\0';
			
			_getErrorText(_hVstrm, nStatus, szBuf, sizeof(szBuf));
			szBuf[511] = '\0';

			fprintf(stderr, "(1)failed to read file: (%s)\n", szBuf);
		}
		
		return dwRet;
	}

	bool writeFile(char* pPtr, int nWriteLen)
	{
		if (!isOpened()) {
			return false;
		}
		
		DWORD dwWritten = 0;
		if (!_writeFile(_hVstrm, _hFile, pPtr, nWriteLen, &dwWritten, 0))
		{
			fprintf(stderr, "(1)failed to write file: (%s)", _getLastError());
			return false;
		}
		
		return true;
	}
	
	void close() {
		CloseHandle(_hFile);
	}

private:

	HANDLE _hVstrm;
	HMODULE _hdll;
	HANDLE _hFile;

	typedef long(*PFVstrmClassOpenEx)(PHANDLE); 
	typedef void(*PFVstrmClassCloseEx)(HANDLE); 
	
	typedef long(*PFVstrmGetLastError)(); 
	typedef long(*PFVstrmClassGetErrorText)(HANDLE,long,PCHAR,ULONG); 
	
	typedef HANDLE(*PFVstrmCreateFile)(HANDLE,LPCTSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
	typedef BOOLEAN (*PFVstrmReadFile)(HANDLE,HANDLE,LPVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	typedef BOOLEAN (*PFVstrmWriteFile)(HANDLE,HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	typedef BOOLEAN(*PFVstrmCloseHandle)(HANDLE vstrmClassHandle, HANDLE fileHandle);
	
	PFVstrmClassOpenEx		_classOpen; 
	PFVstrmClassCloseEx		_classClose;
	
	PFVstrmGetLastError		_getLastError;
	PFVstrmClassGetErrorText _getErrorText;
	
	PFVstrmCreateFile		_createFile;
	PFVstrmReadFile			_readFile; 
	PFVstrmWriteFile		_writeFile;
	PFVstrmCloseHandle		_closeHandle; 
		
};

void usage()
{
	printf("Usage: cp -s <source file>\n");
	printf("       -d <destination>, \"NULL\" for no output, \"STDOUT\" for output to screen, \"MD5\" for caculate the md5 check sum to screen\n");
	printf("       -b <copy bitrate> default 0 bps\n");
	printf("       -i [copy bitrate adjustment interval] default is 40ms\n");
	printf("       -h display this help\n");	
	printf("       -v read from VSTREAM\n\n");
}


int main(int argc, char* argv[])
{
	if (argc <4)
	{
		usage();
		return 0;
	}

	char szSourceFile[256];
	char szOutputFile[256];
	DWORD dwBitrate = 0;
	DWORD dwInterval = 40;
	LONGLONG  offsetStart=0;
	LONGLONG  offsetEnd=0;
	
	VstreamIO vstrm;

	int ch;
	while((ch = getopt(argc, argv, "hHs:S:d:D:b:B:i:I:v")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
		case 'H':
			usage();
			return 0;

		case 's':
		case 'S':
			if (optarg)
			{
				strcpy(szSourceFile, optarg);
			}
			break;

		case 'd':
		case 'D':
			if (optarg)
			{
				strcpy(szOutputFile, optarg);
			}
			break;

		case 'b':
			if (optarg)
			{
				dwBitrate = atoi(optarg);
			}
			break;

		case 'i':
			if (optarg)
			{
				dwInterval = atoi(optarg);
			}
			break;

		case 'v':
			if(!vstrm.load()) {
				return (1);
			}
			break;

		default:
			fprintf(stderr, "(1)Error: unknown option %c specified\n", ch);
			return 1;
		}
	}

	HANDLE hFile1 = INVALID_HANDLE_VALUE;
	if(vstrm.isValid()) {
		if(!vstrm.openFile(szSourceFile, 0)) {
			return 1;
		}
	}
	else {
		hFile1 = CreateFile(szSourceFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING, 0, 0);
		if (hFile1==INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "(1)Fail to open source fille [%s]\n", szSourceFile);
			return 1;
		}
	}

	BOOL bMD5Sum = false;
	HANDLE hFile2 = 0;
	if (!stricmp(szOutputFile, "NULL"))
	{
		//nothing
	}
	else if (!stricmp(szOutputFile, "STDOUT"))
	{
		hFile2 = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hFile2==INVALID_HANDLE_VALUE)
		{			
			fprintf(stderr, "(1)[%s] Fail to open std output with error code %d\n", szSourceFile, GetLastError());
			if(vstrm.isValid()) {
				vstrm.close();
			}
			else {
				CloseHandle(hFile1);
			}

			return 1;
		}
	}
	else if (!stricmp(szOutputFile, "MD5"))
	{
		bMD5Sum = TRUE;
	}
	else
	{
		hFile2 = CreateFile(szOutputFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,0, CREATE_ALWAYS, 0, 0);
		if (hFile2==INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "(1)[%s] Fail to open target file %s with error code %d\n", szSourceFile, szOutputFile, GetLastError());
			CloseHandle(hFile1);
			return 1;
		}
	}

	BitrateControlor bc;
	bc.init(dwBitrate, dwInterval);
	bc.start();

	ZQ::common::MD5ChecksumUtil md5;	

	char buf[1024];
	DWORD dwRead=0;
	DWORD dwToRead = sizeof(buf);
	LONGLONG llProc = 0;

	DWORD dw1 = GetTickCount();
	do
	{
		if(vstrm.isValid()) {
			if(!(dwRead = vstrm.readFile(buf, dwToRead))) {
				break;
			}
		}
		else {
			if(!ReadFile(hFile1, buf, dwToRead, &dwRead, 0))
				break;
		}

		if (dwRead)
		{
			DWORD dwWrite;
			if (hFile2)
				WriteFile(hFile2, buf, dwRead, &dwWrite, 0);
			
			if (bMD5Sum)
				md5.checksum(buf, dwRead);

			llProc += dwRead;
		}
		else
			break;

		bc.control(llProc);
	}while(1);

	bc.control(llProc);

	if (hFile2)
		CloseHandle(hFile2);
	CloseHandle(hFile1);
	
	if (bMD5Sum)
	{
		printf("%s", md5.lastChecksum());
	}

	DWORD dw2 = GetTickCount() - dw1;
	DWORD dwSpeed;
	if (dw2)
	{
		dwSpeed = (DWORD)(llProc*8000/dw2);
	}
	else
		dwSpeed = 0;

	fprintf(stderr, "(0)[%s] copied [%I64d]bytes, spent[%d]ms, required bitrate[%d]bps, actual bitrate[%d]bps\n",
		szSourceFile, llProc, dw2, dwBitrate, dwSpeed);

	return 0;
}
