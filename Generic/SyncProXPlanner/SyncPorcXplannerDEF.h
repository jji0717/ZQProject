#ifndef __SncProXplanner_H__
#define __SncProXplanner_H__
#include <string>
#include <map>
#include <vector>
#include "XplannerSOAPXPlannerSoapBindingProxy.h"
#include "FileLog.h"

#define XPLANNER_SOAP_CLIENT_TIMEOUT    50
//#define ENDPOINT_local    "http://192.168.81.102:7070/soap/XPlanner?wsdl"
//#define ENDPOINT    "http://sp-server:9090/soap/XPlanner?wsdl"

typedef std::map<std::string, std::string >STRS;
typedef std::map<std::string, int >StringIntMap;
typedef std::vector<int>INTS;

typedef struct
{
	std::string userEmail;
	int userIDInProOpen;
	std::string userLogInID;
	int userIDInXplaner;
}People;

typedef std::map<std::string, People>Peoples;

void showCfg();
time_t  convertTime(std::string strtime);
void    logSoapErrorMsg(const XPlannerSoapBinding& xpSoapClient);
bool    getXplannerPersonInfo(std::string endpoint, std::string userid, std::string passwd, StringIntMap& personInfos);
bool    addTimeEntryToXPlanner(std::string endpoint, std::string userid, std::string passwd, ns4__TimeEntryData& timeEdata);
bool    getProjectOpenPersonInfo(StringIntMap& xplannerPersonInfos, Peoples& peoples);
bool    syncData(People people, std::string& startdate, std::string& enddate);
bool    getTimeEntryData(std::string endpoint, std::string userid, std::string passwd,int id);
bool    getPeopleInfo(std::string& userEmail, std::string& userID, StringIntMap& xplannerPersonInfos, People& people);
#endif // __SncProXplanner_H__
