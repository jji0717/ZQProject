

#ifndef _VSTRM_IO_H_
#define _VSTRM_IO_H_

#include "BaseIO.h"

class VStrmIO : public BaseIOI
{
public:
	enum{
		VstrmIoSize = 64*1024
	};

	static	bool Init();
	static	void Uninit();
	static	BaseIOI* Create();

	VStrmIO();
	virtual ~VStrmIO();

	virtual bool Open(const char* szFile, int nOpenFlag);
	virtual LONGLONG GetFileSize(const char* szFile);
//	virtual bool GetFileList(const char* szDir);
	virtual int Read(char* pPtr, int nReadLen);
	virtual bool Write(char* pPtr, int nWriteLen);
	virtual bool Seek(LONGLONG lOffset, int nPosFlag = FP_BEGIN);
	virtual int GetRecommendedIOSize();
	virtual void Close();

	virtual bool ReserveBandwidth(int nbps);
	virtual void ReleaseBandwidth();

	virtual HANDLE FindFirstFile(char* name, WIN32_FIND_DATAA& w);
	virtual bool FindNextFile(HANDLE hHandle, WIN32_FIND_DATAA& w);
	virtual void FindClose(HANDLE hHandle);

	virtual int getFileStats(char* name, FSUtils::fileInfo_t* info);

	static void setClientId(DWORD dwClientId);
	
protected:

	static	bool ReleaseAllBandwidth(DWORD dwClientId);
	
	static void LogVstrmError(int nStatus, const char* szFunc);
	static void LogVstrmError(const char* szFunc);
	static HANDLE _hVstrm;
	static DWORD  _dwClientId;
	HANDLE	_hFile;
	ULONG64 _bwTicket;
	static HINSTANCE _hdll;

	typedef long(*			PFVstrmClassOpenEx)(PHANDLE); 
	typedef void(*			PFVstrmClassCloseEx)(HANDLE); 

	typedef long(*			PFVstrmGetLastError)(); 
	typedef long(*			PFVstrmClassGetErrorText)(HANDLE,long,PCHAR,ULONG); 

	typedef HANDLE(*		PFVstrmCreateFile)(HANDLE,LPCTSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
	typedef BOOLEAN (*		PFVstrmReadFile)(HANDLE,HANDLE,LPVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	typedef BOOLEAN (*		PFVstrmWriteFile)(HANDLE,HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED); 
	typedef BOOLEAN(*		PFVstrmCloseHandle)(HANDLE vstrmClassHandle, HANDLE fileHandle);
	
	typedef HANDLE(*		PFVstrmFindFirstFileEx)(HANDLE,LPCTSTR, void*);
	typedef BOOLEAN(*		PFVstrmFindNextFile)(HANDLE,HANDLE,void*);
	typedef BOOLEAN(*		PFVstrmFindClose)(HANDLE,HANDLE); 

	typedef long(*			PFVstrmClassReserveBandwidth)(HANDLE,void*,PULONG64); 
	typedef long(*			PFVstrmClassReleaseBandwidth)(HANDLE,ULONG64); 
	typedef long(*			PFVstrmClassReleaseAllBandwidth)(HANDLE,ULONG,ULONG); 
	
	static PFVstrmClassOpenEx		_pVstrmClassOpenEx; 
	static PFVstrmClassCloseEx		_pVstrmClassCloseEx;

	static PFVstrmGetLastError		_pVstrmGetLastError;
	static PFVstrmClassGetErrorText _pVstrmClassGetErrorText;

	static PFVstrmCreateFile		_pVstrmCreateFile;
	static PFVstrmReadFile			_pVstrmReadFile; 
	static PFVstrmWriteFile			_pVstrmWriteFile;
	static PFVstrmCloseHandle		_pVstrmCloseHandle; 
		
	static PFVstrmFindFirstFileEx	_pVstrmFindFirstFileEx;
	static PFVstrmFindNextFile		_pVstrmFindNextFile; 
	static PFVstrmFindClose			_pVstrmFindClose; 

	static PFVstrmClassReserveBandwidth		_pVstrmClassReserveBandwidth;
	static PFVstrmClassReleaseBandwidth		_pVstrmClassReleaseBandwidth;
	static PFVstrmClassReleaseAllBandwidth	_pVstrmClassReleaseAllBandwidth;
};


#endif