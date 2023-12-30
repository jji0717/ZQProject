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
// Ident : $Id: CdmiDokan.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/CdmiDokan.cpp $
// 
// 63    3/02/16 6:27p Hui.shao
// 
// 62    11/19/14 8:08p Hui.shao
// userdomain
// 
// 61    6/04/14 6:09p Hui.shao
// 
// 60    10/31/13 5:46p Hui.shao
// 
// 59    10/31/13 5:18p Hui.shao
// 
// 58    10/31/13 1:40p Hui.shao
// ST_MODE_BIT_OFFSET_USER
// 
// 57    8/13/13 11:32a Hui.shao
// Aqua3 start supporting search for both folder and files
// 
// 56    7/03/13 7:24p Hui.shao
// covered attrib +/-R
// 
// 55    6/13/13 6:10p Hui.shao
// flags
// 
// 54    6/13/13 4:42p Hui.shao
// 
// 53    6/13/13 3:53p Hui.shao
// enabled setFileAttrs()
// 
// 52    6/05/13 5:00p Ketao.zhang
// 
// 51    6/03/13 7:37p Hui.shao
// ChildReader dispatch to searchByParent() and readByEnumeration() 
// 
// 50    5/30/13 6:38p Hui.shao
// 
// 49    5/30/13 11:43a Hui.shao
// FindFiles() takes ChildReaders, and sunk fileInfoCache into CdmiFuseOps
// 
// 48    5/23/13 4:55p Hui.shao
// enh#17961 to read the space usage from aqua domain
// 
// 47    5/13/13 5:01p Li.huang
// roll back to version 45
// 
// 45    5/08/13 11:43a Li.huang
// fix bug 18082
// 
// 44    5/02/13 5:08p Li.huang
// 
// 43    4/30/13 8:56a Li.huang
// fix can not zip
// 
// 42    4/18/13 5:05p Hui.shao
// 
// 41    4/09/13 12:04p Li.huang
// 
// 40    4/09/13 11:26a Hui.shao
// bug#17982 to export mounted vol as network share
// 
// 38    4/08/13 11:31a Hui.shao
// 
// 37    4/08/13 11:25a Hui.shao
// ignore client err when read file buffers to len=0
// 
// 36    4/07/13 11:22a Hui.shao
// bug#17981 to prompt when duplicate file name to rename to exists
// 
// 35    4/03/13 11:19a Hui.shao
// support chinese chars in names
// 
// 34    3/13/13 4:26p Hui.shao
// 
// 33    3/08/13 11:20a Li.huang
// 
// 32    3/01/13 10:20a Hui.shao
// mapped some cdmi err to win err
// 
// 31    2/28/13 2:32p Hui.shao
// 
// 30    2/25/13 8:54p Hui.shao
// 
// 29    2/25/13 8:33p Hui.shao
// skip guess if getFileInfo() is certain whether the input path is a dir
// or file
// 
// 28    2/25/13 4:11p Hui.shao
// separated rootUrl and homeContainer (a sub container under rootUrl)
// 
// 27    2/22/13 4:09p Hui.shao
// 
// 25    2/08/13 10:47a Hui.shao
// 
// 24    2/08/13 10:43a Hui.shao
// 
// 23    2/08/13 10:16a Hui.shao
// 
// 22    2/07/13 2:54p Hui.shao
// blacklist of CreateDirectory()
// 
// 21    1/30/13 11:41a Hui.shao
// 
// 20    1/28/13 8:55p Hui.shao
// 
// 18    1/28/13 5:41p Hui.shao
// 
// 17    1/28/13 3:21p Hui.shao
// 
// 16    1/25/13 3:14p Hui.shao
// 
// 15    1/24/13 4:03p Hui.shao
// 
// 14    1/24/13 4:00p Hui.shao
// supported files in subdir
// 
// 13    1/23/13 1:46p Hui.shao
// 
// 12    1/23/13 11:29a Hui.shao
// covered dos commands: dir, mkdir, copy, freespace
// 
// 11    1/23/13 10:15a Hui.shao
// wrapperd fstat into fileinfo
// 
// 10    1/10/13 5:16p Hui.shao
// all entries enumerated
// 
// 9     1/10/13 2:15p Hui.shao
// doReadFile()
// 
// 8     1/09/13 3:40p Hui.shao
// 
// 7     1/09/13 3:13p Hui.shao
// covered initMount + copyInto
// 
// 6     1/07/13 10:54p Hui.shao
// WriteFile()
// 
// 5     1/07/13 8:36p Hui.shao
// findfiles and getfileinfo
// 
// 4     1/07/13 12:03p Hui.shao
// 
// 3     1/06/13 4:22p Hui.shao
// drafted some entries for CdmiDokan
// 
// 2     12/28/12 11:40a Hui.shao
// disk space
// 
// 1     12/27/12 5:06p Hui.shao
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

#include "CdmiDokan.h"
#include "urlstr.h"

#define cdlog (DokanFuse::_log)
#define FOPFMT(_OP, _X)  CLOGFMT(CdmiDokan, #_OP "(%s) " _X), pathname.c_str()

#define CSET_GBK    "936"
#define CSET_UTF8   "65001"
#define LC_NAME_zh_CN   "Chinese_People's Republic of China"
#define LC_NAME_zh_CN_GBK       LC_NAME_zh_CN "." CSET_GBK
#define LC_NAME_zh_CN_UTF8      LC_NAME_zh_CN "." CSET_UTF8
#define LC_NAME_zh_CN_DEFAULT   LC_NAME_zh_CN_GBK

// -----------------------------
// class CdmiDokan
// -----------------------------
CdmiDokan::CdmiDokan(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& mountPoint, const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, uint32 options, uint32 flags, const std::string& locale, uint threadCount)
: DokanFuse(log, mountPoint, rootUrl, options, locale, threadCount), 
  CdmiFuseOps(log, thrdPool, rootUrl, userDomain, homeContainer, flags), _lastHandle(0)
{
	log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiDokan, "mount FUSE: mountpoint[%s] remote[%s], opt[0x%x]"), wcs2mbs(_wRootPath).c_str(), _rootUrl.c_str(), DokanFuse::_dokanOptions.Options);
}

CdmiDokan::~CdmiDokan()
{
}

std::string CdmiDokan::toFilePath(std::wstring& wFilePath, LPCWSTR fileName)
{
	wFilePath = (fileName ? fileName: L"");
	if (!wFilePath.empty() && FNSEPC == wFilePath[0])
		wFilePath = wFilePath.substr(1);

	return wcs2mbs(wFilePath, LC_NAME_zh_CN_UTF8);
}

HANDLE CdmiDokan::newHandle(const std::string& pathname)
{
	ZQ::common::MutexGuard g(_lkHandles);
	uint64 newH  = ++_lastHandle;
	if (0 == newH || INVALID_HANDLE_VALUE == (HANDLE)newH)
	{
		newH  = ++_lastHandle;
		_lastHandle &= 0xfffffffffffffff; // take 60bits instead of full 64bits
	}

	MAPSET(HandleMap, _handleMap, newH, pathname);
	return (HANDLE) newH;
}

void CdmiDokan::closeHandle(HANDLE handle)
{
	ZQ::common::MutexGuard g(_lkHandles);
	_handleMap.erase((uint64)handle);
}

#define BuildFlagStr(val, str, flag) if (val&flag) { str += "|"#flag; }

int CdmiDokan::doGetVolumeInfo(PDOKAN_FILE_INFO DokanFileInfo,
		LPWSTR		VolumeNameBuffer,     DWORD VolumeNameSize,
		LPWSTR		FileSystemNameBuffer, DWORD FileSystemNameSize,
		LPDWORD		VolumeSerialNumber,   LPDWORD MaximumComponentLength, LPDWORD FileSystemFlags)
{
	cdlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiDokan, "GetVolumeInfo() mountpoint[%s] remote[%s], opt[0x%x]"), wcs2mbs(_wMountPoint).c_str(), (_rootUrl + _homeContainer).c_str(), DokanFuse::_dokanOptions.Options);
	wcscpy_s(VolumeNameBuffer, VolumeNameSize / sizeof(WCHAR), L"CdmiVol");
	*VolumeSerialNumber = _volSN;
	*MaximumComponentLength = 256;
	*FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH | 
		FILE_CASE_PRESERVED_NAMES | 
		FILE_SUPPORTS_REMOTE_STORAGE |
//		FILE_UNICODE_ON_DISK |
		FILE_PERSISTENT_ACLS;

	wcscpy_s(FileSystemNameBuffer, FileSystemNameSize / sizeof(WCHAR), L"CdmiFUSE");

	return 0;
}

int CdmiDokan::doUnmount(PDOKAN_FILE_INFO DokanFileInfo)
{
	cdlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiDokan, "Unmount() mountpoint[%s] remote[%s]"), wcs2mbs(_wRootPath).c_str(), _rootUrl.c_str());
	return 0;
}

void CdmiDokan::timeToFileTime(int64 t, FILETIME& ft)
{
	t *=10000;  //convert msec to nsec
	memcpy(&ft, &t, sizeof(ft));
}

int CdmiDokan::cdmiErrToDokanErr(CdmiFuseOps::CdmiRetCode cdmirc)
{
	// cdmirc_OK = 200, cdmirc_Created = 201, cdmirc_Accepted = 202, cdmirc_NoContent = 204, cdmirc_PartialContent = 206,
	// cdmirc_Found = 302, 
	// cdmirc_BadRequest = 400, cdmirc_Unauthorized = 401, cdmirc_Forbidden = 403, cdmirc_NotFound = 404, cdmirc_NotAcceptable = 406, cdmirc_Conflict = 409,
	// cdmirc_ServerError = 500,

	// cdmirc_ExtErr = 700, cdmirc_RequestFailed, cdmirc_RequestTimeout,
	// cdmirc_MAX

	if (CdmiRet_SUCC(cdmirc))
		return 0;

	switch(cdmirc)
	{
	case cdmirc_NotFound:
		return -ERROR_FILE_NOT_FOUND;
	case cdmirc_Unauthorized:
	case cdmirc_Forbidden:
		return -ERROR_ACCESS_DENIED;
	case cdmirc_Conflict:
		return -ERROR_ALREADY_EXISTS;
	default: break;
	}

	return -0xE000 - cdmirc;
}

typedef struct {uint32 from, to;} FlagToFlag;

static DWORD fstatModeToAttrs(const CdmiDokan::FileInfo& fi)
{
#pragma message ( __MSGLOC__ "TODO: convert fstat.st_mode to dwFileAttributes")
	DWORD fileAttr = FILE_ATTRIBUTE_NORMAL;
	if ( S_IFDIR & fi.filestat.st_mode)
		fileAttr = FILE_ATTRIBUTE_DIRECTORY;

	uint8 ownerPermission = (fi.filestat.st_mode >> ST_MODE_BIT_OFFSET_USER) &0xf;
	if (ownerPermission & 0x4) // read permission
		fileAttr |= 0; // do nothing
	if (0 == (ownerPermission & 0x2)) // write permission disabled = read only
		fileAttr |= FILE_ATTRIBUTE_READONLY;

	return fileAttr;
}

void CdmiDokan::fixupPathname(std::string& pathname)
{
	if (FNSEPC != LOGIC_FNSEPC && !pathname.empty())
		std::replace(pathname.begin(), pathname.end(), LOGIC_FNSEPC, FNSEPC);
}

void copyFItoDokanFI(const CdmiFuseOps::FileInfo& fi, LPBY_HANDLE_FILE_INFORMATION FileInfoByHandle)
{
	FileInfoByHandle->dwFileAttributes = fstatModeToAttrs(fi);
	FileInfoByHandle->nFileSizeLow     = (DWORD) (fi.filestat.st_size & 0xffffffff);
	FileInfoByHandle->nFileSizeHigh    = (DWORD) (fi.filestat.st_size >> 32);
	CdmiDokan::timeToFileTime(CdmiFuseOps::time_t2time(fi.filestat.st_atime), FileInfoByHandle->ftLastAccessTime);
	CdmiDokan::timeToFileTime(CdmiFuseOps::time_t2time(fi.filestat.st_mtime), FileInfoByHandle->ftLastWriteTime);
	CdmiDokan::timeToFileTime(CdmiFuseOps::time_t2time(fi.filestat.st_ctime), FileInfoByHandle->ftCreationTime);
}

int CdmiDokan::doGetFileInfo(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, LPBY_HANDLE_FILE_INFORMATION FileInfoByHandle, bool skipGuessContainer)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	fixupPathname(pathname); wPathname = mbs2wcs(pathname, LC_NAME_zh_CN_UTF8);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, ""));

	memset(FileInfoByHandle, 0x00, sizeof(BY_HANDLE_FILE_INFORMATION));
	FileInfoByHandle->dwVolumeSerialNumber = _volSN;

	FileInfo fi;
	if (readFileInfoFromCache(pathname, fi))
	{
		// copy the cached info as the returning result
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, "taking the cached file info, size[%lld]"), fi.filestat.st_size);
		copyFItoDokanFI(fi, FileInfoByHandle);
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, "size[%lld]"), fi.filestat.st_size);
		return 0;
	}

	bool bIsACertainDir = false;
	if (pathname.empty() || (FNSEPC == pathname[pathname.length() -1]))
		bIsACertainDir = true; // it is certainly a directory

	Json::Value result;
	std::string uri = pathToUri(pathname); // + LOGIC_FNSEPS;
	CdmiFuseOps::CdmiRetCode cdmirc = cdmirc_BadRequest;

	if (!skipGuessContainer || bIsACertainDir)
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, "trying cdmi_ReadContainer(%s) to touch"), uri.c_str());
		cdmirc = cdmi_ReadContainer(result, uri);

		if (CdmiRet_SUCC(cdmirc))
		{
			bIsACertainDir = true; // this is confirmed as an existing container on the server
			FileInfoByHandle->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			if (result.isMember("metadata"))
			{
				Json::Value& metadata = result["metadata"];
				int64 t;
				if (metadata.isMember("cdmi_ctime"))
				{
					t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_ctime"].asString().c_str());
					timeToFileTime(t, FileInfoByHandle->ftCreationTime);
				}

				if (metadata.isMember("cdmi_mtime"))
				{
					t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_mtime"].asString().c_str());
					timeToFileTime(t, FileInfoByHandle->ftLastWriteTime);
				}

				if (metadata.isMember("cdmi_atime"))
				{
					t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_atime"].asString().c_str());
					timeToFileTime(t, FileInfoByHandle->ftLastAccessTime);
				}

				if (metadata.isMember("cdmi_size"))
				{
					uint64 size =0;
					if (metadata["cdmi_size"].isIntegral())
						size = metadata["cdmi_size"].asInt64();
					else
						size = _atoi64(metadata["cdmi_size"].asString().c_str());

					FileInfoByHandle->nFileSizeLow     = (DWORD) (size & 0xffffffff);
					FileInfoByHandle->nFileSizeHigh    = (DWORD) (size >> 32);
				}
			}

			return 0;
		} // if (CdmiRet_SUCC(cdmirc)
	}
	
	if (bIsACertainDir)
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(GetFileInfo, "cdmi_ReadContainer(%s) as folder failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	// step 1. query CDMI for the attr of the source file
	uri = pathToUri(pathname);

	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileInfo, "calling cdmi_ReadDataObject(%s) to read attributes"), uri.c_str());
	cdmirc = cdmi_ReadDataObject(result, uri +"?metadata", std::string());

	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(GetFileInfo, "cdmi_ReadDataObject(%s) as file failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	char hint[260];
	snprintf(hint, sizeof(hint), "cdmi_ReadDataObject(%s)", uri.c_str());
	if (parseDataObject(pathname, result, fi, hint) <0)
		return -2;
/*

	fi.revision = -1;
	if (result.isMember("version"))
	{
		try {
			fi.revision = result[version"].asInt();
		}
		catch(...) {}
	}

	if (!result.isMember("metadata"))
	{
		cdlog(ZQ::common::Log::L_WARNING, FOPFMT(GetFileInfo, "cdmi_ReadDataObject(%s) no metadata received"), uri.c_str());
		return -1;
	}

	FileInfo_reset(fi);
	if (!dataObjMetadataToFstat(result["metadata"], fi))
	{
		cdlog(ZQ::common::Log::L_WARNING, FOPFMT(GetFileInfo, "cdmi_ReadDataObject(%s) failed to convert fstat"), uri.c_str());
		return -2;
	}
*/

	// step 2. copy the fi to the returning value
	copyFItoDokanFI(fi, FileInfoByHandle);

	// step 3. put this query result into the cache for next use
	cacheFileInfo(pathname, fi);

	return 0;
}

typedef std::map <int, ChildReader* > ReaderMap;

typedef struct _FindTxn {
	std::string      txnId;
	PDOKAN_FILE_INFO pDokanFI;
	PFillFindData    fFillFindData;
	ReaderMap        readers;
	int              cFileRead;
} FindTxn;

typedef std::map<std::string, FindTxn > FindTxnMap;
static FindTxnMap sFindTxnMap;
static ZQ::common::Mutex slkFindTxnMap;

void CdmiDokan::OnChildInfo(const std::string& pathName, FileInfo& fi, const std::string& txnId)
{
	// step 3. copy file info to findData
	WIN32_FIND_DATAW findData;
	memset(&findData, 0x00, sizeof(findData));
	findData.dwFileAttributes  = fstatModeToAttrs(fi);

	findData.nFileSizeLow     = (DWORD) (fi.filestat.st_size & 0xffffffff);
	findData.nFileSizeHigh    = (DWORD) (fi.filestat.st_size >> 32);
	timeToFileTime(time_t2time(fi.filestat.st_atime), findData.ftLastAccessTime);
	timeToFileTime(time_t2time(fi.filestat.st_mtime), findData.ftLastWriteTime);
	timeToFileTime(time_t2time(fi.filestat.st_ctime), findData.ftCreationTime);

	std::wstring wsubfile = mbs2wcs(pathName, LC_NAME_zh_CN_UTF8);
	if (wsubfile.length()>1 && (FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes))
		wsubfile=wsubfile.substr(0, wsubfile.length()-1);

	// FillFindData() only take the leaf name
	size_t pos = wsubfile.find_last_of(L'\\');
	if (std::wstring::npos !=pos)
		wsubfile=wsubfile.substr(pos+1);

	memcpy(findData.cFileName, wsubfile.c_str(), wsubfile.length() * sizeof(wchar_t));

	ZQ::common::MutexGuard g(slkFindTxnMap);
	FindTxnMap::iterator itTxn = sFindTxnMap.find(txnId);
	if (sFindTxnMap.end() == itTxn)
		return;

	if (itTxn->second.fFillFindData)
	{
		itTxn->second.fFillFindData(&findData, itTxn->second.pDokanFI);
//		cdlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiDokan, "OnChildInfo() txn[%s] added path[%s] subfile[%S]"), txnId.c_str(), pathName.c_str(), wsubfile.c_str());
	}

	itTxn->second.cFileRead++;
}

void CdmiDokan::OnChildReaderStopped(int readerId, bool bCancelled, const std::string& txnId)
{
	ZQ::common::MutexGuard g(slkFindTxnMap);
	FindTxnMap::iterator itTxn = sFindTxnMap.find(txnId);
	if (sFindTxnMap.end() == itTxn)
		return;
	itTxn->second.readers.erase(readerId);
}


int CdmiDokan::doFindFiles(PDOKAN_FILE_INFO	    DokanFileInfo,
						   LPCWSTR				PathName,
						   PFillFindData		FillFindData)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, PathName);

	fixupPathname(pathname);
	std::string oldPathname = pathname;
	size_t pos = pathname.find_last_of(FNSEPS LOGIC_FNSEPS);
	std::string fnPattern;
	if (std::string::npos != pos)
	{
		fnPattern = pathname.substr(pos+1);
		if (std::string::npos == fnPattern.find_first_of("*?[]"))
			fnPattern = "";
		else 
			pathname = pathname.substr(0, pos);
	}
	else if (std::string::npos != pathname.find_first_of("*?[]"))
	{ 
		fnPattern = pathname;
		pathname = ""; 
	}

	if (!pathname.empty() && (FNSEPC != pathname[pathname.length() -1]))
		pathname += FNSEPC;

	wPathname = mbs2wcs(pathname, LC_NAME_zh_CN_UTF8);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, "inputPath[%s], fnPattern[%s]"), oldPathname.c_str(), fnPattern.c_str());

	int count=0;
	std::string uri = pathToUri(pathname);

	//TODO: CdmiFuseOps::CdmiRetCode cdmirc = readDir(uri);

	Json::Value result;
	CdmiFuseOps::CdmiRetCode cdmirc = cdmi_ReadContainer(result, uri);

	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(FindFiles, "cdmi_ReadContainer(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	std::string thisObjId;
	if (result.isMember("objectID"))
		thisObjId = result["objectID"].asString();
	
	// the dir . and .. to comfort dos command
	WIN32_FIND_DATAW findData;
	memset(&findData, 0x00, sizeof(findData));
	findData.dwFileAttributes  = FILE_ATTRIBUTE_DIRECTORY;

	FILETIME ft;
	timeToFileTime(ZQ::common::now(), ft);
	findData.ftCreationTime    = ft;
	findData.ftLastAccessTime  = ft;
	findData.ftLastWriteTime   = ft;

	if (result.isMember("metadata"))
	{
		Json::Value& metadata = result["metadata"];
		int64 t;
		if (metadata.isMember("cdmi_ctime"))
		{
			t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_ctime"].asString().c_str());
			timeToFileTime(t, findData.ftCreationTime);
		}

		if (metadata.isMember("cdmi_mtime"))
		{
			t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_mtime"].asString().c_str());
			timeToFileTime(t, findData.ftLastWriteTime);
		}

		if (metadata.isMember("cdmi_atime"))
		{
			t = ZQ::common::TimeUtil::ISO8601ToTime(metadata["cdmi_atime"].asString().c_str());
			timeToFileTime(t, findData.ftLastAccessTime);
		}

		if (metadata.isMember("cdmi_size"))
		{
			uint64 size =0;
			if (metadata["cdmi_size"].isIntegral())
				size = metadata["cdmi_size"].asInt64();
			else
				size = _atoi64(metadata["cdmi_size"].asString().c_str());

			findData.nFileSizeLow  = (DWORD) (size & 0xffffffff);
			findData.nFileSizeHigh  = (DWORD) (size >> 32);
		}
	}

	if (NULL != FillFindData)
	{
		wcscpy_s(findData.cFileName, sizeof(findData.cFileName) / sizeof(WCHAR) -1, L".");
		FillFindData(&findData, DokanFileInfo);

		wcscpy_s(findData.cFileName, sizeof(findData.cFileName) / sizeof(WCHAR) -1, L"..");
		FillFindData(&findData, DokanFileInfo);
	}

	if (!result.isMember("children"))
		return 0; // empty container

	Json::Value& children = result["children"];

#define BATCH_ATTR_READ
#ifdef BATCH_ATTR_READ
#define DEFAULT_BATCH_NO   (2)
#define MAX_FILE_PER_BATCH (1000)
#define MIN_FILE_PER_BATCH (50)

	size_t filesPerBatch = children.size() / (DEFAULT_BATCH_NO +1);
	if (filesPerBatch >MAX_FILE_PER_BATCH)
		filesPerBatch = MAX_FILE_PER_BATCH;
	if (filesPerBatch <MIN_FILE_PER_BATCH)
		filesPerBatch = MIN_FILE_PER_BATCH;

	StrList filesOfBatch;
	StrList subDirs;

	FindTxn findTxn;
	{
		ZQ::common::Guid gid;
		gid.create();
		char buf[40];
		gid.toCompactIdstr(buf, sizeof(buf)-2);
		findTxn.txnId = buf;
	}

	findTxn.fFillFindData = FillFindData;
	findTxn.pDokanFI = DokanFileInfo;
	findTxn.cFileRead = 0;

	int readerId=0;
	int readOffset=0;

	for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
	{
		std::string subfile = (*itF).asString();
		std::wstring wSubfile, wPath = wPathname + mbs2wcs(subfile, LC_NAME_zh_CN_UTF8);
		subfile = toFilePath(wSubfile, wPath.c_str());
// Aqua3 start supporting search for both folder and files
//		if (!subfile.empty() && LOGIC_FNSEPC == subfile[subfile.length() -1])
//			subDirs.push_back(subfile); // this is a sub-dir
//		else 
			filesOfBatch.push_back(subfile);

		if (filesOfBatch.size() >= filesPerBatch)
		{
			ChildReader* pReader = new ChildReader(*this, ++readerId, thisObjId, pathname, filesOfBatch, findTxn.txnId, readOffset, readOffset + filesOfBatch.size() -1);
			readOffset += filesOfBatch.size();
			if (NULL == pReader)
				continue;
			MAPSET(ReaderMap, findTxn.readers, readerId, pReader); 
			filesOfBatch.clear();
		}
	}

	if (subDirs.size() >0)
	{ 
		// the reader for sub-dirs
		ChildReader* pReader = new ChildReader(*this, ++readerId, "", pathname, subDirs, findTxn.txnId, 0, -1);
		if (NULL != pReader)
			MAPSET(ReaderMap, findTxn.readers, readerId, pReader); 
	}

	if (filesOfBatch.size() >0)
	{
		ChildReader* pReader = new ChildReader(*this, ++readerId, thisObjId, pathname, filesOfBatch, findTxn.txnId, readOffset, readOffset + filesOfBatch.size() -1);
		readOffset += filesOfBatch.size();
		if (NULL != pReader)
			MAPSET(ReaderMap, findTxn.readers, readerId, pReader); 
	}

	int64 stampExp = ZQ::common::now() + 28*1000; // maximal to wait 28sec
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(FindFiles, "starting %d readers as txn[%s] to read children"), findTxn.readers.size(), findTxn.txnId.c_str());
	{
		ZQ::common::MutexGuard g(slkFindTxnMap);
		for (ReaderMap::iterator itR = findTxn.readers.begin(); itR != findTxn.readers.end(); itR++)
			itR->second->start();
		MAPSET(FindTxnMap, sFindTxnMap, findTxn.txnId, findTxn); 
	}

	bool bContinue =true;

	while (bContinue) 
	{
		if (ZQ::common::now() > stampExp)
			bContinue =false;
		else
		{
			ZQ::common::MutexGuard g(slkFindTxnMap);
			FindTxnMap::iterator it = sFindTxnMap.find(findTxn.txnId);
			if (sFindTxnMap.end() == it || it->second.readers.size() <=0)
				bContinue = false;
		}

		::Sleep(100);
	}

	::Sleep(100);

	{
		ZQ::common::MutexGuard g(slkFindTxnMap);
		FindTxnMap::iterator it = sFindTxnMap.find(findTxn.txnId);
		if (sFindTxnMap.end() != it)
		{
			count = it->second.cFileRead;
			for (ReaderMap::iterator itR = it->second.readers.begin(); itR != it->second.readers.end(); itR++)
				itR->second->stop();
			sFindTxnMap.erase(it);
		}
	}

#else
	for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
	{
		std::string subfile = (*itF).asString();
		std::wstring wsubfile = mbs2wcs(subfile, LC_NAME_zh_CN_UTF8);

		memset(&findData, 0x00, sizeof(findData));
		// step 1. fixup the subfilepath with container path
		std::string subfileuri = uri + subfile;

		// step 2. get the file attrs
		BY_HANDLE_FILE_INFORMATION fileInfo;
		if (0 != doGetFileInfo(NULL, (wPathname + wsubfile).c_str(), &fileInfo, true))
			continue;
		
		// step 3. copy file info to findData
		findData.dwFileAttributes  = fileInfo.dwFileAttributes;
		findData.ftCreationTime    = fileInfo.ftCreationTime;
		findData.ftLastAccessTime  = fileInfo.ftLastAccessTime;
		findData.ftLastWriteTime   = fileInfo.ftLastWriteTime;
		findData.nFileSizeHigh     = fileInfo.nFileSizeHigh;
		findData.nFileSizeLow      = fileInfo.nFileSizeLow;

		if (wsubfile.length()>1 && (FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes))
			wsubfile=wsubfile.substr(0, wsubfile.length()-1);

		memcpy(findData.cFileName, wsubfile.c_str(), wsubfile.length() * sizeof(wchar_t));

		// step 4. call to fill the finddata
		if (NULL != FillFindData)
			FillFindData(&findData, DokanFileInfo);

		count++;
	}
#endif // BATCH_ATTR_READ

	cdlog(LOGL_COND_WARN(count!=children.size()), FOPFMT(FindFiles, "read %d of %d children"), count, children.size());
	return 0;
}

int CdmiDokan::doSetEndOfFile(PDOKAN_FILE_INFO  DokanFileInfo, 
							  LPCWSTR			FileName,
							  LONGLONG			ByteOffset)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetEndOfFile, "%lld"), ByteOffset);

	HANDLE handle = (HANDLE) DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetEndOfFile, "invalid handle"));
		return -1;
	}

	Json::Value result;
	std::string uri = pathToUri(pathname), location;
	CdmiFuseOps::CdmiRetCode cdmirc = nonCdmi_UpdateDataObject(uri, location, "", ByteOffset, 0, NULL, ByteOffset);
	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(SetEndOfFile, "nonCdmi_UpdateDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	return 0;
}

int CdmiDokan::doSetAllocationSize(PDOKAN_FILE_INFO DokanFileInfo, 
								   LPCWSTR			FileName,
								   LONGLONG			AllocSize)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetAllocationSize, "subtituting to doSetEndOfFile()"));
	doSetEndOfFile(DokanFileInfo, FileName, AllocSize);
	return 0;
}

int CdmiDokan::doCreateDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR DirName)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, DirName);
	if (pathname.empty() || pathname[pathname.length()-1] != FNSEPC)
	{ pathname+=FNSEPS; wPathname+= L"\\"; }

	// we don't allow RECYCLER or System Volume Information
	static const char* folderNameBlacklist[] = {
		"$RECYCLE.BIN" FNSEPS, 
		"RECYCLER" FNSEPS, 
		"System Volume Information" FNSEPS,
		NULL };

	for (int i =0; folderNameBlacklist[i]; i++)
	{
		// encountered more incompatible on Windows explorer: if (0 == pathname.compare(0, strlen(folderNameBlacklist[i]), folderNameBlacklist[i]))
		if (0 == pathname.compare(folderNameBlacklist[i]))
		{
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateDirectory, "skip folder[%s] that was in blacklist"), pathname.c_str());
			return -1;
		}
	}

	if (0 == pathname.compare("RECYCLER" FNSEPS) || 0 == pathname.compare("System Volume Information"))
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateDirectory, "skip path[%s]"), pathname.c_str());
		return -1;
	}

	std::string uri = pathToUri(pathname);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateDirectory, "call nonCdmi_CreateContainer(%s) to create a new container"), uri.c_str());
	CdmiFuseOps::CdmiRetCode cdmirc = nonCdmi_CreateContainer(uri);
	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(CreateDirectory, "nonCdmi_CreateContainer(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		
		//fix bug 18082, 创建长Folder失败, Aaua返回值为403, 应该为 404
		if(cdmirc_NotFound == cdmirc)
			return -ERROR_PATH_NOT_FOUND;

		return cdmiErrToDokanErr(cdmirc);
	}

	return 0;
}

int CdmiDokan::doOpenDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR DirName)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, DirName);
	if (pathname.empty() || pathname[pathname.length()-1] != FNSEPC)
	{ pathname+=FNSEPS; wPathname+= L"\\"; }

	Json::Value result;
	std::string uri = pathToUri(pathname);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(OpenDirectory, "calling cdmi_ReadContainer(%s) to touch the container"), uri.c_str());
	CdmiFuseOps::CdmiRetCode cdmirc = cdmi_ReadContainer(result, uri);

	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(OpenDirectory, "cdmi_ReadContainer(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	DokanFileInfo->Context = (ULONG64) newHandle(pathname);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(OpenDirectory, "new handle[0x%llx]"), DokanFileInfo->Context);
	return 0;
}

int CdmiDokan::doDeleteDirectory(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR DirName)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, DirName);
	if (pathname.empty() || pathname[pathname.length()-1] != FNSEPC)
	{ pathname+=FNSEPS; wPathname+= L"\\"; }

	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(DeleteDirectory, "calling nonCdmi_DeleteContainer()"));

	std::string uri = pathToUri(pathname);
	CdmiFuseOps::CdmiRetCode cdmirc = nonCdmi_DeleteContainer(uri);

	if (CdmiRet_SUCC(cdmirc))
		return 0;

	cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(DeleteDirectory, "nonCdmi_DeleteContainer(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
	return ERROR_DIR_NOT_EMPTY;
}



int CdmiDokan::doFlushFileBuffers(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
	// not support data cache
	return 0;
}

static void flagsToStr_AccessMode(uint32 flags, std::string& str)
{
	BuildFlagStr(flags, str, GENERIC_READ);
	BuildFlagStr(flags, str, GENERIC_WRITE);
	BuildFlagStr(flags, str, GENERIC_EXECUTE);
	BuildFlagStr(flags, str, DELETE);
	BuildFlagStr(flags, str, FILE_READ_DATA);
	BuildFlagStr(flags, str, FILE_READ_ATTRIBUTES);
	BuildFlagStr(flags, str, FILE_READ_EA);
	BuildFlagStr(flags, str, READ_CONTROL);
	BuildFlagStr(flags, str, FILE_WRITE_DATA);
	BuildFlagStr(flags, str, FILE_WRITE_ATTRIBUTES);
	BuildFlagStr(flags, str, FILE_WRITE_EA);
	BuildFlagStr(flags, str, FILE_APPEND_DATA);
	BuildFlagStr(flags, str, WRITE_DAC);
	BuildFlagStr(flags, str, WRITE_OWNER);
	BuildFlagStr(flags, str, SYNCHRONIZE);
	BuildFlagStr(flags, str, FILE_EXECUTE);
	BuildFlagStr(flags, str, STANDARD_RIGHTS_READ);
	BuildFlagStr(flags, str, STANDARD_RIGHTS_WRITE);
	BuildFlagStr(flags, str, STANDARD_RIGHTS_EXECUTE);
}

static void flagsToStr_ShareMode(uint32 flags, std::string& str)
{
	BuildFlagStr(flags, str, FILE_SHARE_READ);
	BuildFlagStr(flags, str, FILE_SHARE_WRITE);
	BuildFlagStr(flags, str, FILE_SHARE_DELETE);
}

static void flagsToStr_FileAttrs(uint32 flags, std::string& str)
{
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_ARCHIVE);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_ENCRYPTED);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_HIDDEN);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_NORMAL);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_OFFLINE);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_READONLY);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_SYSTEM);
	BuildFlagStr(flags, str, FILE_ATTRIBUTE_TEMPORARY);
	BuildFlagStr(flags, str, FILE_FLAG_WRITE_THROUGH);
	BuildFlagStr(flags, str, FILE_FLAG_OVERLAPPED);
	BuildFlagStr(flags, str, FILE_FLAG_NO_BUFFERING);
	BuildFlagStr(flags, str, FILE_FLAG_RANDOM_ACCESS);
	BuildFlagStr(flags, str, FILE_FLAG_SEQUENTIAL_SCAN);
	BuildFlagStr(flags, str, FILE_FLAG_DELETE_ON_CLOSE);
	BuildFlagStr(flags, str, FILE_FLAG_BACKUP_SEMANTICS);
	BuildFlagStr(flags, str, FILE_FLAG_POSIX_SEMANTICS);
	BuildFlagStr(flags, str, FILE_FLAG_OPEN_REPARSE_POINT);
	BuildFlagStr(flags, str, FILE_FLAG_OPEN_NO_RECALL);
	BuildFlagStr(flags, str, SECURITY_ANONYMOUS);
	BuildFlagStr(flags, str, SECURITY_IDENTIFICATION);
	BuildFlagStr(flags, str, SECURITY_IMPERSONATION);
	BuildFlagStr(flags, str, SECURITY_DELEGATION);
	BuildFlagStr(flags, str, SECURITY_CONTEXT_TRACKING);
	BuildFlagStr(flags, str, SECURITY_EFFECTIVE_ONLY);
	BuildFlagStr(flags, str, SECURITY_SQOS_PRESENT);
}

int CdmiDokan::doCreateFile(PDOKAN_FILE_INFO		DokanFileInfo,
							LPCWSTR					FileName,
							DWORD					AccessMode,
							DWORD					ShareMode,
							DWORD					CreationDisposition,
							DWORD					FlagsAndAttributes)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	std::string user, domain;

	{
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
		flagsToStr_ShareMode(ShareMode, flgstrShareMode);
		flagsToStr_AccessMode(AccessMode, flgstrAccessMode);
		flagsToStr_FileAttrs(FlagsAndAttributes, flgstrFlgAndAttr);

		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "by %s@%s via %s, shareMode[(0x%x)%s] accessMode[(0x%x)%s] attr[(0x%x)%s]"),
			user.c_str(), domain.c_str(), createDispStr.c_str(), ShareMode, flgstrShareMode.c_str(), AccessMode, flgstrAccessMode.c_str(), FlagsAndAttributes, flgstrFlgAndAttr.c_str());
	}

	if (0x1000000 & AccessMode) // bug#17982 to export mounted volume as network share
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "by %s@%s, returning ERROR_PRIVILEGE_NOT_HELD per accessMode[(0x%x)]"),
			user.c_str(), domain.c_str(), AccessMode);
		return (-1) * ERROR_PRIVILEGE_NOT_HELD;
	}

	int err =123;
	Json::Value result;
	CdmiFuseOps::CdmiRetCode cdmirc;
	std::string uri = pathToUri(pathname);

	do {
		if (pathname.empty() || 0 == pathname.compare(FNSEPS))
		{
			// this is the root dir
			if (OPEN_EXISTING == CreationDisposition)
				err =0;
			break;
		}

		if (std::string::npos != pathname.find_first_of("*?[]"))
		{
			// we don't support any wildcast in doCreateFile:
			// DokanFuse/614     | 00001A7C]  CreateFile(d:\temp\hp\*) by hui.shao@SE1-1920, accessMode[(0x80)|FILE_READ_ATTRIBUTES] shareMode[(0x7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
			// DokanFuse/636     | 00001A7C]  CreateFile(d:\temp\hp\*) error(123)
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "wildcast not support, error(%d)"), err);
			break;
		}

		err = 10;
		FileInfo dummyFi;
		if (OPEN_EXISTING == CreationDisposition && !readFileInfoFromCache(pathname, dummyFi)) // only OPEN_EXISTING may be used to touch a subdir
		{
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "trying cdmi_ReadContainer(%s) to touch the container"), uri.c_str());
			cdmirc = cdmi_ReadContainer(result, uri);
			if (CdmiRet_SUCC(cdmirc)) { err =0; break; }
		}

		// this is a file
		err = 20;
		std::string location;
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "calling cdmi_ReadDataObject(%s) to touch the file"), uri.c_str());
		cdmirc = cdmi_ReadDataObject(result, uri +"?metadata", location);

		if (OPEN_EXISTING == CreationDisposition)
		// if (OPEN_ALWAYS== CreationDisposition || OPEN_EXISTING == CreationDisposition)
		{
			if (CdmiRet_SUCC(cdmirc))
				err =0;
			else
			{
				cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(CreateFile, "cdmi_ReadDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
				err = cdmiErrToDokanErr(cdmirc);
			}

			break; 
		}

		err++;
		if (cdmirc_OK == cdmirc && TRUNCATE_EXISTING == CreationDisposition)
		{
			err =0;
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "calling nonCdmi_UpdateDataObject(%s) per TRUNCATE_EXISTING"), uri.c_str());
			cdmirc = nonCdmi_UpdateDataObject(uri, location, "", 0, 0, NULL, 0);
			break;
		}

		err++;
		if (cdmirc_OK == cdmirc && CREATE_NEW == CreationDisposition)
		{
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "calling nonCdmi_DeleteDataObject(%s) to delete the existing per CREATE_NEW"), uri.c_str());
			cdmirc = nonCdmi_DeleteDataObject(uri);
			uncacheFileInfo(pathname);
			// no break; here
		}

		err++;
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CreateFile, "calling nonCdmi_CreateDataObject(%s) to create a new file"), uri.c_str());
		cdmirc = nonCdmi_CreateDataObject(uri, "");
		if (CdmiRet_SUCC(cdmirc))
			err =0;

	} while(0);

	if (err !=0)
	{
		if (SYNCHRONIZE & AccessMode) // move file will have this flag
			err = -2;
		else if (err >0)
			err *= -1;

		cdlog(ZQ::common::Log::L_INFO, FOPFMT(CreateFile, "created: uri[%s] handle[0x%llx] err(%d)"), uri.c_str(), DokanFileInfo->Context, err);
		return err;
	}

	// if file/dir exists on server, put a dummy handle into the context
	DokanFileInfo->Context = (ULONG64) newHandle(pathname);
	cdlog(ZQ::common::Log::L_INFO, FOPFMT(CreateFile, "created: uri[%s] handle[0x%llx]"), uri.c_str(), DokanFileInfo->Context);
	return 0;
}

int CdmiDokan::doCloseFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(CloseFile, "handle[0x%llx]"), DokanFileInfo->Context);
	closeHandle((HANDLE)DokanFileInfo->Context);
	DokanFileInfo->Context = NULL;
	return 0;
}

int CdmiDokan::doCleanup(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "handle[0x%llx] doc[%d]"), DokanFileInfo->Context, DokanFileInfo->DeleteOnClose);
	closeHandle((HANDLE)DokanFileInfo->Context);
	DokanFileInfo->Context = NULL;

	if (DokanFileInfo->DeleteOnClose) 
	{
		std::string uri = pathToUri(pathname), api;
		CdmiFuseOps::CdmiRetCode cdmirc;
		if (DokanFileInfo->IsDirectory)
		{
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "calling nonCdmi_DeleteContainer(%s)"), uri.c_str());
			api = "nonCdmi_DeleteContainer";
			cdmirc = nonCdmi_DeleteContainer(uri);
			if (CdmiRet_SUCC(cdmirc))
				return 0;
		}
		else 
		{
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(Cleanup, "calling nonCdmi_DeleteDataObject(%s)"), uri.c_str());
			api = "nonCdmi_DeleteDataObject";
			cdmirc = nonCdmi_DeleteDataObject(uri);
			uncacheFileInfo(pathname);

			if (CdmiRet_SUCC(cdmirc))
				return 0;
		}

		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(Cleanup, "%s(%s) failed: %s(%d)"), api.c_str(), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
	}

	return -1;
}

int CdmiDokan::doDeleteFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName)
{
	// the true deleting has been covered in doCleanup()
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(DeleteFile, "handle[0x%llx]"), DokanFileInfo->Context);
	return 0;
}

int CdmiDokan::doSetFileAttrs(PDOKAN_FILE_INFO DokanFileInfo,
		LPCWSTR				FileName,
		DWORD				FileAttributes)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	std::string attrstr;
	flagsToStr_FileAttrs(FileAttributes, attrstr);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileAttrs, "attr[0x%x, %s]"), FileAttributes, attrstr.c_str());

	// check the existing attributes
	Json::Value result;
	std::string uri = pathToUri(pathname);
	CdmiFuseOps::CdmiRetCode cdmirc = cdmi_ReadDataObject(result, uri +"?metadata", std::string());
	if (!CdmiRet_SUCC(cdmirc) || !result.isMember("metadata"))
	{
		cdlog(ZQ::common::Log::L_ERROR, FOPFMT(SetFileAttrs, "cdmi_ReadDataObject(%s) for metadata failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	int revision = -1;
	if (result.isMember("version"))
	{
		try {
			revision = result["version"].asInt();
		}
		catch(...) {}
	}

	Json::Value& metadata = result["metadata"];
//	static const Json::Value::iterator itJNil;
//	Json::Value::iterator itOwnerACL =itJNil;
	bool bFindExistingOwner =false;
	Json::Value ownerACE, newACL;
	// {"acetype":"0x00000000","identifier":"OWNER@","aceflags":"0x00000083","acemask":"0xffffffff"},

	uint32 acetype = CDMI_ACE_ACCESS_ALLOWED_TYPE;
	uint32 acemask=0xffffffff;
	// {"acetype":"0x00000000","identifier":"OWNER@","aceflags":"0x00000083","acemask":"0xffffffff"},
	if (metadata.isMember("cdmi_acl"))
	{
		Json::Value& metadataACL = metadata["cdmi_acl"];
		for (Json::Value::iterator it = metadataACL.begin(); it != metadataACL.end(); it++)
		{
			// about aceflags
			uint32 aceflags = 0x00;
			if ((*it).isMember("aceflags"))
				aceflags = readFlags((*it)["aceflags"], ACE_FlagFlags, aceflags);

			// about acemask
			acemask = 0x00;
			if ((*it).isMember("acemask"))
				acemask = readFlags((*it)["acemask"], ACE_FlagMasks, acemask);

			// about acetype
			acetype = 0x00;
			if ((*it).isMember("acetype"))
				acetype = readFlags((*it)["acetype"], ACE_FlagTypes, acetype);
			
			std::string tmpstr; 
			COPY_METADATA_VAL(tmpstr, (*it), identifier, String);
			if (0 == tmpstr.compare("OWNER@"))
			{
				ownerACE = *it;
//				itOwnerACL = it;
				bFindExistingOwner = true;
				continue;
			}

			newACL.append(*it);
		}
	}

	if (!bFindExistingOwner)
	{
		acemask=0xffffffff;
		cdlog(ZQ::common::Log::L_WARNING, FOPFMT(SetFileAttrs, "cdmi_ReadDataObject(%s) didn't find existing OWRNER@ cdmi_acl, initialize with acemask[0x%x]"), uri.c_str(), acemask);
	}
//	else ownerACE = *itOwnerACL;

	if (CDMI_ACE_ACCESS_ALLOWED_TYPE | acetype)
	{
		acemask = ~acemask;
		acetype = CDMI_ACE_ACCESS_ALLOWED_TYPE;
	}

	uint32 mask=1;
	acemask |= CDMI_ACE_READ_OBJECT;
	for (int i=0; i<(sizeof(FileAttributes)*8); i++, mask<<=1)
	{
		switch(FileAttributes & mask)
		{
		case FILE_ATTRIBUTE_ARCHIVE: break;
		case FILE_ATTRIBUTE_ENCRYPTED: break;
		case FILE_ATTRIBUTE_HIDDEN: break;
		case FILE_ATTRIBUTE_NORMAL: break;
		case FILE_ATTRIBUTE_NOT_CONTENT_INDEXED: break;
		case FILE_ATTRIBUTE_OFFLINE: break;
		case FILE_ATTRIBUTE_READONLY: acemask &= ~(CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL); break;
		case FILE_ATTRIBUTE_SYSTEM: break;
		case FILE_ATTRIBUTE_TEMPORARY: break;
		case FILE_FLAG_WRITE_THROUGH: break;
		case FILE_FLAG_OVERLAPPED: break;
		case FILE_FLAG_NO_BUFFERING: break;
		case FILE_FLAG_RANDOM_ACCESS: break;
		case FILE_FLAG_SEQUENTIAL_SCAN: break;
		case FILE_FLAG_DELETE_ON_CLOSE: break;
		case FILE_FLAG_BACKUP_SEMANTICS: break;
		case FILE_FLAG_POSIX_SEMANTICS: break;
		case FILE_FLAG_OPEN_REPARSE_POINT: break;
		case FILE_FLAG_OPEN_NO_RECALL: break;
		case SECURITY_ANONYMOUS: break;
		case SECURITY_IDENTIFICATION: break;
		case SECURITY_IMPERSONATION: break;
		case SECURITY_DELEGATION: break;
		case SECURITY_CONTEXT_TRACKING: break;
		case SECURITY_EFFECTIVE_ONLY: break;
		// case SECURITY_SQOS_PRESENT: break;
		default: break;
		}
	}

	char buf[20];
	std::string strChangeTxn;
	snprintf(buf, sizeof(buf)-2, "0x%08x", acemask);
	try {
		char tmp[100]="";
		snprintf(tmp, sizeof(tmp)-2, "acemask[%s=>0x%08x]", ownerACE["acemask"].asString().c_str(), acemask);
		strChangeTxn = tmp;
	} catch(...) {}
	ownerACE["acemask"]  = buf;
	ownerACE["aceflags"] = "0x03"; // always apply on the current one and the children
	ownerACE["acetype"]  = "0x00"; // always take CDMI_ACE_ACCESS_ALLOWED_TYPE

	newACL.append(ownerACE);
	metadata["cdmi_acl"] =newACL;

	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileAttrs, "calling cdmi_UpdateDataObject(%s) to update metadata, version[%d] %s"), uri.c_str(), revision, strChangeTxn.c_str());
	std::string location;
	cdmirc = cdmi_UpdateDataObjectEx(location, uri+"?metadata", metadata, 0, std::string(""), revision);
	uncacheFileInfo(pathname);

	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(SetFileAttrs, "cdmi_UpdateDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	return 0;
}

int CdmiDokan::doSetFileTime(PDOKAN_FILE_INFO DokanFileInfo, 
							 LPCWSTR				FileName,
							 CONST FILETIME*		CreationTime,
							 CONST FILETIME*		LastAccessTime,
							 CONST FILETIME*		LastWriteTime)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	char buf[100];
	int64 timeint, stampNow = ZQ::common::now();
	Properties metadata;
	std::string timestr;

#define FileTimeToMetaData(_Metadata, _Key, _FT) \
	if (NULL == _FT) timeint =0; else { memcpy(&timeint, _FT, sizeof(timeint)); timeint /= 10000; } \
	if (timeint >0) { ZQ::common::TimeUtil::TimeToUTC(timeint, buf, sizeof(buf)-2); MAPSET(Properties, _Metadata, #_Key, buf); timestr += std::string(#_Key "[") +buf +"] "; } 

	FileTimeToMetaData(metadata, cdmi_ctime, CreationTime);
	FileTimeToMetaData(metadata, cdmi_atime, LastAccessTime);
	FileTimeToMetaData(metadata, cdmi_mtime, LastWriteTime);
	if (timestr.empty())
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileTime, "ignore no changes"));
		return 0;
	}

	// Json::Value result;
	std::string uri = pathToUri(pathname), location;
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileTime, "calling cdmi_UpdateDataObject(%s) to update metadata: %s"), uri.c_str(), timestr.c_str());
	CdmiRetCode cdmirc = cdmi_UpdateDataObject(location, uri, metadata, 0);
	if (!CdmiRet_SUCC(cdmirc))
	{
		cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(SetFileTime, "cdmi_UpdateDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmiErrToDokanErr(cdmirc);
	}

	return 0;
}

int CdmiDokan::doWriteFile(PDOKAN_FILE_INFO DokanFileInfo, 
						   LPCWSTR		FileName,
						   LPCVOID		Buffer,
						   DWORD		NumberOfBytesToWrite,
						   LPDWORD		NumberOfBytesWritten,
						   LONGLONG		Offset)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	uint64	offset = (uint64)Offset;
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "offset[%lld] length[%d]"), offset, NumberOfBytesToWrite);

	bool	newOpened = false;
	*NumberOfBytesWritten =0;

	// step 1. if handle not opened, re-open it temporarily
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		DokanFileInfo->Context = (ULONG64) newHandle(pathname);
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "invalid handle, registered w/ new handle(0x%llx)"), DokanFileInfo->Context);
		newOpened = true;
	}

	Json::Value result;
	FileInfo fi;
	FileInfo_reset(fi);
	CdmiFuseOps::CdmiRetCode cdmirc;
	int err =10;
	std::string uri = pathToUri(pathname);

	do {
		// step 2. adjust offset if it is write from EOF
		if (DokanFileInfo->WriteToEndOfFile)
		{
			// check the fileinfo for the filesize
			cdmirc = cdmi_ReadDataObject(result, uri +"?metadata", std::string());

			if (!CdmiRet_SUCC(cdmirc))
			{
				cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(WriteFile, "cdmi_ReadDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
				err = 0xE000 + cdmirc;
				break;
			}

			char hint[260];
			snprintf(hint, sizeof(hint), "cdmi_ReadDataObject(%s)in WriteFile()", uri.c_str());
			if (parseDataObject(pathname, result, fi, hint) <0)
			{
				cdlog(ZQ::common::Log::L_WARNING, FOPFMT(WriteFile, "illegal DataObject to parse"));
				err = 3; 
				break; 
			}
/*
			if (!result.isMember("metadata"))
			{
				cdlog(ZQ::common::Log::L_WARNING, FOPFMT(WriteFile, "no metadata received"));
				err = 3; 
				break; 
			}

			if (!dataObjMetadataToFstat(result["metadata"], fi))
			{
				cdlog(ZQ::common::Log::L_WARNING, FOPFMT(WriteFile, "failed to covert fstat"));
				err = 4;
				break;
			}
*/
			offset = fi.filestat.st_size;
			cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "seek to offset[%lld] per WriteToEndOfFile"), offset);
		}

		std::string location;
		cdmirc = nonCdmi_UpdateDataObject(uri, location, DEFAULT_CONTENT_MIMETYPE, offset, NumberOfBytesToWrite, (char*) Buffer);

		if (!CdmiRet_SUCC(cdmirc))
		{ 
			cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(WriteFile, "nonCdmi_UpdateDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
			err = 0xE000 + cdmirc;
			break;
		}

		*NumberOfBytesWritten = NumberOfBytesToWrite;
		if ((int64)offset + NumberOfBytesToWrite > fi.filestat.st_size)
			fi.filestat.st_size = offset + NumberOfBytesToWrite;
		
		// succeeded here
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(WriteFile, "data written, offset[%lld] [%d]bytes new size[%lld]"),
			offset, NumberOfBytesToWrite, (int64) fi.filestat.st_size);

		err =0;
	} while (0);

	if (0 == err)
	{
		uncacheFileInfo(pathname);
		return 0;
	}

	if (newOpened)
	{
		// withdraw the handle per failures
		closeHandle((HANDLE) DokanFileInfo->Context);
		DokanFileInfo->Context = NULL;
	}

	// return (-1) * err;
	return cdmiErrToDokanErr(cdmirc);
}

int CdmiDokan::doReadFile(PDOKAN_FILE_INFO DokanFileInfo, 
						  LPCWSTR				FileName,
						  LPVOID				Buffer,
						  DWORD				    Length,
						  LPDWORD				pReadLength,
						  LONGLONG			    Offset)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	HANDLE	handle = (HANDLE) DokanFileInfo->Context;
	uint64	offset = (ULONG) Offset;
	uint len = Length;

	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "handle[0x%llx] offset[%lld], length[%d]"), DokanFileInfo->Context, offset, len);
	bool	newOpened = false;
	*pReadLength =0;

	// step 1. if handle not opened, re-open it temporarily
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		DokanFileInfo->Context = (ULONG64) newHandle(pathname);
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "invalid handle, registered w/ new handle[0x%llx]"), DokanFileInfo->Context);
		newOpened = true;
	}

//	Json::Value result;
//	FileInfo fi;
	CdmiFuseOps::CdmiRetCode cdmirc;
	
	int err =10;
	std::string uri = pathToUri(pathname), location, contentType;
	
	do
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "calling nonCdmi_ReadDataObject() to read [%d]bytes from offset[%lld]"), Length, offset);
		cdmirc = nonCdmi_ReadDataObject(uri, contentType, location, offset, len, (char*)Buffer);
		if (!CdmiRet_SUCC(cdmirc))
		{ 
			if (cdmirc_InvalidRange == cdmirc)
			{
				cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "nonCdmi_ReadDataObject(%s) %s(%d), force to empty buffer"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
				len =0; err = 0;
			}
			else
			{
				cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(ReadFile, "nonCdmi_ReadDataObject(%s) failed: %s(%d)"), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
				err = 0xE000 + cdmirc;
			}

			break;
		}

		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(ReadFile, "read [%d]bytes from offset[%lld], returned [%d]bytes"), Length, offset, len);
		if (len >=0)
			*pReadLength = len;

		err = 0;
		return 0;

	} while(0);

	if (newOpened)
	{
		// withdraw the handle per failures
		closeHandle((HANDLE) DokanFileInfo->Context);
		DokanFileInfo->Context = NULL;
	}

	return (-1) * err;
}

int CdmiDokan::doMoveFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName, // existing file name
						  LPCWSTR NewFileName, BOOL ReplaceIfExisting)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	std::wstring wnewPath; std::string newpath = toFilePath(wnewPath, NewFileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "-> %s"), newpath.c_str());

	std::string rootSvrPath;
//	if (rootSvrPath.empty())
//	{
//		ZQ::common::URLStr urlstr(_rootUrl.c_str());
//		rootSvrPath = std::string("/") + urlstr.getPath();
//	}

	if (DokanFileInfo->Context)
	{
		// should close? or rename at closing?
		closeHandle((HANDLE) DokanFileInfo->Context);
		DokanFileInfo->Context = NULL;
	}

	// test if the source file is a dir or not
	bool bIsDir = false;
	std::string srcuri = pathToUri(pathname);

	Json::Value srcResult;
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "pinging source[%s] via container"), srcuri.c_str());
	CdmiFuseOps::CdmiRetCode cdmirc = cdmi_ReadContainer(srcResult, srcuri); // +"?metadata");

	if (CdmiRet_SUCC(cdmirc))
	{
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "source[%s] is known as a folder: %s(%d)"), srcuri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		if (srcuri.length() >1 && LOGIC_FNSEPC!= srcuri[srcuri.length() -1])
			srcuri += LOGIC_FNSEPS;

		bIsDir = true;
	}

	Json::Value destResult;
	CdmiFuseOps::Properties metadata;
	//if (srcResult.isMember("metadata"))
	//	metadata = srcResult["metadata"];

	srcuri = rootSvrPath + srcuri;
	std::string desturi = pathToUri(newpath);
	if (bIsDir)
	{
		Json::Value exports;
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "calling cdmi_CreateContainer(%s) to create a new container"), desturi.c_str());
		cdmirc = cdmi_CreateContainer(destResult, desturi, metadata, exports, "", "", "", srcuri);
		if (!CdmiRet_SUCC(cdmirc))
		{
			cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(MoveFile, "cdmi_CreateContainer(%s) failed: %s(%d)"), desturi.c_str(), cdmiRetStr(cdmirc), cdmirc);
			return cdmiErrToDokanErr(cdmirc);
		}
	}
	else
	{
		StrList valuetransferencoding;
		cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(MoveFile, "calling cdmi_CreateDataObject(%s) to create a new file"), desturi.c_str());
		cdmirc = cdmi_CreateDataObject(destResult, desturi, "", metadata, "", valuetransferencoding, "", "", "", "", srcuri);
		uncacheFileInfo(pathname);
		if (!CdmiRet_SUCC(cdmirc))
		{
			cdlog(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), FOPFMT(MoveFile, "cdmi_CreateDataObject(%s) failed: %s(%d)"), desturi.c_str(), cdmiRetStr(cdmirc), cdmirc);
			return cdmiErrToDokanErr(cdmirc);
		}
	}

	cdlog(ZQ::common::Log::L_INFO, FOPFMT(MoveFile, "moved %s[%s] to [%s]: srcuri[%s]=>dest[%s]"), (bIsDir?"dir":"file"), pathname.c_str(), newpath.c_str(), srcuri.c_str(), desturi.c_str());
	return 0;
}

int CdmiDokan::doLockFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
						  LONGLONG ByteOffset, LONGLONG Length)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(LockFile, "handle[0x%llx]"), DokanFileInfo->Context);

#pragma message ( __MSGLOC__ "TODO: doLockFile() seems nothing to do w/ CDMI")

	return 0;
}

int CdmiDokan::doUnlockFile(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
							LONGLONG ByteOffset, LONGLONG Length)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(UnlockFile, "handle[0x%llx]"), DokanFileInfo->Context);

#pragma message ( __MSGLOC__ "TODO: doUnlockFile() seems nothing to do w/ CDMI")

	return 0;
}

int CdmiDokan::doGetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
								 PSECURITY_INFORMATION SecurityInfo, PSECURITY_DESCRIPTOR SecurityDcptr,
								 ULONG BufferLength, PULONG LengthNeeded)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(GetFileSecurity, "handle[0x%llx]"), DokanFileInfo->Context);
	HANDLE	handle = (HANDLE) DokanFileInfo->Context;

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		cdlog(ZQ::common::Log::L_WARNING, FOPFMT(GetFileSecurity, "invalid handle"));
		return -1;
	}

#pragma message ( __MSGLOC__ "TODO: doGetFileSecurity() cover WinAPI::GetUserObjectSecurity() via CDMI here")
	return -1; // not supported at the moment
}


int CdmiDokan::doSetFileSecurity(PDOKAN_FILE_INFO DokanFileInfo, LPCWSTR FileName,
								 PSECURITY_INFORMATION SecurityInfo, PSECURITY_DESCRIPTOR SecurityDcptr,
								 ULONG SecurityDescriptorLength)
{
	std::wstring wPathname; std::string pathname = toFilePath(wPathname, FileName);
	cdlog(ZQ::common::Log::L_DEBUG, FOPFMT(SetFileSecurity, "handle[0x%llx]"), DokanFileInfo->Context);
	HANDLE	handle = (HANDLE) DokanFileInfo->Context;

	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		cdlog(ZQ::common::Log::L_WARNING, FOPFMT(SetFileSecurity, "invalid handle"));
		return -1;
	}

#pragma message ( __MSGLOC__ "TODO: doSetFileSecurity() cover WinAPI::SetUserObjectSecurity() via CDMI here")
	return -1; // not supported at the moment
}

int CdmiDokan::doGetDiskFreeSpace(PDOKAN_FILE_INFO DokanFileInfo,
								  PULONGLONG FreeBytesAvailable, PULONGLONG TotalNumberOfBytes, PULONGLONG TotalNumberOfFreeBytes)
{
	int64 freebytes  = MIN_BYTE_SIZE >>2;
	int64 totalbytes = MIN_BYTE_SIZE;
	getDiskSpace(freebytes, totalbytes);

	if (totalbytes < MIN_BYTE_SIZE)
		totalbytes = MIN_BYTE_SIZE;

	*TotalNumberOfBytes = (ULONGLONG) totalbytes;
	*FreeBytesAvailable = *TotalNumberOfFreeBytes = (ULONGLONG) freebytes;
	return 0;
}

