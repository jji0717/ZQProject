
#include "CECommon.h"
#include "VstrmIO.h"
#include "vstrmuser.h"
#include "vstrmtypes.h"


#define VsIo			"VsIo"

using namespace ZQ::common;

#define MOLOG	(glog)

HANDLE VStrmIO::_hVstrm = NULL;
DWORD VStrmIO::_dwClientId = 0;
HINSTANCE VStrmIO::_hdll = NULL;

VStrmIO::PFVstrmClassOpenEx	VStrmIO::	_pVstrmClassOpenEx = NULL; 
VStrmIO::PFVstrmClassCloseEx		VStrmIO::_pVstrmClassCloseEx = NULL;

VStrmIO::PFVstrmCreateFile		VStrmIO::_pVstrmCreateFile = NULL;
VStrmIO::PFVstrmReadFile		VStrmIO::_pVstrmReadFile = NULL; 
VStrmIO::PFVstrmWriteFile		VStrmIO::_pVstrmWriteFile = NULL;
VStrmIO::PFVstrmCloseHandle		VStrmIO::_pVstrmCloseHandle = NULL; 

VStrmIO::PFVstrmFindFirstFileEx	VStrmIO::_pVstrmFindFirstFileEx = NULL;
VStrmIO::PFVstrmFindNextFile		VStrmIO::_pVstrmFindNextFile = NULL; 
VStrmIO::PFVstrmFindClose			VStrmIO::_pVstrmFindClose = NULL; 

VStrmIO::PFVstrmGetLastError		VStrmIO::_pVstrmGetLastError = NULL;
VStrmIO::PFVstrmClassGetErrorText	VStrmIO::_pVstrmClassGetErrorText = NULL;

VStrmIO::PFVstrmClassReserveBandwidth		VStrmIO::_pVstrmClassReserveBandwidth = NULL;
VStrmIO::PFVstrmClassReleaseBandwidth		VStrmIO::_pVstrmClassReleaseBandwidth = NULL;
VStrmIO::PFVstrmClassReleaseAllBandwidth		VStrmIO::_pVstrmClassReleaseAllBandwidth = NULL;


VStrmIO::VStrmIO()
{	
	_bwTicket = 0;
}

VStrmIO::~VStrmIO()
{
	Close();
}

bool VStrmIO::Init()
{
	VSTATUS vStatus;
	_hVstrm = INVALID_HANDLE_VALUE;

	_hdll = LoadLibrary("VstrmDLLEx.dll");
	if (!_hdll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VsIo, "Failed to load VstrmDll, error is %d"),GetLastError());
		return false;
	}

	_pVstrmClassOpenEx = (PFVstrmClassOpenEx)GetProcAddress(_hdll, "VstrmClassOpenEx");
	_pVstrmClassCloseEx = (PFVstrmClassCloseEx)GetProcAddress(_hdll, "VstrmClassCloseEx");
	
	_pVstrmCreateFile = (PFVstrmCreateFile)GetProcAddress(_hdll, "VstrmCreateFile");
	_pVstrmReadFile = (PFVstrmReadFile)GetProcAddress(_hdll, "VstrmReadFile");
	_pVstrmCloseHandle = (PFVstrmCloseHandle)GetProcAddress(_hdll, "VstrmCloseHandle");

	_pVstrmFindFirstFileEx = (PFVstrmFindFirstFileEx)GetProcAddress(_hdll, "VstrmFindFirstFileEx");
	_pVstrmFindNextFile = (PFVstrmFindNextFile)GetProcAddress(_hdll, "VstrmFindNextFile");
	_pVstrmFindClose = (PFVstrmFindClose)GetProcAddress(_hdll, "VstrmFindClose");

	_pVstrmGetLastError = (PFVstrmGetLastError)GetProcAddress(_hdll, "VstrmGetLastError");
	_pVstrmClassGetErrorText = (PFVstrmClassGetErrorText)GetProcAddress(_hdll, "VstrmClassGetErrorText");

	_pVstrmClassReserveBandwidth = (PFVstrmClassReserveBandwidth)GetProcAddress(_hdll, "VstrmClassReserveBandwidth");
	_pVstrmClassReleaseBandwidth = (PFVstrmClassReleaseBandwidth)GetProcAddress(_hdll, "VstrmClassReleaseBandwidth");
	_pVstrmClassReleaseAllBandwidth = (PFVstrmClassReleaseAllBandwidth)GetProcAddress(_hdll, "VstrmClassReleaseAllBandwidth");

	if (!_pVstrmClassOpenEx||!_pVstrmClassCloseEx||!_pVstrmCreateFile||!_pVstrmReadFile 
		||!_pVstrmCloseHandle||!_pVstrmFindFirstFileEx||!_pVstrmFindNextFile||!_pVstrmFindClose
		||!_pVstrmGetLastError||!_pVstrmClassReserveBandwidth||!_pVstrmClassReleaseBandwidth
		||!_pVstrmClassGetErrorText ||!_pVstrmClassReleaseAllBandwidth)
	{

		MOLOG(Log::L_ERROR, CLOGFMT(VsIo, "Failed to load VstrmDll API"));
		return false;
	}

	vStatus = _pVstrmClassOpenEx(&_hVstrm);
	if (vStatus != VSTRM_SUCCESS) 
	{
		LogVstrmError(vStatus, "VstrmClassOpenEx()");
		return false;
	}

	ReleaseAllBandwidth(_dwClientId);
	MOLOG(Log::L_INFO, CLOGFMT(VsIo, "Vstrm initialized successful, vstrm handle[0x%08x], clientid[%u]"), _hVstrm, _dwClientId);
	return true;
}

BaseIOI* VStrmIO::Create()
{
	return new VStrmIO();
}

void VStrmIO::Uninit()
{
	if (_hVstrm && _hVstrm!= INVALID_HANDLE_VALUE)
	{		
		ReleaseAllBandwidth(_dwClientId);
		_pVstrmClassCloseEx(_hVstrm);
		_hVstrm = NULL;
		MOLOG(Log::L_INFO, CLOGFMT(VsIo, "Vstrm uninitialized successful, vstrm handle[0x%08x]"), _hVstrm);
	}

	if (_hdll)
	{
		FreeLibrary(_hdll);
		_hdll = NULL;
		_pVstrmClassOpenEx = NULL;
		_pVstrmClassCloseEx = NULL;

		_pVstrmCreateFile = NULL;
		_pVstrmReadFile =NULL;
		_pVstrmCloseHandle = NULL;

		_pVstrmFindFirstFileEx = NULL;
		_pVstrmFindNextFile = NULL;
		_pVstrmFindClose = NULL;

		_pVstrmGetLastError = NULL;
		_pVstrmClassGetErrorText =  NULL;

		_pVstrmClassReserveBandwidth = NULL;
		_pVstrmClassReleaseBandwidth = NULL;
		_pVstrmClassReleaseAllBandwidth = NULL;
	}
}

bool VStrmIO::Open(const char* szFile, int nOpenFlag)
{
	std::string strFile;
	if (szFile[0] == '/' || szFile[0] == '\\')
		strFile = szFile + 1;
	else
		strFile = szFile;

	_hFile = _pVstrmCreateFile(_hVstrm, 
		strFile.c_str(), 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);
	if (INVALID_HANDLE_VALUE == _hFile)
	{
		char buf[256];
		sprintf(buf, "open file[%s]", strFile.c_str());
		LogVstrmError(buf);
		return false;
	}
	
	_strFile = szFile;
	_bOpened = true;
	return true;
}

LONGLONG VStrmIO::GetFileSize(const char* szFile)
{
	std::string strFile;
	if (szFile[0] == '/' || szFile[0] == '\\')
		strFile = szFile + 1;
	else
		strFile = szFile;

	DLL_FIND_DATA_LONG findData = {0};

	VHANDLE fileHandle = _pVstrmFindFirstFileEx(_hVstrm, strFile.c_str(), &findData);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
		_pVstrmFindClose(_hVstrm,fileHandle);
		return fileSize.QuadPart;
	}

	return 0;
}

int VStrmIO::Read(char* pPtr, int nReadLen)
{
	DWORD dwRet = 0;
	if (!_pVstrmReadFile(_hVstrm, _hFile, pPtr, nReadLen, &dwRet, 0))
	{		
		int nStatus = _pVstrmGetLastError();
		if (nStatus == 0x00000026)
		{
			//ERROR_HANDLE_EOF			
			return dwRet;
		}
		
		char buf[256];
		sprintf(buf, "read file[%s]", _strFile.c_str());
		LogVstrmError(nStatus, buf);
	}

	return dwRet;
}

bool VStrmIO::Write(char* pPtr, int nWriteLen)
{
	if (!_bOpened)
		return false;

	DWORD dwWritten = 0;
    if (!_pVstrmWriteFile(_hVstrm, _hFile, pPtr, nWriteLen, &dwWritten, 0))
	{
		char buf[256];
		sprintf(buf, "write file[%s]", _strFile.c_str());
		LogVstrmError(buf);
		return false;
	}

	return true;
}

bool VStrmIO::Seek(LONGLONG lOffset, int nPosFlag)
{
	MOLOG(Log::L_ERROR, CLOGFMT(VsIo, "Seek not implemented"));
	return false;
}

bool VStrmIO::ReserveBandwidth(int nbps)
{
	if (!_dwClientId)
	{
		//skip the bandwidth management if client id is 0
		return true;
	}

	// reserve VStrm bandwidth
	VSTATUS	statusTicket = ERROR_SUCCESS;
	VSTRM_BANDWIDTH_RESERVE_BLOCK   rbFile = {0};
    PVSTRM_BANDWIDTH_RESERVE_BLOCK	pRbFile=&rbFile;

	// The Bw Mgr considers bandwidth requests
    // to be from the perspective of the PCI Bus, not the disks. So, to get data
    // onto the disks they must READ from the PCI Bus, so ask for READ BW here,
    // even tho we are putting data onto the disks using writes. 
	rbFile.ClientId         = _dwClientId;
	rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
	rbFile.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_FILE;
    rbFile.BwTarget         = (void*)(_strFile.c_str()); 
 	rbFile.MaxBandwidth		= nbps;	// passed in with request
	rbFile.MinBandwidth		= nbps;	// passed in with request
	rbFile.ReservedBandwidth = NULL;

	MOLOG(Log::L_INFO,  CLOGFMT(VsIo, "File[%s] request to reserve bw %dbps, clientId %u"), _strFile.c_str(), nbps, _dwClientId);
    statusTicket = _pVstrmClassReserveBandwidth(_hVstrm, pRbFile, &_bwTicket);
	if (statusTicket != VSTRM_SUCCESS)
	{
		char buf[256];
		sprintf(buf, "reserve bw for file[%s]", _strFile.c_str());
		LogVstrmError(buf);

		return false;
	}

	MOLOG(Log::L_INFO,  CLOGFMT(VsIo, "File[%s] bandwidth[%d] reserved successful"), _strFile.c_str(), nbps);

  	return true;
}

void VStrmIO::ReleaseBandwidth()
{
	if (!_bwTicket)
		return;

	MOLOG(Log::L_INFO,  CLOGFMT(VsIo, "File[%s] request to release bandwidth"), _strFile.c_str());

    VSTATUS	statusTicket = _pVstrmClassReleaseBandwidth(_hVstrm, _bwTicket);
//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed **** ?
#if 0
	if (statusTicket != VSTRM_SUCCESS)
	{
		char buf[256];
		sprintf(buf, "release bw for file[%s]", _strFile.c_str());
		LogVstrmError(buf);
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(VsIo, "File[%s] bw released successful"), _strFile.c_str());
	}
#else
	MOLOG(Log::L_INFO, CLOGFMT(VsIo, "File[%s] bw released successful"), _strFile.c_str());
#endif
	
	_bwTicket = 0;		
}

void VStrmIO::setClientId(DWORD dwClientId)
{
	_dwClientId = dwClientId;
}

bool VStrmIO::ReleaseAllBandwidth(DWORD dwClientId)
{
	MOLOG(Log::L_INFO,  CLOGFMT(VsIo, "Release all allocated bandwidth for client id[%u]"), dwClientId);

	VSTATUS	statusTicket = _pVstrmClassReleaseAllBandwidth(_hVstrm, dwClientId, 0);
#if 1
	if (statusTicket != VSTRM_SUCCESS)
	{
		char buf[256];
		sprintf(buf, "VstrmClassReleaseAllBandwidth for client id[%u]", dwClientId);
		LogVstrmError(buf);
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(VsIo, "BW for client id[%u] released successful"), dwClientId);
	}
#else
	MOLOG(Log::L_INFO, CLOGFMT(VsIo, "BW for client id[%u] released successful"), dwClientId);
#endif
	
	return true;
}

void VStrmIO::Close()
{
	if (!_bOpened || !_hFile || _hFile==INVALID_HANDLE_VALUE)
		return;

	_pVstrmCloseHandle(_hVstrm, _hFile);
	_hFile = NULL;

	MOLOG(Log::L_INFO, CLOGFMT(VsIo, "File[%s] closed"), _strFile.c_str());

	ReleaseBandwidth();
	_bOpened = false;
}

int VStrmIO::GetRecommendedIOSize()
{
	return VstrmIoSize;
}

void VStrmIO::LogVstrmError(int nStatus, const char* szFunc)
{
	char szBuf[512];
	szBuf[0] = '\0';

	_pVstrmClassGetErrorText(_hVstrm, nStatus, szBuf, sizeof(szBuf));
	szBuf[511] = '\0';

	MOLOG(Log::L_ERROR, CLOGFMT(VsIo, "Failed in [%s], errorcode[0x%08x], %s"), 
		szFunc, nStatus, szBuf);	
}

void VStrmIO::LogVstrmError(const char* szFunc)
{
	int nStatus = _pVstrmGetLastError();
	LogVstrmError(nStatus, szFunc);
}

time_t FiletimeToTimet(FILETIME ft)
{
	LONGLONG ll = *((LONGLONG*)&ft);
	ll -= 116444736000000000;
	if (ll<=0)
		ll = 0;

	return (time_t)(ll/10000000);
}

int VStrmIO::getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr)
{
	bool flagslashend = false;

	if (filepath == NULL || infoptr == NULL)
		return false;

	//remove any trailing slashes if necessary
	size_t len = strlen(filepath);
	if (len <= 0)
		return false;

	if (*(filepath+len-1) == '\\' || *(filepath+len-1) == '/')
	{
		//only remove the trailing slash if it is not the only slash
		if (strchr(filepath,'\\') != (filepath+len-1) && strchr(filepath,'/') != (filepath+len-1))
		{
			*(filepath+len-1) = '\0';
			flagslashend = true;
		}
	}

	const char* f = (*filepath == '\\' || *filepath == '/') ? filepath + 1 : filepath;
	DLL_FIND_DATA_LONG data = {0};
	VHANDLE fileHandle = _pVstrmFindFirstFileEx(_hVstrm, f, &data);
	if(fileHandle == INVALID_HANDLE_VALUE) 
	{
		return false;
	}
	_pVstrmFindClose(_hVstrm, fileHandle);

	LARGE_INTEGER fileSize = {data.w.nFileSizeLow, data.w.nFileSizeHigh};

	memset(infoptr,0,sizeof(FSUtils::fileInfo_t));

	infoptr->timeaccess = FiletimeToTimet(data.w.ftLastAccessTime);
	infoptr->timecreate = FiletimeToTimet(data.w.ftCreationTime);
	infoptr->timemod = FiletimeToTimet(data.w.ftLastWriteTime);
	infoptr->size = fileSize.QuadPart;
	infoptr->groupid = 0;
	infoptr->userid = 0;
	infoptr->mode = 0 | ~S_IFDIR;
	infoptr->devnum = 0;
	infoptr->inodnum = 0;   //only useful in UNIX
	infoptr->nlinks = 1;  //only useful in UNIX (always 1 in WINDOWS)

	if (flagslashend != 0)
		FSUtils::checkSlashEnd(filepath,len+1);   //restore the trailing slash
	return true;
}

HANDLE VStrmIO::FindFirstFile(char* name, WIN32_FIND_DATAA& w)
{
	DLL_FIND_DATA_LONG data = {0};
	VHANDLE fileHandle = _pVstrmFindFirstFileEx(_hVstrm, name, &data);
	if(fileHandle == INVALID_HANDLE_VALUE) 
	{
		return INVALID_HANDLE_VALUE;
	}
	
	w = data.w;
	return fileHandle;
}

bool VStrmIO::FindNextFile(HANDLE hHandle, WIN32_FIND_DATAA& w)
{
	DLL_FIND_DATA_LONG data = {0};
	if (!_pVstrmFindNextFile(_hVstrm, hHandle, &data))
	{
		return false;
	}

	w = data.w;
	return true;
}

void VStrmIO::FindClose(HANDLE hHandle)
{
	_pVstrmFindClose(_hVstrm, hHandle);
}
