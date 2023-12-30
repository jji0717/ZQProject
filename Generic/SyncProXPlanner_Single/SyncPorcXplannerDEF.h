#ifndef __SncProXplanner_H__
#define __SncProXplanner_H__
#include <string>
#include <map>
#include <vector>
#include "XplannerSOAPXPlannerSoapBindingProxy.h"

#define XPLANNER_SOAP_CLIENT_TIMEOUT    50
//#define ENDPOINT_local    "http://192.168.81.102:7070/soap/XPlanner?wsdl"
//#define ENDPOINT    "http://sp-server:9090/soap/XPlanner?wsdl"

typedef std::map<std::string, std::string >STRS;
typedef std::map<std::string, int >StringIntMap;
typedef std::vector<int>INTS;


time_t  convertTime(std::string strtime);
void    logSoapErrorMsg(const XPlannerSoapBinding& xpSoapClient);
bool    addTimeEntryToXPlanner(std::string endpoint, std::string userid, std::string passwd, ns4__TimeEntryData& timeEdata);
bool    syncData(int  userId, std::string& day, int taskId, const float& hours, const std::string& comment, const std::string& strUser, const std::string& passwd);

bool    getXplannerPersonInfo( StringIntMap& personInfos);
bool checkTask(const std::string& userid, const std::string& passwd, int taskID);
#endif // __SncProXplanner_H__
