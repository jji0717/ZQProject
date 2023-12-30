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
// Ident : $Id: utils.h,v 1.5 2004/08/09 10:33:51 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : utilities
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/utils.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     08-12-19 17:07 Yixin.tian
// merge for Linux OS
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:35 Fei.huang
// 
// 2     07-09-13 17:25 Fei.huang
// use int in 64bits for file size
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 1     04-12-03 13:56 Jie.zhang
// Revision 1.5  2004/08/09 10:33:51  jshen
// After adjust to UNICODE
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:17:50  jshen
// add comments
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================

#ifndef __FSUTILS_H__
#define __FSUTILS_H__

#include <ZQ_common_conf.h>
#include <sys/stat.h>
#include <string>

#ifdef ZQ_OS_MSWIN
#include <tchar.h>
#endif

#define FSUTILS_MAXPATH 265     //max length of a file/dir path

#ifdef WIN32
#define FSUTILS_SLASHC '\\'    //Windows slash
#define FSUTILS_SLASHS "\\"    //Windows slash
#else
#define FSUTILS_SLASHC '/'     //UNIX slash
#define FSUTILS_SLASHS "/"     //UNIX slash
#endif

class FSUtils  
{
public:
	typedef struct
	{
		long groupid;
		time_t timeaccess;
		time_t timecreate;
		long devnum;
		long mode;
		time_t timemod;
		int64 size;
		long userid;
		long inodnum;   //only useful in UNIX
		long nlinks;    //only useful in UNIX
	} fileInfo_t;

	typedef struct
	{
		double totalbytes;
		double bytesfree;
	} diskInfo_t;

	//directory/fileinfo functions
	static long dirGetFirstFile(const char *dirpath, char *filename, int maxfilename);
	static int dirGetNextFile(long handle, char *filename, int maxfilename);
	static int dirClose(long handle);
	static int dirIsEmpty(char *dirpath);
	static int getFileStats(char *filepath, fileInfo_t *infoptr);
	static int setFileTime(char *filepath, long timemod, long timeaccess = 0);
	static int getUsrName(long uid, char *username, int maxusername);
	static int getGrpName(long gid, char *grpname, int maxgrpname);
	static int getPrmString(long mode, char *prmstr, int maxprmstr);

	//path functions
	static bool buildPath(char* buf, size_t maxsize, const char *rootpath, const char *cwd, const char *arg);
	static int validatePath(char *path);
	static int checkSlashLead(char *path, size_t maxpathsize);
	static int checkSlashEnd(char *path, size_t maxpathsize);
	static bool checkSlashEnd(std::string& path);
	static bool checkSlash(std::string& path, const char *wrkdir = NULL);
	static int checkSlash(char *outpath, const char *wrkdir = NULL);
	static int checkSlashUNIX(char *outpath, const char *wrkdir = NULL);
	static bool checkSlashUNIX(std::string& outpath, const char *wrkdir = NULL);
	static int catPath(char *path, size_t maxpathsize, const char *dir);
	static int getFileName(char *filename, size_t maxfilename, const char *fullpath);
	static int getDirPath(char *dirname, size_t maxdirname, const char *fullpath);
	static char *getRelPathPtr(char *fullpath, char *rootpath);
	static int removeSlashLead(char *path);
	static int removeSlashEnd(char *path);
	static int removeSlashEnd(std::string& path);
	static int chCWD(char *cwd);
	static bool getCWD(std::string& cwd);
	static int getCWD(char *cwd, int maxcwd);

	//basic file functions
	static bool delFile(const char *path);
	static bool mkDir(const char *path);
	static bool rmDir(const char *path);
	static bool rename(const char *oldname, const char *newname);

	//advanced file functions
	static int copyFiles(char *input, char *output);
	static int copySingleFile(char *inputfilename, char *outputfilename);
	static int moveFiles(char *input, char *output);
	static int deleteFiles(char *input, int flagkeepdirs = 0);

	//disk drive information functions
	static bool getDiskInfo(diskInfo_t& diskinfo, const _TCHAR *fullpath);

	static bool matchWildcard(const char *wildcard, const char *filename);

private:
	static bool compactPath(char *path);
	static bool copyPaths(char *outpath, size_t maxpathsize, const char *path1, const char *path2);
	static char *getFileNamePtr(char *path);
	static void createRelativePath(char *path, size_t maxpath);
	static bool verifyOutputDir(char *path, size_t maxpath);
};

#endif //__FSUTILS_H__
