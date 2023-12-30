#include "stdafx.h"
#include "SyncPorcXplannerDEF.h"
#include "XPlannerSoapBinding.nsmap"
#include <algorithm>

time_t  convertTime(std::string strtime)
{ 
	int nyear, nmonth, nday, nhours = 0, nMin = 0, nSec = 0;
	char temp[64];
	memset(temp, 0 , 64);
	memcpy(temp, strtime.c_str(), strtime.size());
	::sscanf(temp, "%d-%d-%d", &nyear, &nmonth, &nday);
//	printf("%d %d %d\n",nyear ,nmonth,nday);

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
		printf("SOAP FAULT: SocketErrNo=%d ErrorCode=%d  FaultCode=<%s> FaultString=<%s>\n", 
			xpSoapClient.soap->errnum, xpSoapClient.soap->error, *soap_faultcode(xpSoapClient.soap), *soap_faultstring(xpSoapClient.soap));


		s = soap_faultdetail(xpSoapClient.soap);
		if (s && *s)
		{
			printf("Detail: %s\n", *s);
		}
	}
}

/////bool syncData()
/// sync specify people data
///@param[in]  endpoint:  Soap interface endpoint   
///@param[in]  userid:    login userID
///@param[in]  passwd:    login passwd
///@param[in]  timeEdata: time entry data
///@retrun     ture: succesful add time entry date to xplanner.
///            false: fail to add time entry date to xplanner

#define timeout (60)
static std::string endpoint = "http://sp-server:9090/soap/XPlanner?wsdl";

bool  addTimeEntryToXPlanner(std::string endpoint, std::string userid, std::string passwd, ns4__TimeEntryData& timeEdata)
{
	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = timeout;
	xpSoapClient.soap->send_timeout = timeout;
	xpSoapClient.soap->recv_timeout = timeout;
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

		printf("Xplanner add TimeEntry: XPlannerUserID: %d  TaskID: %d ReportDate: %s Description: %s Duration: %0.2f\n", 
			    timeEdata.person1Id, timeEdata.taskId, durationTemp, timeEdata.description, timeEdata.duration);

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
bool  syncData(int userId, std::string& day, int taskId, const float& hours, const std::string& comment, const std::string& strUser, const std::string& passwd)
{
	//±£´æ¼ÇÂ¼
	time_t _time= convertTime(day);
	ns4__TimeEntryData timeEdata;
	timeEdata.taskId = taskId;
	timeEdata.person1Id = userId;
	timeEdata.reportDate =&_time ;
	timeEdata.description = (char*)comment.c_str();
	timeEdata.duration = (double)hours;

	std::string xpusername = strUser; 
	std::string xppasswd = passwd;

	bool bret = addTimeEntryToXPlanner(endpoint, xpusername, xppasswd, timeEdata);

	if( ! bret)
	{
		printf("ERROR: failed to add timeEntry to Xplanner LoginID:%s day:%s taskId:%d, hours: %f\n", strUser.c_str(), day.c_str(), taskId, hours);
	}

	return true;
}

/////bool getXplannerPersonInfo()
/// get Xplanner person infomation
///@param[in]  endpoint:  Soap interface endpoint   
///@param[in]  userid:    login userID
///@param[in]  passwd:    login passwd
///@param[out] personInfos: persons collect in xplanner
///@retrun     ture: succesful get Xplanner person infomation
///            false: fail to get Xplanner person infomation
bool  getXplannerPersonInfo( StringIntMap& personInfos)
{
	printf("get Xplanner people infomation...");

	std::string userid = "li.huang"; 
	std::string passwd = "qwerty";

	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = timeout;
	xpSoapClient.soap->send_timeout = timeout;
	xpSoapClient.soap->recv_timeout = timeout;
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
				personInfos[t->userId] = t->id;
			}        
		}
		printf("get people infomation successfully\n");
	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}
	return true;
}


bool checkTask(const std::string& userid, const std::string& passwd, int taskID)
{
	INTS gTaskInfo;
	XPlannerSoapBinding xpSoapClient;

	xpSoapClient.endpoint = (char*)endpoint.c_str();
	xpSoapClient.soap->connect_timeout = timeout;
	xpSoapClient.soap->send_timeout = timeout;
	xpSoapClient.soap->recv_timeout = timeout;
	xpSoapClient.soap->userid = (char*)userid.c_str();
	xpSoapClient.soap->passwd = (char*)passwd.c_str();

	ns6__getTaskResponse response;
	if(xpSoapClient.ns6__getTask(taskID, response) == 0)
	{
		// invoke successfully

		ns4__TaskData * t = (ns4__TaskData *)response._getTaskReturn;   

		if(t != NULL)
		{
			std::cout<< "\t" << t->id << "\t"  << t->name << "\t" << t->storyId << std::endl;
			gTaskInfo.push_back(t->id);
		}
		else 
		{
			printf("check task %d successfully\n", taskID);
		}

	}
	else
	{
		// soap level error
		logSoapErrorMsg(xpSoapClient);
		return false;
	}
	if(gTaskInfo.size() > 0)
		return true;
	return false;
}