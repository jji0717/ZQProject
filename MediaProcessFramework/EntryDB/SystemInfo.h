/**********************************************************
 $Id: SystemInfo.h,v 1.4 2003/06/09 14:55:31 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:00 $

Copyright (C) 2002-2003 Shao Hui.
All Rights Reserved, Shao Hui.
**********************************************************/

#ifndef __SystemInfo_h__
#define __SystemInfo_h__

#include "ExpatDB.h"

#include <windows.h>

#include <string>
#include <vector>

ENTRYDB_NAMESPACE_BEGIN

class EDOS_API SystemInfo;

class SystemInfo : public ExpatDB
{
public:

	SystemInfo();
	~SystemInfo() {};

	void refreshMemInfo();
	void refreshDisplayInfo();
	void refreshProcInfo();
	void refreshNetIFInfo();
	void refreshEnvInfo();

	static bool fileinfo(ExpatDB& filedb, const char* filename);

private:
	void initInfo();
	void cpuinfo();
	void OSInfo();
	void localeInfo();

//	uint8 nCPU, nFloppy, nHD, nMiceBtn; //, nMonitor;

//	std::string mFloppy, mHDs, mPrn, mComs, mNic;
	
//	std::string mMachineName, mWinVersion, mLanguage, mCountry, mWorkPath, mWinPath;

	bool bNtKernel, bW9xKernel;
	bool bKernelDebug;
};

#define SYSINFO_SECTION(_X)				LOGIC_SEPS LOGIC_SEPS _X

#define SYSINFO_PROCESSOR				"processor"
#define SYSINFO_OS						"os"

extern EDOS_API SystemInfo gblSysInfo;

ENTRYDB_NAMESPACE_END


#endif // __SystemInfo_h__

