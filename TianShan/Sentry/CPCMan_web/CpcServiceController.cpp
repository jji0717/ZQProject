// CpcServiceController.cpp: implementation of the CpcServiceController class.
//
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif

#include "CpcServiceController.h"
#include <Ice/Ice.h>

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "CPCServiceCtrl"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CpcServiceController::CpcServiceController(IHttpRequestCtx *pHttpRequestCtx) : BasePage(pHttpRequestCtx)
{

}

CpcServiceController::~CpcServiceController()
{
}

bool CpcServiceController::get()
{
	IHttpResponse& responser = _reqCtx->Response();

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));

	// --------------------------------------------------------------------
	// 取得页面自身保存的参数，第一次访问该页面时，这些值肯定没有。
	// --------------------------------------------------------------------
	//_sessCount = atoi(_varMap[SessionCountKey].c_str());

	std::string methodtype ="";// _varMap[SessionGroupKey];
	std::string startId = "";//_varMap[StartId];
//	int maxcount = 0;//atoi(_varMap[MaxSessionCout].c_str());

	TianShanIce::ContentProvision::CPEInsts insts;
	TianShanIce::ContentProvision::CPEInsts::iterator iter;
	int cpeNumber = 0;
	try
	{
		
		glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "Invoke listRegisteredCPE()"));
		insts = _cpc->listRegisteredCPE();
		cpeNumber = insts.size();

		if (cpeNumber <= 0)
		{
			    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "There is no session."));
				sprintf(buff, "There is no sessions  in current snapshot");
				responser<<buff;
				// show a link for return
				url.clear();
	
				return true;
		}
		glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "%d registered cpeservice exist."),cpeNumber);

    }
	catch (const TianShanIce::ClientError& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "listRegisteredCPE caught %s: %s<br>", ex.ice_name().c_str(), ex.message.c_str());
		setLastError(szBuf);//
		snprintf(szBuf, sizeof(szBuf) - 1, "Refresh page. if this error occurs again, there is must be something wrong with the server");
		addToLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "listRegisteredCPE caught %s<br>", ex.ice_name().c_str());
		setLastError(szBuf);
		glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}
	// write page's functionality
	responser<<"<H2>Registered CPE Service List</H2>";
	responser<<"<table class='listTable'>";

	// write table header line
	responser<<"<tr class='listTable'>";
	responser<<"	<th>NetId</th>";
	responser<<"	<th>Proxy</th>";
	responser<<"	<th>Methods</th>";
	responser<<"</tr>";

	// write table data line
	for (iter = insts.begin(); iter < insts.end(); iter++)
	{
		TianShanIce::ContentProvision::MethodInfos methodInfos = (*iter)->listMethods();
		TianShanIce::ContentProvision::MethodInfos::iterator itor;

		std::string proxystr = _ic->proxyToString(*iter);
		int rownum = methodInfos.size();

		if(rownum == 0)
		{
			responser<<"<tr>";
		
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>",(*iter)->getNetId().c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>",proxystr.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "	<td>%s</td>","");
			responser<<szBuf;
			responser<<"</tr>";
		}
		else
		{
			responser<<"<tr>";
			bool flag = false;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td rowspan=%d>%s</td>",rownum,(*iter)->getNetId().c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td rowspan=%d>%s</td>",rownum,proxystr.c_str());
			responser<<szBuf;

			for(itor = methodInfos.begin(); itor != methodInfos.end(); itor++)
			{
				if(flag)
				{
					responser<<"<tr>";
					flag = false;
				}
				snprintf(szBuf, sizeof(szBuf) - 1, "	<td>%s</td>",itor->methodType.c_str());
				responser<<szBuf;
				if(!flag)
				{
					responser<<"</tr>";
					flag = true;
				}
			}
		}
		
	}
	// write end table flag
	responser<<"</table>";
	responser<<"<br>";

    return true;
}

bool CpcServiceController::post()
{
	IHttpResponse& responser = _reqCtx->Response();

	url.clear();
	url.setPath(CPCServicePagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(contentprovisonclusterAddressKey, _varMap[contentprovisonclusterAddressKey].c_str());
	responser<<"<script language=\"javascript\">";
	snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	responser<<"</script>";
	return true;
}
