// CpcMethodController.cpp: implementation of the CpcMethodController class.
//
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif

#include "CpcMethodController.h"
#include <Ice/Ice.h>

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "CPCMethodCtrl"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CpcMethodController::CpcMethodController(IHttpRequestCtx *pHttpRequestCtx) : BasePage(pHttpRequestCtx)
{

}

CpcMethodController::~CpcMethodController()
{
}




//#define SESSIONCTRL_ACTION_UPDATESITE      'u'
//#define SESSIONCTRL_ACTION_REMOVESITE      'r'
//#define SESSIONCTRL_ACTION_UPDATELIMIT     'l'
//#define SESSIONCTRL_ACTION_SETPROP         'p'
//#define SESSIONCTRL_ACTIONSET_SITE         "urlp"
//
//#define SESSIONCTRL_VAR_NAME               "session#name"
//#define SESSIONCTRL_VAR_DESC               "session#desc"
//#define SESSIONCTRL_VAR_MAXBW              "session#maxbw"
//#define SESSIONCTRL_VAR_MAXSESS            "session#maxsess"
//#define SESSIONCTRL_VARPREFIX_PROP         "session.prop#"
// this function work together withm AdminCtrl_Site.html
bool CpcMethodController::get()
{
	IHttpResponse& responser = _reqCtx->Response();

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));
	
	TianShanIce::ContentProvision::CPEInsts insts;
	TianShanIce::ContentProvision::CPEInsts::iterator iter;
//	int cpeNumber = 0;
	try
	{
	
		glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "Invoke listRegisteredCPE()"));
		insts = _cpc->listRegisteredCPE();

  //      cpeNumber = insts.size();
		//if (cpeNumber <= 0)
		//{
		//	    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "There is no session."));
		//		sprintf(buff, "There is no sessions  in current snapshot");
		//		responser<<buff;
		//		// show a link for return
		//		url.clear();
	
		//		return true;
		//}
		//glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "%d cpeservice exist.",cpeNumber));

	}
	catch (const TianShanIce::ClientError& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Get session list caught %s: %s<br>", ex.ice_name().c_str(), ex.message.c_str());
		setLastError(szBuf);//
		snprintf(szBuf, sizeof(szBuf) - 1, "Refresh page. if this error occurs again, there is must be something wrong with the server");
		addToLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Get session list caught %s<br>", ex.ice_name().c_str());
		setLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}

	// write page's functionality
	responser<<"<H2>Registered Method List</H2>";
	responser<<"<table class='listTable'>";

	// write table header line
	responser<<"<tr class='listTable'>";
	responser<<"	<th>Methods</th>";
	responser<<"</tr>";

	// write table data line
	int i = 1;
	std::map<std::string,int> methodMap;
	for (iter = insts.begin(); iter < insts.end(); iter++)
	{	
		TianShanIce::ContentProvision::MethodInfos methodInfo = (*iter)->listMethods();
		TianShanIce::ContentProvision::MethodInfos::iterator itor;

		for(itor = methodInfo.begin(); itor != methodInfo.end(); itor++)
		{
			methodMap.insert(make_pair(itor->methodType,i++));
		}
	}

	for(std::map<std::string,int>::iterator it = methodMap.begin();
		it != methodMap.end(); it++)
	{
		responser<<"<tr>";

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>",  it->first.c_str());
		responser<<szBuf;

		responser<<"</tr>";
	}

	// write end table flag
	responser<<"</table>";
	responser<<"<br>";

	return true;

}

bool CpcMethodController::post()
{
	IHttpResponse& responser = _reqCtx->Response();

	url.clear();
	url.setPath(CPCMethodPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setPath(contentprovisonclusterAddressKey);

	
	responser<<"<script language=\"javascript\">";
	snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	responser<<"</script>";
	return true;
}
