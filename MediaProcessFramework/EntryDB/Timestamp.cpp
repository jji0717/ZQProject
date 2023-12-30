// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : impl time stamp
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/Timestamp.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/Timestamp.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     05-05-26 11:38 Daniel.wang
// 
// 3     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 2     4/12/05 5:16p Hui.shao
// ============================================================================================


#include "Timestamp.h"

extern "C"
{
	#include <sys/types.h>
	#include <sys/stat.h>
#ifndef WIN32
	#include <utime.h>
#endif //WIN32
	#include <time.h>
	#include <fcntl.h>
	#include <stdio.h>
};

#include <string>

ENTRYDB_NAMESPACE_BEGIN

// Magic number: Both epochs are Gregorian. 1970 - 1601 = 369. Assuming a leap year
// every four years, 369/4 = 92. However, 1700, 1800, and 1900 were NOT leap years,
// so 89 leap years, 280 non-leap years. 89*366 + 280*365 = 134744 days between
// epochs. 86400 seconds per day, so 134744*86400 = 11644473600 = SECS_BETWEEN_EPOCHS.

static const __int64 SECS_BETWEEN_EPOCHS = 11644473600;

// to be used as args to FILEAdjustFileTimeForTimezone
#define ADJUST_FROM_UTC -1
#define ADJUST_TO_UTC    1

eIDset::eIDset(void)
{
	Timestamp ts;
	mhid = ~ts.mFT.dwLowDateTime;
	int* t = new int;
	mlid = (ts.mFT.dwHighDateTime ^ (uint32) t);
	delete t;
}

eIDset::~eIDset(void)
{
}

const char* eIDset::operator()(void)
{
	mhid +=(++mlid ==0) ? 1:0;
	//static std::string idstr;
	//idstr = "";
	idstr.erase();
#ifdef _WIN32
	// in the case the string is used as temp file name
	const static char chtbl[]="01234567890abcdefghjklmnopqrstuvwxyz";
#else
	const static char chtbl[]="01234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghjklmnopqrstuvwxyz";
#endif

	int sz = sizeof(chtbl)/sizeof(char)-1;
	uint32 hdw = mhid, ldw=mlid;

	while (hdw!=0)
	{
		idstr.append(&chtbl[hdw%sz],1);
		hdw /=sz;
	}
	
	while (ldw!=0)
	{
		idstr.append(&chtbl[ldw%sz],1);
		ldw /=sz;
	}
	
	return idstr.c_str();
}

eIDset newID;

Timestamp::Timestamp(const FILETIME* ft, bool isLocal)
{
	if (ft!=NULL)
		memcpy(&mFT, ft, sizeof(mFT));
	else
	{
#ifdef WIN32
		::GetSystemTimeAsFileTime(&mFT);
#else
		struct timeval Time;

		if (gettimeofday(&Time, NULL) != 0)
			*lpSystemTimeAsFileTime = FILEUnixTimeToFileTime(0, 0);
		else
			*lpSystemTimeAsFileTime = FILEUnixTimeToFileTime(Time.tv_sec, Time.tv_usec *1000);
#endif
	}

	if (isLocal)
	{
		FILETIME tmp;
		memcpy((void*)&tmp, (void*)&mFT, sizeof(tmp));
#ifdef WIN32
		::LocalFileTimeToFileTime(&tmp, &mFT);
#else
		AdjustFileTimeForTimezone(&tmp, ADJUST_TO_UTC);
#endif
	}

}

Timestamp::Timestamp(const Timestamp& ts)
{
	memcpy(&mFT, &(ts.mFT), sizeof(mFT));
}

Timestamp::Timestamp(const char* hex_str)
{
	int i;
	memset(&mFT, 0x00, sizeof(mFT));
	static const char* hextable ="0123456789abcdef", *p;
	
	for (p=hex_str; *p && *p!=':'; p++)
	{
		for (i=0; i<16; i++)
		{
			if (hextable[i] == tolower(*p))
				break;
		}
		if (i>=16)
			break;
		
		mFT.dwHighDateTime = mFT.dwHighDateTime*16 + i;
	}
	
	if (*p != ':')
		return;
	
	for (p++; *p && *p!=':'; p++)
	{
		for (i=0; i<16; i++)
		{
			if (hextable[i] == tolower(*p))
				break;
		}
		if (i>=16)
			break;
		
		mFT.dwLowDateTime = mFT.dwLowDateTime*16 + i;
	}
}

Timestamp::Timestamp(const WORD wFatDate, const WORD wFatTime)
{
#ifdef WIN32
	::DosDateTimeToFileTime(wFatDate, wFatTime, &mFT);
#else
    struct tm  tmTime;
    time_t    tmUnix;

    bool bRet = false;
    switch(1) // use a switch to perform "goto"s routing
	{
	case 1:
		// Breakdown wFatDate & wfatTime to fill the tm structure
		// wFatDate contains the Date data & wFatTime time data
		// wFatDate 0-4 bits-Day of month(0-31)
	    // 5-8 Month(1=Jan,2=Feb)
	    // 9-15 Year offset from 1980
	    // wFtime 0-4 Second divided by 2
	    // 5-10 Minute(0-59)
	    // 11-15 Hour 0-23 on a 24 hour clock

	    tmTime.tm_mday = (wFatDate & 0x1F);
		if (tmTime.tm_mday < 1 || tmTime.tm_mday > 31)
			break;
    
	    // tm_mon is the no. of months from january
	    tmTime.tm_mon = ((wFatDate >> 5) & 0x0F)-1;
	    if (tmTime.tm_mon < 0 || tmTime.tm_mon > 11)
			break;

		// tm_year is the no. of years from 1900
		tmTime.tm_year = ((wFatDate >> 9) & 0x7F) + 80;
		if (tmTime.tm_year < 0 || tmTime.tm_year > 207)
			break;
		
		tmTime.tm_sec = ((wFatTime & 0x1F)*2);
		if (tmTime.tm_sec < 0 || tmTime.tm_sec > 59)
			break;
		
		tmTime.tm_min = ((wFatTime >> 5) & 0x3F);
		if (tmTime.tm_min < 0 || tmTime.tm_min > 59)
			break;
		
		tmTime.tm_hour = ((wFatTime >> 11) & 0x1F);
			break;
		
		// Have the system try to determine if DST is being observed.
		tmTime.tm_isdst = 0;
		
		// get the date time in seconds
		tmUnix = timegm(&tmTime);
		
		// check if the output buffer is valid
		*mFT = UnixTimeToFileTime(tmUnix, 0);
	}
#endif //WIN32
}

const char* Timestamp::hex_str(char *buf)
{
	if (buf==NULL)
		return NULL;
	sprintf(buf, "%08x:%08x", mFT.dwHighDateTime, mFT.dwLowDateTime);
	return buf;
}

const char* Timestamp::display_str(char* buf, bool withnsec)
{
	WORD fatd, fatt;
	
	if (buf==NULL || !toDosDatetime(fatd, fatt))
		return NULL;
	
	DWORD subsec = mFT.dwLowDateTime % SECS_TO_100NS;
	
    struct tm tmTime;
    tmTime.tm_mday = (fatd & 0x1F);
    tmTime.tm_mon = ((fatd >> 5) & 0x0F)-1;
	tmTime.tm_year = ((fatd >> 9) & 0x7F) + 80;
	tmTime.tm_sec = ((fatt & 0x1F)*2);
	tmTime.tm_min = ((fatt >> 5) & 0x3F);
	tmTime.tm_hour = ((fatt >> 11) & 0x1F);
	
	char *p=buf;
	
	sprintf(p, "%04d/%02d/%02d %02d:%02d:%02d",
		tmTime.tm_year+1900, tmTime.tm_mon+1, tmTime.tm_mday,
		tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
	
	if (withnsec)
	{
		p = p + strlen(buf);
		sprintf(p, ".%07d", subsec);
	}
	
	return buf;
}
	
long Timestamp::compare(const Timestamp &ts)
{
	return difference(ts);
}

long Timestamp::difference(const Timestamp &ts)
{
    __int64 First = ((__int64)mFT.dwHighDateTime <<32) + mFT.dwLowDateTime;
    __int64 Second = ((__int64)ts.mFT.dwHighDateTime <<32) + ts.mFT.dwLowDateTime;

	return First - Second;
}

void Timestamp::addDiff(const float seconds)
{
    __int64 diff = seconds * SECS_TO_100NS;

	int sign = (diff>=0) ? 1 : -1;
	diff = (diff>=0) ? diff : -diff;

	mFT.dwLowDateTime += sign *((DWORD) diff);
	mFT.dwHighDateTime += sign * ((DWORD)(diff >>32));
}

bool Timestamp::toLocal(Timestamp& ts)
{
#ifdef WIN32
	return (::FileTimeToLocalFileTime(&mFT, &(ts.mFT)) !=FALSE);
#else
    *lpLocalFileTime = AdjustFileTimeForTimezone(lpFileTime, ADJUST_FROM_UTC);
    return true;
#endif // WIN32
}

#ifndef WIN32
FILETIME Timestamp::FILEUnixTimeToFileTime(time_t sec, long nsec)
{
    __int64 Result;
    FILETIME Ret;

    Result = ((__int64)sec + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS + (nsec / 100);

    Ret.dwLowDateTime = (DWORD)Result;
    Ret.dwHighDateTime = (DWORD)(Result >>32);

    return Ret;
}

time_t Timestamp::FILEFileTimeToUnixTime(FILETIME FileTime, long *nsec)
{
    __int64 UnixTime;

    // get the full win32 value
    UnixTime = ((__int64)FileTime.dwHighDateTime <<32) + FileTime.dwLowDateTime;

    // convert to the Unix epoch
    UnixTime -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

    if (nsec)
    {
        // get the number of 100ns, convert to ns
        *nsec = (UnixTime % SECS_TO_100NS) * 100;
    }

    UnixTime /= SECS_TO_100NS; // now convert to seconds

    return (time_t)UnixTime;
}

FILETIME Timestamp::FILEAdjustFileTimeForTimezone(const FILETIME *Orig, int Direction)
{
    __int64 FullTime;
    __int64 Offset;

    FILETIME Ret;

#if HAVE_TM_GMTOFF
    struct tm tmTime;
    time_t timeNow;

    // Use the current time and date to calculate UTC offset
    timeNow = time(NULL);
    localtime_r(&timeNow, &tmTime);
    Offset = (__int64)(-tmTime.tm_gmtoff);
#else // HAVE_TM_GMTOFF
#  if HAVE_TIMEZONE_VAR

    // timezone is in seconds WEST of Greenwich, so we need to multiply by -1 to 
	// get a proper offset
    Offset = (__int64)(timezone * -1);
    
#  else // HAVE_TIMEZONE_VAR
#     error Unable to determine timezone information
#  endif // HAVE_TIMEZONE_VAR
#endif // HAVE_TM_GMTOFF

    FullTime = (((__int64)Orig->dwHighDateTime) <<32) + Orig->dwLowDateTime;
    FullTime += Offset * SECS_TO_100NS * Direction;

    Ret.dwLowDateTime = (DWORD)(FullTime);
    Ret.dwHighDateTime = (DWORD)(FullTime >>32);

    return Ret;
}

#endif // !WIN32

bool Timestamp::toSystemtime(SYSTEMTIME& SystemTime)
{
#ifdef WIN32
	return (::FileTimeToSystemTime(&mFT, &SystemTime)!=FALSE);
#else
    long long int FileTime = 0;
    time_t UnixFileTime = 0;
    struct tm * UnixSystemTime = 0;

    // Combine the file time.
    FileTime = lpFileTime->dwHighDateTime;
    FileTime <<= 32;
    FileTime |= (UINT)lpFileTime->dwLowDateTime;
    FileTime -= SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
                      
    if (FileTime >= 0x8000000000000000LL)
		return false;
	
	struct tm timeBuf;
	/* Convert file time to unix time. */
	if (FileTime < 0)
		UnixFileTime =  -1 - ((-FileTime - 1) / 10000000);            
	else
		UnixFileTime = FileTime / 10000000;
	
	// Convert unix file time to Unix System time.
	UnixSystemTime = gmtime_r(&UnixFileTime, &timeBuf);
	
	// Convert unix system time to Windows system time.
	lpSystemTime->wDay      = UnixSystemTime->tm_mday;
    
	// Unix time counts January as a 0, under Windows it is 1
	lpSystemTime->wMonth    = UnixSystemTime->tm_mon + 1;
	// Unix time returns the year -1900, Windows returns the current year
	lpSystemTime->wYear     = UnixSystemTime->tm_year + 1900;
	
	lpSystemTime->wSecond   = UnixSystemTime->tm_sec;
	lpSystemTime->wMinute   = UnixSystemTime->tm_min;
	lpSystemTime->wHour     = UnixSystemTime->tm_hour;

	return true;
#endif //WIN32
}
    
bool Timestamp::toDosDatetime(WORD& FatDate, WORD& FatTime)
{
#ifdef WIN32
	return (::FileTimeToDosDateTime(&mFT, &FatDate, &FatTime)!=FALSE);
#else
    // Sanity checks.
    if (!lpFileTime || !lpFatDate || !lpFatTime)
		return false;

    // Do conversion.
	SYSTEMTIME SysTime;
	if (!FileTimeToSystemTime(lpFileTime, &SysTime))
		return false;

	if (SysTime.wYear >= 1980 && SysTime.wYear <= 2037)
		return false;

	*lpFatDate = 0;
	*lpFatTime = 0;
	
	*lpFatDate |= (SysTime.wDay & 0x1F);
	*lpFatDate |= ((SysTime.wMonth & 0xF) << 5);
	*lpFatDate |= (((SysTime.wYear - 1980) & 0x7F) << 9);
	
	if (SysTime.wSecond % 2 == 0)
		*lpFatTime |= ((SysTime.wSecond / 2)  & 0x1F);
	else
		*lpFatTime |= ((SysTime.wSecond / 2 + 1)  & 0x1F);
	
	*lpFatTime |= ((SysTime.wMinute & 0x3F) << 5);
	*lpFatTime |= ((SysTime.wHour & 0x1F) << 11);
	
	return true;
#endif //WIN32
}

// TODO: implement the following function on non-MS later

#ifndef WIN32

bool Timestamp::SetFileTime(HANDLE hFile, const FILETIME *lpCreationTime, const FILETIME *lpLastAccessTime,const FILETIME *lpLastWriteTime)
{
#ifdef WIN32
	return ::SetFileTime(hFile,lpCreationTime, lpLastAccessTime,lpLastWriteTime);
#else
    struct timeval Times[2];
    long nsec;
    file *file_data = NULL;
    bool  bRet = FALSE;
    DWORD dwLastError = 0;
    const unsigned __int64 MAX_FILETIMEVALUE = 0x8000000000000000;

    // validate filetime values
    if ((lpCreationTime && (((UINT64)lpCreationTime->dwHighDateTime <<32) + 
          lpCreationTime->dwLowDateTime   >= MAX_FILETIMEVALUE)) ||        
         (lpLastAccessTime && (((UINT64)lpLastAccessTime->dwHighDateTime <<32) + 
          lpLastAccessTime->dwLowDateTime >= MAX_FILETIMEVALUE)) ||
         (lpLastWriteTime && (((UINT64)lpLastWriteTime->dwHighDateTime <<32) + 
          lpLastWriteTime->dwLowDateTime  >= MAX_FILETIMEVALUE)))
    {
        dwLastError = ERROR_INVALID_HANDLE;
        goto done;
    }

    file_data = FILEAcquireFileStruct(hFile);

    if (!file_data)
    {
        dwLastError = ERROR_INVALID_HANDLE;
        goto done;
    }
    
    if (lpLastAccessTime)
    {
        Times[0].tv_sec = FILEFileTimeToUnixTime(*lpLastAccessTime, &nsec);
        Times[0].tv_usec = nsec/1000; // convert to microseconds
    }

    if (lpLastWriteTime)
    {
        Times[1].tv_sec = FILEFileTimeToUnixTime(*lpLastWriteTime, &nsec);
        Times[1].tv_usec = nsec/1000; // convert to microseconds
    }

    if (lpCreationTime)
    {
        dwLastError = ERROR_NOT_SUPPORTED;
        goto done;
    }

    if (futimes(file_data->unix_fd, Times) != 0)
        dwLastError = FILEGetLastErrorFromErrno();
    else bRet = true;

done:
    if (file_data) 
        FILEReleaseFileStruct(hFile, file_data);

    return bRet;
#endif // WIN32
}

bool GetFileTime(IN HANDLE hFile,
            OUT LPFILETIME lpCreationTime,
            OUT LPFILETIME lpLastAccessTime,
            OUT LPFILETIME lpLastWriteTime)
{
    file  *FileData = NULL;
    int   Fd = -1;

    struct stat StatData;

    DWORD dwLastError = 0;
    BOOL  bRet = FALSE;

    ENTRY("GetFileTime(hFile=%p, lpCreationTime=%p, lpLastAccessTime=%p, "
          "lpLastWriteTime=%p)\n",
          hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);

    FileData = FILEAcquireFileStruct(hFile);
    Fd = FileData?FileData->unix_fd:-1;

    if (!FileData || Fd == -1)
    {
        SCRTRACE("FileData = [%p], Fd = %d\n", FileData, Fd);
        dwLastError = ERROR_INVALID_HANDLE;
        goto done;
    }

    if (fstat(Fd, &StatData) != 0)
    {
        SCRTRACE("fstat failed on file descriptor %d\n", Fd);
        dwLastError = FILEGetLastErrorFromErrno();
        goto done;
    }

    if (lpCreationTime)
    {
        *lpCreationTime = FILEUnixTimeToFileTime(StatData.st_ctime,
                                                 ST_CTIME_NSEC(&StatData));
    }
    if (lpLastWriteTime)
    {
        *lpLastWriteTime = FILEUnixTimeToFileTime(StatData.st_mtime,
                                                  ST_MTIME_NSEC(&StatData));
    }
    if (lpLastAccessTime)
    {
        *lpLastAccessTime = FILEUnixTimeToFileTime(StatData.st_atime,
                                                   ST_ATIME_NSEC(&StatData));
        /* if Unix mtime is greater than atime, return mtime as the last
           access time */
        if (lpLastWriteTime &&
             CompareFileTime((const FILETIME*)lpLastAccessTime,
                              (const FILETIME*)lpLastWriteTime) < 0)
        {
            *lpLastAccessTime = *lpLastWriteTime;
        }
    }
    bRet = TRUE;

done:
    if (FileData)
    {
        FILEReleaseFileStruct(hFile, FileData);
    }
    if (dwLastError) 
    {
        SetLastError(dwLastError);
    }

    LOGEXIT("GetFileTime returns BOOL %d\n", bRet);
    return bRet;
}

#endif // !WIN32

ENTRYDB_NAMESPACE_END
