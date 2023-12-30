#include "stdafx.h"
#include "SyncProXplannerCfg.h"
#include "SyncPorcXplannerDEF.h"
#include "libpq-fe.h"
#include "XPlannerSoapBinding.nsmap"
#include <algorithm>

ZQ::common::Config::Loader<SyncProXplannerCfg> gSyncProXplannerCfg("SyncProXplanner.xml");
ZQ::common::FileLog *syncLog = NULL;

void showCfg()
{
	printf("%d %s\n",gSyncProXplannerCfg.syncDays, gSyncProXplannerCfg.dayofweek.c_str());
	PeopleMap::iterator itor;
	for(itor = gSyncProXplannerCfg.peoples.begin(); itor != gSyncProXplannerCfg.peoples.end(); itor++)
	{
		printf("%-20s \t %-20s \n", itor->second.userEmail.c_str(), itor->second.userID.c_str());
		std::map<std::string, std::string>::iterator itorParam;
		for(itorParam = itor->second.peopleParam.begin(); itorParam != itor->second.peopleParam.end(); itorParam++)
		{
			printf("\t %-20s \t %-20s \n", itorParam->first.c_str(), itorParam->second.c_str());
		}
	}
} 
time_t  convertTime(std::string strtime)
{ 
	int nyear, nmonth, nday, nhours, nMin, nSec;
	char temp[64];
	memset(temp, 0 , 64);
	memcpy(temp, strtime.c_str(), strtime.size()-3);
	::sscanf(temp, "%d-%d-%d %d:%d:%d", &nyear, &nmonth, &nday, &nhours, &nMin, &nSec);

	struct tm _tm;
	memset(&_tm, 0, sizeof(struct tm));
	_tm.tm_year = nyear - 1900;
	_tm.tm_mon = nmonth -1;
	_tm.tm_mday = nday;
	_tm.tm_hour= nhours;
	_tm.tm_min = nMin;
	_tm.tm_sec = nSec;

	time_t _timet;
	_timet =  mktime(&_tm);
	return _timet;
}

void logSoapErrorMsg(const XPlannerSoapBinding& xpSoapClient)
{
	if(xpSoapClient.soap->error)
	{ 
		// following codes from soap_print_fault(cmeSoapClient.soap, stderr);
		const char **s;
		if (!*soap_faultcode(xpSoapClient.soap))
			soap_set_fault(xpSoapClient.soap);
#ifdef _DEBUG
		printf("SOAP FAULT: SocketErrNo=%d ErrorCode=%d  FaultCode=<%s> FaultString=<%s>\n", 
			xpSoapClient.soap->errnum, xpSoapClient.soap->error, *soap_faultcode(xpSoapClient.soap), *soap_faultstring(xpSoapClient.soap));
#endif

		(*syncLog)(ZQ::common::Log::L_ERROR, 
			"SOAP FAULT: SocketErrNo=%d ErrorCode=%d  FaultCode=<%s> FaultString=<%s>", 
			xpSoapClient.soap->errnum, xpSoapClient.soap->error, *soap_faultcode(xpSoapClient.soap), *soap_faultstring(xpSoapClient.soap));
        (*syncLog).flush();

		s = soap_faultdetail(xpSoapClient.soap);
		if (s && *s)
		{
#ifdef _DEBUG
	      printf("Detail: %s\n", *s);
#endif		
          (*syncLog)(ZQ::common::Log::L_ERROR,"Detail: %s", *s );
		}
	}
    (*syncLog).flush();
}
/////bool getXplannerPersonInfo()
/// get Xplanner person infomation
///@param[in]  endpoint:  Soap interface endpoint   
///@param[in]  userid:    login userID
///@param[in]  passwd:    login passwd
///@param[out] personInfos: persons collect in xplanner
///@retrun     ture: succesful get Xplanner person infomation
///            false: fail to get Xplanner person infomation
bool  getXplannerPersonInfo(std::string endpoint, std::string userid, std::string passwd, StringIntMap& personInfos)
{
#ifdef _DEBUG
	printf("get Xplanner people infomation...");
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"get XPlanner people infomation...");

	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->send_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->recv_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->userid = (char*)userid.c_str();
	xpSoapClient.soap->passwd = (char*)passwd.c_str();

	ns6__getPeopleResponse response;
	if(xpSoapClient.ns6__getPeople(response) == 0)
	{	
		// invoke successfully
		for (int i=0; i < response.getPeopleReturn->__size; ++i) 
		{   
			ns4__PersonData * t = ((ArrayOf_USCOREtns1_USCOREPersonData*)response.getPeopleReturn)->__ptr[i];   

			if( t->userId)
			{
#ifdef _DEBUG
				printf("LoginID: %-20s\tUserID: %-10d\tUserName: %s\n", t->userId, t->id, t->name);
#endif
				(*syncLog)(ZQ::common::Log::L_INFO, "LoginID: %-20s\tUserID: %-10d\tUserName: %s", t->userId, t->id, t->name );
				(*syncLog).flush();
				personInfos[t->userId] = t->id;
			}        
		}
#ifdef _DEBUG
              printf("get people infomation successfully\n");
#endif
		(*syncLog)(ZQ::common::Log::L_INFO,"get XPlanner people infomation successfully" );
	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}
	return true;
}
/////bool getProjectOpenPersonInfo()
/// get ProjectOpen person infomation
///@param[in]   XplannerPersonInfos: persons infomation in Xplanner   
///@param[out]  peoples: peoples collect
///@retrun     ture: succesful get ProjectOpen person infomation
///            false: fail to get ProjectOpen person infomation

bool  getProjectOpenPersonInfo(StringIntMap& xplannerPersonInfos, Peoples& peoples)
{
#ifdef _DEBUG
	printf("get Project Open people infomation...");
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"Enter get ProjectOpen peoples infomation ...");

	char* pghost = (char*)gSyncProXplannerCfg.host.c_str();      //IP address string is also ok        
	char* pgport = (char*)gSyncProXplannerCfg.port.c_str();                         
	char* dbName = (char*)gSyncProXplannerCfg.dbName.c_str();     
	char* username = (char*)gSyncProXplannerCfg.username.c_str();      //postegres is the username that 
	char* password = (char*)gSyncProXplannerCfg.passwd.c_str();         //set when insalled cannot get directly	
	char* pgoptions = NULL;           
	char* pgtty = NULL;  

	PGconn* conn_pointer = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, username, password);

	if (PQstatus(conn_pointer) == CONNECTION_BAD)       
	{
#ifdef _DEBUG
		printf("cannot connect to the database %s %s %s %s", pghost, pgport, dbName, username);
#endif
		(*syncLog)(ZQ::common::Log::L_ERROR, "cannot connect to the database %s %s %s %s", pghost, pgport, dbName, username);
		return false;
	}

	PeopleMap::iterator itor;
	for(itor = gSyncProXplannerCfg.peoples.begin(); itor != gSyncProXplannerCfg.peoples.end(); itor++)
	{
		int userID;
		std::string  string_for_query = "select party_id  from parties where email='" + itor->first + "'";//SQL query
		PGresult * pgresult = PQexec(conn_pointer, string_for_query.c_str());
		int	row =  PQntuples(pgresult);
		if(row > 0)
		{
			char* pvalue = PQgetvalue(pgresult, 0, 0);
			userID = atoi(pvalue);
			People people;
			people.userEmail = itor->first;
			people.userIDInProOpen = userID;
			people.userLogInID = itor->second.userID;
            
			StringIntMap::iterator itorXpId = xplannerPersonInfos.find(itor->second.userID);
			if(itorXpId == xplannerPersonInfos.end())
			{
#ifdef _DEBUG
                printf("missed match projectopen userEmail = %s with xplanner userID = %s\n", itor->first.c_str(), itor->second.userID.c_str());
#endif
				(*syncLog)(ZQ::common::Log::L_ERROR,"missed match projectopen userEmail = %s with xplanner userID = %s",
					     itor->first.c_str(), itor->second.userID.c_str());
//				return false;
				continue;
			}
			else
			{
				people.userIDInXplaner = itorXpId->second;
				peoples[itor->first] = people;
			}
#ifdef _DEBUG
			printf("ProjectOpen:%-20s%-7d Xplanner:%-20s%-7d\n",
				people.userEmail.c_str(), people.userIDInProOpen , people.userLogInID.c_str(), people.userIDInXplaner);			
#endif
			(*syncLog)(ZQ::common::Log::L_INFO, "ProjectOpen LoginId: %-20s%-7d Xplanner Login: %-20sXplanner UserID: %-7d",
				people.userEmail.c_str(), people.userIDInProOpen , people.userLogInID.c_str(), people.userIDInXplaner);
			(*syncLog).flush();
		}
		else
		{
#ifdef _DEBUG
			printf("can't find people infomation userEmail = %s with xplanner userID = %s\n", itor->first.c_str(), itor->second.userID.c_str());
#endif
            (*syncLog)(ZQ::common::Log::L_ERROR, "can't find people infomation userEmail = %s with xplanner userID = %s", 
				itor->first.c_str(), itor->second.userID.c_str());
//			return false;
		}
	}
#ifdef _DEBUG
	printf("leave getProjectOpenPersonInfo() ");
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"Leave  getProjectOpenPersonInfo() ");
	return true;
}

bool checkTask(std::string endpoint, std::string userid, std::string passwd, int taskID)
{
	INTS gTaskInfo;
	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->send_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->recv_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->userid = (char*)userid.c_str();
	xpSoapClient.soap->passwd = (char*)passwd.c_str();

	ns6__getTaskResponse response;
	if(xpSoapClient.ns6__getTask(taskID, response)== 0)
	{
		// invoke successfully

		ns4__TaskData * t = (ns4__TaskData *)response._getTaskReturn;   

		std::cout<< "\t" << t->id << "\t"  << t->name << "\t" << t->storyId << std::endl;
		gTaskInfo.push_back(t->id);
#ifdef _DEBUG
		printf("check task %d successfully\n", taskID);
#endif
		
		(*syncLog)(ZQ::common::Log::L_INFO, "check task %d successfully", taskID);
	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}
	return true;
}
/////bool syncData()
/// sync specify people data
///@param[in]  endpoint:  Soap interface endpoint   
///@param[in]  userid:    login userID
///@param[in]  passwd:    login passwd
///@param[in]  timeEdata: time entry data
///@retrun     ture: succesful add time entry date to xplanner.
///            false: fail to add time entry date to xplanner

bool  addTimeEntryToXPlanner(std::string endpoint, std::string userid, std::string passwd, ns4__TimeEntryData& timeEdata)
{
	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->send_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->recv_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->userid = (char*)userid.c_str();
	xpSoapClient.soap->passwd = (char*)passwd.c_str();

	ns6__addTimeEntryResponse response;
	if(xpSoapClient.ns6__addTimeEntry(&timeEdata, response) == 0)
	{
		time_t timeDuration = *(timeEdata.reportDate);
		struct tm* duration;
		duration = localtime(&timeDuration);
		char durationTemp[64];
		sprintf(durationTemp, "%04d-%02d-%02d\0", duration->tm_year + 1900, duration->tm_mon +1, duration->tm_mday);
#ifdef _DEBUG
		printf(""Xplanner add TimeEntry: XPlannerUserID: %d  TaskID: %d ReportDate: %s Description: %s Duration: %0.2f\n", 
			timeEdata.person1Id, timeEdata.taskId, durationTemp, 
			timeEdata.description, timeEdata.duration);
#endif

		(*syncLog)(ZQ::common::Log::L_INFO, "Xplanner add TimeEntry: XPlannerUserID: %d  TaskID: %d ReportDate: %s Description: %s Duration: %0.2f", 
			timeEdata.person1Id, timeEdata.taskId, durationTemp, 
			timeEdata.description, timeEdata.duration);
		(*syncLog).flush();
	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}

	return true;
}

/////bool syncData()
/// sync specify people data
///@param[in]  people:     people info
///@param[in]  startdate:  sync start date 
///@param[in]  enddate:    sync end date 
///@retrun     ture: succesful sync specify people data.
///            false: fail to sync specify people data
bool  syncData(People people, std::string& startdate, std::string& enddate)
{
#ifdef _DEBUG
	printf("######Enter syncData: userEmail:%s XplannerLoginID:%s StartDate:%s EndDate:%s\n######", 
		people.userEmail.c_str(), people.userLogInID.c_str(), startdate.c_str(), enddate.c_str());
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"######Enter syncData: userEmail:%s XplannerLoginID:%s StartDate:%s EndDate:%s######", 
		people.userEmail.c_str(), people.userLogInID.c_str(), startdate.c_str(), enddate.c_str());
	(*syncLog).flush();

	static STRS proIdTaskID;
	char* pghost = (char*)gSyncProXplannerCfg.host.c_str();      //IP address string is also ok        
	char* pgport = (char*)gSyncProXplannerCfg.port.c_str();                         
	char* dbName = (char*)gSyncProXplannerCfg.dbName.c_str();     
	char* username = (char*)gSyncProXplannerCfg.username.c_str();      //postegres is the username that 
	char* password = (char*)gSyncProXplannerCfg.passwd.c_str();         //set when insalled cannot get directly	
	char* pgoptions = NULL;           
	char* pgtty = NULL;  

	PGconn* conn_pointer = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, username, password);

	if (PQstatus(conn_pointer) == CONNECTION_BAD)       
	{
#ifdef _DEBUG
		printf("cannot connect to the database '%s','%s','%s','%s','%s'\n",
			pghost, pgport,dbName,username,password);
#endif
		(*syncLog)(ZQ::common::Log::L_ERROR,"cannot connect to the database '%s','%s','%s','%s','%s'",
			pghost, pgport,dbName,username,password);
		return false;
	}
	char strQuery[2048];
	memset(strQuery, 0, 2048);
	sprintf(strQuery, "select user_id, project_id, day , hours,note from im_hours where user_id = %d and day >= '%s' and day <= '%s' and issync=false",
		    people.userIDInProOpen, startdate.c_str(), enddate.c_str());

	PGresult * pgresult = PQexec(conn_pointer, strQuery);
	int	row =  PQntuples(pgresult);
	int column = PQnfields(pgresult);
	int i, k;
	for(k = 0; k < row; k++)
	{
		std::string user_id = "", project_id = "", day = "", hours = "", note = "", task_id;
		for(i = 0; i < column; i++)
		{
			char* pvalue = PQgetvalue(pgresult, k, i);
			switch(i)
			{
			case 0:
				user_id = pvalue;
				break;
			case 1:
				project_id = pvalue;
				break;
			case 2:
				day = pvalue;
				break;
			case 3:
				hours = pvalue;
				break;
			case 4:
				note = pvalue;
				break;
			default:
				break;
			}
		}
		STRS::iterator itor = proIdTaskID.find(project_id);
		if(itor == proIdTaskID.end())
		{
			std::string stringQuery = "select task_id from im_projects where project_id =" + project_id;
			PGresult * pgresult = PQexec(conn_pointer, stringQuery.c_str());
			int	row =  PQntuples(pgresult);
			if(row > 0)
			{
				char* pvalue = PQgetvalue(pgresult, 0, 0);
				task_id = pvalue;
				if(task_id.size() <1)
				{
#ifdef _DEBUG
					printf("ERROR: cannot find taskID in im_project where project_id = %s, please check\n", project_id.c_str());
#endif
					(*syncLog)(ZQ::common::Log::L_ERROR,"cannot find taskID in im_project where project_id = %s, please check", project_id.c_str());
					(*syncLog).flush();
				//	return false;
					continue;
				}
			}
			else
			{
#ifdef _DEBUG
				printf("WARNING: cannot find project infomation in im_project where project_id = %s\n", project_id.c_str());
#endif
				(*syncLog)(ZQ::common::Log::L_WARNING,"cannot find project infomation in im_project where project_id = %s", project_id.c_str());
				task_id = project_id;
				
				return false;
			}
			proIdTaskID[project_id] = task_id;
		}
		else
			task_id = itor->second;

#ifdef _DEBUG
		printf("ProjectOpen record: %-5d userID:%s  ProjectID:%s TaskId:%s day:%s  hours:%s  note:%s\n", k+1, user_id.c_str(), project_id.c_str(),task_id.c_str(), day.c_str(), hours.c_str(), note.c_str());
#endif
		(*syncLog)(ZQ::common::Log::L_INFO,"ProjectOpen record: %-5d userID:%s  ProjectID:%s  TaskId:%s day:%s  hours:%s  note:%s",
			k+1, user_id.c_str(), project_id.c_str(), task_id.c_str(), day.c_str(), hours.c_str(), note.c_str());
		
		//±£´æ¼ÇÂ¼
		time_t _time= convertTime(day);
		ns4__TimeEntryData timeEdata;
		timeEdata.taskId = atoi(task_id.c_str());
		timeEdata.person1Id = people.userIDInXplaner;
		timeEdata.reportDate =&_time ;
		timeEdata.description = (char*)note.c_str();
		timeEdata.duration = (double)atof(hours.c_str());

		bool bret = addTimeEntryToXPlanner(gSyncProXplannerCfg.endpoint, gSyncProXplannerCfg.xpusername, gSyncProXplannerCfg.xppasswd, timeEdata);
        
		if(bret)
		{
			char strQuery[2048];
			memset(strQuery, 0, 2048);
			
			sprintf(strQuery, "update im_hours set issync = true where user_id = %s and project_id = %s and day = '%s'",
				user_id.c_str(), project_id.c_str(), day.c_str());

//			(*syncLog)(ZQ::common::Log::L_INFO,"%s", strQuery);

			PGresult * pgresult = PQexec(conn_pointer, strQuery);
		}
        user_id = "", project_id = "", day = "", hours = "", note = "";
		(*syncLog).flush();
	}
#ifdef _DEBUG
	printf("######Leave syncData: %s %s %s %s\n######", 
		people.userEmail.c_str(), people.userLogInID.c_str(), startdate.c_str(), enddate.c_str());
#endif

	(*syncLog)(ZQ::common::Log::L_INFO, "######Enter syncData: userEmail:%s XplannerLoginID:%s StartDate:%s EndDate:%s######", 
		people.userEmail.c_str(), people.userLogInID.c_str(), startdate.c_str(), enddate.c_str());
    (*syncLog).flush();
	return true;
}

bool  getTimeEntryData(std::string endpoint, std::string userid, std::string passwd,int id)
{
	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->send_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->recv_timeout = gSyncProXplannerCfg.timeout;
	xpSoapClient.soap->userid = (char*)userid.c_str();
	xpSoapClient.soap->passwd = (char*)passwd.c_str();

	ns6__getTimeEntryResponse response;
	if(xpSoapClient.ns6__getTimeEntry(id, response) == 0)
	{
		ns4__TimeEntryData * t = (ns4__TimeEntryData *)response._getTimeEntryReturn; 

#ifdef _DEBUG
		printf("%d  %d  %d  %d %s\n", 
			t->id, t->person1Id, t->taskId, *t->reportDate, t->description);
#endif

		(*syncLog)(ZQ::common::Log::L_INFO, "%d  %d  %d  %d %s", 
			t->id, t->person1Id, t->taskId, *t->reportDate, t->description);
	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}	
	return true;
}

/////bool getPeopleInfo()
/// get specify people infomation
///@param[in]  userEmail: userEmail for ProjectOpen login
///@param[in]  userID: userID for Xplanner login
///@param[in]  XplannerPersonInfos : all person infomation in Xplanner
///@param[out] people : ProjectOpen userID by userEmail and Xplanner userID by Login userID 
///@return     ture: succesful get people info. false. fail to get people info
bool    getPeopleInfo(std::string& userEmail, std::string& userID, StringIntMap& xplannerPersonInfos, People& people)
{
	char* pghost = (char*)gSyncProXplannerCfg.host.c_str();      //IP address string is also ok        
	char* pgport = (char*)gSyncProXplannerCfg.port.c_str();                         
	char* dbName = (char*)gSyncProXplannerCfg.dbName.c_str();     
	char* username = (char*)gSyncProXplannerCfg.username.c_str();      //postegres is the username that 
	char* password = (char*)gSyncProXplannerCfg.passwd.c_str();         //set when insalled cannot get directly	
	char* pgoptions = NULL;           
	char* pgtty = NULL;  

	PGconn* conn_pointer = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, username, password);

	if (PQstatus(conn_pointer) == CONNECTION_BAD)       
	{
#ifdef _DEBUG
		printf("cannot connect to the database %s %s %s %s\n", pghost, pgport, dbName, username);
#endif
		(*syncLog)(ZQ::common::Log::L_ERROR, "cannot connect to the database %s %s %s %s", pghost, pgport, dbName, username);
		return false;
	}

	int userProOpenID;
	std::string  string_for_query = "select party_id  from parties where email='" + userEmail + "'";//SQL query
	PGresult * pgresult = PQexec(conn_pointer, string_for_query.c_str());
	int	row =  PQntuples(pgresult);
	if(row > 0)
	{
		char* pvalue = PQgetvalue(pgresult, 0, 0);
		userProOpenID = atoi(pvalue);
		people.userEmail = userEmail;
		people.userIDInProOpen = userProOpenID;
		people.userLogInID = userID;

		StringIntMap::iterator itorXpId = xplannerPersonInfos.find(userID);
		if(itorXpId == xplannerPersonInfos.end())
		{
#ifdef _DEBUG
			printf("missed match projectopen userEmail = %s with xplanner userID = %s\n", userEmail.c_str(), userID.c_str());
#endif
			(*syncLog)(ZQ::common::Log::L_ERROR, "missed match projectopen userEmail = %s with xplanner userID = %s", 
				userEmail.c_str(), userID.c_str());
			//				return false;
			return false;
		}
		else
		{
			people.userIDInXplaner = itorXpId->second;
		}
#ifdef _DEBUG
		printf("ProjectOpen:%-20s%-7d Xplanner:%-20s%-7d\n", people.userEmail.c_str(), people.userIDInProOpen , people.userLogInID.c_str(), people.userIDInXplaner);			
#endif
		(*syncLog)(ZQ::common::Log::L_INFO,"ProjectOpen:%-20s%-7d Xplanner:%-20s%-7d",
			people.userEmail.c_str(), people.userIDInProOpen , people.userLogInID.c_str(), people.userIDInXplaner);			
		(*syncLog).flush();
	}
	else
	{
#ifdef _DEBUG
		printf("can't find person infomation userEmail = %s with xplanner userID = %s\n", userEmail.c_str(), userID.c_str());
#endif
		(*syncLog)(ZQ::common::Log::L_ERROR,"can't find people infomation userEmail = %s with xplanner userID = %s", userEmail.c_str(), userID.c_str());
	    return false;
	}
	(*syncLog).flush();
	return true;
}