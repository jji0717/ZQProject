// MethodController.cpp: implementation of the MethodController class.
//
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#endif
#include "MethodController.h"
#include <Ice/Ice.h>

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "MethodCtrl"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
MethodController::MethodController(IHttpRequestCtx *pHttpRequestCtx) : BasePage(pHttpRequestCtx)
{

}

MethodController::~MethodController()
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
bool MethodController::get()
{
	IHttpResponse& responser = _reqCtx->Response();

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));
	
	::TianShanIce::ContentProvision::MethodInfos result;
	::TianShanIce::ContentProvision::MethodInfos::iterator itor;
	try
	{	
		result = _ps->listMethods();
	
 
		int methodcount = 0;
		methodcount = result.size();
		if (methodcount <= 0)
		{
			    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "There is no method."));
				sprintf(buff, "There is no method under in current snapshot");
				responser<<buff;
				// show a link for return
				url.clear();
			/*	url.setPath(MethodPagePath);
				url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
				url.setPath(contentprovisonAddressKey);

				_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">Return</a>", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
				responser<<szBuf;*/
				return true;
		}
		
	}
	catch (const TianShanIce::ClientError& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Get method list caught %s: %s<br>", ex.ice_name().c_str(), ex.message.c_str());
		setLastError(szBuf);//
		snprintf(szBuf, sizeof(szBuf) - 1, "Refresh page. if this error occurs again, there is must be something wrong with the server");
		addToLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Get method list caught %s<br>", ex.ice_name().c_str());
		setLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}

	// write page's functionality
	responser<<"<H2>Method List</H2>";
	responser<<"<table class='listTable'>";

	// write table header line
	responser<<"<tr class='listTable'>";
	responser<<"	<th>Method Type</th>";
	responser<<"	<th>Current Bandwidth Used (Kbps)</th>";
	responser<<"	<th>Max Bandwidth Available (Kbps)</th>";
	responser<<"	<th>Active Sessions</th>";
	responser<<"	<th>Maximum Sessions</th>";
	responser<<"</tr>";

	// write table data line
	for (itor = result.begin(); itor < result.end(); itor++)
	{
		responser<<"<tr>";

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>",(*itor).methodType.c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"</td>",(*itor).allocatedKbps);
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"</td>", (*itor).maxKbps);
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"</td>", (*itor).sessions);
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"</td>", (*itor).maxsessions);
		responser<<szBuf;

		responser<<"</tr>";
	}

	// write end table flag
	responser<<"</table>";
	responser<<"<br>";

	return true;

}

bool MethodController::post()
{
	IHttpResponse& responser = _reqCtx->Response();

	url.clear();
	url.setPath(MethodPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setPath(contentprovisonAddressKey);

	
	responser<<"<script language=\"javascript\">";
	snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	responser<<"</script>";
	return true;
}
