/**********************************************************
 $Id: SystemInfo.cpp,v 1.5 2003/06/16 16:13:51 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:00 $

Copyright (C) 2002-2003 Hui Shao.
All Rights Reserved, Hui Shao.
**********************************************************/

#if defined(WIN32) && !defined(_WINSOCK2_H_)
	#include <winsock2.h>
	#define _WINSOCK2_H_
#endif

#if defined(WIN32) && !defined(_WS2TCPIP_H_)
	#include <ws2tcpip.h>
	#define _WS2TCPIP_H_
#endif

#ifdef WIN32
#	define SOCKET_HANDLE	int
#	pragma comment(lib, "WS2_32.lib")
#   pragma comment(lib, "version.lib")
#else
#	define SOCKET_HANDLE	int
#endif

#include "SystemInfo.h"
#include "Timestamp.h"
#include "EDBNil.h"

ENTRYDB_NAMESPACE_BEGIN

// int SystemInfo::nCPU = -1, FailReport::nFloppy = -1, FailReport::nHD = -1, FailReport::nMiceBtn = -1; //, nMonitor;
SystemInfo gblSysInfo;

SystemInfo::SystemInfo()
           : ExpatDB()
{
	char buf[MAX_COMPUTERNAME_LENGTH+2];

	openRoot();
	// computer name
	DWORD nLen= MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerName(buf, &nLen))
		setAttribute("MachineName", buf);

	setAttribute(ELEM_TYPE_ATTR, "computer");
	initInfo();
}

void SystemInfo::cpuinfo()
{
	char buf[2048];
	ExpatDB cpuinfodb;
	cpuinfodb.setAttribute(ELEM_TYPE_ATTR, SYSINFO_PROCESSOR);

	// cpu info
	try
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		cpuinfodb.setAttribute("number", itoa(sysinfo.dwNumberOfProcessors, buf, 10));
		cpuinfodb.setAttribute("pagesize", itoa(sysinfo.dwPageSize, buf, 10));
		cpuinfodb.setAttribute("activemask", itoa(sysinfo.dwActiveProcessorMask, buf, 16));
		
		switch (sysinfo.dwProcessorType)
		{
		case PROCESSOR_INTEL_386:
			cpuinfodb.setAttribute("type", "Intel 386");
			break;
		case PROCESSOR_INTEL_486:
			cpuinfodb.setAttribute("type", "Intel 486");
			break;
		case PROCESSOR_INTEL_PENTIUM:
			if (sysinfo.wProcessorLevel == 6)
			{
				buf[0]=buf[1] =0x00;
				int iMod = (sysinfo.wProcessorRevision >> 8) & 0xff;
				int iStp = sysinfo.wProcessorRevision & 0xff;

				cpuinfodb.setAttribute("model", itoa(iMod,buf, 10));
				cpuinfodb.setAttribute("stepping", itoa(iStp, buf, 10));
				cpuinfodb.setAttribute("MMX", IsProcessorFeaturePresent (PF_MMX_INSTRUCTIONS_AVAILABLE) ? "yes" : "no");
				
				switch(iMod)
				{
				case 1:
					cpuinfodb.setAttribute("type", "Intel Pentium Pro");
					break;
				case 3:
					cpuinfodb.setAttribute("type", "Intel Pentium II");
					break;
				case 5:
					cpuinfodb.setAttribute("type", "Intel Pentium II Xeon or Celeron");
					break;
				case 6:
					cpuinfodb.setAttribute("type", "Intel Celeron");
					break;
				case 7:
				case 8:
					cpuinfodb.setAttribute("type", "Intel Pentium III or Pentium III Xeon");
					break;
				default:
					cpuinfodb.setAttribute("type", "Other Pentium processor");
					break;
				}
			}
			else
				cpuinfodb.setAttribute("type", "Pentium");
			break;
		case PROCESSOR_MIPS_R4000:
			cpuinfodb.setAttribute("type", "MIPS R4000");
			break;
		case PROCESSOR_ALPHA_21064:
			cpuinfodb.setAttribute("type", "Alpha 21064");
			break;
			
		default:
			if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_PPC)
				cpuinfodb.setAttribute("type", "PowerPC");
			else
				cpuinfodb.setAttribute("type", "Unknown");
			break;
		}

		// Get the processor speed info.
		HKEY hKey;
		LONG result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);

		// Check if the function has succeeded.
		if (result == ERROR_SUCCESS)
		{
			DWORD data, dataSize = MAX_PATH;
			result = ::RegQueryValueEx (hKey, "~MHz", NULL, NULL,
				(LPBYTE)&data, &dataSize);
			
			if (result == ERROR_SUCCESS)
				cpuinfodb.setAttribute("speed", itoa(data, buf, 10));

			dataSize = sizeof (buf);
			result = ::RegQueryValueEx (hKey, "VendorIdentifier", NULL, NULL,
						(LPBYTE)buf, &dataSize);

			if (result == ERROR_SUCCESS)
				cpuinfodb.setAttribute("vendor", buf);
			RegCloseKey (hKey);
		}
	}
	catch(...) {}

	import(cpuinfodb, SYSINFO_SECTION(SYSINFO_PROCESSOR));
}

void SystemInfo::OSInfo()
{
	char buf[2048];
	ExpatDB osinfodb;
	osinfodb.setAttribute(ELEM_TYPE_ATTR, SYSINFO_OS);
	// os info
	try
	{
		OSVERSIONINFOEX osinfo;
		memset(&osinfo, 0, sizeof(osinfo));
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		
		bool bEx = false;
		if( !(bEx = ::GetVersionEx((OSVERSIONINFO *)&osinfo)))
		{
			osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if(!::GetVersionEx((OSVERSIONINFO *)&osinfo))
				return;
		}

		buf[0]=buf[1] =0x00;
		bNtKernel = false;
		sprintf(buf, "%d.%d", osinfo.dwMajorVersion, osinfo.dwMinorVersion);
		osinfodb.setAttribute("version", buf);
		osinfodb.setAttribute("build", itoa(osinfo.dwBuildNumber, buf, 10));

		std::string suit, edition;
		switch(osinfo.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			// Windows NT
			bNtKernel = true;
			osinfodb.setAttribute("type", "Windows NT");
/*		
			if (bEx)
			{
				switch (osinfo.wProductType)
				{
				case VER_NT_WORKSTATION:
					edition = (versionInfo.dwMinorVersion > 0) ? IDS_WHISTLER_PROF : IDS_WIN2K_PROF;
					break;
				case VER_NT_DOMAIN_CONTROLLER:
					edition = IDS_WIN2K_DOMAIN_CONTROLLER;
					break;
				case VER_NT_SERVER:
					edition = (versionInfo.dwMinorVersion > 0) ? IDS_WHISTLER_SERVER : IDS_WIN2K_SERVER;
					break;
				}
				osinfodb.setAttribute("edition", edition.c_str());
				
				if (osinfo.wSuiteMask & VER_SUITE_DATACENTER)
					suit += IDS_SUITE_DATACENTER "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_ENTERPRISE)
					suit += IDS_SUITE_ENTERPRISE "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_BACKOFFICE)
					suit += IDS_SUITE_BACKOFFICE "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_SMALLBUSINESS)
					suit += IDS_SUITE_SMALLBUSINESS "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED)
					suit += IDS_SUITE_SMALLBUSINESS_RESTRICTED "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_TERMINAL)
					suit += IDS_SUITE_TERMINAL "; ";
				
				if (osinfo.wSuiteMask & VER_SUITE_PERSONAL)
					suit += IDS_SUITE_PERSONAL "; ";
				
				osinfodb.setAttribute("suit", suit.c_str());
			}
			else
			{
				// there is no direct way of telling from ODVERSIONINFO thats is it 
				// workstation or server version.
				// There we need to check in the registry.
				if (versionInfo.dwMajorVersion <= 4)
				{
					m_bstrPlatform.LoadString (IDS_WINDOWS_NT);
				}
				else if (versionInfo.dwMajorVersion == 5 &&	versionInfo.dwMinorVersion == 0)
				{
					m_bstrPlatform.LoadString (IDS_WINDOWS_2000);
				}
				else if (versionInfo.dwMajorVersion == 5 &&
					versionInfo.dwMinorVersion > 0)
				{
					m_bstrPlatform.LoadString (IDS_WINDOWS_WHISTLER);
				}
				
				dataSize = sizeof (data);
				result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,
					"System\\CurrentControlSet\\Control\\ProductOptions",
					0, KEY_QUERY_VALUE, &hKey);
				
				// If there is error in opening the registry key, return
				if (result != ERROR_SUCCESS)
				{
					dwError = ::GetLastError ();
					MakeErrorDesc (dwError, lpErrDesc);
					return Error (lpErrDesc, IID_IOSInformation, E_FAIL);
				}
				
				result = ::RegQueryValueEx (hKey, _T("ProductType"), NULL, NULL, (LPBYTE) data,
					&dataSize);
				
				// Make sure to close the reg key
				RegCloseKey (hKey);
				
				if (result != ERROR_SUCCESS)
				{
					dwError = GetLastError ();
					MakeErrorDesc (dwError, lpErrDesc);
					return Error (lpErrDesc, IID_IOSInformation, E_FAIL);
				}
				
				// Check what string has been returned
				if (lstrcmpi (data, "WinNT") == 0)
				{
					resourceID = IDS_NT_WORKSTATION;
				}
				else if (lstrcmpi (data, "ServerNT") == 0)
				{
					resourceID = IDS_NT_SERVER;
				}
				else
				{
					resourceID = IDS_NT_DOMAIN_CONTROLLER;
				}
				
				bstrTemp.LoadString (resourceID);
				m_bstrPlatform.Append (_T ("-"));
				m_bstrPlatform.Append (bstrTemp);
			}
			
			// Check the version number
			switch (versionInfo.dwMajorVersion)
			{
			case 3:
				resourceID = IDS_NT_351;
				break;
			case 4:
				resourceID = IDS_NT_40;
				break;
			case 5:
				resourceID = IDS_NT_50;
				if (versionInfo.dwMinorVersion > 0)
				{
					resourceID = IDS_NT_51;
				}
				break;
			}
			
			m_bstrMinorVersion.LoadString (resourceID);
*/

			break;
			
		case VER_PLATFORM_WIN32_WINDOWS:
			bW9xKernel =true;
			// Windows 98 will have be greater than 4.0
			if (osinfo.dwMajorVersion > 4 || ((osinfo.dwMajorVersion = 4) && (osinfo.dwMinorVersion > 0)))
				osinfodb.setAttribute("type", "Windows 98");
			else
				osinfodb.setAttribute("type", "Windows 95");
			break;
			
		case VER_PLATFORM_WIN32s:
			//Windows 3.1
			osinfodb.setAttribute("type", "Windows 3.x");
			break;
			
		default:
			// Unknown OS
			osinfodb.setAttribute("type", "Unknown Microsoft Windows");
			break;
		} // switch
	}
	catch(...) {}
	import(osinfodb, SYSINFO_SECTION(SYSINFO_OS));
}

void SystemInfo::localeInfo()
{
	char buf[2048];
	ExpatDB localedb;
	localedb.setAttribute(ELEM_TYPE_ATTR, "locale");
	// locale info
	try
	{
		// Retrieves the active input locale identifier.
		HKL hKl = ::GetKeyboardLayout (0);
		
		// LOWORD of the returned value contains the language identifier.
		LANGID langID = LOWORD ((DWORD)hKl);
		
		// Retirieve the information about the locale.
		LCID lcID = MAKELCID (langID, SORT_DEFAULT);
		
		::GetLocaleInfo (lcID, LOCALE_IDEFAULTANSICODEPAGE, buf, 7);
		localedb.setAttribute("codepage", buf);
		
		::GetLocaleInfo (lcID, LOCALE_IDEFAULTCODEPAGE, buf, 7);
		localedb.setAttribute("oemcodepage", buf);
		
		::GetLocaleInfo (lcID, LOCALE_SENGCOUNTRY , buf, MAX_PATH);
		localedb.setAttribute("country", buf);
		
		::GetLocaleInfo (lcID, LOCALE_SENGLANGUAGE , buf, MAX_PATH);
		localedb.setAttribute("language", buf);
		
		::GetLocaleInfo (lcID, LOCALE_STIMEFORMAT , buf, 100);
		localedb.setAttribute("timeformat", buf);
		
		::GetLocaleInfo (lcID, LOCALE_SLONGDATE  , buf, 100);
		localedb.setAttribute("dateformat", buf);
		
		::GetLocaleInfo (lcID, LOCALE_SCURRENCY, buf, 7);
		localedb.setAttribute("currency", buf);
		
		::GetLocaleInfo (lcID, LOCALE_ITIME, buf, 3);
		localedb.setAttribute("tfspec", (atoi(buf) == 0) ? "AM/PM 12-hour format" : "24-hour format");
		
		// Get calendar type
		::GetLocaleInfo (lcID, LOCALE_ICALENDARTYPE, buf, 3);
		static const char* calendartbl[] =
		{
			"Unknown",
			"Gregorian - Localized",
			"Gregorian - English",
			"Year of - Japan",
			"Year of - Taiwan",
			"Tangun Era - Korea",
			"Hijri - Arabic lunar",
			"Thai",
			"Hebrew - Lunar",
			"Gregorian Middle East French",
			"Gregorian Arabic",
			"Gregorian Transliterated English",
			"Gregorian Transliterated French"
		};

		int i=atoi(buf);
		i= (i<1 || i>=(sizeof(calendartbl)/ sizeof(char*)))?0:i;
		localedb.setAttribute("calendar", calendartbl[i]);

	}
	catch(...) {}

	import(localedb, SYSINFO_SECTION("locale"));
	
}

void SystemInfo::initInfo()
{
	cpuinfo();
	OSInfo();
	localeInfo();

	refreshNetIFInfo();
	refreshEnvInfo();
/*
	try
	{
		bMice = (GetSystemMetrics(SM_MOUSEPRESENT)!=0);
		if (bMice)
		{
			nMiceBtn =GetSystemMetrics(SM_CMOUSEBUTTONS);
//			bMiceWheel =(GetSystemMetrics(SM_MOUSEWHEELPRESENT)!=0);
		}
		else
		{
			nMiceBtn = 0;
			// bMiceWheel =false;
		}

//		nMonitor = GetSystemMetrics(SM_CMONITORS);
//		if (nMonitor < 0)
//			nMonitor = 1;
	}
	catch(...) {}
*/

	
}

void SystemInfo::refreshMemInfo()
{
	char buf[2048];

	// Memory
	try
	{
		deleteEntry(SYSINFO_SECTION("memory"));

		ExpatDB memdb;
		{
			Timestamp now, lnow;
			now.toLocal(lnow);
			memdb.setAttribute("asof", lnow.display_str(buf, true));
		}
		memdb.setAttribute(ELEM_TYPE_ATTR, "memory");

		MEMORYSTATUS ms;

		memset(&ms, 0x00, sizeof(ms));
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);

		memdb.newChild("memspace");
		memdb.setAttribute("type", "physical");
		memdb.setAttribute("total", itoa(ms.dwTotalPhys, buf, 10));
		memdb.setAttribute("available", itoa(ms.dwAvailPhys, buf, 10));
		memdb.openParent();
	
		memdb.newChild("memspace");
		memdb.setAttribute("type", "virtual");
		memdb.setAttribute("total", itoa(ms.dwTotalVirtual, buf, 10));
		memdb.setAttribute("available", itoa(ms.dwAvailVirtual, buf, 10));
		memdb.openParent();
	
		memdb.newChild("memspace");
		memdb.setAttribute("type", "pagefile");
		memdb.setAttribute("total", itoa(ms.dwTotalPageFile, buf, 10));
		memdb.setAttribute("available", itoa(ms.dwAvailPageFile, buf, 10));
		memdb.openParent();

		import(memdb, SYSINFO_SECTION("memory"));
	}
	catch(...) {}
}

void SystemInfo::refreshDisplayInfo()
{
	char buf[2048];
	// display
	try
	{
		deleteEntry(SYSINFO_SECTION("display"));

		ExpatDB dispdb;
		dispdb.setAttribute(ELEM_TYPE_ATTR, "display");

		HWND lHwnd = GetDesktopWindow();
		if (lHwnd)
		{
			HDC lDC = GetDC(lHwnd);
			if (lDC)
			{
				dispdb.setAttribute("horz",	itoa(GetDeviceCaps(lDC, HORZRES), buf, 10));
				dispdb.setAttribute("vert",	itoa(GetDeviceCaps(lDC, VERTRES), buf, 10));
				dispdb.setAttribute("colorbit", itoa(GetDeviceCaps(lDC, BITSPIXEL), buf, 10));
				ReleaseDC(lHwnd, lDC);
			}
		}

		import(dispdb, SYSINFO_SECTION("display"));
	}
	catch(...) {}
}

void SystemInfo::refreshProcInfo()
{
	deleteEntry(SYSINFO_SECTION("processes"));

	ExpatDB procdb;
	procdb.setAttribute(ELEM_TYPE_ATTR, "processes");

/*
    if (bW9xKernel)
	{
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
		if (hSnap == 0)
			return;

		MODULEENTRY32 modules;
		memset(&modules, 0x00, sizeof(modules));
        module.dwSize = sizeof(module);
		HANDLE hRet = Module32First(hSnap, &module);

		while (hRet != NULL)
		{
            fi.pathname = module.szExePath;
			associateFileinfo(fi);
			vFiles.push_back(fi);
            hRet = Module32Next(hSnap, module)
        }

        CloseHandle(hSnap)
	}
	else */
	if (bNtKernel)
	{
        HANDLE hProcess = GetCurrentProcess();
        if (hProcess !=NULL)
		{
			try
			{
				HMODULE Modules[32];
				DWORD nCbNeeded;
				HMODULE hLib = ::LoadLibrary("psapi.dll");
				if (hLib==NULL)
					return;
				
				typedef BOOL (*EnumProcessModules_t) (HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
				EnumProcessModules_t EnumProcessModules = (EnumProcessModules_t) ::GetProcAddress(hLib, "EnumProcessModules");
				
				if (EnumProcessModules!=NULL && EnumProcessModules(hProcess, Modules, sizeof(Modules), &nCbNeeded))
				{
					for(int i = 0; i <=(nCbNeeded / 8) - 1; i++)
					{
						char fn[MAX_PATH];
						DWORD len = GetModuleFileName(Modules[i], fn, MAX_PATH-1);
						if (len <= 0)
							continue;

						if (!procdb.openEntry(SYSINFO_SECTION("processes"), true)
							|| !procdb.newChild("process"))
							continue;

						fileinfo(procdb, fn);
					}
				}
			}
			catch(...){}
		}
	}

	import(procdb, SYSINFO_SECTION("processes"));

}

bool SystemInfo::fileinfo(ExpatDB& filedb, const char* filename)
{
	if (filename==NULL || *filename==0x00)
		return false;

	filedb.setAttribute("file", filename);

//	fileinfo.timestamp = fileinfo.size =0;
//	fileinfo.fversion = fileinfo.pversion = fileinfo.pname= fileinfo.compname=fileinfo.fdesc="<n/a>";

	// file version
	DWORD hVersion, VersionSize;

    VersionSize = GetFileVersionInfoSize((char*) filename, &hVersion);
	if (VersionSize >0)
	{
		BYTE* sBlock = new BYTE[VersionSize+1];
		
		if (GetFileVersionInfo((char*) filename, hVersion, VersionSize, sBlock)>0)
		{
			char str[MAX_PATH];
			VS_FIXEDFILEINFO *vInfo;
			UINT nSize;
            if (VerQueryValue(sBlock, "\\", (LPVOID*) &vInfo, &nSize) && nSize>0)
			{
				sprintf(str, "%d,%d,%d,%d",
						HIWORD(vInfo->dwFileVersionMS),
						LOWORD(vInfo->dwFileVersionMS),
						HIWORD(vInfo->dwFileVersionLS),
						LOWORD(vInfo->dwFileVersionLS));
				filedb.setAttribute("fileVersion", str);

				sprintf(str, "%d,%d,%d,%d",
						HIWORD(vInfo->dwProductVersionMS),
						LOWORD(vInfo->dwProductVersionMS),
						HIWORD(vInfo->dwProductVersionLS),
						LOWORD(vInfo->dwProductVersionLS));
				filedb.setAttribute("productVersion", str);
			}

			LPVOID lpInfo;

            if (VerQueryValue(sBlock, "\\VarFileInfo\\Translation", (LPVOID*)&lpInfo, &nSize) && nSize>0)
			{
				char subBlock[MAX_PATH], *p=subBlock;
				DWORD lanC;
				memcpy(&lanC, lpInfo, 4);
				sprintf(subBlock, "\\StringFileInfo\\%02x%02x%02x%02x\\" ,
					    (lanC & 0xff00)>>8, lanC & 0xff,(lanC & 0xff000000)>>24, 
                        (lanC & 0xff0000)>>16);
				p =subBlock + strlen(subBlock);

				strcpy(p, "ProductName");
				if (VerQueryValue(sBlock, subBlock, (LPVOID*)&lpInfo, &nSize) && nSize>0)
					filedb.setAttribute("productName", (char*)lpInfo);

				strcpy(p, "CompanyName");
				if (VerQueryValue(sBlock, subBlock, (LPVOID*)&lpInfo, &nSize) && nSize>0)
					filedb.setAttribute("companyName", (char*)lpInfo);

				strcpy(p, "FileDescription");
				if (VerQueryValue(sBlock, subBlock, (LPVOID*)&lpInfo, &nSize) && nSize>0)
					filedb.setAttribute("fileDescription", (char*)lpInfo);
			}
		}

		delete []sBlock;
	}

	char buf[2048];
	SECURITY_ATTRIBUTES sa;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile !=INVALID_HANDLE_VALUE)
	{
		DWORD fsz= GetFileSize(hFile, NULL);
		filedb.setAttribute("size", itoa(fsz, buf, 10));
/*
		if (GetFileTime(hFile, FTCreateTime, FTModTime, FTAccessTime))
		{
			switch (FTimeType)
			{
			case FT_ACCESSED:
				FileTimeToLocalFileTime(FTAccessTime, FTLocTime);
				FileTimeToSystemTime(FTLocTime, STLocTime);
				break;
			case FT_CREATED:
				FileTimeToLocalFileTime(FTCreateTime, FTLocTime)
				FileTimeToSystemTime(FTLocTime, STLocTime);
				break;
			case FT_MODIFIED:
				FileTimeToLocalFileTime(FTModTime, FTLocTime)
				FileTimeToSystemTime(FTLocTime, STLocTime)
				break;
			}

		}
*/
		CloseHandle(hFile);
	}

	return true;
}

#define MAX_INTERFACES	32

void SystemInfo::refreshNetIFInfo()
{
		deleteEntry(SYSINFO_SECTION("netinterfaces"));

		ExpatDB nifdb;
		nifdb.setAttribute(ELEM_TYPE_ATTR, "netinterfaces");

#ifdef WIN32

	SOCKET sd;

    if ((sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0)) == SOCKET_ERROR)
		return;

    INTERFACE_INFO InterfaceList[MAX_INTERFACES];
    unsigned long nBytesReturned;

    if (WSAIoctl(sd,
				SIO_GET_INTERFACE_LIST,
				0,
				0,
				&InterfaceList,
				sizeof(InterfaceList),
				&nBytesReturned,
				0,
				0) == SOCKET_ERROR)
		return;

	int ifcount=nBytesReturned / sizeof(INTERFACE_INFO);

	if (ifcount <=0)
		return;

	char buf[256];
    for (int i = 0; i < ifcount; ++i)
	{
	    sockaddr_in *pAddress;

		sprintf(buf, "if%02d", i);

		nifdb.newChild("nif");
		nifdb.setAttribute("name", buf);

		// ----
		pAddress = (sockaddr_in *) &(InterfaceList[i].iiAddress);
		nifdb.setAttribute("ip", inet_ntoa(pAddress->sin_addr));

		uint64 ipaddr = pAddress->sin_addr.S_un.S_addr;

		// ----
		pAddress = (sockaddr_in *) &(InterfaceList[i].iiNetmask);
		nifdb.setAttribute("netmask", inet_ntoa(pAddress->sin_addr));
		
		uint64 netmask = pAddress->sin_addr.S_un.S_addr;

		// ----
		uint64 subnet = ipaddr & netmask;
		uint64 broadcastIpExpected = (~netmask & 0xffffffff) | subnet;

//		pAddress->sin_addr.S_un.S_addr=broadcastIpExpected;
//		SString b = inet_ntoa(pAddress->sin_addr);

		pAddress = (sockaddr_in *) &(InterfaceList[i].iiBroadcastAddress);
		pAddress->sin_addr.S_un.S_addr &=broadcastIpExpected;
		nifdb.setAttribute("broadcast", inet_ntoa(pAddress->sin_addr));

		nifdb.setAttribute("flags", ltoa(InterfaceList[i].iiFlags, buf, 16));

		nifdb.openParent();
    }

	shutdown(sd, 2);
	closesocket(sd);

#else

	int				sockfd;
	struct ifconf	ifc;
	struct ifreq	ifrlist[MAX_INTERFACES];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	ifc.ifc_len = sizeof(ifrlist);
	ifc.ifc_req = ifrlist;

	ioctl(sockfd, SIOCGIFCONF, &ifc);

	int ifcount= ifc.ifc_len / sizeof(struct ifreq);

	for (int i=0; i < ifcount; i++)
	{
		int len = sizeof(struct ifreq);
		nif_t aNif;

		struct ifreq		ifreq_rec;
		struct sockaddr_in *sinptr;

		#ifdef	HAVE_SOCKADDR_SA_LEN
		len =(len < ifrlist[i].ifr_addr.sa_len) ? ifrlist[i].ifr_addr.sa_len : len;		// length > 16
		#endif

		aNif.family= ifrlist[i].ifr_addr.sa_family;
		aNif.flags = ifrlist[i].ifr_flags;

		switch (aNif.family)
		{
			case AF_INET:
				aNif.name = ifrlist[i].ifr_name;
				sinptr = (struct sockaddr_in *) &ifrlist[i].ifr_addr;
				aNif.addr = inet_ntoa(sinptr->sin_addr);

				ifreq_rec = ifrlist[i]
				ioctl(sockfd, SIOCGIFNETMASK, &ifreq_rec);
				sinptr = (struct sockaddr_in*) &ifreq_rec.ifr_broadaddr;
				aNif.netmask = inet_ntoa(sinptr->sin_addr);

				if (aNif.flags & IFF_BROADCAST)
				{
					ifreq_rec = ifrlist[i]
					ioctl(sockfd, SIOCGIFBRDADDR, &ifreq_rec);
					sinptr = (struct sockaddr_in*) &ifreq_rec.ifr_broadaddr;

					aNif.broadaddr = inet_ntoa(sinptr->sin_addr);
				}

				break;

			default:
				printf("%s\n", ifrlist[i].ifr_name);
		}

		mNetIFs.push_back(aNif);
	}
	
	shutdown(sockfd, 2);
	close(sockfd);
#endif

	import(nifdb, SYSINFO_SECTION("netinterfaces"));
}


void SystemInfo::refreshEnvInfo()
{
	deleteEntry(SYSINFO_SECTION("environment"));
	
	ExpatDB envdb;
	envdb.setAttribute(ELEM_TYPE_ATTR, "environment");
	
	int i=0;
	for (const char* p =_environ[i]; p!=NULL ; p= _environ[++i])
	{
		std::string exp = p;
		int pos = exp.find_first_of('=');
		if (pos<=0 || pos>=exp.length())
			continue;
		
		envdb.newChild("var");
		envdb.setAttribute("name", exp.substr(0, pos).c_str());
		envdb.setContent(exp.substr(pos+1).c_str());
		envdb.openParent();
	}
	import(envdb, SYSINFO_SECTION("environment"));
}

ENTRYDB_NAMESPACE_END
