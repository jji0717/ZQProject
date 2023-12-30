// SystemInfo.cpp: implementation of the SystemInfo class.
//
//////////////////////////////////////////////////////////////////////
#include "SystemInfo.h"
//#include "MPFLogHandler.h"
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
//#include "stdafx.h"

//#include "float.h"
#include "SystemInfo_def.h"
#include <stdio.h>
#include "MPFCommon.h"

using namespace ZQ::MPF::utils;


MPF_SYSTEMINFO_NAMESPACE_BEGIN

USE_MPF_NAMESPACE
//***************CPU************************//

LARGE_INTEGER		SystemInfo::liOldIdleTime		={0,0};
LARGE_INTEGER		SystemInfo::liOldSystemTime		={0,0};
char				SystemInfo::strOsVersion[MAX_OS_VERION_STRLEN] = {0};

map<DWORD,string>	SystemInfo::Interfaces;		
map<DWORD,DWORD>	SystemInfo::Bandwidths;	
DWORD				SystemInfo::InBytes				=0;	
DWORD				SystemInfo::OutBytes			=0;
DWORD				SystemInfo::TotalBytes			=0;
time_t				SystemInfo::ElapseTime			=0;
SystemInfo::TrafficType			SystemInfo::CurrentTrafficType	=AllTraffic;

void SystemInfo::Init()
{	
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;
 
	WSAStartup( wVersionRequested, &wsaData );

	// init CPU
	CPUGetUsage();
	
	// init Network
	NetworkGetInterfaces();
}

void SystemInfo::UnInit()
{
	WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
// for CPU
//////////////////////////////////////////////////////////////////////////
int SystemInfo::CPUGetUsage()
{
	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
	
	PROCNTQSI NtQuerySystemInformation;
	
	
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	SYSTEM_TIME_INFORMATION        SysTimeInfo;
	SYSTEM_BASIC_INFORMATION       SysBaseInfo;
	double                         dbIdleTime	=.0;
	double                         dbSystemTime	=.0;
	LONG                           status;
	
	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandleA("ntdll"),
		"NtQuerySystemInformation"
		);
	
	if (!NtQuerySystemInformation)
		return 0;
	
	ULONG ulInfoLength = 0;
	// get number of processors in the system
	ulInfoLength = sizeof(SysBaseInfo);
	status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,ulInfoLength,&ulInfoLength);
	if (status != NO_ERROR)
		return 0;
	
	// get new system time
	ulInfoLength = sizeof(SysTimeInfo);
	status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,ulInfoLength,&ulInfoLength);
	if (status!=NO_ERROR)
		return 0;
	
	// get new CPU's idle time
	ulInfoLength = sizeof(SysPerfInfo);
	status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,ulInfoLength,&ulInfoLength);
	if (status != NO_ERROR)
		return 0;
	
	// if it's a first call - skip it
	if (liOldIdleTime.QuadPart != 0)
	{
		// CurrentValue = NewValue - OldValue
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
		
		// CurrentCpuIdle = IdleTime / SystemTime
		dbIdleTime = dbIdleTime / dbSystemTime;
		
		// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
		
		
	}
	
	// store new CPU's idle and system time
	liOldIdleTime = SysPerfInfo.liIdleTime;
	liOldSystemTime = SysTimeInfo.liKeSystemTime;
	return (int)dbIdleTime+1;
}

string SystemInfo::CPUGetFixInfo()
{
	string strRet="";
	// cpu info
	try
	{
		//////////////////////////////////////////////////////////////////////////
		// Query registry to get CPU info.
		// It is better to get this way since it covers more detailed info.
		//////////////////////////////////////////////////////////////////////////
		
		HKEY hKey;
		LONG result = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
		
		// Check if the function has succeeded.
		if (result == ERROR_SUCCESS)
		{
			char temp[512];
			DWORD dataSize=sizeof(temp);
			ZeroMemory(temp,dataSize);

			result = ::RegQueryValueExA (hKey, "ProcessorNameString", NULL, NULL,
				(LPBYTE)temp, &dataSize);
			
			if (result == ERROR_SUCCESS)
			{
				strRet+=temp;
			}
			else
			{
				//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() RegQueryValueEx failed");
			}
		
			RegCloseKey (hKey);
		}
		else
		{
			//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() RegOpenKeyEx failed");
		}
	}
	catch(...) 
	{
		//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() Got exception");
		return "";
	}

	string::size_type realbegin = strRet.find_first_not_of(" ",0);
	if(realbegin!=string::npos)
	{
		strRet = strRet.substr(realbegin);
	}
	return strRet;
}

//////////////////////////////////////////////////////////////////////////
// for OS
//////////////////////////////////////////////////////////////////////////
const char*	SystemInfo::OSGetVersion()
{
	static bool s_bFirst = true;
	if (s_bFirst)
	{
		OSVERSIONINFO osi;
		osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (FALSE == ::GetVersionEx(&osi))
			sfstrncpy(strOsVersion, "[Unknown]", MAX_OS_VERION_STRLEN);
		s_bFirst = false;

		if (3 == osi.dwMajorVersion)
			sfstrncpy(strOsVersion, "Microsoft Windows NT 3", MAX_OS_VERION_STRLEN);
		else if (4 == osi.dwMajorVersion)
		{
			if (VER_PLATFORM_WIN32_NT == osi.dwPlatformId)
				sfstrncpy(strOsVersion, "Microsoft Windows NT 4", MAX_OS_VERION_STRLEN);
			else if (0 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows 95", MAX_OS_VERION_STRLEN);
			else if (10 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows 98", MAX_OS_VERION_STRLEN);
			else if (90 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows Me", MAX_OS_VERION_STRLEN);
			else
				sfstrncpy(strOsVersion, "[Unknown]", MAX_OS_VERION_STRLEN);
		}
		else if (5 == osi.dwMajorVersion)
		{
			if (0 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows 2000", MAX_OS_VERION_STRLEN);
			else if (1 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows XP", MAX_OS_VERION_STRLEN);
			else if (2 == osi.dwMinorVersion)
				sfstrncpy(strOsVersion, "Microsoft Windows Server 2003 family", MAX_OS_VERION_STRLEN);
			else
				sfstrncpy(strOsVersion, "[Unknown]", MAX_OS_VERION_STRLEN);
		}
		else
			sfstrncpy(strOsVersion, "[Unknown]", MAX_OS_VERION_STRLEN);
	}

	return strOsVersion;
}

//////////////////////////////////////////////////////////////////////////
// for Memory
//////////////////////////////////////////////////////////////////////////
long SystemInfo::MemoryGetAvailable()
{
	MEMORYSTATUS ms;
	try
	{
		memset(&ms, 0x00, sizeof(ms));
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);	
	}
	catch(...) 
	{
		return 0;
		//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::MemoryGetAvailable() Got exception");
	}
	return ms.dwAvailPhys;
}

long SystemInfo::MemoryGetTotal()
{
	MEMORYSTATUS ms;
	try
	{		
		memset(&ms, 0x00, sizeof(ms));
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);			
	}
	catch(...) 
	{
		return 0;
		//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::MemoryGetTotal() Got exception");
	}
	return ms.dwTotalPhys;
}

//////////////////////////////////////////////////////////////////////////
// for Network
//////////////////////////////////////////////////////////////////////////
double SystemInfo::NetworkGetInterfaceTraffic(int interfaceNumber)
{
	double dRet=.0;

	try
	{
		DWORD				dwIndex;  
		DWORD				dwResult;  
		
		MIB_IFROW			IfRow;  
		
		dwIndex  =  interfaceNumber;  
		
		IfRow.dwIndex  =  dwIndex;
		
		// get interface detail info
		if((dwResult =  GetIfEntry(&IfRow))  !=  NO_ERROR)  
		{  
			// TODO: Log
			dRet = 0;
		}  
		else
		{
			DWORD deltaIn, deltaOut, deltaTotal;
			time_t	deltaElapse;

			// get delta bytes since last retrieve
			deltaIn		= IfRow.dwInOctets-InBytes;
			deltaOut	= IfRow.dwOutOctets-OutBytes;
			deltaTotal	= IfRow.dwInOctets + IfRow.dwOutOctets - TotalBytes;
			time(&deltaElapse);
			deltaElapse = deltaElapse-ElapseTime;

			// set current bytes
			InBytes		= IfRow.dwInOctets;
			OutBytes	= IfRow.dwOutOctets;
			TotalBytes	= IfRow.dwInOctets + IfRow.dwOutOctets;
			time(&ElapseTime);

			// fix them if overflow
			if((long)deltaIn<0)		deltaIn		= (long)deltaIn+ULONG_MAX;
			if((long)deltaOut<0)	deltaOut	= (long)deltaOut+ULONG_MAX;
			if((long)deltaTotal<0)	deltaTotal	= (long)deltaTotal+ULONG_MAX;

			switch(CurrentTrafficType) {
			case AllTraffic:
				dRet = (double)deltaTotal/(double)deltaElapse;
				break;
			case IncomingTraffic:
				dRet = (double)deltaIn/(double)deltaElapse;
				break;
			case OutGoingTraffic:
				dRet = (double)deltaOut/(double)deltaElapse;
				break;
			}
		}
	}
	catch(...)
	{
		//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::NetworkGetInterfaces() Got exception");
		dRet = .0;
	}

	return dRet;
}


BOOL SystemInfo::NetworkGetInterfaces()
{
	bool bRet = true;

	PIP_ADAPTER_INFO	pAdapterInfo = NULL;

	try
	{
		Interfaces.clear();
		Bandwidths.clear();

		DWORD				dwIndex;  
		DWORD				dwResult;  
		ULONG				OutBufLen;  
		PIP_ADAPTER_INFO	pAdapterNext;  
		MIB_IFROW			IfRow;  

		OutBufLen  =  0;  
		dwResult    =  GetAdaptersInfo(NULL,&OutBufLen);  
		if(OutBufLen<=0)
			return false;

		pAdapterInfo = new IP_ADAPTER_INFO[OutBufLen/sizeof(IP_ADAPTER_INFO)+1];

		dwResult = GetAdaptersInfo(pAdapterInfo, &OutBufLen);

		pAdapterNext  =  pAdapterInfo;  
		while(pAdapterNext  !=  NULL)  
		{  
			dwIndex  =  pAdapterNext->Index;  
			
			IfRow.dwIndex  =  dwIndex;
			
			// get interface detail info
			if((dwResult =  GetIfEntry(&IfRow))  !=  NO_ERROR)  
			{  
				// TODO: Log
				bRet = false;
				break;  
			}  

			Interfaces[dwIndex] = pAdapterNext->Description;
			Bandwidths[dwIndex] = IfRow.dwSpeed;

			InBytes		= IfRow.dwInOctets;
			OutBytes	= IfRow.dwOutOctets;
			TotalBytes	= IfRow.dwInOctets + IfRow.dwOutOctets;
			time(&ElapseTime);

			// next adapter
			pAdapterNext  =  pAdapterNext->Next;  
		}  

	}
	catch(...)
	{
		//MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::NetworkGetInterfaces() Got exception");
		bRet = false;
	}
	
	if(pAdapterInfo)
		delete []pAdapterInfo;

	return bRet;
}


int SystemInfo::NetworkGetInterfacesCount()
{
	return Interfaces.size();
}


BOOL SystemInfo::NetworkGetInterfaceName(string &InterfaceName, int index)
{
	InterfaceName="";

	if(index<0)
		return false;

	map<DWORD, string>::const_iterator iter = Interfaces.find(index);
	if(iter==Interfaces.end())
	{
		return false;
	}

	InterfaceName=Interfaces[index];
	return true;
}

int SystemInfo::NetworkGetInterfaceIndexByName(const char* itname)
{
	int nRet=-1;
	
	if(!itname)
		return -1;
	
	map<DWORD, string>::const_iterator iter = NULL;

	for(iter=Interfaces.begin(); iter!=Interfaces.end(); iter++)
	{
		if(iter->second==itname)
		{
			nRet = iter->first;
			break;
		}
	}

	return nRet;
}

int SystemInfo::NetworkGetInterfaceIndexByIP(const char* ipaddr)
{
	int nRet=-1;
	
	if(!ipaddr)
		return -1;
	
	struct hostent *hostp;
	unsigned long	hostb;
	char	addrip[32];

	if ((hostb = inet_addr(ipaddr)) == INADDR_NONE) {
		if ((hostp = gethostbyname(ipaddr)) == 0) { //use this line if you want to enter host names
			return -1;
		}
		in_addr ad;
		ad.S_un.S_un_b.s_b1 = hostp->h_addr[0];
		ad.S_un.S_un_b.s_b2 = hostp->h_addr[1];
		ad.S_un.S_un_b.s_b3 = hostp->h_addr[2];
		ad.S_un.S_un_b.s_b4 = hostp->h_addr[3];
		strncpy(addrip,inet_ntoa(ad),32);
	}
	else
	{
		strncpy(addrip, ipaddr, 32);
	}

	DWORD dwSize=0;
	::GetAdaptersInfo(NULL, &dwSize);

	if(dwSize<=0)
		return -1;

	PIP_ADAPTER_INFO aplist = new IP_ADAPTER_INFO[dwSize/sizeof(IP_ADAPTER_INFO)+1];
	PIP_ADAPTER_INFO apcurr = NULL;

	if(NO_ERROR!=::GetAdaptersInfo(aplist, &dwSize))
	{
		delete[] aplist;
		return -1;
	}

	apcurr = aplist;
	
	while(apcurr)
	{
		IP_ADDR_STRING addrlist = apcurr->IpAddressList;
		IP_ADDR_STRING* addcurr = &addrlist;

		while(addcurr)
		{
			IP_ADDRESS_STRING addr = addcurr->IpAddress;
			if(addr.String && 0==strncmp(addr.String, addrip, strlen(addr.String)))
			{
				nRet = apcurr->Index;
				break;
			}

			addcurr = addcurr->Next;
		}

		if(nRet!=-1)	// already match
			break;

		apcurr = apcurr->Next;
	}

	delete []aplist;

	return nRet;
}

DWORD SystemInfo::NetworkGetInterfaceBandwidth(int index)
{
	if(Bandwidths.size()==0)
		return 0;

	map<DWORD, DWORD>::const_iterator iter = Bandwidths.find(index);
	if(iter == Bandwidths.end())
	{
		return 0;
	}
	
	return Bandwidths[index];
}

void SystemInfo::NetworkSetTrafficType(TrafficType trafficType)
{
	CurrentTrafficType = trafficType;
}
MPF_SYSTEMINFO_NAMESPACE_END