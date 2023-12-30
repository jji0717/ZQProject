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
// Ident : $Id: utils.cpp,v 1.5 2004/08/09 10:33:51 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : utilities
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/utils.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     09-05-22 17:43 Yixin.tian
// 
// 3     08-12-19 17:07 Yixin.tian
// merge for Linux OS
// 
// 2     08-03-28 16:12 Build
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:35 Fei.huang
// added getFileStats for Vstream storage
// 
// 2     07-09-13 17:24 Fei.huang
// get file size by vstream IO
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 1     05-09-06 13:58 Jie.zhang
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 3     04-11-19 17:01 Jie.zhang
// Revision 1.5  2004/08/09 10:33:51  jshen
// After adjust to UNICODE
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:17:54  jshen
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

extern "C"
{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#include <ctype.h>
#include <stdarg.h> //for va_list, etc.

#ifdef WIN32
#include <conio.h>  //for getch()
#include <io.h>
#include <direct.h>
#include <sys/utime.h>
#else
#include <glob.h>
#include <unistd.h>
#include <grp.h>  //for getgrgid
#include <pwd.h>  //for getpwuid
#include <utime.h>
#endif

#ifdef SOLARIS
#include <sys/statvfs.h>
#endif

#ifdef BSD
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#define fsbtoblk(num, fsbs, bs) (((fsbs) != 0 && (fsbs) < (bs)) ? (num) / ((bs) / (fsbs)) : (num) * ((fsbs) / (bs)))
#endif

#ifdef LINUX
#include <sys/vfs.h>
#endif

};

//Needed for getDiskInfo()
#ifdef WIN32
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif 
#endif


#include "utils.h"


//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// Functions used for
// directory/fileinfo
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Get the first filename in a directory listing.
//
// [in] dirpath     : Path of the directory to list.
// [out] filename   : First filename in the directory listing.
// [in] maxfilename : Max size of the "filename" string.
//
// Return : On success a handle to the directory listing is returned.
//          On failure 0 is returned.
//
long FSUtils::dirGetFirstFile(const char *dirpath, char *filename, int maxfilename)
{
	long handle = 0;
	char dirpathbuff[FSUTILS_MAXPATH];

	if (dirpath == NULL)
		return false;

	strcpy(dirpathbuff,dirpath);    //make a local copy of dirpath

	if (strchr(dirpathbuff,'*') == NULL)
	{
		//check if wildcard
		checkSlashEnd(dirpathbuff,strlen(dirpath)+3); //make sure the path ends in a slash (if no wildcard)
		strcat(dirpathbuff,"*");    //add the wildcard (if not already done)
	}

	checkSlash(dirpathbuff);        //make sure the path has correct slashes

#ifdef ZQ_OS_MSWIN
	__int64 hdir;
	struct _finddata_t fileinfo;

	if ((hdir = _findfirst(dirpathbuff,&fileinfo)) == -1L)
		return false;                //error opening the dir

	handle = (long) hdir;
	if (filename != NULL)
	{
		strncpy(filename, fileinfo.name, maxfilename-1);
		*(filename + maxfilename-1) = '\0';
	}
#else
	glob_t tmpGlob;

	//No paths were matched
	if (glob(dirpathbuff,0,NULL,&tmpGlob) != 0 || tmpGlob.gl_pathc == 0)
		return false;

	handle = (long)(&tmpGlob);
	if (filename != NULL)
	{
		//only copy the filename (not the path)
		getFileName(tmpGlob.gl_pathv[0], strlen(tmpGlob.gl_pathv[0])+1,tmpGlob.gl_pathv[0]);
		strncpy(filename,tmpGlob.gl_pathv[0],maxfilename-1);
		*(filename + maxfilename-1) = '\0';
	}

	tmpGlob.gl_offs = 1;   //set the offset

#endif

	return handle;
}

//////////////////////////////////////////////////////////////////////
// Get the next filename in a directory listing.
//
// [in] handle      : Handle to the directory listing.
// [out] filename   : Next filename in the directory listing.
// [in] maxfilename : Max size of the "filename" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::dirGetNextFile(long handle, char *filename, int maxfilename)
{
#ifdef ZQ_OS_MSWIN
	struct _finddata_t fileinfo;

	if (_findnext(handle,&fileinfo) == -1L)
		return false;
	if (filename != NULL)
	{
		strncpy(filename,fileinfo.name,maxfilename-1);
		*(filename+maxfilename-1) = '\0';
	}
#else
	glob_t *pglob = (glob_t *)handle;

	if (pglob->gl_pathv[pglob->gl_offs] == NULL)
		return false;
	if (filename != NULL)
	{
		//only copy the filename (not the path)
		getFileName(pglob->gl_pathv[pglob->gl_offs],strlen(pglob->gl_pathv[pglob->gl_offs])+1,pglob->gl_pathv[pglob->gl_offs]);
		strncpy(filename,pglob->gl_pathv[pglob->gl_offs],maxfilename-1);
		*(filename+maxfilename-1) = '\0';
	}
	(pglob->gl_offs)++;   //increment the offset
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////
// Close the handle to the directory listing.
//
// [in] handle : Handle to the directory listing.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::dirClose(long handle)

{
#ifdef ZQ_OS_MSWIN
	_findclose(handle);
#else
	((glob_t *)handle)->gl_offs = 0;  //reset the offset
	globfree((glob_t *)handle);
	delete (glob_t *)handle;
#endif

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Checks if a directory is "".
//
// [in] dirpath : Path of the directory to check.
//
// Return : Returns 1 if the directory is not "" and 0 if the
//          directory is "" or does not exist.
//
int FSUtils::dirIsEmpty(char *dirpath)
{
	long handle;
	char buffer[3];

	if ((handle = dirGetFirstFile(dirpath,buffer,sizeof(buffer))) <= 0)
		return false;
	do
	{
		if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
		{
			dirClose(handle);
			return true;
		}
	} while (dirGetNextFile(handle,buffer,sizeof(buffer)) > 0);
	dirClose(handle);

	return false;
}

//////////////////////////////////////////////////////////////////////
// Gets information about a file.
//
// [in] filepath : Path to the file.
// [out] infoptr : Structure containing the file information.
//
// Return : On success 1 is returned.  0 is returned if there was as
//          error accessing the file or the file does not exist.
//
int FSUtils::getFileStats(char *filepath, fileInfo_t *infoptr)
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

#ifdef ZQ_OS_MSWIN
	const char* f = (*filepath == '\\' || *filepath == '/') ? filepath + 1 : filepath;

// 	if(VStrmIO::getFileStats(f, infoptr)) {
// 		return true;
// 	}
	
	struct _stat filestatus;
	if (_stat(filepath,&filestatus) != 0)
	{
		if (flagslashend != 0)
			checkSlashEnd(filepath,len+1); //restore the trailing slash
		return false;
	}
#else
	struct stat filestatus;
	if (stat(filepath,&filestatus) != 0)
	{
		if (flagslashend != 0)
			checkSlashEnd(filepath,len+1); //restore the trailing slash
		return false;
	}
#endif

	memset(infoptr,0,sizeof(fileInfo_t));
	infoptr->timeaccess = filestatus.st_atime;
	infoptr->timecreate = filestatus.st_ctime;
	infoptr->timemod = filestatus.st_mtime;
	infoptr->size = filestatus.st_size;
	infoptr->groupid = filestatus.st_gid;
	infoptr->userid = filestatus.st_uid;
	infoptr->mode = filestatus.st_mode;
	infoptr->devnum = filestatus.st_dev;
	infoptr->inodnum = filestatus.st_ino;   //only useful in UNIX
	infoptr->nlinks = filestatus.st_nlink;  //only useful in UNIX (always 1 in WINDOWS)

	if (flagslashend != 0)
		checkSlashEnd(filepath,len+1);   //restore the trailing slash
	return true;
}





//////////////////////////////////////////////////////////////////////
// Sets the file modification and access time.
//
// [in] filepath   : Path to the file.
// [in] timemode   : Time to set the file modification time to.
// [in] timeaccess : Time to set the file access time to.
//                   By default the access time will be set to the
//                   same values as the modification time.
//
// Return : On success 1 is returned.  0 is returned if there was as
//          error accessing the file or the file does not exist.
//
int FSUtils::setFileTime(char *filepath, long timemod, long timeaccess /*=0*/)
{
	long accesstime;

	//by default set the access time to the modification time
	accesstime = (timeaccess == 0) ? timemod : timeaccess;

#ifdef ZQ_OS_MSWIN
	struct _utimbuf utimebuffer;
	utimebuffer.modtime = timemod;
	utimebuffer.actime = accesstime;
	if (_utime(filepath,&utimebuffer) != 0)
		return false;
#else
	struct utimbuf utimebuffer;
	utimebuffer.modtime = timemod;
	utimebuffer.actime = accesstime;
	if (utime(filepath,&utimebuffer) != 0)
		return false;
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////
// Gets the user name based on the user ID (for UNIX).
//
// [in] uid         : The user ID.
// [out] username   : Name of the user corresponding to uid.
// [in] maxusername : Max size of the "username" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::getUsrName(long uid, char *username, int maxusername)
{

	if (username == NULL || maxusername == 0)
		return false;

#ifndef WIN32
	struct passwd *pw;

	if ((pw = getpwuid(uid)) != NULL)
	{
		strncpy(username,pw->pw_name,maxusername-1);
		username[maxusername-1] = '\0';
		return true;
	}
#endif

	//use generic user by default
	strncpy(username,"owner",maxusername-1);
	username[maxusername-1] = '\0';

	return true;
}

//////////////////////////////////////////////////////////////////////
// Gets the group name based on the group ID (for UNIX).
//
// [in] gid        : The group ID.
// [out] grpname   : Name of the group corresponding to gid.
// [in] maxgrpname : Max size of the "grpname" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::getGrpName(long gid, char *grpname, int maxgrpname)
{

	if (grpname == NULL || maxgrpname == 0)
		return false;

#ifndef WIN32
	struct group *grp;

	if ((grp = getgrgid(gid)) != NULL)
	{
		strncpy(grpname,grp->gr_name,maxgrpname-1);
		grpname[maxgrpname-1] = '\0';
		return true;
	}
#endif

	//use generic group by default
	strncpy(grpname,"group",maxgrpname-1);
	grpname[maxgrpname-1] = '\0';

	return true;
}

//////////////////////////////////////////////////////////////////////
// Get the permission string based on the mode for the file/directory.
//
// [in] mode      : The mode of the file/directory.
// [out] prmstr   : Permission string for the file or directory.
// [in] maxprmstr : Max size of the "prmstr" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// Example: (mode & 0777) = 0755 -> "rwxr-xr-x"
//
int FSUtils::getPrmString(long mode, char *prmstr, int maxprmstr)
{
	char *ptr = prmstr;

	if (prmstr == NULL || maxprmstr == 0)
		return false;

	*prmstr = '\0';
	if (maxprmstr < 10)
		return false;      //permissions string must be at least 10 chars

	//Build the permission string one char at a time
	*(ptr++) = (mode & 0400) ? 'r' : '-';
	*(ptr++) = (mode & 0200) ? 'w' : '-';
	*(ptr++) = (mode & 0100) ? 'x' : '-';
	*(ptr++) = (mode & 0040) ? 'r' : '-';
	*(ptr++) = (mode & 0020) ? 'w' : '-';
	*(ptr++) = (mode & 0010) ? 'x' : '-';
	*(ptr++) = (mode & 0004) ? 'r' : '-';
	*(ptr++) = (mode & 0002) ? 'w' : '-';
	*(ptr++) = (mode & 0001) ? 'x' : '-';
	*ptr = '\0';    //terminate the string

	return true;
}

////////////////////////////////////////
// Functions used for building paths
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Creates the path based on the inputs "rootdir," "cwd," and
// "arg."  The entire path is returned.  The buffer returned has
// one extra byte in case a trailing slash (or other char) needs to
// be added.
//
// [in] rootpath : Base path of the path to be built.
// [in] cwd      : CWD relative to the base path.
// [in] arg      : New file/directory path (relative to CWD).
//
// Return : On success the full path to the new directory is returned.
//          On failure NULL is returned.
//
// NOTE: All directory paths should always end in a slash.  "cwd" should
//       never start with a slash.
//
// Example: rootpath = /ftpd/, cwd = site/files/upload/, 
//          arg = ../download/file1.dat
//          --> /ftpd/site/files/download/file1.dat
//
bool FSUtils::buildPath(char* buf, size_t maxsize, const char *rootpath, const char *cwd, const char *arg)
{
	char *argbuff, *tmppath;

	if (rootpath == NULL || cwd == NULL)
		return false;

	//if no arguments are present, do nothing
	if (arg == NULL || *arg == '\0')
		return copyPaths(buf,maxsize,rootpath,cwd);

	if ((argbuff = new char[strlen(arg)+2]) == NULL)
		return false;
	strcpy(argbuff,arg);    //make a copy of the arguments

	//if the path ends in a '.' add an ending slash
	if (*(argbuff+strlen(argbuff)-1) == '.')
		checkSlashEnd(argbuff,strlen(arg)+2);

	if ((tmppath = new char[strlen(cwd)+strlen(argbuff)+1]) == NULL)
	{
		delete[] argbuff;
		return false;
	}

	*tmppath = '\0';
	checkSlash(argbuff);
	switch (*argbuff)
	{
	case FSUTILS_SLASHC:
		{
			if (*(argbuff+1) != '\0')  //if there was more than '\' or '/'
				strcat(tmppath,argbuff+1);
		}
		break;
	default:
		{
			strcpy(tmppath,cwd);
			strcat(tmppath,argbuff);
		}
		break;
	} // end switch

	compactPath(tmppath); //make an absolute path

	//NOTE: add +2 in case a trailing slash is added
	copyPaths(buf,strlen(rootpath)+strlen(tmppath)+1,rootpath,tmppath);

	delete[] argbuff;
	delete[] tmppath;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Checks if a path exists.
//
// [in] path : Path to be checked.
//
// Return : Returns 0 if the path DNE.  Returns 1 if path is a
//          directory and 2 if a file.
//
int FSUtils::validatePath(char *path)
{
	fileInfo_t fileinfo;
	int retval;

	if (getFileStats(path,&fileinfo) != 0)
		retval = (fileinfo.mode & S_IFDIR) ? 1 : 2;
	else retval = 0;     //path DNE

	return retval;
}

//////////////////////////////////////////////////////////////////////
// Makes sure the path starts with '/' or '\'.
//
// [in] path        : Path to be checked.
// [in] maxpathsize : Max size of the "path" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// NOTE: The type of slash will depend on the local platform.
//       Also, the "path" buffer must be large enough to contain
//       an added slash character.
//
int FSUtils::checkSlashLead(char *path, size_t maxpathsize)
{
	char buffer[FSUTILS_MAXPATH];

	if (path == NULL || maxpathsize == 0)
		return false;

	if (*path == '\\' || *path == '/')
		return true;  //path already starts with a slash

	if ((unsigned)maxpathsize < (strlen(path)+2))
		return false;  //path is not big enough

	strcpy(buffer,path);

	strcpy(path, FSUTILS_SLASHS);
	strcat(path, buffer);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Makes sure the path ends in '/' or '\'.
//
// [in] path        : Path to be checked.
// [in] maxpathsize : Max size of the "path" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// NOTE: The type of slash will depend on the local platform.
//       Also, the "path" buffer must be large enough to contain
//       an added slash character.
//
int FSUtils::checkSlashEnd(char *path, size_t maxpathsize)
{
	if (path == NULL || maxpathsize == 0)
		return false;

	size_t len = strlen(path);

	if (len > 0 && (path[len-1] == '\\' || path[len-1] == '/'))
		return true;  //path already ends in a slash

	if (maxpathsize < (len+2))
		return false;  //path is not big enough

	//make sure the path ends in a '\' or '/'
	strcat(path,FSUTILS_SLASHS);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Converts any '/' to '\' for WINDOWS or '\' to '/' for UNIX.
// "wrkdir" is used to avoid overwriting the string.
// Set "wrkdir" to NULL to use "outpath" only.
//
// [in/out] outpath : Path with the new slashes.  It is also an input
//                    when "wrkdir" is NULL.
// [in] wrkdir      : Path the slashes will be converted on.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// NOTE: If "wrkdir" is used, "outpath" must be at least the
//       same size as "wrkdir."
//
bool FSUtils::checkSlashEnd(std::string& path)
{
	size_t len = path.length();

	if (len > 0)
	{
		if (path[len-1] == '\\' || path[len-1] == '/')
			return true;  //path already ends in a slash
	}

	path += FSUTILS_SLASHS;    //for Windows

	return true;
}

bool FSUtils::checkSlash(std::string& path, const char *wrkdir /*=NULL*/)
{
	char pathbuf[MAX_PATH*2];
	strcpy(pathbuf, path.c_str());
	if(checkSlash(pathbuf, wrkdir))
	{
		path = pathbuf;
		return true;
	}
	path = "";
	return false;
}

int FSUtils::checkSlash(char *outpath, const char *wrkdir /*=NULL*/)
{
	char *ptr, *pathptr;

	if (outpath == NULL)
		return false;

	if (wrkdir != NULL)
		strcpy(outpath,wrkdir);

	//convert all '/' to '\' for WINDOWS and '\' to '/' for UNIX
	pathptr = outpath;

	while ((ptr = strchr(pathptr,'/')) != NULL)
	{
		*ptr = FSUTILS_SLASHC;
		pathptr = ptr + 1;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Similar to checkSlash() except it always outputs '/' slashes.
//
// [in/out] outpath : Path with the new slashes.  It is also an input
//                    when "wrkdir" is NULL.
// [in] wrkdir      : Path the slashes will be converted on.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// NOTE: If "wrkdir" is used, "outpath" must be at least the
//       same size as "wrkdir."
//
int FSUtils::checkSlashUNIX(char *outpath, const char *wrkdir /*=NULL*/)
{
	char *ptr;

	if (outpath == NULL)
		return false;

	if (wrkdir != NULL)
		strcpy(outpath,wrkdir);

	while ((ptr = strchr(outpath,'\\')) != NULL)
	{
		*ptr = '/';
		ptr++;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Adds a directory path to a base path.
//
// [in/out] path    : Base path the directory should be appended to.
// [in] maxpathsize : Max size of the "path" string.
// [in] dir         : Directory to append to "path".
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool FSUtils::checkSlashUNIX(std::string& outpath, const char *wrkdir /*= NULL*/)
{
	if (wrkdir != NULL)
		outpath = wrkdir;

	char* buf = new char[outpath.length() +1];
	if (buf == NULL)
		return false;
	strcpy(buf, outpath.c_str());

	for (char* ptr = buf; (ptr = strchr(buf,'\\')) != NULL; ptr++)
		*ptr = '/';

	outpath = buf;
	delete[] buf;

	return true;
}


int FSUtils::catPath(char *path, size_t maxpathsize, const char *dir)
{
	char *ptr;

	if (path == NULL || dir == NULL)
		return false;

	if ((ptr = strrchr(path,FSUTILS_SLASHC)) == NULL)
		return false;

	*(ptr+1) = '\0';
	size_t len = strlen(path);
	strncat(path,dir,maxpathsize-1-len);
	path[maxpathsize-1] = '\0';

	return true;
}

//////////////////////////////////////////////////////////////////////
// Returns only the filename from the full path.
//
// [out] filename   : Filename portion of the full path.
// [in] maxfilename : Max size of the "filename" string.
// [in] fullpath    : Path to extract the filename from.
//
// Return : On success 1 is returned.  Returns 0 if the path is
//          invalid.
//
int FSUtils::getFileName(char *filename, size_t maxfilename, const char *fullpath)
{
	char *ptr, pathbuf[FSUTILS_MAXPATH];

	if (filename == NULL || maxfilename == 0 || fullpath == NULL)
		return false;

	//check to see if the fullpath is valid
	if (strstr(fullpath,"\\\\") != NULL || strstr(fullpath,"//") != NULL)
		return false;  //invalid path

	strcpy(pathbuf,fullpath);

	if ((ptr = strrchr(pathbuf, FSUTILS_SLASHC)) == NULL)
	{
		strncpy(filename, pathbuf, maxfilename-1);
		filename[maxfilename-1] = '\0';
	}
	else
	{
		strncpy(filename, (ptr+1), maxfilename-1);
		filename[maxfilename-1] = '\0';
	}

	//Do not allow any file names with '\' or '/' in it.
	if (strchr(filename,FSUTILS_SLASHC) != NULL || *filename == '\0')
		return false;  //invalid filename

	return true;
}

//////////////////////////////////////////////////////////////////////
// Returns only the directory name from the full path.
//
// [out] dirname   : Directory name portion of the full path.
// [in] maxdirname : Max size of the "dirname" string.
// [in] fullpath   : Path to extract the directory name from.
//
// Return : On success 1 is returned.  Returns 0 if the path is
//          invalid.
//
int FSUtils::getDirPath(char *dirname, size_t maxdirname, const char *fullpath)
{
	char *ptr, pathbuf[FSUTILS_MAXPATH];

	if (dirname == NULL || maxdirname == 0 || fullpath == NULL)
		return false;

	strcpy(pathbuf,fullpath);

	if ((ptr = strrchr(pathbuf,FSUTILS_SLASHC)) == NULL)
	{
		*dirname = '\0';    //the fullpath is a filename
		return true;
	}

	if ((ptr-pathbuf+2) <= (long)maxdirname)
	{
		strncpy(dirname, pathbuf, ptr - pathbuf +1);   //copy the dir part
		dirname[ptr - pathbuf +1] = '\0';
	}
	else
		*dirname = '\0';    //the buffer is too small

	return true;
}

//////////////////////////////////////////////////////////////////////
// Returns a pointer into "fullpath" after the "rootpath" part.
//
// [in] fullpath : Full path to get a pointer into.
// [in] rootpath : Root path to get te pointer after.
//
// Return : A pointer into "fullpath" after the "rootpath" part.
//          If "fullpath" does not begin with "rootpath", NULL is
//          returned.
//
char *FSUtils::getRelPathPtr(char *fullpath, char *rootpath)
{

	//make sure fullpath begins with userroot
#ifdef ZQ_OS_MSWIN
	if (_strnicmp(fullpath,rootpath,strlen(rootpath)) != 0)   //case insensitive for Windows
#else
	if (strncmp(fullpath,rootpath,strlen(rootpath)) != 0)     //case sensitive for UNIX
#endif
		return NULL;   //fullpath does not start with rootpath

	return(fullpath + strlen(rootpath));
}

//////////////////////////////////////////////////////////////////////
// Remove any leading slashes from a path.
//
// [in/out] path : Path to remove leading slashes from.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::removeSlashLead(char *path)
{
	int i;

	//remove any leading '/'
	if (*path == '/' || *path == '\\')
	{
		for (i = 1; path[i] != '\0'; i++)
			path[i-1] = path[i];
		path[i-1] = '\0';
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Remove any trailing slashes from a path.
//
// [in/out] path : Path to remove trailing slashes from.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::removeSlashEnd(char *path)
{
	if (path == NULL)
		return false;

	//remove any trailing slashes
	size_t len = strlen(path);
	if (len > 0)
	{
		if (*(path+len-1) == '\\' || *(path+len-1) == '/')
			*(path+len-1) = '\0';
	}

	return true;
}

int FSUtils::removeSlashEnd(std::string& path)
{
	//remove any trailing slashes
	size_t len;
	for (len = path.length(); path[len-1] == '\\' || path[len-1] == '/'; len--);
	path = (len>0) ? path.substr(0, len) : "";

	return true;
}

//////////////////////////////////////////////////////////////////////
// Sets the current working directory.
//
// [in] cwd : New current working directory.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::chCWD(char *cwd)
{

	if (cwd == NULL)
		return false;

	if (chdir(cwd) == 0)
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////
// Gets the current working directory.
//
// [out] cwd   : Current working directory.
// [in] maxcwd : Max size of the "cwd" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool FSUtils::getCWD(std::string& cwd)
{
	char cwdbuf[FSUTILS_MAXPATH] = "";
	if (getcwd(cwdbuf,FSUTILS_MAXPATH) != NULL)
		cwdbuf[FSUTILS_MAXPATH-1] = '\0';   //make sure the path is NULL terminated

	cwd = cwdbuf;

	return true;
}

////////////////////////////////////////
// Functions for basic filesystem
// actions
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Deletes a specified file.
//
// [in] path : Path to the file to be deleted.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool FSUtils::delFile(const char *path)
{

	if (path == NULL)
		return false;

	if (remove(path) == 0)
		return true;  //file successfully deleted
	else
		return false;  //error deleting the file
}

//////////////////////////////////////////////////////////////////////
// Make a directory.
//
// [in] path : Path for the directory to be created.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool FSUtils::mkDir(const char *path)
{
	if (path == NULL)
		return false;

#ifdef ZQ_OS_MSWIN
	if (mkdir(path) == 0)
	{
#else
	if (mkdir(path,0000755) == 0)
	{
		//make dir w/permissions "rwxr-xr-x"
#endif
		return true;  //dir successfully created
	}
	else return false;  //error creating the dir
}

//////////////////////////////////////////////////////////////////////
// Remove a directory.
//
// [in] path : Path of the directory to remove.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
// NOTE: The directory being removed must be "".
//
bool FSUtils::rmDir(const char *path)
{
	if (path == NULL)
		return false;

	return (rmdir(path) == 0);
}

//////////////////////////////////////////////////////////////////////
// rename a file or directory.
//
// [in] oldname : file/dir to be renamed.
// [in] newname : new file/dir name.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool FSUtils::rename(const char *oldname, const char *newname)
{

	if (oldname == NULL || newname == NULL)
		return false;

	return (rename(oldname,newname) == 0);
}


////////////////////////////////////////
// Functions for advanced filesystem
// actions
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Copy files or directories.
//
// [in] input  : file or directory to copy
// [in] output : destination file/directory
//
// Return : returns the number of files copied.
//
// NOTE: If "input" is a file and "output" is a directory, a file
//       with the name "input" will be created in the "output"
//       directory.  If "input" is a directory and "output" is an
//       existing directory, then the contents of "input" will
//       recursively be copied to the "output" directory.  If "input"
//       is a directory and "output" is not an existing directory,
//       a new directory will be created for that directory.  If
//       "input" is a wildcard, output is assumed to be a directory.
//       If both "input" and "output" are files, "input" will be
//       copied to "output".  Wildcards can be used when specifying
//       "input".
int FSUtils::copyFiles(char *input, char *output)
{
	char *outputbuff, *inputbuff, *indirptr = NULL, *infileptr = NULL;
	int type, ncopied = 0, flagdir = 0, flagwildcard = 0;
	long handle;

	if (input == NULL || output == NULL)
		return false;

	if ((inputbuff = new char[strlen(input)+3]) == NULL)
		return false;

	strcpy(inputbuff,input);
	checkSlash(inputbuff);  //make sure the slashes are correct for the OS

	indirptr = inputbuff;     //set the dir pointer to the start of the path
	if (strchr(inputbuff,'*') == NULL)
	{
		//if no wildcards
		type = validatePath(inputbuff);
		if (type == 0)
		{
			delete[] inputbuff;
			return false;  //input path does not exist (exit)
		}
		if (type == 1)
		{
			flagdir = 1;    //input is a directory
		}
		else
		{
			flagdir = 0;    //input is a file
			if ((infileptr = getFileNamePtr(inputbuff)) == NULL)
				infileptr = inputbuff;
		}
	}
	else
	{
		flagwildcard = 1;   //the path contains wildcards
		if ((infileptr = getFileNamePtr(inputbuff)) == NULL)
		{
			createRelativePath(inputbuff,strlen(input)+3);  //add leading "./" if necessary
			infileptr = inputbuff;      //only a file name was specified
		}
		else
		{
			*(infileptr - 1) = '\0';    //terminate the dir name
		}
	}

	//if only a single file should be copied
	if (flagdir == 0 && flagwildcard == 0)
	{
		type = validatePath(output);
		if (type == 0)
		{
			delete[] inputbuff;
			//output is a new filename
			return(copySingleFile(input,output));
		}
		else
		{
			if (type == 1)
			{
				char *buffer = new char[strlen(output)+strlen(infileptr)+2];
				//the output path is a directory
				if (buffer == NULL)
				{
					delete[] inputbuff;
					return false;
				}

				strcpy(buffer,output);
				checkSlash(buffer);
				checkSlashEnd(buffer,strlen(output)+strlen(infileptr)+2);
				strcat(buffer,infileptr);   //build the full output file path
				ncopied = copySingleFile(inputbuff,buffer);   //copy the file
				delete[] buffer;
				delete[] inputbuff;
				return(ncopied);
			}
			else
			{
				delete[] inputbuff;
				return(copySingleFile(input,output));   //overwrite an existing file
			}
		}
	}

	//copy the output path to a buffer
	if ((outputbuff = new char [strlen(output) +2]) == NULL)
	{
		delete[] inputbuff;
		return false;
	}
	strcpy(outputbuff,output);
	//make sure output is a directory (create if necessary)
	if (verifyOutputDir(outputbuff,strlen(output)+2) == 0)
	{
		delete[] outputbuff;
		delete[] inputbuff;
		//printf("copyFiles: Invalid output path '%s'\n",output);
		return false;  //could not verify/create the output dir
	}

	//allocate buffers
	char buffer[FSUTILS_MAXPATH], tmpout[FSUTILS_MAXPATH], tmpin[FSUTILS_MAXPATH];

	//if a wildcard was specified
	if (flagwildcard != 0)
	{
		//scan through the input directory
		if ((handle = dirGetFirstFile(inputbuff,buffer,FSUTILS_MAXPATH)) != 0)
		{
			do
			{
				if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
				{
					if (matchWildcard(infileptr,buffer) != 0)
					{
						if (infileptr != inputbuff)
						{
							snprintf(tmpin,FSUTILS_MAXPATH-1,"%s",indirptr);
							checkSlashEnd(tmpin,FSUTILS_MAXPATH);
						}
						else
						{
							*tmpin = '\0';
						}
						strncat(tmpin,buffer,FSUTILS_MAXPATH-strlen(tmpin)-1);
						tmpin[FSUTILS_MAXPATH-1] = '\0';
						snprintf(tmpout,FSUTILS_MAXPATH,"%s%s",outputbuff,buffer);
						ncopied += copySingleFile(tmpin,tmpout);
					}
				}
			} while (dirGetNextFile(handle,buffer,FSUTILS_MAXPATH) != 0);
			dirClose(handle);
		}
	}
	else if (flagdir != 0)
	{
		//scan through the input directory
		if ((handle = dirGetFirstFile(inputbuff,buffer,FSUTILS_MAXPATH)) != 0)
		{
			do
			{
				if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
				{
					snprintf(tmpin,FSUTILS_MAXPATH-1,"%s",indirptr);
					checkSlashEnd(tmpin,FSUTILS_MAXPATH);   //make sure str ends in a slash
					strncat(tmpin,buffer,FSUTILS_MAXPATH-strlen(tmpin)-1);
					tmpin[FSUTILS_MAXPATH-1] = '\0';
					type = validatePath(tmpin);
					if (type != 0)
					{
						snprintf(tmpout,FSUTILS_MAXPATH,"%s%s",outputbuff,buffer);
						if (type == 2)
							ncopied += copySingleFile(tmpin,tmpout);    //tmpin is a file
						else
							ncopied += copyFiles(tmpin,tmpout);         //tmpin is a dir (recurse)
					}
				}
			} while (dirGetNextFile(handle,buffer,FSUTILS_MAXPATH) != 0);
			dirClose(handle);
		}
	}

	delete[] outputbuff;
	delete[] inputbuff;

	return(ncopied);
}

//////////////////////////////////////////////////////////////////////
// Copies a single file
//
// [in] inputfilename  : file to be copied
// [in] outputfilename : destination file
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FSUtils::copySingleFile(char *inputfilename, char *outputfilename)
{
	FILE *fdr, *fdw;

	if (validatePath(inputfilename) != 2)
		return false;  //input is not a file

	if ((fdr = fopen(inputfilename,"rb")) == NULL)
		return false;
	if ((fdw = fopen(outputfilename,"wb")) == NULL)
	{
		fclose(fdr);
		return false;
	}
	//printf("Copy %s -> %s\n",inputfilename,outputfilename);

	char buffer[512];
	size_t nbytes;

	while ((nbytes = fread(buffer,1,sizeof(buffer),fdr)) > 0)
		fwrite(buffer,1,nbytes,fdw);

	fclose(fdw);
	fclose(fdr);

	return true;
}

//////////////////////////////////////////////////////////////////////
// Similar to copyFiles except only files are moved (not directories)
//
// [in] input  : file(s) to be moved
// [in] output : destination file/directory
//
// Return : returns the number of files moved.
//
// NOTE: Wildcards can be used when specifying "input".
//
int FSUtils::moveFiles(char *input, char *output)
{
	char *outputbuff, *inputbuff, *indirptr = NULL, *infileptr = NULL;
	int type, nmoved = 0, flagwildcard = 0;
	long handle;

	if (input == NULL || output == NULL)
		return false;

	if ((inputbuff = new char[strlen(input)+3]) == NULL)
		return false;

	strcpy(inputbuff,input);
	checkSlash(inputbuff);  //make sure the slashes are correct for the OS

	indirptr = inputbuff;     //set the dir pointer to the start of the path
	if (strchr(inputbuff,'*') == NULL)
	{
		//if no wildcards
		if (validatePath(inputbuff) != 2)
		{
			delete[] inputbuff;
			return false;  //input path must be a file
		}
		if ((infileptr = getFileNamePtr(inputbuff)) == NULL)    //input is a file
			infileptr = inputbuff;
	}
	else
	{
		flagwildcard = 1;   //the path contains wildcards
		if ((infileptr = getFileNamePtr(inputbuff)) == NULL)
		{
			createRelativePath(inputbuff,strlen(input)+3);  //add leading "./" if necessary
			infileptr = inputbuff;      //only a file name was specified
		}
		else
			*(infileptr - 1) = '\0';    //terminate the dir name
	}

	//if only a single file should be moved
	if (flagwildcard == 0)
	{
		type = validatePath(output);
		if (type == 0)
		{
			delete[] inputbuff;
			//printf("Move %s -> %s\n",input,output);
			if (rename(input,output) == 0)
				//output is a new file name (file -> file)
				return true;
			else
				return false;
		}
		else
		{
			if (type == 1)
			{
				//the output path is a directory (file -> dir/file)
				char *buffer = new char[strlen(output)+strlen(infileptr)+2];
				if (buffer == NULL)
				{
					delete[] inputbuff;
					return false;
				}
				strcpy(buffer,output);
				checkSlash(buffer);
				checkSlashEnd(buffer,strlen(output)+strlen(infileptr)+2);
				strcat(buffer,infileptr);   //build the full output file path
				remove(buffer);             //delete the previous file (if necessary)
				//printf("Move %s -> %s\n",inputbuff,buffer);
				nmoved = (rename(inputbuff,buffer) == 0) ? 1 : 0;   //move the file
				delete[] buffer;
				delete[] inputbuff;
				return(nmoved);
			}
			else
			{
				delete[] inputbuff;
				remove(output);                 //delete the previous file
				//printf("Move %s -> %s\n",input,output);
				return (rename(input,output) == 0);
			}
		}
	}

	//copy the output path to a buffer
	if ((outputbuff =new char[strlen(output)+2]) == NULL)
	{
		delete[] inputbuff;
		return false;
	}
	strcpy(outputbuff,output);
	//make sure output is a directory (create if necessary)
	if (verifyOutputDir(outputbuff,strlen(output)+2) == 0)
	{
		delete[] outputbuff;
		delete[] inputbuff;
		return false;  //could not verify/create the output dir
	}

	//if a wildcard was specified
	if (flagwildcard != 0)
	{
		char buffer[FSUTILS_MAXPATH], tmpout[FSUTILS_MAXPATH], tmpin[FSUTILS_MAXPATH];
		//scan through the input directory
		if ((handle = dirGetFirstFile(inputbuff,buffer,FSUTILS_MAXPATH)) != 0)
		{
			do
			{
				if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
				{
					if (matchWildcard(infileptr,buffer) != 0)
					{
						if (infileptr != inputbuff)
						{
							strncpy(tmpin,indirptr,FSUTILS_MAXPATH-2);
							tmpin[FSUTILS_MAXPATH-2] = '\0';
							checkSlashEnd(tmpin,FSUTILS_MAXPATH);
						}
						else
						{
							*tmpin = '\0';
						}
						strncat(tmpin,buffer,FSUTILS_MAXPATH-strlen(tmpin)-1);
						tmpin[FSUTILS_MAXPATH-1] = '\0';
						snprintf(tmpout,FSUTILS_MAXPATH,"%s%s",outputbuff,buffer);
						remove(tmpout);             //delete the previous file (if necessary)
						//printf("Move %s -> %s\n",tmpin,tmpout);
						nmoved += (rename(tmpin,tmpout) == 0) ? 1 : 0;
					}
				}
			} while (dirGetNextFile(handle,buffer,FSUTILS_MAXPATH) != 0);
			dirClose(handle);
		}
	}

	delete[] outputbuff;
	delete[] inputbuff;

	return(nmoved);
}

//////////////////////////////////////////////////////////////////////
// Deletes the file(s)/directory specified.
//
// [in] input        : file(s)/dir to be deleted
// [in] flagkeepdirs : if not 0, only files, not dirs, will be deleted
//
// Return : returns the number of files and dirs deleted.
//
// NOTE: Wildcards can be used when specifying "input".  If "input"
//       is a directory, it will be recursively deleted
//
int FSUtils::deleteFiles(char *input, int flagkeepdirs /*=0*/)
{
	char *inputbuff, *indirptr = NULL, *infileptr = NULL;
	int type, ndeleted = 0, flagdir = 0, flagwildcard = 0;
	long handle;

	if (input == NULL)
		return false;

	if ((inputbuff = new char[strlen(input)+3]) == NULL)
		return false;
	strcpy(inputbuff,input);
	checkSlash(inputbuff);  //make sure the slashes are correct for the OS

	indirptr = inputbuff;     //set the dir pointer to the start of the path
	if (strchr(inputbuff,'*') == NULL)
	{
		//if no wildcards
		type = validatePath(inputbuff);
		if (type == 0)
		{
			delete[] inputbuff;
			return false;  //input path does not exist (exit)
		}
		if (type == 1)
		{
			flagdir = 1;    //input is a directory
		}
		else
		{
			flagdir = 0;    //input is a file
			if ((infileptr = getFileNamePtr(inputbuff)) == NULL)
				infileptr = inputbuff;
		}
	}
	else
	{
		flagwildcard = 1;   //the path contains wildcards
		if ((infileptr = getFileNamePtr(inputbuff)) == NULL)
		{
			createRelativePath(inputbuff,strlen(input)+3);  //add leading "./" if necessary
			infileptr = inputbuff;      //only a file name was specified
		}
		else
		{
			*(infileptr - 1) = '\0';    //terminate the dir name
		}
	}

	char buffer[FSUTILS_MAXPATH], tmpin[FSUTILS_MAXPATH];

	//if only a single file should be deleted
	if (flagdir == 0 && flagwildcard == 0)
	{
		ndeleted = (remove(input) == 0) ? 1 : 0;
	}
	else if (flagwildcard != 0)
	{
		//scan through the input directory
		if ((handle = dirGetFirstFile(inputbuff,buffer,FSUTILS_MAXPATH)) != 0)
		{
			do
			{
				if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
				{
					if (matchWildcard(infileptr,buffer) != 0)
					{
						if (infileptr != inputbuff)
						{
							strncpy(tmpin,indirptr,FSUTILS_MAXPATH-2);
							tmpin[FSUTILS_MAXPATH-2] = '\0';
							checkSlashEnd(tmpin,FSUTILS_MAXPATH);
						}
						else
						{
							*tmpin = '\0';
						}
						strncat(tmpin,buffer,FSUTILS_MAXPATH-strlen(tmpin)-1);
						tmpin[FSUTILS_MAXPATH-1] = '\0';
						//printf("Delete %s\n",tmpin);
						ndeleted += (remove(tmpin) == 0) ? 1 : 0;
					}
				}
			} while (dirGetNextFile(handle,buffer,FSUTILS_MAXPATH) != 0);
			dirClose(handle);
		}
	}
	else if (flagdir != 0)
	{
		//scan through the input directory
		if ((handle = dirGetFirstFile(inputbuff,buffer,FSUTILS_MAXPATH)) != 0)
		{
			do
			{
				if (strcmp(buffer,".") != 0 && strcmp(buffer,"..") != 0)
				{
					strncpy(tmpin,indirptr,FSUTILS_MAXPATH-2);
					tmpin[FSUTILS_MAXPATH-2] = '\0';
					checkSlashEnd(tmpin,FSUTILS_MAXPATH);   //make sure str ends in a slash
					strncat(tmpin,buffer,FSUTILS_MAXPATH-strlen(tmpin)-1);
					tmpin[FSUTILS_MAXPATH-1] = '\0';
					type = validatePath(tmpin);
					if (type != 0)
					{
						if (type == 2)
						{
							//printf("Delete %s\n",tmpin);
							ndeleted += (remove(tmpin) == 0) ? 1 : 0;    //tmpin is a file
						}
						else
						{
							ndeleted += deleteFiles(tmpin,flagkeepdirs); //tmpin is a dir (recurse)
						}
					}
				}
			} while (dirGetNextFile(handle,buffer,FSUTILS_MAXPATH) != 0);
			dirClose(handle);
		}
		//printf("Delete directory %s\n",inputbuff);
		if (flagkeepdirs == 0)
			ndeleted += (rmdir(inputbuff) == 0) ? 1 : 0; //delete the root dir
	}

	delete[] inputbuff;

	return(ndeleted);
}

//////////////////////////////////////////////////////////////////////
// Gets information for the disk specified by "fullpath."
//
// [in] fullpath  : path to get the disk information for
// [out] diskinfo : information for the specified disk
//
// Return : On success true is returned
//
bool FSUtils::getDiskInfo(diskInfo_t& diskinfo, const _TCHAR *fullpath)
{
	bool retval = false;

	//initialize the disk info struct
	memset(&diskinfo,0,sizeof(diskInfo_t));

#ifdef ZQ_OS_MSWIN    // Windows
	ULARGE_INTEGER freebytes, totalbytes, totalfreebytes;
	_TCHAR buffer[4] = _T("C:\\");
	if (fullpath != NULL)
		buffer[0] = *fullpath;

	if (GetDiskFreeSpaceEx(buffer,&freebytes,&totalbytes,&totalfreebytes))
	{
		diskinfo.bytesfree = (double)((freebytes.HighPart * 4294967296) + freebytes.LowPart);
		diskinfo.totalbytes = (double)((totalbytes.HighPart * 4294967296) + totalbytes.LowPart);
		retval = true;
	}
#endif

#ifdef BSD  // BSD
	long i, mntsize, blocksize;
	int headerlen, maxlen = 0;
	struct statfs *mntbuf;
	mntsize = getmntinfo(&mntbuf,MNT_NOWAIT);
	for (i = 0; i < mntsize; i++)
	{
		if (strncasecmp(mntbuf[i].f_mntonname,fullpath,strlen(mntbuf[i].f_mntonname)) == 0)
		{
			if (strlen(mntbuf[i].f_mntonname) > (unsigned)maxlen)
			{
				getbsize(&headerlen,&blocksize);
				diskinfo.bytesfree = (double)fsbtoblk(mntbuf[i].f_bavail,mntbuf[i].f_bsize,blocksize) * (double)blocksize;
				diskinfo.totalbytes = (double)fsbtoblk(mntbuf[i].f_blocks,mntbuf[i].f_bsize,blocksize) * (double)blocksize;
				maxlen = strlen(mntbuf[i].f_mntonname);
				retval = 1;
			}
		}
	}
#endif

#ifdef SOLARIS  // Solaris
	statvfs_t buf;
	if (statvfs(fullpath,&buf) == 0)
	{
		diskinfo.bytesfree = (double)buf.f_bfree * (double)buf.f_frsize;
		diskinfo.totalbytes = (double)buf.f_blocks * (double)buf.f_frsize;
		retval = true;
	}
#endif

#ifdef LINUX    // Linux
	struct statfs buf;
	if (statfs(fullpath,&buf) == 0)
	{
		diskinfo.bytesfree = (double)buf.f_bfree * (double)buf.f_bsize;
		diskinfo.totalbytes = (double)buf.f_blocks * (double)buf.f_bsize;
		retval = true;
	}
#endif

	return retval;
}

//////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////

//Converts a path with relative components into an absolute path.
//NOTE: This function assumes that all paths have first gone
//      through checkSlash() and CheckEnd().
bool FSUtils::compactPath(char *path)
{
	char *tmpptr, *pathptr,	*ptr;
	char buffer[FSUTILS_MAXPATH];

	//remove all occurrences of	"./" in	the	path.
	//look for "./"	only
	pathptr	= path;

	while ((ptr = strstr(pathptr,"." FSUTILS_SLASHS)) != NULL)
	{
		if (ptr	== path	|| *(ptr-1)	!= '.')
		{
			//make xxx/./yyy/ -> "xxx/yyy/"
			*ptr = '\0';
			strcpy(buffer,path);
			strcat(buffer,ptr+2);
			strcpy(path,buffer);
			pathptr	= ptr;
		}
		else
		{
			pathptr	= ptr +	2;
		}
	}

	//remove all occurrences of	"../" in the path.
	//look for "../" only
	while ((ptr = strstr(path,".." FSUTILS_SLASHS)) != NULL)
	{
		if (ptr == path)
			*ptr = '\0';
		else
			*(ptr-1) ='\0';
		if ((tmpptr = strrchr(path,FSUTILS_SLASHC)) == NULL)
		{
			//make "yyy/../zzz/" -> "zzz/"
			strcpy(buffer,ptr+3);
			strcpy(path,buffer);
		}
		else
		{
			//make "xxx/yyy/../zzz/" -> "xxx/zzz/"
			*(tmpptr+1) = '\0';
			strcpy(buffer,path);
			strcat(buffer,ptr+3);
			strcpy(path,buffer);
		}
	}

	return true;
}

//Copies "path1path2" into "outpath"
bool FSUtils::copyPaths(char *outpath, size_t maxpathsize, const char *path1, const char *path2)
{
	if (maxpathsize < 1)
		return false;

	strncpy(outpath,path1,maxpathsize-1);
	outpath[maxpathsize-1] = '\0';

	size_t len = strlen(outpath);
	strncat(outpath,path2,maxpathsize-1-len);
	outpath[maxpathsize-1] = '\0';

	return true;
}

char *FSUtils::getFileNamePtr(char *path)
{
	char *fileptr;

	if ((fileptr = strrchr(path,FSUTILS_SLASHC)) == NULL)
		return NULL;

	fileptr++;
	if (fileptr == '\0')    //no filename
		return NULL;
	else
		return(fileptr);
}

void FSUtils::createRelativePath(char *path, size_t maxpath)
{
	char tmpbuff[FSUTILS_MAXPATH];

	if (maxpath < 3)
		return;

	//check if the path is already a relative path
	if (strncmp(path,"." FSUTILS_SLASHS,2) == 0 || strncmp(path,".." FSUTILS_SLASHS,3) == 0)
		return;

	strcpy(tmpbuff,path);

	strcpy(path,"." FSUTILS_SLASHS);
	strncat(path,tmpbuff,maxpath-3);
	path[maxpath-1] = '\0';
}

//NOTE: path may grow by 1 byte (from checkSlashEnd())
bool FSUtils::verifyOutputDir(char *path, size_t maxpath)
{

	checkSlash(path);

	removeSlashEnd(path);   //remove the ending slash if necessary

	mkDir(path);            //create the path if necessary

	//make sure the dir exists
	if (validatePath(path) != 1)
		return false;

	if ((strlen(path) + 2) <= (unsigned)maxpath)
		checkSlashEnd(path,maxpath);    //make sure there is an ending slash

	return true;
}

//////////////////////////////////////////////////////////////////////
// Checks if "filename" is contained in the "wildcard" string.  The
// wildcard character is '*'.
//
// [in] wildcard    : string containing the wildcard characters
//                    Ex. 192.168.*.*
// [in] matchstring : string to match to the wildcard.
//                    Ex. 192.168.60.211
//
// Return : true if "matchstring" is contained in "wildcard"
//
bool FSUtils::matchWildcard(const char *wildcard, const char *matchstring)
{
	char *ptri, *ptra, *ptr1, *ptr2;
	char *allowed= new char[strlen(wildcard)+1];

	if (allowed== NULL)
		return false;

	strcpy(allowed,wildcard);
	ptra = allowed;
	ptri = (char *)matchstring;
	for (;;)
	{
		if ((ptr1 = strchr(ptra,'*')) != NULL)
		{
			*ptr1 = '\0';
			if (ptra == allowed)
			{
				if (strncmp(ptra,ptri,strlen(ptra)) != 0)
				{
					delete[] allowed;
					return false;
				}
				ptr2 = ptri;
			}
			else
			{
				if ((ptr2 = strstr(ptri,ptra)) == NULL)
				{
					delete[] allowed;
					return false;
				}
			}
			ptri = ptr2 + strlen(ptra);
			ptra = ptr1 + 1;
		}
		else
		{
			if (ptra == allowed)
			{
				if (strcmp(ptra,ptri) != 0)
				{
					delete[] allowed;
					return false;
				}
				else
				{
					delete[] allowed;
					return true;
				}
			}
			else
			{
				if (*ptra == '\0')
				{
					delete[] allowed;
					return true;
				}
				if ((ptr2 = strstr(ptri,ptra)) == NULL)
				{
					delete[] allowed;
					return false;
				}
				if (*(ptr2 + strlen(ptra)) != '\0')
				{
					delete[] allowed;
					return false;
				}
				delete[] allowed;
				return true;
			}
		}
	}

	delete[] allowed;
	return false;
}

