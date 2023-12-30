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
// Ident : $Id: CdmiFuseTest.cpp Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/win/CdmiFuseTest.cpp $
// 
// 5     1/09/13 4:56p Hui.shao
// 
// 4     1/07/13 10:54p Hui.shao
// WriteFile()
// 
// 3     1/07/13 8:36p Hui.shao
// findfiles and getfileinfo
// 
// 2     1/07/13 12:03p Hui.shao
// 
// 1     1/06/13 6:49p Hui.shao
// unit test without turning on FUSE driver
// ===========================================================================

#include "CdmiDokan.h"
#include "FileLog.h"
#include "getopt.h"


class CdmiFuseTest : public CdmiDokan
{
public:
	CdmiFuseTest(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& mountPoint, const std::string& url, uint32 options, const std::string& locale="", uint threadCount=0)
		: CdmiDokan(log, thrdPool, mountPoint, url, options, locale, threadCount)
	{
		memset(&_dokanFileInfo, 0x00, sizeof(_dokanFileInfo));
		_dokanFileInfo.DokanOptions = new DOKAN_OPTIONS;
		if (_dokanFileInfo.DokanOptions)
		{
			memset(_dokanFileInfo.DokanOptions, 0x00, sizeof(DOKAN_OPTIONS));
			_dokanFileInfo.DokanOptions->GlobalContext = (ULONG64) this;
		}

	}

	virtual ~CdmiFuseTest()
	{
		if (_dokanFileInfo.DokanOptions)
			delete _dokanFileInfo.DokanOptions;
		_dokanFileInfo.DokanOptions = NULL;
	}

public:
	void testInitMount();

	void testDosDir();
	void testDosCopyFileInto();
	void testDosCopyFileOut();

protected:
	DOKAN_FILE_INFO	_dokanFileInfo;
};

// #define TEST_SRC_ROOT  L"d:\\temp\\hp\\"
#define TEST_FILENAME  L"ReleaseNote_TianShan1.16.txt"
#define TEST_SRC_ROOT  L""

void CdmiFuseTest::testDosCopyFileInto()
{
	/*
	=> 12-27 17:41:05.612 [    INFO ] [CdmiFuse/151     | 00000C20]  ***** starting FUSE: MP[m:\]=>URL[d:\temp\hp], opt[9]

	12-27 17:41:21.891 [   DEBUG ] [DokanFuse/593     | 00001518]  CreateFile(d:\temp\hp\*) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:21.891 [   DEBUG ] [DokanFuse/615     | 00001518]  error code: 123
	12-27 17:41:21.893 [   DEBUG ] [DokanFuse/593     | 00001960]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(1)|FILE_SHARE_READ] accessMode[(120089)|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|READ_CONTROL|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] attr[(80)|FILE_ATTRIBUTE_NORMAL]
	12-27 17:41:21.893 [   DEBUG ] [DokanFuse/615     | 00001960]  error code: 2
	12-27 17:41:21.895 [   DEBUG ] [DokanFuse/593     | 000016B4]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:21.895 [   DEBUG ] [DokanFuse/615     | 000016B4]  error code: 2

	12-27 17:41:21.896 [   DEBUG ] [DokanFuse/593     | 00001D7C]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via CREATE_ALWAYS, shareMode[(0)] accessMode[(130197)|DELETE|FILE_READ_DATA|FILE_READ_ATTRIBUTES|READ_CONTROL|FILE_WRITE_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA|FILE_APPEND_DATA|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] attr[(2020)|FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED]
	12-27 17:41:21.897 [   ERROR ] [DokanFuse/706     | 00001BD4]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) invalid handle
	12-27 17:41:21.898 [    INFO ] [DokanFuse/692     | 00001518]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) closed
	12-27 17:41:21.898 [   DEBUG ] [DokanFuse/593     | 00001960]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via OPEN_ALWAYS, shareMode[(0)] accessMode[(130197)|DELETE|FILE_READ_DATA|FILE_READ_ATTRIBUTES|READ_CONTROL|FILE_WRITE_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA|FILE_APPEND_DATA|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] attr[(2020)|FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED]
	12-27 17:41:21.899 [   DEBUG ] [CdmiFuse/84      | 000016B4]  GetVolumeInformation()

	12-27 17:41:21.899 [   DEBUG ] [DokanFuse/891     | 00001D7C]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	12-27 17:41:21.899 [   DEBUG ] [DokanFuse/940     | 00001D7C]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 0
	12-27 17:41:21.899 [   DEBUG ] [DokanFuse/1136    | 00001BD4]  SetEndOfFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 1028922
	12-27 17:41:21.901 [   DEBUG ] [DokanFuse/809     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 0, length 65536
	12-27 17:41:21.901 [   DEBUG ] [DokanFuse/849     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 0
	12-27 17:41:21.902 [   DEBUG ] [DokanFuse/809     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 65536, length 65536
	12-27 17:41:21.902 [   DEBUG ] [DokanFuse/849     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 65536
	12-27 17:41:21.902 [   DEBUG ] [DokanFuse/809     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 131072, length 65536
	12-27 17:41:21.903 [   DEBUG ] [DokanFuse/849     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 131072
	12-27 17:41:21.903 [   DEBUG ] [DokanFuse/809     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 196608, length 65536
	12-27 17:41:21.903 [   DEBUG ] [DokanFuse/849     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 196608
	12-27 17:41:21.904 [   DEBUG ] [DokanFuse/809     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 262144, length 65536
	12-27 17:41:21.904 [   DEBUG ] [DokanFuse/849     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 262144
	12-27 17:41:21.905 [   DEBUG ] [DokanFuse/809     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 327680, length 65536
	12-27 17:41:21.905 [   DEBUG ] [DokanFuse/849     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 327680
	12-27 17:41:21.906 [   DEBUG ] [DokanFuse/809     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 393216, length 65536
	12-27 17:41:21.906 [   DEBUG ] [DokanFuse/849     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 393216
	12-27 17:41:21.907 [   DEBUG ] [DokanFuse/809     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 458752, length 65536
	12-27 17:41:21.907 [   DEBUG ] [DokanFuse/849     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 458752
	12-27 17:41:21.907 [   DEBUG ] [DokanFuse/809     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 524288, length 65536
	12-27 17:41:21.908 [   DEBUG ] [DokanFuse/849     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 524288
	12-27 17:41:21.909 [   DEBUG ] [DokanFuse/809     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 589824, length 65536
	12-27 17:41:21.909 [   DEBUG ] [DokanFuse/849     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 589824
	12-27 17:41:21.910 [   DEBUG ] [DokanFuse/809     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 655360, length 65536
	12-27 17:41:21.910 [   DEBUG ] [DokanFuse/849     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 655360
	12-27 17:41:21.911 [   DEBUG ] [DokanFuse/809     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 720896, length 65536
	12-27 17:41:21.911 [   DEBUG ] [DokanFuse/849     | 00001960]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 720896
	12-27 17:41:21.912 [   DEBUG ] [DokanFuse/809     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 786432, length 65536
	12-27 17:41:21.912 [   DEBUG ] [DokanFuse/849     | 000016B4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 786432
	12-27 17:41:21.912 [   DEBUG ] [DokanFuse/809     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 851968, length 65536
	12-27 17:41:21.912 [   DEBUG ] [DokanFuse/849     | 00001D7C]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 851968
	12-27 17:41:21.913 [   DEBUG ] [DokanFuse/809     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 917504, length 65536
	12-27 17:41:21.913 [   DEBUG ] [DokanFuse/849     | 00001BD4]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 65536, offset 917504
	12-27 17:41:21.914 [   DEBUG ] [DokanFuse/809     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) offset 983040, length 45882
	12-27 17:41:21.914 [   DEBUG ] [DokanFuse/849     | 00001518]  WriteFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) write 45882, offset 983040
	12-27 17:41:21.914 [   DEBUG ] [DokanFuse/1221    | 00001960]  SetFileAttributes(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 0x0
	12-27 17:41:21.916 [   DEBUG ] [DokanFuse/1245    | 00001960]  SetFileTime(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	12-27 17:41:21.916 [   ERROR ] [DokanFuse/706     | 000016B4]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) invalid handle
	12-27 17:41:21.916 [    INFO ] [DokanFuse/692     | 00001D7C]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) closed

	12-27 17:41:21.917 [   DEBUG ] [DokanFuse/593     | 00001BD4]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:21.918 [   DEBUG ] [DokanFuse/891     | 00001518]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	12-27 17:41:21.918 [   DEBUG ] [DokanFuse/940     | 00001518]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	12-27 17:41:21.918 [   ERROR ] [DokanFuse/706     | 00001960]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) invalid handle
	12-27 17:41:21.919 [    INFO ] [DokanFuse/692     | 000016B4]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) closed

	12-27 17:41:21.919 [   DEBUG ] [DokanFuse/593     | 00001D7C]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(100100)|FILE_WRITE_ATTRIBUTES|SYNCHRONIZE] attr[(0)]
	12-27 17:41:21.920 [   DEBUG ] [DokanFuse/1221    | 00001BD4]  SetFileAttributes(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 0xa0
	12-27 17:41:21.920 [   DEBUG ] [DokanFuse/1245    | 00001BD4]  SetFileTime(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	12-27 17:41:21.920 [   ERROR ] [DokanFuse/706     | 00001518]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) invalid handle
	12-27 17:41:21.921 [    INFO ] [DokanFuse/692     | 00001960]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) closed
	*/

	doCreateFile(&_dokanFileInfo, L"*", 0x80, 0x7, OPEN_EXISTING, 0x0); // => error code: 123 
	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x120089, 0x1, OPEN_EXISTING, 0x80); // => error code: 2 
	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x80, 0x7, OPEN_EXISTING, 0x0); // => error code: 2 

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x130197, 0x0, CREATE_ALWAYS, 0x2020); // => error code: 123 
	doCleanup(&_dokanFileInfo, TEST_FILENAME);
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x130197, 0x0, OPEN_ALWAYS, 0x2020);
	BY_HANDLE_FILE_INFORMATION fi;
	doGetFileInfo(&_dokanFileInfo, TEST_FILENAME, &fi);
	doSetEndOfFile(&_dokanFileInfo, TEST_FILENAME, 80);

	char buf[1024]="1234567890abcdefghijklmnopqrstuvwxyz";
	DWORD byteToWrite=0;
/*
for (int offset = 0; offset <10222; offset+=byteToWrite)
	{
		byteToWrite = 10222 - offset;
		if (byteToWrite > sizeof(buf))
			byteToWrite = sizeof(buf);

		doWriteFile(&_dokanFileInfo, TEST_FILENAME, buf, byteToWrite, &byteToWrite, offset);
	}
*/
	for (int offset = 0; offset <500; offset+=byteToWrite)
	{
		byteToWrite = 36;
		if (0!=doWriteFile(&_dokanFileInfo, TEST_FILENAME, buf, byteToWrite, &byteToWrite, offset))
			break;
	}

	doSetFileAttrs(&_dokanFileInfo, TEST_FILENAME, 0x0);
	doSetFileTime(&_dokanFileInfo, TEST_FILENAME, NULL, NULL, NULL);
	doCleanup(&_dokanFileInfo, TEST_FILENAME);
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x80, 0x7, OPEN_EXISTING, 0x0); 
	doGetFileInfo(&_dokanFileInfo, TEST_FILENAME, &fi);
	doCleanup(&_dokanFileInfo, TEST_FILENAME); // => invalid handle
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x100100, 0x7, OPEN_EXISTING, 0x0); 
	doSetFileAttrs(&_dokanFileInfo, TEST_FILENAME, 0xa0); 
	doSetFileTime(&_dokanFileInfo, TEST_FILENAME, NULL, NULL, NULL);
	doCleanup(&_dokanFileInfo, TEST_FILENAME); // => invalid handle
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);
}

void CdmiFuseTest::testInitMount()
{
	/*
	=> 12-27 17:41:05.612 [    INFO ] [CdmiFuse/151     | 00000C20]  ***** starting FUSE: MP[m:\]=>URL[d:\temp\hp], opt[9]
	12-27 17:41:08.757 [   DEBUG ] [DokanFuse/593     | 00001960]  CreateFile(d:\temp\hp\autorun.inf) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:08.757 [   DEBUG ] [DokanFuse/615     | 00001960]  error code: 2
	12-27 17:41:08.759 [   DEBUG ] [DokanFuse/593     | 00001BD4]  CreateFile(d:\temp\hp\AutoRun.inf) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:08.759 [   DEBUG ] [DokanFuse/615     | 00001BD4]  error code: 2
	12-27 17:41:08.759 [   DEBUG ] [DokanFuse/650     | 00001518]  OpenDirectory(d:\temp\hp\) 
	12-27 17:41:08.760 [   DEBUG ] [CdmiFuse/84      | 000016B4]  GetVolumeInformation()
	12-27 17:41:08.760 [   DEBUG ] [CdmiFuse/84      | 00001D7C]  GetVolumeInformation()
	12-27 17:41:08.760 [   ERROR ] [DokanFuse/706     | 00001960]  Cleanup(d:\temp\hp\) invalid handle
	12-27 17:41:08.761 [    INFO ] [DokanFuse/692     | 00001BD4]  CloseFile(d:\temp\hp\) closed
	12-27 17:41:08.761 [   DEBUG ] [DokanFuse/650     | 00001518]  OpenDirectory(d:\temp\hp\) 
	12-27 17:41:08.761 [   DEBUG ] [CdmiFuse/84      | 000016B4]  GetVolumeInformation()
	12-27 17:41:08.762 [   DEBUG ] [CdmiFuse/84      | 00001D7C]  GetVolumeInformation()
	12-27 17:41:08.762 [   ERROR ] [DokanFuse/706     | 00001960]  Cleanup(d:\temp\hp\) invalid handle
	12-27 17:41:08.762 [    INFO ] [DokanFuse/692     | 00001BD4]  CloseFile(d:\temp\hp\) closed
	12-27 17:41:08.765 [   DEBUG ] [DokanFuse/650     | 00001518]  OpenDirectory(d:\temp\hp\) 
	12-27 17:41:08.765 [   DEBUG ] [CdmiFuse/84      | 000016B4]  GetVolumeInformation()
	12-27 17:41:08.765 [   DEBUG ] [CdmiFuse/84      | 00001D7C]  GetVolumeInformation()
	12-27 17:41:08.766 [   ERROR ] [DokanFuse/706     | 00001960]  Cleanup(d:\temp\hp\) invalid handle
	12-27 17:41:08.767 [    INFO ] [DokanFuse/692     | 00001BD4]  CloseFile(d:\temp\hp\) closed
	12-27 17:41:08.768 [   DEBUG ] [DokanFuse/593     | 00001518]  CreateFile(d:\temp\hp\) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	12-27 17:41:08.768 [   DEBUG ] [DokanFuse/891     | 000016B4]  GetFileInfo(d:\temp\hp\) 
	12-27 17:41:08.769 [   DEBUG ] [DokanFuse/940     | 000016B4]  GetFileInfo(d:\temp\hp\) GetFileInformationByHandle success, file size: 16384
	12-27 17:41:08.769 [   ERROR ] [DokanFuse/706     | 00001D7C]  Cleanup(d:\temp\hp\) invalid handle
	12-27 17:41:08.770 [    INFO ] [DokanFuse/692     | 00001BD4]  CloseFile(d:\temp\hp\) closed
	12-27 17:41:08.771 [   DEBUG ] [DokanFuse/593     | 00001518]  CreateFile(d:\temp\hp\.svn\wc.db) by hui.shao@SE1-1920 via OPEN_EXISTING, shareMode[(7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE] accessMode[(80)|FILE_READ_ATTRIBUTES] attr[(0)]
	*/
	doCreateFile(&_dokanFileInfo, L"autorun.inf", 0x7, 0x80, OPEN_EXISTING, 0x0); // => error code: 2
	doCreateFile(&_dokanFileInfo, L"autorun.inf", 0x7, 0x80, OPEN_EXISTING, 0x0); // => error code: 2

	WCHAR VolumeName[MAX_PATH], FileSystemName[MAX_PATH];
	DWORD VolumeSN, MaxComponentLen, FileSystemFlags;
	doOpenDirectory(&_dokanFileInfo, TEST_SRC_ROOT);
	doGetVolumeInfo(&_dokanFileInfo, VolumeName, MAX_PATH, FileSystemName, MAX_PATH, &VolumeSN, &MaxComponentLen, &FileSystemFlags);
	doGetVolumeInfo(&_dokanFileInfo, VolumeName, MAX_PATH, FileSystemName, MAX_PATH, &VolumeSN, &MaxComponentLen, &FileSystemFlags);
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT);
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT); // closed

	BY_HANDLE_FILE_INFORMATION fi;
	doCreateFile(&_dokanFileInfo, TEST_SRC_ROOT, 0x7, 0x80, OPEN_EXISTING, 0x0); // => error code: 2
	doGetFileInfo(&_dokanFileInfo, TEST_SRC_ROOT, &fi);
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT); // invalid handle
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT); // closed
}

void CdmiFuseTest::testDosDir()
{
	/*
	01-07 09:53:13.531 [   DEBUG ] [DokanFuse/671     | 00002160]  OpenDirectory(d:\temp\hp\) 
	01-07 09:53:13.531 [   DEBUG ] [DokanFuse/1404    | 00000564]  GetVolumeInformation()
	01-07 09:53:13.531 [   DEBUG ] [DokanFuse/1404    | 00000BD4]  GetVolumeInformation()
	01-07 09:53:13.531 [ WARNING ] [DokanFuse/727     | 00001A7C]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:13.531 [    INFO ] [DokanFuse/713     | 0000077C]  CloseFile(d:\temp\hp\) closed
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/671     | 00002160]  OpenDirectory(d:\temp\hp\) 
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/1404    | 00000564]  GetVolumeInformation()
	01-07 09:53:20.021 [ WARNING ] [DokanFuse/727     | 00000BD4]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:20.021 [    INFO ] [DokanFuse/713     | 00001A7C]  CloseFile(d:\temp\hp\) closed

	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/614     | 0000077C]  CreateFile(d:\temp\hp\) by hui.shao@SE1-1920, accessMode[(0x80)|FILE_READ_ATTRIBUTES] shareMode[(0x7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/912     | 00002160]  GetFileInfo(d:\temp\hp\) 
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/930     | 00002160]  GetFileInfo(d:\temp\hp\) GetFileInformationByHandle success, file size: 16384
	01-07 09:53:20.021 [ WARNING ] [DokanFuse/727     | 00000564]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:20.021 [    INFO ] [DokanFuse/713     | 00000BD4]  CloseFile(d:\temp\hp\) closed
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/614     | 00001A7C]  CreateFile(d:\temp\hp\*) by hui.shao@SE1-1920, accessMode[(0x80)|FILE_READ_ATTRIBUTES] shareMode[(0x7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/636     | 00001A7C]  CreateFile(d:\temp\hp\*) error(123)
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/671     | 0000077C]  OpenDirectory(d:\temp\hp\) 
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/1404    | 00002160]  GetVolumeInformation()
	01-07 09:53:20.021 [ WARNING ] [DokanFuse/727     | 00000564]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:20.021 [    INFO ] [DokanFuse/713     | 00000BD4]  CloseFile(d:\temp\hp\) closed
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/671     | 00001A7C]  OpenDirectory(d:\temp\hp\) 
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/989     | 0000077C]  FindFiles(d:\temp\hp\\*) 
	01-07 09:53:20.021 [   DEBUG ] [DokanFuse/1017    | 0000077C]  FindFiles(d:\temp\hp\\*) FindFiles return 86 entries in d:\temp\hp\\*
	01-07 09:53:20.052 [ WARNING ] [DokanFuse/727     | 00000BD4]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:20.052 [    INFO ] [DokanFuse/713     | 00001A7C]  CloseFile(d:\temp\hp\) closed
	01-07 09:53:20.052 [   DEBUG ] [DokanFuse/671     | 0000077C]  OpenDirectory(d:\temp\hp\) 
	01-07 09:53:20.052 [ WARNING ] [DokanFuse/727     | 00000564]  Cleanup(d:\temp\hp\) invalid handle
	01-07 09:53:20.052 [    INFO ] [DokanFuse/713     | 00000BD4]  CloseFile(d:\temp\hp\) closed
	*/

	BY_HANDLE_FILE_INFORMATION fi;
	WCHAR VolumeName[MAX_PATH], FileSystemName[MAX_PATH];
	DWORD VolumeSN, MaxComponentLen, FileSystemFlags;

	doOpenDirectory(&_dokanFileInfo, TEST_SRC_ROOT);
	doGetVolumeInfo(&_dokanFileInfo, VolumeName, MAX_PATH, FileSystemName, MAX_PATH, &VolumeSN, &MaxComponentLen, &FileSystemFlags);
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT);
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT); // closed
	doCreateFile(&_dokanFileInfo, TEST_SRC_ROOT, 0x80, 0x7, OPEN_EXISTING, 0x0);
	doGetFileInfo(&_dokanFileInfo, TEST_SRC_ROOT, &fi); // =>success
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT); // closed

	doCreateFile(&_dokanFileInfo, TEST_SRC_ROOT L"*", 0x80, 0x7, OPEN_EXISTING, 0x0); //=> error 123
	doOpenDirectory(&_dokanFileInfo, TEST_SRC_ROOT);
	doGetVolumeInfo(&_dokanFileInfo, VolumeName, MAX_PATH, FileSystemName, MAX_PATH, &VolumeSN, &MaxComponentLen, &FileSystemFlags);
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT);
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT);

	doOpenDirectory(&_dokanFileInfo, TEST_SRC_ROOT);
	doFindFiles(&_dokanFileInfo, TEST_SRC_ROOT L"\\*", NULL); // => return 86 entries in d:\temp\hp\\*
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT);
	doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT);
}

void CdmiFuseTest::testDosCopyFileOut()
{
	/*
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/615     | 00001424]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920, accessMode[(0x80)|FILE_READ_ATTRIBUTES] shareMode[(0x7)|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/913     | 00001D98]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/931     | 00001D98]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/732     | 00002138]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/708     | 00001BA4]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt)

	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/672     | 00000F58]  OpenDirectory(d:\temp\hp\) 
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/990     | 00001424]  FindFiles(d:\temp\hp\\*) 
	01-07 11:02:31.226 [   DEBUG ] [DokanFuse/1018    | 00001424]  FindFiles(d:\temp\hp\\*) FindFiles return 86 entries in d:\temp\hp\\*

	01-07 11:02:31.242 [   DEBUG ] [DokanFuse/615     | 00001D98]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920, accessMode[(0x120089)|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|READ_CONTROL|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] shareMode[(0x3)|FILE_SHARE_READ|FILE_SHARE_WRITE], OPEN_EXISTING, attr[(0x80)|FILE_ATTRIBUTE_NORMAL]
	01-07 11:02:31.242 [   DEBUG ] [DokanFuse/773     | 00002138]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) reading 512B from 0x0
	01-07 11:02:31.242 [    INFO ] [DokanFuse/809     | 00002138]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) read 512byte from offset[0x0]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/615     | 00001BA4]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920, accessMode[(0x120089)|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|READ_CONTROL|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] shareMode[(0x5)|FILE_SHARE_READ|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00000F58]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00000F58]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/732     | 00001424]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/708     | 00001D98]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 

	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/615     | 00002138]  CreateFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) by hui.shao@SE1-1920, accessMode[(0x120089)|FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA|READ_CONTROL|SYNCHRONIZE|STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_EXECUTE] shareMode[(0x5)|FILE_SHARE_READ|FILE_SHARE_DELETE], OPEN_EXISTING, attr[(0x0)]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00001BA4]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00001BA4]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00000F58]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00000F58]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00001424]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00001424]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00001D98]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00001D98]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/913     | 00002138]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/931     | 00002138]  GetFileInfo(d:\temp\hp\ReleaseNote_TianShan1.16.txt) GetFileInformationByHandle success, file size: 1028922
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/1401    | 00001BA4]  GetVolumeInformation()

	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/773     | 00000F58]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) reading 262144B from 0x0
	01-07 11:02:36.031 [    INFO ] [DokanFuse/809     | 00000F58]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) read 262144byte from offset[0x0]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/773     | 00001424]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) reading 262144B from 0x40000
	01-07 11:02:36.031 [    INFO ] [DokanFuse/809     | 00001424]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) read 262144byte from offset[0x40000]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/773     | 00001D98]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) reading 262144B from 0x80000
	01-07 11:02:36.031 [    INFO ] [DokanFuse/809     | 00001D98]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) read 262144byte from offset[0x80000]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/773     | 00002138]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) reading 262144B from 0xc0000
	01-07 11:02:36.031 [    INFO ] [DokanFuse/809     | 00002138]  ReadFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) read 242490byte from offset[0xc0000]
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/732     | 00001BA4]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/708     | 00000F58]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/732     | 00001424]  Cleanup(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/708     | 00001D98]  CloseFile(d:\temp\hp\ReleaseNote_TianShan1.16.txt) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/732     | 00001BA4]  Cleanup(d:\temp\hp\) 
	01-07 11:02:36.031 [   DEBUG ] [DokanFuse/708     | 00000F58]  CloseFile(d:\temp\hp\) 
	*/

	BY_HANDLE_FILE_INFORMATION fi;
	WCHAR VolumeName[MAX_PATH], FileSystemName[MAX_PATH];
	DWORD VolumeSN, MaxComponentLen, FileSystemFlags;

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x80, 0x7, OPEN_EXISTING, 0x0); // => succ
	doGetFileInfo(&_dokanFileInfo, TEST_FILENAME, &fi);
	doCleanup(&_dokanFileInfo, TEST_FILENAME);
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);

	doOpenDirectory(&_dokanFileInfo, TEST_SRC_ROOT);
	doFindFiles(&_dokanFileInfo, TEST_SRC_ROOT L"\\*", NULL); // => return 86 entries in d:\temp\hp\\*
	doCleanup(&_dokanFileInfo, TEST_SRC_ROOT);
	// doCloseFile(&_dokanFileInfo, TEST_SRC_ROOT);

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x120089, 0x3, OPEN_EXISTING, 0x80); // => succ
	char buf[64*1024];
	DWORD byteToRead=512;
	doReadFile(&_dokanFileInfo, TEST_FILENAME, buf, byteToRead, &byteToRead, 0); // reading 512B from 0x0

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x120089, 0x5, OPEN_EXISTING, 0x0); // => succ
	doGetFileInfo(&_dokanFileInfo, TEST_FILENAME, &fi);
	doCleanup(&_dokanFileInfo, TEST_FILENAME);
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);

	doCreateFile(&_dokanFileInfo, TEST_FILENAME, 0x120089, 0x5, OPEN_EXISTING, 0x0); // => succ
	doGetFileInfo(&_dokanFileInfo, TEST_FILENAME, &fi);
	doGetVolumeInfo(&_dokanFileInfo, VolumeName, MAX_PATH, FileSystemName, MAX_PATH, &VolumeSN, &MaxComponentLen, &FileSystemFlags);

	int64 size = fi.nFileSizeHigh;
	size <<=32; size += fi.nFileSizeLow;

	for (int64 offset = 0; offset < size; offset+=byteToRead)
	{
		int64 left = size - offset;
		byteToRead = (left > sizeof(buf)) ? sizeof(buf) : (DWORD) left;

		doReadFile(&_dokanFileInfo, TEST_FILENAME, buf, byteToRead, &byteToRead, offset);
	}

	doCleanup(&_dokanFileInfo, TEST_FILENAME); // => invalid handle
	doCloseFile(&_dokanFileInfo, TEST_FILENAME);
}