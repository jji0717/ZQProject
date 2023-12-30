// SyncProXPlanner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SyncProXplannerCfg.h"
#include "SyncPorcXplannerDEF.h"
#include "SyncDataTrd.h"
#include <stdio.h>
#include <iostream> 
#include <stdlib.h>
#include "getopt.h" 

#define CONFIGFILEPATH "SyncProXplanner.xml"
#define INIFILEPATH    "synctime.ini"
#define SYNCLOGFILENAME "SyncProXplanner.log"

extern ZQ::common::Config::Loader<SyncProXplannerCfg> gSyncProXplannerCfg;
extern ZQ::common::FileLog *syncLog;

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;
SyncDataTrd* pSyncDataTrd = NULL;

void ShowHelpInfo()
{
	printf("run the application as follow:\n\n");
	printf("\t-p      indicate the SyncProXplanner.xml file path\n");
	printf("\t     e.g  -p [SyncProXplanner.xml file path]\n");
	printf("\t-a      sync data  by start date and end date for configration\n");
	printf("\t     e.g  -a -S[startDate]  -E[endData]\n");
	printf("\t-s      sync data  by start date and end date for somebody\n");
	printf("\t     e.g  -s -M[userEmail]  -U[userID]   -S[startDate]  -E[endData]\n");
	printf("\t-M      [userEmail] ProjectOpen login userEmail\n");
	printf("\t-U      [userID]    Xplanner login userID\n");
	printf("\t-S      [startDate] sync start date:  format [xxxx-xx-xx]\n");
	printf("\t-E      [endData]   sync end date:    format [xxxx-xx-xx]\n");
	printf("\t-o      automatic sync data with configration file config\n");
	printf("\t-h      display this help\n");
}
void getConfigfilePath(std::string& configPath, std::string& INIfilepath, std::string& LogFilePath)
{
	std::string    strCurDir;
	char           sModuleName[1025];

	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.rfind('\\');
	configPath = strCurDir.substr(0,nIndex + 1);

	LogFilePath = configPath + SYNCLOGFILENAME;
	INIfilepath = configPath + INIFILEPATH;
	configPath += CONFIGFILEPATH;
}
bool checkDate(char* date)
{
	int nLen = strlen(date);
	if(nLen != 10)
		return false;
	int nYear; 
	int nMonth;
	int nDay;

	if(sscanf(date, "%d-%d-%d", &nYear, &nMonth, &nDay) != 3)
		return false;
	if(nYear < 1900 && nYear > 3000)
		return false;
	if(nMonth < 1 && nMonth > 12)
		return false;
	if(nDay < 1 && nDay > 31)
		return false;
	return true;
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
      ShowHelpInfo();
	  exit(0);
	}
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install handler                     \n");
		return -1;
	}

	bool bSyncData = false;
	bool bAutomatic = false;
	bool bSyncAll = false;
	std::string userEmail = "";
	std::string userID = "";
	std::string startDate = "";
	std::string endDate = "";
	std::string configPath = "";
	std::string INIfilepath = "";
	std::string LogFilePath = "";

	getConfigfilePath(configPath, INIfilepath, LogFilePath);

    char ch;
	while((ch = getopt(argc, argv, "hosap:M:U:S:E:")) != EOF)
	{
		switch (ch)
		{
		case 'h':
			ShowHelpInfo();
			exit(0);
		case 'o':
			bAutomatic = true;
			break;
		case 'p':
            configPath = optarg;
			break;
		case 'a':
			bSyncAll = true;
			break;
		case 's':
			bSyncData = true;
			break;
		case 'M':
			userEmail = optarg;
			break;
		case 'U':
			userID = optarg;
			break;
		case 'S':
			startDate = optarg;
			if(!checkDate((char*)startDate.c_str()))
			{
				printf("the sync start date format is [xxxx-xx-xx], please check\n");
				exit(0);
			}
			break;
		case 'E':
			endDate = optarg;
			if(!checkDate((char*)endDate.c_str()))
			{
				printf("the sync end date format is [xxxx-xx-xx], please check\n");
				exit(0);
			}
			break;
		case '?':
			ShowHelpInfo();
			exit(0);
		default:
			printf("Error: unknown option specified\n");
			exit(1);
		}
	}
    ///init log file
	printf("Init log file '%s'\n", LogFilePath.c_str());

	try{
			syncLog = NULL;
			syncLog = new ZQ::common::FileLog(
			LogFilePath.c_str(),
			7,
			ZQLOG_DEFAULT_FILENUM,
            ZQLOG_DEFAULT_FILESIZE
			);
	}catch (...) {
		printf("caught unknown exception during create FileLog [%s]", LogFilePath.c_str());
		exit(1);
	}

    printf("Init log file successfully\n", LogFilePath.c_str());

	if (_access (configPath.c_str(), 0 ) != 0  )
	{
		printf("configration file '%s' not exist\n", configPath.c_str());
		return 0;
	}

	///init log file
	printf("load configration file '%s'\n", configPath.c_str());

	if(!gSyncProXplannerCfg.load(configPath.c_str(), 0))
	{
		printf("Unable to Load '%s' \n", configPath.c_str());
		return -1;
	}

	//get peopleinfo in xplanner
	StringIntMap xplannerPersonInfos;
    bool bret = getXplannerPersonInfo(gSyncProXplannerCfg.endpoint, gSyncProXplannerCfg.xpusername, gSyncProXplannerCfg.xppasswd, xplannerPersonInfos);
	if(!bret)
	{
		return -1;
	}

	//sync peopleinfo in ProjectOpen
	Peoples peoples;
    bret = getProjectOpenPersonInfo(xplannerPersonInfos, peoples);
	if(!bret)
	{
		return -1;
	}

	/// 同步指定起始日期的某人的数据
   if(bSyncData)
   {
	   if( userEmail.size() < 1 || userID.size()< 1 || startDate.size()< 1 || endDate.size()< 1)
	   {
		   printf("the correct command like this:\n");
		   printf("\t-a      sync data  by start date and end date for configration\n");
		   printf("\t     e.g  -a -S[startDate]  -E[endData]\n");
		   return -1;
	   }
	   if(endDate < startDate)
	   {
		   printf("the end date '%s' must be >= start date '%s'\n", 
			   endDate.c_str(), startDate.c_str());
		   return -1;
	   }
	   People people;
	   if(!getPeopleInfo(userEmail, userID, xplannerPersonInfos, people))
	   {
		   return -1;
	   }
	   syncData(people, startDate, endDate);
   }
   /// 同步指定起始日期配置文件中的所有人的数据
   if(bSyncAll)
   {
	   if(startDate.size()< 1 || endDate.size()< 1)
	   {
		   printf("the correct command like this:\n");
		   printf("-s      sync data  by start date and end date for somebody\n");
		   printf("   e.g  -s -m[userEmail]  -u[userID]   -S[startDate]  -E[endData]");
		   return -1;
	   }
	   Peoples::iterator itorPeople;

	   if(endDate < startDate)
	   {
		   printf("the end date '%s' must be >= start date '%s'\n", 
			   endDate.c_str(), startDate.c_str());
		   return -1;
	   }
	   for(itorPeople = peoples.begin(); itorPeople != peoples.end(); itorPeople++)
	   {
		   syncData(itorPeople->second, startDate, endDate);
	   }
   }

   if(!bAutomatic)
   {
	   return 0;
   }
   else
   {
	   pSyncDataTrd = new SyncDataTrd(peoples, INIfilepath);
	   if(!pSyncDataTrd)       
	   {
		   printf("fail to create sync data thread, out of memory");
		   exit(0);
	   }
	   pSyncDataTrd->start();
   }
	printf("\"Ctrl-C\" at any time to exit the program.\n");
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\rAutomatic data sync is waiting %c", chs[chi]);
		Sleep(200);
	}
	try
	{
		if(pSyncDataTrd)
			delete pSyncDataTrd;
		pSyncDataTrd = NULL;
		if(syncLog)
			delete syncLog;
		syncLog = NULL;
	}
	catch (...)
	{		
	}
	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if(pSyncDataTrd)
			pSyncDataTrd->stop();
		bQuit = true;	
		break;
	}
	return TRUE;
}

