// SystemInfo.cpp: implementation of the SystemInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "MPFLogHandler.h"
USE_MPF_NAMESPACE

#include "stdafx.h"
#include "SystemInfo.h"
#include "float.h"
#include "SystemInfo_def.h"

MPF_SYSTEMINFO_NAMESPACE_BEGIN

//***************CPU************************//

LARGE_INTEGER		SystemInfo::liOldIdleTime		={0,0};
LARGE_INTEGER		SystemInfo::liOldSystemTime		={0,0};
double				SystemInfo::lasttraffic			=0.0;
int					SystemInfo::CurrentInterface	=-1;
int					SystemInfo::CurrentTrafficType	= AllTraffic;

vector<string>		SystemInfo::Interfaces;		
vector<long>		SystemInfo::Bandwidths;	
map<string,long>	SystemInfo::TotalTraffics;

	
void SystemInfo::Init()
{	
	// init CPU
	CPUGetUsage();
	
	// init Network
	NetworkGetInterfaces();
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
		GetModuleHandle("ntdll"),
		"NtQuerySystemInformation"
		);
	
	if (!NtQuerySystemInformation)
		return 0;
	
	
	// get number of processors in the system
	status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
	if (status != NO_ERROR)
		return 0;
	
	// get new system time
	status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
	if (status!=NO_ERROR)
		return 0;
	
	// get new CPU's idle time
	status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
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
		LONG result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
		
		// Check if the function has succeeded.
		if (result == ERROR_SUCCESS)
		{
			char temp[512];
			DWORD dataSize=sizeof(temp);
			ZeroMemory(temp,dataSize);

			result = ::RegQueryValueEx (hKey, "ProcessorNameString", NULL, NULL,
				(LPBYTE)temp, &dataSize);
			
			if (result == ERROR_SUCCESS)
			{
				strRet+=temp;
			}
			else
			{
				MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() RegQueryValueEx failed");
			}
		
			RegCloseKey (hKey);
		}
		else
		{
			MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() RegOpenKeyEx failed");
		}
	}
	catch(...) 
	{
		MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::CPUGetFixInfo() Got exception");
	}

	string::size_type realbegin = strRet.find_first_not_of(" ",0);
	if(realbegin!=string::npos)
	{
		strRet = strRet.substr(realbegin);
	}
	return strRet;
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
		MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::MemoryGetAvailable() Got exception");
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
		MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::MemoryGetTotal() Got exception");
	}
	return ms.dwTotalPhys;
}

//////////////////////////////////////////////////////////////////////////
// for Network
//////////////////////////////////////////////////////////////////////////
double SystemInfo::NetworkGetTraffic(int interfaceNumber)
{
	try
	{
#define DEFAULT_BUFFER_SIZE 40960L
		
		string InterfaceName;
		
		InterfaceName = Interfaces.at(interfaceNumber);
		
		unsigned char *data = new unsigned char [DEFAULT_BUFFER_SIZE];

		DWORD type;

		DWORD size = DEFAULT_BUFFER_SIZE;

		DWORD ret;
		
		while((ret = RegQueryValueEx(
			HKEY_PERFORMANCE_DATA, "510", NULL, &type, data, &size)) != ERROR_SUCCESS) 
		{
			if(ret == ERROR_MORE_DATA) 
			{
				size += DEFAULT_BUFFER_SIZE;		
				delete [] data;
				data = new unsigned char [size];
			} 
			else 
			{
				delete [] data;
				return 1;
			}
		}

		PERF_DATA_BLOCK *dataBlockPtr = (PERF_DATA_BLOCK *)data;
		
		PERF_OBJECT_TYPE *objectPtr = FirstObject(dataBlockPtr);
		
		for(int a=0 ; a<(int)dataBlockPtr->NumObjectTypes ; a++) 
		{
			char nameBuffer[255];
		
			if(objectPtr->ObjectNameTitleIndex == 510) 
			{
				DWORD processIdOffset = ULONG_MAX;
			
				PERF_COUNTER_DEFINITION *counterPtr = FirstCounter(objectPtr);
			
				for(int b=0 ; b<(int)objectPtr->NumCounters ; b++) 
				{

					if((int)counterPtr->CounterNameTitleIndex == CurrentTrafficType)
						processIdOffset = counterPtr->CounterOffset;
		
					counterPtr = NextCounter(counterPtr);
				}

				if(processIdOffset == ULONG_MAX) {
					delete [] data;
					return 1;
				}
				
				PERF_INSTANCE_DEFINITION *instancePtr = FirstInstance(objectPtr);
				
				DWORD fullTraffic;
				DWORD traffic;
				
				for(b=0 ; b<objectPtr->NumInstances ; b++) 
				{
					wchar_t *namePtr = (wchar_t *) ((BYTE *)instancePtr + instancePtr->NameOffset);
				
					PERF_COUNTER_BLOCK *counterBlockPtr = GetCounterBlock(instancePtr);
					
					char *pName = WideToMulti(namePtr, nameBuffer, sizeof(nameBuffer));
					
					fullTraffic = *((DWORD *) ((BYTE *)counterBlockPtr + processIdOffset));

					TotalTraffics.insert(pair<string,long>(InterfaceName,fullTraffic));

					if(stricmp(InterfaceName.c_str(),pName)==0)
					{
						traffic = *((DWORD *) ((BYTE *)counterBlockPtr + processIdOffset));
						double acttraffic = (double)traffic;
						double trafficdelta;
						
						if(CurrentInterface != interfaceNumber)
						{
							lasttraffic = acttraffic;
							trafficdelta = 0.0;
							CurrentInterface = interfaceNumber;
						}
						else
						{
							trafficdelta = acttraffic - lasttraffic;
							lasttraffic = acttraffic;
						}
						delete [] data;
						return(trafficdelta);
					}
					
			
					instancePtr = NextInstance(instancePtr);
				}
			}
			
			
			objectPtr = NextObject(objectPtr);
		}
		
		delete [] data;
		return 0;
	}

	catch(...)
	{
		MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::NetworkGetTraffic() Got exception");
		return 0;
	}
}


BOOL SystemInfo::NetworkGetInterfaces()
{
	try
	{
#define DEFAULT_BUFFER_SIZE 40960L
		
		Interfaces.clear();

		unsigned char *data = (unsigned char*)malloc(DEFAULT_BUFFER_SIZE);
		DWORD type;
		DWORD size = DEFAULT_BUFFER_SIZE;
		DWORD ret;
		
		char s_key[4096];
		sprintf( s_key , "%d" , 510 );
		
		ZeroMemory(data, DEFAULT_BUFFER_SIZE);
		while((ret = RegQueryValueEx(HKEY_PERFORMANCE_DATA, s_key, 0, &type, data, &size)) != ERROR_SUCCESS) {
			while(ret == ERROR_MORE_DATA) 
			{
				size += DEFAULT_BUFFER_SIZE;
				data = (unsigned char*) realloc(data, size);
			} 
			if(ret != ERROR_SUCCESS)
			{
				free(data);
				return FALSE;
			}
		}
		
		PERF_DATA_BLOCK	 *dataBlockPtr = (PERF_DATA_BLOCK *)data;
		
		PERF_OBJECT_TYPE *objectPtr = FirstObject(dataBlockPtr);
		
		for(int a=0 ; a<(int)dataBlockPtr->NumObjectTypes ; a++) 
		{
			char nameBuffer[512];
			if(objectPtr->ObjectNameTitleIndex == 510) 
			{
				DWORD processIdOffset = ULONG_MAX;
				PERF_COUNTER_DEFINITION *counterPtr = FirstCounter(objectPtr);
				
				for(int b=0 ; b<(int)objectPtr->NumCounters ; b++) 
				{
					if(counterPtr->CounterNameTitleIndex == 520)
						processIdOffset = counterPtr->CounterOffset;
					
					counterPtr = NextCounter(counterPtr);
				}
				
				if(processIdOffset == ULONG_MAX) {
					free(data);
					return 1;
				}
				
				PERF_INSTANCE_DEFINITION *instancePtr = FirstInstance(objectPtr);
				
				for(b=0 ; b<objectPtr->NumInstances ; b++) 
				{
					wchar_t *namePtr = (wchar_t *) ((BYTE *)instancePtr + instancePtr->NameOffset);
					PERF_COUNTER_BLOCK *counterBlockPtr = GetCounterBlock(instancePtr);
					ZeroMemory(nameBuffer, sizeof(nameBuffer));
					char *pName = WideToMulti(namePtr, nameBuffer, sizeof(nameBuffer));
					DWORD bandwith = *((DWORD *) ((BYTE *)counterBlockPtr + processIdOffset));				
					long tottraff = 0;

					Interfaces.push_back(pName);
					Bandwidths.push_back(bandwith);
					TotalTraffics.insert(pair<string,long>("",tottraff));  // initial 0, just for creating the list
					
					instancePtr = NextInstance(instancePtr);
				}
			}
			objectPtr = NextObject(objectPtr);
		}
		free(data);
		
		return TRUE;
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_WARNING, "SystemInfo::NetworkGetInterfaces() Got exception");
		return FALSE;
	}
}


int SystemInfo::NetworkGetInterfacesCount()
{
	int nSize;
	if((nSize=Interfaces.size())==0)return 0;
	return nSize-1;
}


BOOL SystemInfo::NetworkGetInterfaceName(string &InterfaceName, int index)
{
	int nSize;
	if((nSize=Interfaces.size())==0 ||
		nSize-1<index)
		return FALSE;

	InterfaceName=Interfaces.at(index);
	return TRUE;
}


DWORD SystemInfo::NetworkGetInterfaceBandwidth(int index)
{
	int nSize=Bandwidths.size();
	if(nSize==0)return 0;
	if(nSize-1<index)return 0;

	return Bandwidths.at(index)/ 8;
}


DWORD SystemInfo::NetworkGetInterfaceTotalTraffic(int index)
{
	DWORD	totaltraffic = 0;
	int nSize;
	if((nSize=TotalTraffics.size())==0 || nSize-1<index)		
		return 0;
	
	if((nSize=Interfaces.size())==0 || nSize-1<index)
		return 0;

	string strInter=Interfaces.at(index);

	map<string,long>::iterator it;
	it=TotalTraffics.find(strInter);
 
	if(it!=TotalTraffics.end())
	{
		totaltraffic = it->second; 
		TotalTraffics.erase(it);
	}

	if(totaltraffic == 0.0)
	{
		NetworkGetTraffic(index);
		it=TotalTraffics.find(strInter);
		totaltraffic = it->second; 
		TotalTraffics.erase(it);
	}
		
	return(totaltraffic);
}

void SystemInfo::NetworkSetTrafficType(int trafficType)
{
	CurrentTrafficType = trafficType;
}
MPF_SYSTEMINFO_NAMESPACE_END