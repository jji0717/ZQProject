

#include <Log.h>
#include "VstrmProc.h"
#include "VvxParser.h"
#include "vstrmuser.h"
#include <vsiolib.h>

#if defined _UNICODE || defined UNICODE
	#undef _UNICODE
	#undef UNICODE
#endif

using namespace ZQ::common;


//extern bool 
//       LogMsg       ( DWORD         dwTraceLevel, 
//                      LPCTSTR       lpszFmt, ... );
//
//extern bool
//       LogMsg       ( LPCTSTR       lpszFmt, ... );


// Vstrm DLL entry points
//HINSTANCE hVstrmDLL = 0;

//VSTATUS (*pVstrmClassOpenEx)(PHANDLE) = 0;
//VSTATUS (*pVstrmClassCloseEx)(HANDLE) = 0;
//VSTATUS (*pVstrmGetLastError)(void) = 0;
//VSTATUS (*pVstrmClassGetErrorText)(HANDLE, VSTATUS, PCHAR, ULONG) = 0;
//VHANDLE (*pVstrmFindFirstFileEx)(HANDLE, LPCSTR, LPDLL_FIND_DATA_LONG) = 0;
//BOOLEAN (*pVstrmFindNextFileEx)(HANDLE, VHANDLE, LPDLL_FIND_DATA_LONG) = 0;
//BOOLEAN (*pVstrmFindClose)(HANDLE, VHANDLE);
//HANDLE (*pVstrmFindFirstFileNotification)(HANDLE, PVSTRM_FILE_EVENT, LPSTR, LPSTR) = 0;
//BOOLEAN (*pVstrmFindNextFileNotification)(HANDLE, HANDLE, PVSTRM_FILE_EVENT, LPSTR, LPSTR) = 0;
//BOOLEAN (*pVstrmFindCloseFileNotification)(HANDLE, HANDLE) = 0;
//VSTATUS (*pVstrmClassForEachObject)(HANDLE, ULONG, COUNT_CB, FOR_EACH_OBJECT_CB, PVOID) = 0;
//VSTATUS (*pVstrmClassGetInfo)(HANDLE, ULONG, ULONG, PVOID, ULONG, PULONG) = 0;
//VHANDLE (*pVstrmCreateFile)(HANDLE, LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, VHANDLE) = 0;
//BOOLEAN (*pVstrmCloseHandle)(HANDLE, VHANDLE) = 0;
//BOOLEAN (*pVstrmReadFile)(HANDLE, VHANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) = 0;
//VSTATUS (*pVstrmClassForEachSession)(HANDLE, COUNT_CB, FOR_EACH_SESSION_CB, PVOID) = 0;
//VSTATUS (*pVstrmClassUnloadEx)(HANDLE, ULONG) = 0;
//BOOLEAN (*pVstrmSetFilePrivateData)(HANDLE, VHANDLE, PVOID, ULONG, PVOID, ULONG) = 0;
//DWORD	(*pVstrmSetFilePointer)(HANDLE vstrmClassHandle, VHANDLE handle, LONG p2, PLONG p3, DWORD p4);


HANDLE				_hClass = INVALID_HANDLE_VALUE;

// exit flags
bool exitFlag = false;
HANDLE exitEvent = NULL;

// active thread count
LONG activeThreadCount = 0;

// Get the last Vstrm error
class GetLast
{
public:
	GetLast()
	{
		m_err = VstrmGetLastError();
	}

	char tmp[1024];
	char *Str(HANDLE classDrv)
	{
		VSTATUS err = VstrmClassGetErrorText(classDrv, m_err, tmp, 1024);
		if(VSTRM_SUCCESS != err)
		{
			sprintf(tmp, "VstrmClassGetErrorText() failed: [%#08x]", err);
		}

		return tmp;
	}
	
	VSTATUS Err(void)
	{
		return m_err;
	}

private:
	VSTATUS m_err;
};


///*****************************************************************************/
//// load Vstrm.dll entry points
///*****************************************************************************/
//bool LoadVstrmDll(void)
//{
//	bool status = false;
//
//	do {
//		hVstrmDLL = LoadLibrary(L"VstrmDLL.dll");
//		if(!hVstrmDLL)
//		{
//			glog(Log::L_ERROR, "Can't load VstrmDLL.dll");
//			break;
//		}
//
//		struct SVsEntries {
//			char *pName;
//			FARPROC *pAddr;
//		} vsEntries[] = {
//		{"VstrmClassOpenEx", (FARPROC *)&pVstrmClassOpenEx},
//		{"VstrmClassCloseEx", (FARPROC *)&pVstrmClassCloseEx},
//		{"VstrmClassGetErrorText", (FARPROC *)&pVstrmClassGetErrorText},
//		{"VstrmGetLastError", (FARPROC *)&pVstrmGetLastError},
//		{"VstrmFindFirstFileNotification", (FARPROC *)&pVstrmFindFirstFileNotification},
//		{"VstrmFindNextFileNotification", (FARPROC *)&pVstrmFindNextFileNotification},
//		{"VstrmFindCloseFileNotification", (FARPROC *)&pVstrmFindCloseFileNotification},
//		{"VstrmClassForEachObject", (FARPROC *)&pVstrmClassForEachObject},
//		{"VstrmClassGetInfo", (FARPROC *)&pVstrmClassGetInfo},
//		{"VstrmFindFirstFileEx", (FARPROC *)&pVstrmFindFirstFileEx},
//		{"VstrmFindNextFileEx", (FARPROC *)&pVstrmFindNextFileEx},
//		{"VstrmFindClose", (FARPROC *)&pVstrmFindClose},
//		{"VstrmCreateFile", (FARPROC *)&pVstrmCreateFile},
//		{"VstrmClassForEachSession", (FARPROC *)&pVstrmClassForEachSession},
//		{"VstrmClassUnloadEx", (FARPROC *)&pVstrmClassUnloadEx},
//		{"VstrmCloseHandle", (FARPROC *)&pVstrmCloseHandle},
//		{"VstrmReadFile", (FARPROC *)&pVstrmReadFile},
//		{"VstrmSetFilePrivateData", (FARPROC *)&pVstrmSetFilePrivateData},
//		{"VstrmSetFilePointer",  (FARPROC *)&pVstrmSetFilePointer},
//		{0,0} };
//
//		for(int i = 0; vsEntries[i].pAddr != 0; i++)
//		{
//			*vsEntries[i].pAddr = GetProcAddress(hVstrmDLL, vsEntries[i].pName);
//
//			if(0 == *vsEntries[i].pAddr)
//			{
//				glog(Log::L_ERROR, "Can't load VstrmDLL routine %s", vsEntries[i].pName);
//				break;
//			}
//		}
//
//		status = true;
//
//	} while(0);
//
//	return status;
//}

bool vsm_Initialize(HANDLE hVstrmClass )
{

	_hClass=hVstrmClass;
	return true;
}
//Do not need this function due to use vstreamclass handle
void vsm_Uninitialize()
{
	if (_hClass != INVALID_HANDLE_VALUE)
	{
		VstrmClassCloseEx(_hClass);
		_hClass = INVALID_HANDLE_VALUE;
	}
}

HRESULT vsm_GetFileSize(const char* szFileName, LONGLONG& llFileSize)
{
	DLL_FIND_DATA_LONG findData = {0};
	
//	wchar_t szBuf[256];
//	ZeroMemory(szBuf,sizeof(szBuf));
//	MultiByteToWideChar(CP_ACP,0,szFileName,strlen(szFileName),szBuf,sizeof(szBuf)/sizeof(wchar_t)-1);
	
	VHANDLE fileHandle =VstrmFindFirstFileEx(_hClass, szFileName, &findData);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
		llFileSize = fileSize.QuadPart;
		
		if(FALSE == VstrmFindClose(_hClass, fileHandle))
		{
			GetLast last;
			glog(Log::L_ERROR, "(%s) VstrmFindClose() failed: %s", szFileName, last.Str(_hClass));
		}

		return 0;
	}
	else
	{
		GetLast last;
		glog(Log::L_DEBUG, "(%s) VstrmFindFirstFileEx failed with code %08x, info: %s", szFileName, last.Err(), last.Str(_hClass));

		if(last.Err() == ERROR_FILE_NOT_FOUND ||
			last.Err() == VSTRM_NO_SUCH_FILE || last.Err() == ERROR_NO_MORE_FILES)
		{
			// if the file's not present, don't need retry
			glog(Log::L_ERROR, "(%s) file not found", szFileName);	
			return VSM_FILE_NOTFOUND;
		}
		else if (last.Err() == VSTRM_NETWORK_NOT_READY)
		{
			return VSM_VSTRM_NOTREAD;
		}
		
		return VSM_OTHERERROR_NOTRETRY;
	}	
}

bool vsm_ParseVvxFile(const char* szFileName, VvxParser& parser)
{
	VHANDLE fileHandle = INVALID_HANDLE_VALUE;
	OBJECT_ID tempId ;

	if( 1)
	{
		VSTATUS ret = VsOpenEx( &fileHandle , (char*)szFileName ,
										GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
										OPEN_EXISTING,
										FILE_FLAG_CACHED,
										0,
										&tempId);
		if( !IS_VSTRM_SUCCESS(ret) )
		{
			GetLast last;
			glog(Log::L_INFO, "(%s) VstrmCreateFile failed: (%s)", szFileName, last.Str(_hClass));

			return false;
		}		
	}
	else
	{
		fileHandle = VstrmCreateFile(_hClass,
			szFileName, 
			GENERIC_READ, 
			FILE_SHARE_READ,
			0, 
			OPEN_EXISTING, 
			0, 
			0);

		if(INVALID_HANDLE_VALUE == fileHandle){
			GetLast last;
			glog(Log::L_INFO, "(%s) VstrmCreateFile failed: (%s)", szFileName, last.Str(_hClass));

			return false;
		}
	}
	
	VVX_INDEX_HEADER header = {0};

	DWORD bytesRead = 0;	
	
	DWORD len = sizeof(header);
	bool bResult = false;

	//get file size
	DLL_FIND_DATA	findData;
	// 	VSTRM_FIND_FILE_FLAGS	findFlag;
	// 
	// 	findFlag._rsvd					= 0;
	// 	findFlag.ReturnHiddenFiles		= false;

	//	VHANDLE findHandle = VstrmFindFirstFileEx3( vstrmHandle , file.c_str()  , findFlag , &findData );
	VHANDLE findHandle = VstrmFindFirstFile( _hClass , szFileName  , &findData );
	if( findHandle != INVALID_HANDLE_VALUE )
	{
		if ( findData.VstrmFileVersion.StructureLength > FIELD_OFFSET( VSTRM_FILE_VERSION, Modulus ) )
		{			
			{
				int64 fileSize = (((int64)findData.w.nFileSizeHigh << 32) | findData.w.nFileSizeLow );
				VstrmFindClose( _hClass , findHandle );
				if( fileSize < len )
					return false;
			}
		}
		VstrmFindClose( _hClass , findHandle );
	}
	else
	{
		return false;
	}
	
//	PVVX_TS_INFORMATION			tsInformation = NULL;
//	PVVX_ES_INFORMATION			esInformation = NULL;
	PVVX_V6_SUBFILE_INFORMATION	subFileInformation = NULL;		

	do{	
		if(1)
		{
			if(FALSE == VsRead(  fileHandle, (char*) &header, len, &bytesRead, 0))
			{
				GetLast last;
				glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

				break;
			}
		}
		else
		{
			if(FALSE == VstrmReadFile(_hClass, fileHandle, &header, len, &bytesRead, 0))
			{
				GetLast last;
				glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

				break;
			}
		}
		
		if(bytesRead != len)
		{
			glog(Log::L_ERROR, "(%s) bytes read(%d) != read len requested(%d)",
				szFileName, bytesRead, len);
			
			break;
		}
		
		if(strcmp((char *)header.signature, VVX_INDEX_SIGNATURE))
		{
			glog(Log::L_ERROR, "(%s) invalid vvx header signature", szFileName);
			
			break;
		}
		
		if(header.majorVersion == 5) 
		{
			glog(Log::L_INFO, "(%s) vvx header version 5", szFileName);

			parser._dwBitrate = header.mpegBitRate;
			parser._dwHorizontalSize = header.horizontalSize;
			parser._dwVerticalSize = header.verticalSize;
			parser._dwVideoBitrate = header.videoBitRate;
			parser._dwFrameRateCode = header.frameRateCode;
			parser._nSubFileCount = header.subFileInformationCount;

#pragma message(__MSGLOC__"NOTE: bad logic")
// 			PVVX_SUBFILE_INFORMATION subFileInformation = 0;
// 			subFileInformation = (PVVX_SUBFILE_INFORMATION)BYTE_OFFSET_POINTER(&header, header.subFileInformationOffset);
// 
// 			for (size_t i = 0; i < header.subFileInformationCount; i++)
// 			{
// 				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
// 				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
// 			}
// 			
// 			bResult = true;
			
			break;
		}

		else if(header.majorVersion == 7) 
		{
			glog(Log::L_INFO, "(%s) vvx header version 7", szFileName);

			PVVX_V7_INDEX_HEADER hdr = (PVVX_V7_INDEX_HEADER)&header;

			*(UQUADWORD*)&parser._fileTime = hdr->systemTime;
			parser._dwBitrate = hdr->streamBitRate;
			parser._dwHorizontalSize = hdr->horizontalSize;
			parser._dwVerticalSize = hdr->verticalSize;
			parser._dwVideoBitrate = hdr->videoBitRate;
//			parser._dwStreamType = hdr->streamType;
			parser._dwFrameRateCode = hdr->frameRateCode;
			parser._nSubFileCount = hdr->subFileInformationCount;
//			parser._tsInformationCount = hdr->tsInformationCount;
//			parser._esInformationCount = hdr->esInformationCount;

			if(1)
			{
				LARGE_INTEGER seekPos;
				seekPos.QuadPart = hdr->tsInformationOffset;
				if (!VsSeek(fileHandle, tempId , &seekPos, 0 ) )
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
			else
			{
				if (!VstrmSetFilePointer(_hClass, fileHandle, hdr->tsInformationOffset, 0, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
				
//			tsInformation = new VVX_TS_INFORMATION[hdr->tsInformationCount];
//			len = sizeof(VVX_TS_INFORMATION) * hdr->tsInformationCount;
//			if(FALSE == VstrmReadFile(_hClass, fileHandle, tsInformation, len, &bytesRead, 0))
//			{
//				GetLast last;
//				glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));
//				
//				break;
//			}
//			
//			if(bytesRead != len)
//			{
//				glog(Log::L_ERROR, "(%s) bytes read(%d) != read len requested(%d)",
//					szFileName, bytesRead, len);
//				
//				break;
//			}
//			
//			for (ULONG i = 0; i < hdr->tsInformationCount; i++)
//			{
//				parser._tsProgramNumber[i] = tsInformation[i].programNumber;
//				parser._tsPMTPid[i] = tsInformation[i].pmtPid;
//				parser._tsPCRPid[i] = tsInformation[i].pcrPid;
//				parser._tsVideoPid[i] = tsInformation[i].videoPid;
//			}
			
//			if (!VstrmSetFilePointer(_hClass, fileHandle, hdr->esInformationOffset, 0, 0))
//			{
//				GetLast last;
//				glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));
//
//				break;			
//			}
//				
//			esInformation = new VVX_ES_INFORMATION[hdr->esInformationCount];
//			len = sizeof(VVX_ES_INFORMATION) * hdr->esInformationCount;
//			if(FALSE == VstrmReadFile(_hClass, fileHandle, esInformation, len, &bytesRead, 0))
//			{
//				GetLast last;
//				glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));
//				
//				break;
//			}
//			
//			if(bytesRead != len)
//			{
//				glog(Log::L_ERROR, "(%s) bytes read(%d) != read len requested(%d)",
//					szFileName, bytesRead, len);
//				
//				break;
//			}
//			
//			for (i = 0; i < hdr->esInformationCount; i++)
//			{
//				parser._esStreamType[i] = esInformation[i].streamType;
//				parser._esStreamFlags[i] = esInformation[i].streamFlags;
//				parser._esPID[i] = esInformation[i].pid;		
//			}

			if(1)
			{
				LARGE_INTEGER seekPos;
				seekPos.QuadPart = hdr->subFileInformationOffset;
				if (!VsSeek(fileHandle, tempId , &seekPos, 0 ) )
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
			else
			{
				if (!VstrmSetFilePointer(_hClass, fileHandle, hdr->subFileInformationOffset, 0, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
				
			subFileInformation = new VVX_V6_SUBFILE_INFORMATION[hdr->subFileInformationCount];
			len = sizeof(VVX_V6_SUBFILE_INFORMATION) * hdr->subFileInformationCount;

			if(1)
			{
				if(FALSE ==VsRead( fileHandle, (char*)subFileInformation, len, &bytesRead, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

					break;
				}
			}
			else
			{
				if(FALSE ==VstrmReadFile(_hClass, fileHandle, subFileInformation, len, &bytesRead, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

					break;
				}
			}
			
			if(bytesRead != len)
			{
				glog(Log::L_DEBUG, "(%s) bytes read(%d) != read len requested(%d)",
					szFileName, bytesRead, len);
				
				break;
			}
			
			for (ULONG i = 0; i < hdr->subFileInformationCount; i++)
			{
				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
//				parser._subFileType[i] = subFileInformation[i].fileType;
//				parser._subFileSpeed_denominator[i] = subFileInformation[i].speed.denominator;
//				parser._subFileSpeed_numerator[i] = subFileInformation[i].speed.numerator;
				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
			}	
		}
		else if(header.majorVersion == 6) {
			glog(Log::L_INFO, "(%s) vvx header version 6", szFileName);

			PVVX_V6_INDEX_HEADER hdr = (PVVX_V6_INDEX_HEADER)&header;

			*(UQUADWORD*)&parser._fileTime = hdr->systemTime;
			parser._dwBitrate = hdr->streamBitRate;
			parser._dwHorizontalSize = hdr->horizontalSize;
			parser._dwVerticalSize = hdr->verticalSize;
			parser._dwVideoBitrate = hdr->videoBitRate;
//			parser._dwStreamType = hdr->streamType;
			parser._dwFrameRateCode = hdr->frameRateCode;
			parser._nSubFileCount = hdr->subFileInformationCount;
//			parser._tsInformationCount = hdr->tsInformationCount;
//			parser._esInformationCount = hdr->esInformationCount;


			if(1)
			{
				LARGE_INTEGER seekPos;
				seekPos.QuadPart = hdr->subFileInformationOffset;
				if (!VsSeek(fileHandle, tempId , &seekPos, 0 ) )
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
			else
			{
				if (!VstrmSetFilePointer(_hClass, fileHandle, hdr->subFileInformationOffset, 0, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmSetFilePointer failed %s", szFileName, last.Str(_hClass));

					break;			
				}
			}
				
			subFileInformation = new VVX_V6_SUBFILE_INFORMATION[hdr->subFileInformationOffset];
			len = sizeof(VVX_V6_SUBFILE_INFORMATION) * hdr->subFileInformationCount;
			if(1)
			{
				if(FALSE ==VsRead( fileHandle, (char*)subFileInformation, len, &bytesRead, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

					break;
				}
			}
			else
			{
				if(FALSE ==VstrmReadFile(_hClass, fileHandle, subFileInformation, len, &bytesRead, 0))
				{
					GetLast last;
					glog(Log::L_ERROR, "(%s) VstrmReadFile failed %s", szFileName, last.Str(_hClass));

					break;
				}
			}
			
			if(bytesRead != len)
			{
				glog(Log::L_DEBUG, "(%s) bytes read(%d) != read len requested(%d)",
					szFileName, bytesRead, len);
				
				break;
			}
			
			for (ULONG i = 0; i < hdr->subFileInformationCount; i++)
			{
				strcpy(parser._subFileExtension[i], (const char*)(subFileInformation[i].fileExtension));
//				parser._subFileType[i] = subFileInformation[i].fileType;
//				parser._subFileSpeed_denominator[i] = subFileInformation[i].speed.denominator;
//				parser._subFileSpeed_numerator[i] = subFileInformation[i].speed.numerator;
				parser._subFilePlayTimeInMilliSeconds[i] = subFileInformation[i].playTimeInMilliSeconds;
			}	
		}
		else {
			glog(Log::L_ERROR, "(%s) invalid vvx header major_version (%d)",
								szFileName, header.majorVersion);

			break;
		}


	

		bResult = true;
	}while(0);

//	if (tsInformation)
//		delete tsInformation;
//	if (esInformation)
//		delete esInformation;
	if (subFileInformation)
		delete subFileInformation;
	
	if(1)
	{
		VsClose(fileHandle,tempId);
	}
	else
	{
		if(FALSE == VstrmCloseHandle(_hClass, fileHandle))
		{
			GetLast last;
			glog(Log::L_ERROR, "(%s) VstrmCloseHandle failed %s", szFileName, last.Str(_hClass));
		}
	}

	return bResult;
}
