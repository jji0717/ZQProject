

#ifndef _VSTRM_BASE_HEADER_
#define _VSTRM_BASE_HEADER_

#include "Log.h"
#include <string>

class VstrmBaseFunc
{
public:
	static bool init(int nBandwidthClientID, std::string& errmsg);
	static HANDLE getVstrmClassHandle();
	static void unInit();

	static int getVstrmError(std::string& strErr);
	static void getVstrmError(unsigned int status, std::string& strErr);

	static long reserveBandwidth(DWORD dwBandwidth, ULONG64& ulTicket);
	static void releaseBandwidth(const ULONG64& ulTicket);

	static 	HANDLE createFile(const char* szFilename,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
	static 	HANDLE writeFile(HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	static 	HANDLE readFile(HANDLE,LPVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	static 	HANDLE closeFile(HANDLE fileHandle);

protected:
	static HANDLE			_hVstrmClassHandle;
	static unsigned int		_uVstrmBwCltID;

};


#endif