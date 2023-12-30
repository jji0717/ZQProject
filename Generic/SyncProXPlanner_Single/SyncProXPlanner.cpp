// SyncProXPlanner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SyncPorcXplannerDEF.h"
#include <stdio.h>
#include <iostream> 
#include <stdlib.h>

#define CONFIGFILEPATH "SyncProXplanner.xml"
#define INIFILEPATH    "synctime.ini"
#define SYNCLOGFILENAME "SyncProXplanner.log"


BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;


void ShowHelpInfo()
{
	printf("run the application as follow:\n\n");
	printf("\tuserId passwd date<XXXX-XX-XX> taskId hours comment <连续天数>\n");
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
int getUserId(std::string userId)
{
	//get peopleinfo in xplanner
	StringIntMap xplannerPersonInfos;
	bool bret = getXplannerPersonInfo(xplannerPersonInfos);
	if(!bret)
	{
		return -1;
	}
	
	StringIntMap::iterator itor = xplannerPersonInfos.find(userId);
	if(itor != xplannerPersonInfos.end())
		return itor->second;
	return -1;
}
/*
std::vector<std::string> split(std::string str,std::string pattern)  
{  
	std::string username= "111";
	std::string owner= "2222@";
	if(std::string::npos != owner.find('@'))
	{
		std::vector<std::string> result = split("/cdmifod/", "/");

		if(result.size() > 1)
		{
			username +="@";
			for(int i = result.size() -1; i > 0; i--)
			{
				username += result[i];
				if( i != 1)
					username +=".";
			}
		}
	}

	std::string::size_type pos;  
	std::vector<std::string> result;  
	str += pattern; 
	int size = str.size();  
	for(int i = 0; i< size; i++)  
	{  
		pos = str.find(pattern,i);  
		if(pos < size  && i != pos)  
		{  
			std::string s = str.substr(i,pos - i);  
			result.push_back(s);  
			i = pos+ pattern.size() - 1;  
		}  
	}  
	return result;  
}  
*/
int main(int argc, char* argv[])
{
	if(argc < 7)
	{
      ShowHelpInfo();
	  exit(0);
	}
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install handler                     \n");
		return -1;
	}

	std::string strUserID = argv[1];
	std::string strPasswd = argv[2];
	std::string date = argv[3];
	int taskId = atoi(argv[4]);
	float hours = atof(argv[5]);
	std::string comment = argv[6];

	if(taskId < 1)
	{
		printf("please Check TaskId\n");
		return 0;
	}
	else
	{
		if( !checkTask(strUserID, strPasswd, taskId))
		{
			printf("unknown taskId[%d] for user[%s]", taskId, strUserID.c_str());
			return 0;
		}
	}
    
	if(!checkDate((char*)date.c_str()))
	{
		printf("data format: XXXX-XX-XX\n");
		return 0;
	}

	if(hours <=0)
	{
		printf("hours must > 0\n");
		return 0;
	}
	int userId = getUserId(strUserID);

	if(userId < 1)
	{
       printf("unkonwn user[%s]", strUserID.c_str());
	   return 0;
	}

	int days  = 1;
	if(argc> 7)
		days = atoi(argv[7]);

	bool bSkipWeedEnd = false;
	if(days > 1)
		bSkipWeedEnd = true;

	for(; days > 0; days--)
	{
//		printf("data: %s ****************8", date.c_str());
		int nYear; 
		int nMonth;
		int nDay;
		sscanf((char*)date.c_str(), "%4d-%02d-%02d", &nYear, &nMonth, &nDay);
//		printf("data: %d %d %d\n", nYear, nMonth, nDay);
		switch(nMonth)
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if(nDay > 31)
			{
				printf("invalid date[%s]\n",date.c_str());
				return 0;
			}
			break;
		case 2:
			if((nYear % 4 && nDay > 28) || (nYear % 4 == 0 && nDay > 29) )
			{
				printf("invalid date[%s]\n",date.c_str());
				return 0;
			}
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if(nDay > 30)
			{
				printf("invalid date[%s]\n",date.c_str());
				return 0;
			}
			break;
		default:
			break;
		}
		time_t time = convertTime(date);
		struct tm* ptm  = localtime(&time);

		if(!bSkipWeedEnd  || (ptm->tm_wday != 0 && ptm->tm_wday != 6))
		{
			syncData(userId, date, taskId, hours, comment, strUserID, strPasswd);
			//printf("*********data: %s ****************\n", date.c_str());
		}
		else
			printf("skip date[%s] Week[%d]\n",date.c_str(), ptm->tm_wday );

		char buf[64]="";
		sprintf(buf, "%4d-%02d-%02d", nYear, nMonth, nDay + 1);
		date = buf;
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
		bQuit = true;	
		break;
	}
	return TRUE;
}

