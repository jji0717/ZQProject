// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: DokanFuse.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/DokanFuse.cpp $
// 
// 29    12/12/13 1:29p Hui.shao
// %lld
// 
// 28    7/19/13 6:31p Hui.shao
// read the os and processor info
// 
// 27    7/19/13 3:55p Hui.shao
// 
// 26    7/19/13 3:06p Hui.shao
// impl install/remove dokan driver
// 
// 25    5/22/13 11:17a Li.huang
// 
// 24    4/19/13 2:22p Li.huang
// 
// 23    4/19/13 11:30a Hui.shao
// 
// 22    4/18/13 6:31p Hui.shao
// 
// 21    4/18/13 5:42p Hui.shao
// support multiple mounting
// 
// 20    4/07/13 11:33a Hui.shao
// to trace the error code for each fuse entry
// 
// 19    4/03/13 11:19a Hui.shao
// support chinese chars in names
// 
// 18    3/01/13 10:46a Hui.shao
// 
// 17    2/25/13 8:33p Hui.shao
// skip guess if getFileInfo() is certain whether the input path is a dir
// or file
// 
// 16    2/22/13 4:12p Hui.shao
// 
// 15    1/24/13 4:00p Hui.shao
// supported files in subdir
// 
// 14    1/23/13 11:29a Hui.shao
// covered dos commands: dir, mkdir, copy, freespace
// 
// 13    1/18/13 2:32p Hui.shao
// 
// 12    1/09/13 5:20p Li.huang
// 
// 11    1/09/13 3:03p Hui.shao
// 
// 10    1/07/13 8:36p Hui.shao
// findfiles and getfileinfo
// 
// 9     1/07/13 12:03p Hui.shao
// 
// 8     1/06/13 4:22p Hui.shao
// drafted some entries for CdmiDokan
// 
// 7     12/28/12 11:40a Hui.shao
// disk space
// 
// 6     12/27/12 5:06p Hui.shao
// splitted
// 
// 5     12/27/12 4:04p Hui.shao
// 
// 4     12/26/12 8:06p Hui.shao
// smoketest
// 
// 3     12/26/12 6:18p Hui.shao
// log and DokanNest
// 
// 2     12/26/12 12:24p Hui.shao
// 
// 1     12/24/12 5:10p Hui.shao
// created
// ===========================================================================

#include "DokanFuse.h"

// #define ENTRY_TRACE
#define DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL

// #if (sizeof(void*) > sizeof(ULONG64))
// #  error ULONG64 failed to hold pointer, adjust the logic about dipatching by DokanOptions->GlobalContext
// #endif

#ifdef ENTRY_TRACE
#define RETURN_ENTRY_ERRCODE(_ERR, _ENTRYNAME) fuse->_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanNest, #_ENTRYNAME "() return(%d) to entry"), ret); return ret
#else
#define RETURN_ENTRY_ERRCODE(_ERR, _ENTRYNAME) return ret
#endif // ENTRY_TRACE


// -----------------------------
// class DokanNest
// -----------------------------
class DokanNest
{
public:
	DokanNest(DokanFuse& owner)
		: _owner(owner)
	{
		memset(&_owner._dokanOperations, 0x00, sizeof(_owner._dokanOperations));
		_owner._dokanOperations.CreateFile           = _cbCreateFile;
		_owner._dokanOperations.OpenDirectory        = _cbOpenDirectory;
		_owner._dokanOperations.CreateDirectory      = _cbCreateDirectory;
		_owner._dokanOperations.Cleanup              = _cbCleanup;
		_owner._dokanOperations.CloseFile            = _cbCloseFile;
		_owner._dokanOperations.ReadFile             = _cbReadFile;
		_owner._dokanOperations.WriteFile            = _cbWriteFile;
		_owner._dokanOperations.FlushFileBuffers     = _cbFlushFileBuffers;
		_owner._dokanOperations.GetFileInformation   = _cbGetFileInformation;
		_owner._dokanOperations.FindFiles            = _cbFindFiles;
		_owner._dokanOperations.FindFilesWithPattern = NULL;
		_owner._dokanOperations.SetFileAttributes    = _cbSetFileAttributes;
		_owner._dokanOperations.SetFileTime          = _cbSetFileTime;
		_owner._dokanOperations.DeleteFile           = _cbDeleteFile;
		_owner._dokanOperations.DeleteDirectory      = _cbDeleteDirectory;
		_owner._dokanOperations.MoveFile             = _cbMoveFile;
		_owner._dokanOperations.SetEndOfFile         = _cbSetEndOfFile;
		_owner._dokanOperations.SetAllocationSize    = _cbSetAllocationSize;	
		_owner._dokanOperations.LockFile             = _cbLockFile;
		_owner._dokanOperations.UnlockFile           = _cbUnlockFile;
		_owner._dokanOperations.GetFileSecurity      = _cbGetFileSecurity;
		_owner._dokanOperations.SetFileSecurity      = _cbSetFileSecurity;
		_owner._dokanOperations.GetDiskFreeSpace     = _cbGetDiskFreeSpace;
		_owner._dokanOperations.GetVolumeInformation = _cbGetVolumeInformation;
		_owner._dokanOperations.Unmount              = _cbUnmount;
	}

	DokanFuse& _owner;

#define DOKAN_INVALID_CONTEXT	(-0xEF00)
#define DOKAN_EXPC_OCCURED      (-0xEF01)

	// the static operation entries of dokan
	// ----------------------------------
	static int DOKAN_CALLBACK _cbCreateFile(
		LPCWSTR					FileName,
		DWORD					AccessMode,
		DWORD					ShareMode,
		DWORD					CreationDisposition,
		DWORD					FlagsAndAttributes,
		PDOKAN_FILE_INFO		DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (0 == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doCreateFile(DokanFileInfo, FileName, AccessMode,
				ShareMode, CreationDisposition, FlagsAndAttributes);
			RETURN_ENTRY_ERRCODE(ret, CreateFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbCloseFile(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (0 == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doCloseFile(DokanFileInfo, FileName);
			RETURN_ENTRY_ERRCODE(ret, CloseFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbCreateDirectory(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doCreateDirectory(DokanFileInfo, FileName);

			RETURN_ENTRY_ERRCODE(ret, CreateDirectory);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbOpenDirectory(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doOpenDirectory(DokanFileInfo, FileName);

			RETURN_ENTRY_ERRCODE(ret, OpenDirectory);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbDeleteDirectory(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doDeleteDirectory(DokanFileInfo, FileName);
			RETURN_ENTRY_ERRCODE(ret, DeleteDirectory);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbCleanup(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doCleanup(DokanFileInfo, FileName);
			RETURN_ENTRY_ERRCODE(ret, Cleanup);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbReadFile(LPCWSTR				FileName,
		LPVOID				Buffer,
		DWORD				BufferLength,
		LPDWORD				ReadLength,
		LONGLONG			Offset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doReadFile(DokanFileInfo, FileName,
				Buffer, BufferLength, ReadLength, Offset);
			RETURN_ENTRY_ERRCODE(ret, ReadFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}


	static int DOKAN_CALLBACK _cbWriteFile(
		LPCWSTR		FileName,
		LPCVOID		Buffer,
		DWORD		NumberOfBytesToWrite,
		LPDWORD		NumberOfBytesWritten,
		LONGLONG			Offset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doWriteFile(DokanFileInfo, FileName,
				Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Offset);
			RETURN_ENTRY_ERRCODE(ret, WriteFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbFlushFileBuffers(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doFlushFileBuffers(DokanFileInfo, FileName);
			RETURN_ENTRY_ERRCODE(ret, FlushFileBuffers);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}


	static int DOKAN_CALLBACK _cbGetFileInformation(
		LPCWSTR							FileName,
		LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
		PDOKAN_FILE_INFO				DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doGetFileInfo(DokanFileInfo, FileName, HandleFileInformation);
			RETURN_ENTRY_ERRCODE(ret, GetFileInfo);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbFindFiles(
		LPCWSTR				FileName,
		PFillFindData		FillFindData, // function pointer
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doFindFiles(DokanFileInfo, FileName, FillFindData);
			RETURN_ENTRY_ERRCODE(ret, FindFiles);
		}
		//		catch(const std::runtime_error& ex)
		//		{ _log() }
		catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbDeleteFile(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doDeleteFile(DokanFileInfo, FileName);
			RETURN_ENTRY_ERRCODE(ret, DeleteFile);
		} catch (...) {}
		return -2;
	}

	static int DOKAN_CALLBACK _cbMoveFile(
		LPCWSTR				FileName, // existing file name
		LPCWSTR				NewFileName,
		BOOL				ReplaceIfExisting,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doMoveFile(DokanFileInfo, FileName, // existing file name
				NewFileName, ReplaceIfExisting);
			RETURN_ENTRY_ERRCODE(ret, MoveFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbLockFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doLockFile(DokanFileInfo, FileName, ByteOffset, Length);
			RETURN_ENTRY_ERRCODE(ret, LockFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbUnlockFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doUnlockFile(DokanFileInfo, FileName, ByteOffset, Length);
			RETURN_ENTRY_ERRCODE(ret, UnlockFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbSetEndOfFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doSetEndOfFile(DokanFileInfo, FileName, ByteOffset);
			RETURN_ENTRY_ERRCODE(ret, SetEndOfFile);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbSetAllocationSize(
		LPCWSTR				FileName,
		LONGLONG			AllocSize,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doSetAllocationSize(DokanFileInfo, FileName, AllocSize);
			RETURN_ENTRY_ERRCODE(ret, SetAllocationSize);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}


	static int DOKAN_CALLBACK _cbSetFileAttributes(
		LPCWSTR				FileName,
		DWORD				FileAttributes,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doSetFileAttrs(DokanFileInfo, FileName, FileAttributes);
			RETURN_ENTRY_ERRCODE(ret, SetFileAttrs);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbSetFileTime(
		LPCWSTR				FileName,
		CONST FILETIME*		CreationTime,
		CONST FILETIME*		LastAccessTime,
		CONST FILETIME*		LastWriteTime,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doSetFileTime(DokanFileInfo, FileName,
				CreationTime, LastAccessTime, LastWriteTime);
			RETURN_ENTRY_ERRCODE(ret, SetFileTime);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}


	static int DOKAN_CALLBACK _cbGetFileSecurity(
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				BufferLength,
		PULONG				LengthNeeded,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doGetFileSecurity(DokanFileInfo, FileName,
				SecurityInformation, SecurityDescriptor, BufferLength, LengthNeeded);
			RETURN_ENTRY_ERRCODE(ret, GetFileSecurity);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbSetFileSecurity(
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				SecurityDescriptorLength,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doSetFileSecurity(DokanFileInfo, FileName,
				SecurityInformation, SecurityDescriptor, SecurityDescriptorLength);
			RETURN_ENTRY_ERRCODE(ret, SetFileSecurity);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbGetDiskFreeSpace(
		PULONGLONG FreeBytesAvailable,
		PULONGLONG TotalNumberOfBytes,
		PULONGLONG TotalNumberOfFreeBytes,
		PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doGetDiskFreeSpace(DokanFileInfo, FreeBytesAvailable, TotalNumberOfBytes, TotalNumberOfFreeBytes);
			RETURN_ENTRY_ERRCODE(ret, GetDiskFreeSpace);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbGetVolumeInformation(
		LPWSTR		VolumeNameBuffer,
		DWORD		VolumeNameSize,
		LPDWORD		VolumeSerialNumber,
		LPDWORD		MaximumComponentLength,
		LPDWORD		FileSystemFlags,
		LPWSTR		FileSystemNameBuffer,
		DWORD		FileSystemNameSize,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doGetVolumeInfo(DokanFileInfo,
				VolumeNameBuffer, VolumeNameSize, FileSystemNameBuffer, FileSystemNameSize,
				VolumeSerialNumber, MaximumComponentLength, FileSystemFlags);
			RETURN_ENTRY_ERRCODE(ret, GetVolumeInfo);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

	static int DOKAN_CALLBACK _cbUnmount(PDOKAN_FILE_INFO DokanFileInfo)
	{
		try {
			DokanFuse* fuse =(DokanFuse*) DokanFileInfo->DokanOptions->GlobalContext;
			if (NULL == fuse)
				return DOKAN_INVALID_CONTEXT;

			int ret = fuse->doUnmount(DokanFileInfo);
			RETURN_ENTRY_ERRCODE(ret, Unmount);
		} catch (...) {}
		return DOKAN_EXPC_OCCURED;
	}

};

static bool utf8_to_unicode(const std::string& utf8, std::wstring& unicode)
{
	if (utf8.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<wchar_t> buf(new wchar_t[nSize]);	
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buf.get(), nSize);

	unicode = buf.get();

	return true;
#else

	std::string dest;
	if(convert("utf-8", "utf-16le", utf8, dest))
	{
		str2wstr(dest, unicode);	
		return true;
	}

	return false;

#endif
}

static bool unicode_to_utf8(const std::wstring& unicode, std::string& utf8)
{
	if (unicode.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, 0, 0, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<char> buf(new char[nSize]);	
	WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, buf.get(), nSize, 0, 0);

	utf8 = buf.get();
	return true;
#else

	std::string src;
	if(!wstr2str(unicode, src)) {
		return false;
	}
	if(!convert("utf-16le", "utf-8", src, utf8)) {
		return false;
	}

	return false;

#endif

}

// -----------------------------
// class DokanFuse
// -----------------------------
/*
std::wstring DokanFuse::mbs2wcs(const std::string& mbs, const char* locale)
{
	std::wstring ret(mbs.length(), '\0');
	if (NULL != locale)
		setlocale(LC_ALL, locale);
	::mbstowcs(const_cast<wchar_t*>(ret.c_str()), mbs.c_str(), mbs.length());
	return ret;
}

std::string DokanFuse::wcs2mbs(const std::wstring& wcs, const char* locale)
{
	size_t len = wcs.length()*2;
	std::string ret(len, '\0');
	if (NULL != locale)
		setlocale(LC_ALL, locale);
	::wcstombs(const_cast<char*>(ret.c_str()), wcs.c_str(), ret.length());
	for (; len >0 && '\0' == ret[len-1]; len--);
	return ret.substr(0, len);;
}
*/
// /*
std::wstring DokanFuse::mbs2wcs(const std::string& mbs, const char* locale)
{
	std::wstring ret;
	if (!utf8_to_unicode(mbs, ret))
		return L"";

	return ret;
}

std::string DokanFuse::wcs2mbs(const std::wstring& wcs, const char* locale)
{
	std::string ret;
	if (!unicode_to_utf8(wcs, ret))
		return "";

	return ret;
}
// */
DokanFuse::DokanFuse(ZQ::common::Log& log, const std::string& mountPoint, const std::string& rootPath, uint32 options, const std::string& locale, uint threadCount)
: _log(log), _nest(NULL)
{
	memset(&_dokanOptions, 0x00, sizeof(_dokanOptions));
	_dokanOptions.Version     = DOKAN_VERSION;
	_dokanOptions.GlobalContext = (uint64) this;
	_dokanOptions.ThreadCount = threadCount; // use default
	_dokanOptions.Options     = options;

	_wRootPath = (rootPath.empty()) ? L"C:\\" : mbs2wcs(rootPath);
	if (!rootPath.empty() && FNSEPC != rootPath[rootPath.length()-1])
		_wRootPath += L"\\";

	_wMountPoint = (mountPoint.empty()) ? L"M:" : mbs2wcs(mountPoint);
	_dokanOptions.MountPoint  = _wMountPoint.c_str();

	_nest = new DokanNest(*this);
	_desc = std::string("FUSE[") + wcs2mbs(_wMountPoint) + "]=>URL[" + wcs2mbs(_wRootPath) +"]";

	setlocale(LC_ALL, _locale.c_str());
	_volSN = 0x20121213;
	_waitStop=CreateEvent(NULL, FALSE,FALSE,NULL);
}

DokanFuse::~DokanFuse()
{
	if (NULL != _nest)
		delete _nest;
	_nest = NULL;
	if (NULL != _waitStop)
	{
		CloseHandle(_waitStop);
	}
}

uint DokanFuse::_cRunning =0;

std::string DokanFuse::toFilePath(std::wstring& wFilePath, LPCWSTR fileName)
{
	wFilePath = (fileName ? fileName: L"");
	if (!wFilePath.empty() && FNSEPC == wFilePath[0])
		wFilePath = wFilePath.substr(1);

	wFilePath = _wRootPath + wFilePath;
	return wcs2mbs(wFilePath);
}

void DokanFuse::getOpUserName(PDOKAN_FILE_INFO DokanFileInfo, std::string& user, std::string& domain)
{
	HANDLE	handle;
	uint8 buffer[1024];
	DWORD returnLength;
	WCHAR accountName[256];
	WCHAR domainName[256];
	DWORD accountLength = sizeof(accountName) / sizeof(WCHAR);
	DWORD domainLength = sizeof(domainName) / sizeof(WCHAR);

	user = domain ="";

	PTOKEN_USER tokenUser;
	SID_NAME_USE snu;

	handle = DokanOpenRequestorToken(DokanFileInfo);
	if (handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "DokanOpenRequestorToken failed"));
		return;
	}

	if (!::GetTokenInformation(handle, TokenUser, buffer, sizeof(buffer), &returnLength))
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "GetTokenInformaiton failed: %d"), GetLastError());
		CloseHandle(handle);
		return;
	}

	::CloseHandle(handle);

	tokenUser = (PTOKEN_USER)buffer;
	if (!::LookupAccountSidW(NULL, tokenUser->User.Sid, accountName, &accountLength, domainName, &domainLength, &snu))
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "LookupAccountSid failed: %d"), GetLastError());
		return;
	}

	domain = wcs2mbs(std::wstring(domainName));
	user   = wcs2mbs(std::wstring(accountName));
}

#define BuildFlagStr(val, str, flag) if (val&flag) { str += "|"#flag; }
#define FOPFMT(_OP, _X)  CLOGFMT(DokanFuse, #_OP "(%s) " _X), filepath.c_str()

int DokanFuse::doCreateFile(PDOKAN_FILE_INFO		DokanFileInfo,
							LPCWSTR					FileName,
							DWORD					AccessMode,
							DWORD					ShareMode,
							DWORD					CreationDisposition,
							DWORD					FlagsAndAttributes)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	std::string user, domain;

	HANDLE handle;
	DWORD fileAttr;

	std::string createDispStr;
	switch(CreationDisposition)
	{
	case CREATE_NEW:        createDispStr = "CREATE_NEW";        break;
	case OPEN_ALWAYS:       createDispStr = "OPEN_ALWAYS";       break;
	case CREATE_ALWAYS:     createDispStr = "CREATE_ALWAYS";     break;
	case OPEN_EXISTING:     createDispStr = "OPEN_EXISTING";     break;
	case TRUNCATE_EXISTING: createDispStr = "TRUNCATE_EXISTING"; break;
	}

	getOpUserName(DokanFileInfo, user, domain);

	std::string flgstrShareMode, flgstrAccessMode, flgstrFlgAndAttr;
	BuildFlagStr(ShareMode, flgstrShareMode, FILE_SHARE_READ);
	BuildFlagStr(ShareMode, flgstrShareMode, FILE_SHARE_WRITE);
	BuildFlagStr(ShareMode, flgstrShareMode, FILE_SHARE_DELETE);

	BuildFlagStr(AccessMode, flgstrAccessMode, GENERIC_READ);
	BuildFlagStr(AccessMode, flgstrAccessMode, GENERIC_WRITE);
	BuildFlagStr(AccessMode, flgstrAccessMode, GENERIC_EXECUTE);
	BuildFlagStr(AccessMode, flgstrAccessMode, DELETE);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_READ_DATA);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_READ_ATTRIBUTES);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_READ_EA);
	BuildFlagStr(AccessMode, flgstrAccessMode, READ_CONTROL);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_WRITE_DATA);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_WRITE_ATTRIBUTES);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_WRITE_EA);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_APPEND_DATA);
	BuildFlagStr(AccessMode, flgstrAccessMode, WRITE_DAC);
	BuildFlagStr(AccessMode, flgstrAccessMode, WRITE_OWNER);
	BuildFlagStr(AccessMode, flgstrAccessMode, SYNCHRONIZE);
	BuildFlagStr(AccessMode, flgstrAccessMode, FILE_EXECUTE);
	BuildFlagStr(AccessMode, flgstrAccessMode, STANDARD_RIGHTS_READ);
	BuildFlagStr(AccessMode, flgstrAccessMode, STANDARD_RIGHTS_WRITE);
	BuildFlagStr(AccessMode, flgstrAccessMode, STANDARD_RIGHTS_EXECUTE);

	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_ARCHIVE);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_ENCRYPTED);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_HIDDEN);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_NORMAL);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_OFFLINE);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_READONLY);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_SYSTEM);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_ATTRIBUTE_TEMPORARY);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_WRITE_THROUGH);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_OVERLAPPED);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_NO_BUFFERING);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_RANDOM_ACCESS);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_SEQUENTIAL_SCAN);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_DELETE_ON_CLOSE);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_BACKUP_SEMANTICS);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_POSIX_SEMANTICS);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_OPEN_REPARSE_POINT);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, FILE_FLAG_OPEN_NO_RECALL);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_ANONYMOUS);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_IDENTIFICATION);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_IMPERSONATION);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_DELEGATION);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_CONTEXT_TRACKING);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_EFFECTIVE_ONLY);
	BuildFlagStr(FlagsAndAttributes, flgstrFlgAndAttr, SECURITY_SQOS_PRESENT);

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "by %s@%s, accessMode[(0x%x)%s] shareMode[(0x%x)%s], %s, attr[(0x%x)%s]"),
		user.c_str(), domain.c_str(), AccessMode, flgstrAccessMode.c_str(), ShareMode, flgstrShareMode.c_str(), createDispStr.c_str(), FlagsAndAttributes, flgstrFlgAndAttr.c_str());

	// When filePath is a directory, needs to change the flag so that the file can be opened.
	fileAttr = ::GetFileAttributesW(wfilePath.c_str());
	if (fileAttr && fileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
		//AccessMode = 0;
	}

	handle = ::CreateFileW(wfilePath.c_str(),
		AccessMode, //GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE,
		ShareMode,
		NULL, // security attribute
		CreationDisposition,
		FlagsAndAttributes,// |FILE_FLAG_NO_BUFFERING,
		NULL); // template file handle

	if (handle == INVALID_HANDLE_VALUE) 
	{
		DWORD error = ::GetLastError();
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "CreateFileW() error(%d)"), error);
		return error * -1; // error codes are negated value of Windows System Error codes
	}

	// save the file handle in Context
	DokanFileInfo->Context = (ULONG64)handle;

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doCreateDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(CreateDirectory, ""));
	if (!::CreateDirectoryW(wfilePath.c_str(), NULL))
	{
		DWORD error = ::GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(CreateDirectory, "error(%d)"), error);
		return error * -1; // error codes are negated value of Windows System Error codes
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doOpenDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE handle;
	DWORD attr;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(OpenDirectory, ""));

	attr = ::GetFileAttributesW(wfilePath.c_str());
	if (attr == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = ::GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(OpenDirectory, "error(%d)"), error);
		return error * -1;
	}

	if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
		return -1;

	handle = ::CreateFileW(wfilePath.c_str(),
		0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(OpenDirectory, "error(%d)"), error);
		return error * -1;
	}

	DokanFileInfo->Context = (ULONG64)handle;

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doCloseFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	if (!DokanFileInfo->Context) 
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(CloseFile, ""));
		return 0;
	}

	::CloseHandle((HANDLE)DokanFileInfo->Context);
	DokanFileInfo->Context = NULL;
	_log(ZQ::common::Log::L_INFO, FOPFMT(CloseFile, "closed"));

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doCleanup(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	if (!DokanFileInfo->Context) 
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(Cleanup, "invalid handle"));
		return -1;
	}

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "%s"), DokanFileInfo->DeleteOnClose?"DeleteOnClose":"");
	::CloseHandle((HANDLE)DokanFileInfo->Context);
	DokanFileInfo->Context = 0;

	if (DokanFileInfo->DeleteOnClose) 
	{
		if (DokanFileInfo->IsDirectory)
		{
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "deleting directory "));
			if (!RemoveDirectoryW(wfilePath.c_str()))
				_log(ZQ::common::Log::L_WARNING, FOPFMT(Cleanup, "deleting directory error(%d)"), GetLastError());
			else
				_log(ZQ::common::Log::L_INFO, FOPFMT(Cleanup, "deleted directory"));
		}
		else 
		{
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "deleting file"));
			if (::DeleteFileW(wfilePath.c_str()) == 0)
				_log(ZQ::common::Log::L_WARNING, FOPFMT(Cleanup, "deleting file error(%d)"), GetLastError());
			else
				_log(ZQ::common::Log::L_INFO, FOPFMT(Cleanup, "deleted file"));
		}
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doReadFile(PDOKAN_FILE_INFO DokanFileInfo, 
						  LPCWSTR				FileName,
						  LPVOID				Buffer,
						  DWORD				BufferLength,
						  LPDWORD				ReadLength,
						  LONGLONG			Offset)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE) DokanFileInfo->Context;
	ULONG	offset = (ULONG) Offset;
	BOOL	opened = FALSE;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "reading %dB from offset[%lld]"), BufferLength, Offset);

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "invalid handle, create a new one"));
		handle = ::CreateFileW(wfilePath.c_str(),
			GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		if (handle == INVALID_HANDLE_VALUE)
		{
			_log(ZQ::common::Log::L_WARNING, FOPFMT(ReadFile, "create error(%d)"), GetLastError());
			return -1;
		}

		opened = TRUE;
	}

	if (::SetFilePointer(handle, offset, NULL, FILE_BEGIN) == 0xFFFFFFFF) 
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "seek error, offset[%lld]"), Offset);
		if (opened)
			::CloseHandle(handle);
		return -1;
	}


	if (!::ReadFile(handle, Buffer, BufferLength, ReadLength, NULL))
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "read error(%d) buffer length[%d], read length[%d]"),
			::GetLastError(), BufferLength, *ReadLength);
		
		if (opened)
			::CloseHandle(handle);
		return -1;
	}

	_log(ZQ::common::Log::L_INFO, FOPFMT(ReadFile, "read %dbyte from offset[%lld]"), *ReadLength, Offset);

	if (opened)
		::CloseHandle(handle);

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doWriteFile(PDOKAN_FILE_INFO DokanFileInfo, 
						   LPCWSTR		FileName,
						   LPCVOID		Buffer,
						   DWORD		NumberOfBytesToWrite,
						   LPDWORD		NumberOfBytesWritten,
						   LONGLONG			Offset)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	ULONG	offset = (ULONG)Offset;
	BOOL	opened = FALSE;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "offset[%lld] length[%d]"), Offset, NumberOfBytesToWrite);

	// reopen the file
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "invalid handle, cleanuped"));
		handle = ::CreateFileW(	wfilePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (handle == INVALID_HANDLE_VALUE)
		{
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "CreateFile error(%d)"), ::GetLastError());
			return -1;
		}

		opened = TRUE;
	}

	if (DokanFileInfo->WriteToEndOfFile)
	{
		if (SetFilePointer(handle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
		{
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "seek error, offset[EOF], error(%d)"), GetLastError());
			return -1;
		}
	} 
	else if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "seek error, offset[%d], error(%d)"), offset, GetLastError());
		return -1;
	}


	if (!WriteFile(handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, NULL))
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "write error: %u, buffer length: %d, write length: %d"),
			GetLastError(), NumberOfBytesToWrite, *NumberOfBytesWritten);
		return -1;

	}

	_log(ZQ::common::Log::L_INFO, FOPFMT(WriteFile, "wrote %d, offset %d"), *NumberOfBytesWritten, offset);

	// close the file when it is reopened
	if (opened)
		::CloseHandle(handle);

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doFlushFileBuffers(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(FlushFileBuffers, ""), filepath.c_str());

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(FlushFileBuffers, "invalid handle"));
		return 0;
	}

	if (::FlushFileBuffers(handle))
		return 0;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(FlushFileBuffers, "flush error(%d)"), GetLastError());
	return -1;

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doGetFileInfo(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, LPBY_HANDLE_FILE_INFORMATION HandleFileInformation, bool skipGuessContainer)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	BOOL	opened = FALSE;
	_log(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, ""));

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG,  FOPFMT(GetFileInfo, "invalid handle"));

		// If CreateDirectory returned FILE_ALREADY_EXISTS and 
		// it is called with FILE_OPEN_IF, that handle must be opened.
		handle = ::CreateFileW(wfilePath.c_str(), 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (handle == INVALID_HANDLE_VALUE)
			return -1;

		opened = TRUE;
	}

	do {
		if (::GetFileInformationByHandle(handle, HandleFileInformation))
		{
			_log(ZQ::common::Log::L_DEBUG,  FOPFMT(GetFileInfo, "GetFileInformationByHandle success, file size: %d"),
				HandleFileInformation->nFileSizeLow);
			break;
		}

		_log(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, "error(%d)"), GetLastError());

		// FileName is a root directory
		// in this case, FindFirstFile can't get directory information
		if (wcslen(FileName) == 1)
		{
			_log(ZQ::common::Log::L_DEBUG,  FOPFMT(GetFileInfo, "root dir"));
			HandleFileInformation->dwFileAttributes = ::GetFileAttributesW(wfilePath.c_str());
			break;
		}

		WIN32_FIND_DATAW find;
		ZeroMemory(&find, sizeof(WIN32_FIND_DATAW));
		handle = ::FindFirstFileW(wfilePath.c_str(), &find);
		if (handle == INVALID_HANDLE_VALUE)
		{
			_log(ZQ::common::Log::L_DEBUG,  FOPFMT(GetFileInfo, "FindFirstFile error(%d)"), GetLastError());
			return -1;
		}

		HandleFileInformation->dwFileAttributes = find.dwFileAttributes;
		HandleFileInformation->ftCreationTime   = find.ftCreationTime;
		HandleFileInformation->ftLastAccessTime = find.ftLastAccessTime;
		HandleFileInformation->ftLastWriteTime  = find.ftLastWriteTime;
		HandleFileInformation->nFileSizeHigh    = find.nFileSizeHigh;
		HandleFileInformation->nFileSizeLow     = find.nFileSizeLow;

		_log(ZQ::common::Log::L_DEBUG,  FOPFMT(GetFileInfo, "FindFiles OK, file size: %d"), find.nFileSizeLow);
		::FindClose(handle);

	} while(0);

	if (opened)
		::CloseHandle(handle);

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doFindFiles(PDOKAN_FILE_INFO DokanFileInfo, 
						   LPCWSTR				FileName,
						   PFillFindData		FillFindData) // function pointer
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE				hFind;
	WIN32_FIND_DATAW	findData;
	DWORD				error;
	PWCHAR				yenStar = L"\\*";
	int count = 0;

	wfilePath += yenStar;
	filepath = wcs2mbs(wfilePath);
	_log(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, ""));

	hFind = ::FindFirstFileW(wfilePath.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, "invalid file handle. error(%d)"), GetLastError());
		return -1;
	}

	FillFindData(&findData, DokanFileInfo);
	count++;

	while (::FindNextFileW(hFind, &findData) != 0)
	{
		FillFindData(&findData, DokanFileInfo);
		count++;
	}

	error = GetLastError();
	FindClose(hFind);

	if (error != ERROR_NO_MORE_FILES)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, "FindNextFile error. error(%d)"), error);
		return -1;
	}

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, "FindFiles return %d entries in %s"), count, filepath.c_str());

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doDeleteFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(DeleteFile, ""));

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doDeleteDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	HANDLE	hFind;
	WIN32_FIND_DATAW	findData;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(DeleteDirectory, ""));

	if (wfilePath[wfilePath.length()-1] != L'\\')
		wfilePath += L"\\";

	wfilePath += L"*";

	hFind = ::FindFirstFileW(wfilePath.c_str(), &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (NULL != findData.cFileName && wcscmp(findData.cFileName, L"..") != 0 &&
			wcscmp(findData.cFileName, L".") != 0) 
		{
			::FindClose(hFind);
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(DeleteDirectory, "directory[%s] is not empty"), wcs2mbs(std::wstring(findData.cFileName)).c_str());

			return -(int)ERROR_DIR_NOT_EMPTY;
		}

		if (!FindNextFileW(hFind, &findData))
			break;
	}

	::FindClose(hFind);

	if (GetLastError() != ERROR_NO_MORE_FILES)
		return -1;

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doMoveFile(PDOKAN_FILE_INFO DokanFileInfo, 
						  LPCWSTR			FileName, // existing file name
						  LPCWSTR			NewFileName,
						  BOOL				ReplaceIfExisting)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	std::wstring wnewPath; std::string newpath = toFilePath(wnewPath, NewFileName);

	BOOL			status;
	_log(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "-> %s"), newpath.c_str());

	if (DokanFileInfo->Context)
	{
		// should close? or rename at closing?
		::CloseHandle((HANDLE)DokanFileInfo->Context);
		DokanFileInfo->Context = 0;
	}

	if (ReplaceIfExisting)
		status = ::MoveFileExW(wfilePath.c_str(), wnewPath.c_str(), MOVEFILE_REPLACE_EXISTING);
	else
		status = ::MoveFileW(wfilePath.c_str(), wnewPath.c_str());

	if (status == FALSE)
	{
		DWORD error = ::GetLastError();
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "failed status: %d, code: %d"), status, error);
		return -(int)error;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doLockFile(PDOKAN_FILE_INFO DokanFileInfo, 
						  LPCWSTR			FileName,
						  LONGLONG			ByteOffset,
						  LONGLONG			Length)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle;
	LARGE_INTEGER offset;
	LARGE_INTEGER length;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(LockFile, ""));

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(LockFile, "invalid handle"));
		return -1;
	}

	length.QuadPart = Length;
	offset.QuadPart = ByteOffset;

	if (!::LockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart))
	{
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(LockFile, "fail"));
		return -1;
	} 

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(LockFile, "success"));

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doSetEndOfFile(PDOKAN_FILE_INFO DokanFileInfo, 
							  LPCWSTR			FileName,
							  LONGLONG			ByteOffset)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE			handle;
	LARGE_INTEGER	offset;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(SetEndOfFile, "%lld"), ByteOffset);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetEndOfFile, "invalid handle"));
		return -1;
	}

	offset.QuadPart = ByteOffset;
	if (!::SetFilePointerEx(handle, offset, NULL, FILE_BEGIN))
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetEndOfFile, "SetFilePointer error(%d), offset[%lld]"),
			::GetLastError(), ByteOffset);
		return GetLastError() * -1;
	}

	if (!::SetEndOfFile(handle))
	{
		DWORD error = ::GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetEndOfFile, "error(%d)"), error);
		return error * -1;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doSetAllocationSize(PDOKAN_FILE_INFO DokanFileInfo, 
								   LPCWSTR				FileName,
								   LONGLONG			AllocSize)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE			handle;
	LARGE_INTEGER	fileSize;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(SetAllocationSize, "%lld"), AllocSize);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetAllocationSize, "invalid handle"));
		return -1;
	}

	if (::GetFileSizeEx(handle, &fileSize))
	{
		if (AllocSize < fileSize.QuadPart)
		{
			fileSize.QuadPart = AllocSize;
			if (!SetFilePointerEx(handle, fileSize, NULL, FILE_BEGIN))
			{
				_log(ZQ::common::Log::L_WARNING, FOPFMT(SetAllocationSize, "SetFilePointer error[%d] offset[%lld]"), GetLastError(), AllocSize);
				return GetLastError() * -1;
			}

			if (!SetEndOfFile(handle))
			{
				DWORD error = GetLastError();
				_log(ZQ::common::Log::L_WARNING, FOPFMT(SetAllocationSize, "error(%d)"), error);
				return error * -1;
			}
		}
	} 
	else 
	{
		DWORD error = GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetAllocationSize, "error(%d)"), error);
		return error * -1;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doSetFileAttrs(PDOKAN_FILE_INFO DokanFileInfo, 
								   LPCWSTR				FileName,
								   DWORD				FileAttributes)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileAttributes, "attr[0x%x]"), FileAttributes);

	if (!SetFileAttributesW(wfilePath.c_str(), FileAttributes))
	{
		DWORD error = GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetFileAttributes, "error(%d)"), error);
		return error * -1;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doSetFileTime(PDOKAN_FILE_INFO DokanFileInfo, 
							 LPCWSTR				FileName,
							 CONST FILETIME*		CreationTime,
							 CONST FILETIME*		LastAccessTime,
							 CONST FILETIME*		LastWriteTime)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileTime, ""));

	handle = (HANDLE)DokanFileInfo->Context;

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetFileTime, "invalid handle"));
		return -1;
	}

	if (!SetFileTime(handle, CreationTime, LastAccessTime, LastWriteTime))
	{
		DWORD error = GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetFileTime, "error(%d)"), error);
		return error * -1;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doUnlockFile(PDOKAN_FILE_INFO DokanFileInfo, 
							LPCWSTR				FileName,
							LONGLONG			ByteOffset,
							LONGLONG			Length)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);
	HANDLE	handle;
	LARGE_INTEGER	length;
	LARGE_INTEGER	offset;

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(UnlockFile, ""));

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(UnlockFile, "invalid handle"));
		return -1;
	}

	length.QuadPart = Length;
	offset.QuadPart = ByteOffset;

	if (!UnlockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart))
	{
	_log(ZQ::common::Log::L_WARNING, FOPFMT(UnlockFile, "fail"));
		return -1;
	} 

		_log(ZQ::common::Log::L_DEBUG, FOPFMT(UnlockFile, "success"));
#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doGetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, 
								 LPCWSTR					FileName,
								 PSECURITY_INFORMATION	SecurityInformation,
								 PSECURITY_DESCRIPTOR	SecurityDescriptor,
								 ULONG				BufferLength,
								 PULONG				LengthNeeded)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	HANDLE	handle;
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileSecurity, ""));

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(GetFileSecurity, "invalid handle"));
		return -1;
	}

	if (!GetUserObjectSecurity(handle, SecurityInformation, SecurityDescriptor,	BufferLength, LengthNeeded))
	{
		int error = GetLastError();
		if (error == ERROR_INSUFFICIENT_BUFFER)
		{
			_log(ZQ::common::Log::L_WARNING, FOPFMT(GetFileSecurity, "failed: ERROR_INSUFFICIENT_BUFFER"));
			return error * -1;
		}
		else 
		{
			_log(ZQ::common::Log::L_WARNING, FOPFMT(GetFileSecurity, "failed: %d"), error);
			return -1;
		}
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doSetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, 
								 LPCWSTR					FileName,
								 PSECURITY_INFORMATION	SecurityInformation,
								 PSECURITY_DESCRIPTOR	SecurityDescriptor,
								 ULONG				SecurityDescriptorLength)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	HANDLE	handle;
	std::wstring wfilePath; std::string filepath = toFilePath(wfilePath, FileName);

	_log(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileSecurity, ""));

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetFileSecurity, "invalid handle"));
		return -1;
	}

	if (!SetUserObjectSecurity(handle, SecurityInformation, SecurityDescriptor))
	{
		int error = GetLastError();
		_log(ZQ::common::Log::L_WARNING, FOPFMT(SetFileSecurity, "failed: %d"), error);
		return -1;
	}

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}

int DokanFuse::doGetVolumeInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPWSTR		VolumeNameBuffer,     DWORD VolumeNameSize,
		LPWSTR		FileSystemNameBuffer, DWORD FileSystemNameSize,
		LPDWORD		VolumeSerialNumber,   LPDWORD MaximumComponentLength, LPDWORD FileSystemFlags)
{
#ifdef DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "GetVolumeInformation()"));
	wcscpy_s(VolumeNameBuffer, VolumeNameSize / sizeof(WCHAR), L"DokanVol");
	*VolumeSerialNumber = 0x20121225;
	*MaximumComponentLength = 256;
	*FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH | 
		FILE_CASE_PRESERVED_NAMES | 
		FILE_SUPPORTS_REMOTE_STORAGE |
		FILE_UNICODE_ON_DISK |
		FILE_PERSISTENT_ACLS;

	wcscpy_s(FileSystemNameBuffer, FileSystemNameSize / sizeof(WCHAR), L"DokanFUSE");

#endif // DOKANFUSE_WITH_SAMPLE_MIRROR_IMPL
	return 0;
}


int DokanFuse::doUnmount(PDOKAN_FILE_INFO DokanFileInfo)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "Unmount"));
	return 0;
}

int DokanFuse::doGetDiskFreeSpace(PDOKAN_FILE_INFO DokanFileInfo,
		PULONGLONG FreeBytesAvailable, PULONGLONG TotalNumberOfBytes, PULONGLONG TotalNumberOfFreeBytes)
{
	DWORD sectorPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
	if (!::GetDiskFreeSpaceW(_wRootPath.c_str(), &sectorPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters))
		return -1;
	
	*TotalNumberOfBytes = ((ULONGLONG) sectorPerCluster) * bytesPerSector * totalNumberOfClusters;
	*TotalNumberOfFreeBytes = ((ULONGLONG) sectorPerCluster) * bytesPerSector * numberOfFreeClusters;
	*FreeBytesAvailable = *TotalNumberOfFreeBytes;
	return 0;
}

void DokanFuse::stop()
{
	if (_wMountPoint.empty())
		return;
	
	_log(ZQ::common::Log::L_INFO, CLOGFMT(DokanFuse, "stopping instance: %s"), _desc.c_str());
//	DokanUnmount();
	wchar_t * MountPoint =(wchar_t *) _wMountPoint.c_str();
	int status = DokanRemoveMountPoint(MountPoint);
	WaitForSingleObject(_waitStop,10000);
	//Sleep(20000);
}


int DokanFuse::run(void)
{
	_cRunning++;
	_dokanOptions.Options |= DOKAN_OPTION_KEEP_ALIVE;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DokanFuse, "starting instance: %s"), _desc.c_str());
	int status = DokanMain(&_dokanOptions, &_dokanOperations);
	_cRunning--;

	switch (status) {
	case DOKAN_SUCCESS:
		_log(ZQ::common::Log::L_INFO, CLOGFMT(DokanFuse, "%s quit"), _desc.c_str());
		break;

	case DOKAN_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed"), _desc.c_str());
		break;

	case DOKAN_DRIVE_LETTER_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at bad drive letter"), _desc.c_str());
		break;

	case DOKAN_DRIVER_INSTALL_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at installing driver"), _desc.c_str());
		break;
	case DOKAN_START_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at driver error"), _desc.c_str());
		break;
	case DOKAN_MOUNT_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at conflict drive letter"), _desc.c_str());
		break;
	case DOKAN_MOUNT_POINT_ERROR:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at bad mount point"), _desc.c_str());
		break;
	default:
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(DokanFuse, "%s failed at err(%d)"), _desc.c_str(), status);
		break;
	}
     SetEvent(_waitStop);
	return 0;
}

#ifdef DOKANX 
#  define DRIVER_NAME "dokanx"
#else
#  define DRIVER_NAME "Dokan"
#endif // DOKANX 


bool DokanFuse::installDriver()
{
	char sysdir[MAX_PATH];
	::GetSystemDirectory(sysdir, MAX_PATH);
	std::string cmd = std::string("copy /Y .\\" DRIVER_NAME ".dll ") + sysdir +"\\";
	system(cmd.c_str());
	
	// determine the source file of driver to copy
	char sourcePath[200]=".\\";
	OSVERSIONINFOEX osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	if(::GetVersionEx((OSVERSIONINFO *)&osinfo))
	{
		std::string winstr = "unknown";
		if (VER_PLATFORM_WIN32_NT != osinfo.dwPlatformId)
		{
			printf("Driver[%s] doesn't support this OS id[%d]\n", DRIVER_NAME, osinfo.dwPlatformId);
			return false;
		}

		std::string procr_arch;
		SYSTEM_INFO si;
		memset(&si, 0, sizeof(si));
		GetSystemInfo(&si);
        switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_IA64:
				procr_arch = "ia64";
				break;

		case PROCESSOR_ARCHITECTURE_AMD64:
				procr_arch = "amd64";
				break;

		default:
				procr_arch = "x86";
				break;
		}

//		if (0 = procr_arch.compare(ia64))

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724834(v=vs.85).aspx
		if ( osinfo.dwMajorVersion == 5)
		{
			if (osinfo.dwMinorVersion == 0 )
				winstr = "win2000";
			else if ( osinfo.dwMinorVersion == 1 )
				winstr = "winxp";
			else if (osinfo.dwMinorVersion == 2 )
				winstr = "win2003";

			// winxp driver is compatible with win2000 and 2003, so take xp 
			winstr = "wxp";
		}

		else if ( osinfo.dwMajorVersion <= 4 )
			winstr = "winnt4";

		else if ( osinfo.dwMajorVersion == 6)
		{
			if (osinfo.dwMinorVersion == 0 )
				winstr = (VER_NT_WORKSTATION == osinfo.wProductType) ? "winvista" : "win2008";
			else if ( osinfo.dwMinorVersion == 1 )
				winstr = (VER_NT_WORKSTATION == osinfo.wProductType) ? "win7" : "win2008r2";
			else if ( osinfo.dwMinorVersion == 2 )
				winstr = (VER_NT_WORKSTATION == osinfo.wProductType) ? "win8" : "win2012";
		}

		snprintf(sourcePath, sizeof(sourcePath) -2, ".\\fre_%s_%s\\", winstr.c_str(), procr_arch.c_str());
	}

	std::string driverPath = sysdir;
	driverPath += "\\drivers\\" DRIVER_NAME ".sys";
	cmd = std::string("copy /Y ") + sourcePath + DRIVER_NAME ".sys " + driverPath;
	printf("Driver[%s] installing, sourcePath[%s] imagePath[%s]\n", DRIVER_NAME, sourcePath, driverPath.c_str());
	system(cmd.c_str());
/*
	cmd = std::string("sc create ") + DRIVER_NAME + " binPath=" + driverPath;
	cmd += " type=filesys";
	system(cmd.c_str());
*/

	SC_HANDLE hCtrl = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE | SC_MANAGER_CONNECT);
	if (NULL == hCtrl)
	{
		printf("failed to open SCM");
		return false;
	}

	SC_HANDLE hDriver = CreateService(hCtrl, DRIVER_NAME, DRIVER_NAME, 0,
        SERVICE_FILE_SYSTEM_DRIVER, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
        driverPath.c_str(), NULL, NULL, NULL, NULL, NULL);
    
    if (NULL == hDriver)
	{
		DWORD dwErr = GetLastError();
        if (ERROR_SERVICE_EXISTS == dwErr)
            printf("Driver[%s] is already installed\n", DRIVER_NAME);
		else
			printf("Driver[%s] failed to install: err(%d)\n", DRIVER_NAME, dwErr);

		CloseServiceHandle(hCtrl);
        return false;
    }

    CloseServiceHandle(hDriver);

	hDriver = OpenService(hCtrl, DRIVER_NAME,
        SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
    
    if (NULL != hDriver)
	{

		if (::StartService(hDriver, 0, NULL))
			printf("Driver[%s] started\n", DRIVER_NAME);
	}
    
    CloseServiceHandle(hDriver);
    CloseServiceHandle(hCtrl);

	return true;
}

bool DokanFuse::removeDriver()
{
	printf("Driver[%s] deleting\n", DRIVER_NAME);
	std::string cmd = std::string("sc delete ") + DRIVER_NAME;
	system(cmd.c_str()); // simple enough

/*
	SC_HANDLE hCtrl = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS | SC_MANAGER_CONNECT);
	if (NULL == hCtrl)
	{
		printf("failed to open SCM");
		return false;
	}

	SC_HANDLE hDriver = OpenService(hCtrl, DRIVER_NAME,
        SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);

    if (NULL == hDriver)
	{
		DWORD dwErr = GetLastError();
		printf("Driver[%s] failed to open: err(%d)\n", DRIVER_NAME, dwErr);

		CloseServiceHandle(hCtrl);
        return false;
    }

	bool succ = false;

	do {
		SERVICE_STATUS ss;
		QueryServiceStatus(hDriver, &ss);

		if (ss.dwCurrentState == SERVICE_RUNNING)
		{
			if (::ControlService(hDriver, SERVICE_CONTROL_STOP, &ss))
				printf("Driver[%s] stopped\n", DRIVER_NAME);
			else
			{
				printf("Driver[%s] failed to stop: err(%d)\n", DRIVER_NAME, GetLastError());
				break;
			}
		}

		if (!::DeleteService(hDriver))
		{
			printf("Driver[%s] failed to delete: err(%d)\n", DRIVER_NAME, GetLastError());
            break;
        }
		
		succ = true;
		printf("Driver[%s] deleted\n", DRIVER_NAME, GetLastError());
	} while(0);
    
    CloseServiceHandle(hDriver);
    CloseServiceHandle(hCtrl);

	if (succ)
	{
		char sysdir[MAX_PATH];
		::GetSystemDirectory(sysdir, MAX_PATH);
		std::string cmd = std::string("del /Q ") + sysdir + "\\" DRIVER_NAME ".dll";
		system(cmd.c_str());
		cmd = std::string("del /Q ") + sysdir + "\\drivers\\" DRIVER_NAME ".sys";
		system(cmd.c_str());
	}

	return succ;
*/
	return true;
}
