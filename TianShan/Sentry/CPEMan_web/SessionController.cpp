// SessionController.cpp: implementation of the SessionController class.
//
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#endif
#include "SessionController.h"
#include <Ice/Ice.h>
#include<time.h> 

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "SessionCtrl"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
SessionController::SessionController(IHttpRequestCtx *pHttpRequestCtx) : BasePage(pHttpRequestCtx)
{

}

SessionController::~SessionController()
{
}


typedef struct _ContentSessionDesc
{
	std::string contentname,methodType,status,netId;
	std::string schebegtime,scheendtime;
//	std::string pb,tb;
    int64 processedBytes,totalBytes;
} ContentSessionDesc;

typedef std::vector<ContentSessionDesc> ContentSessionDescs;

bool rule(const ContentSessionDesc& fir,const ContentSessionDesc& sec)
{
	if(strcmp(fir.schebegtime.c_str(),sec.schebegtime.c_str())< 0) 
    {
        return true;
    }
    else
    {
        return false; 
    }
}

bool SessionController::get()
{
	IHttpResponse& responser = _reqCtx->Response();

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));

	// --------------------------------------------------------------------
	// 取得配置中的值
	// --------------------------------------------------------------------
	_sessionNumberPerPage = atoi(_varMap[SessionNumberPerPageKey].c_str());
	if (_sessionNumberPerPage == 0)
	{
		// 如果没有该key，则默认每页显示50条记录
		_sessionNumberPerPage = 50;
	}
#ifndef _DEBUG
	if (_sessionNumberPerPage < 5)
	{
		// 如果配置小于5，调整为5
		_sessionNumberPerPage = 5;
	}
#endif

	// --------------------------------------------------------------------
	// 取得页面自身保存的参数，第一次访问该页面时，这些值肯定没有。
	// --------------------------------------------------------------------
	//_sessCount = atoi(_varMap[SessionCountKey].c_str());
	//_curPage = atoi(_varMap[PageSequenceKey].c_str());

	std::string methodtype ="";// _varMap[SessionGroupKey];
	std::string startId = "";//_varMap[StartId];
	int maxcount = 0;//atoi(_varMap[MaxSessionCout].c_str());

	ContentSessionDescs CSdescs;
	int sesscount = 0;
	int waitcount = 0;
	int privisioncout = 0;
	int stoppedcount = 0;
	int createcount = 0;
	int readycount = 0;
	int acceptcount = 0;
	char ltime[MAX_PATH];
	//NGODr2c1::CtxDatas data;
	try
	{

		::TianShanIce::StrValues paramNames;
		TianShanIce::ContentProvision::ProvisionInfo proInfo;
		::TianShanIce::Properties params;
		paramNames.push_back(SYS_PROP(contentName));
		paramNames.push_back(SYS_PROP(contentMethod));
		paramNames.push_back(SYS_PROP(netId));
		paramNames.push_back(SYS_PROP(volume));
		paramNames.push_back(SYS_PROP(scheduledStart));
		paramNames.push_back(SYS_PROP(scheduledEnd));
		paramNames.push_back(SYS_PROP(processedBytes));
		paramNames.push_back(SYS_PROP(totalBytes));

		TianShanIce::ContentProvision::ProvisionInfos listInfo;
		std::vector< ::TianShanIce::ContentProvision::ProvisionInfo>::iterator iter;
		glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "Invoke listSessions()"));
		listInfo = _ps->listSessions(methodtype,paramNames,startId,maxcount);
		time_t  curtime=time(0); 
        tm tim= *localtime( &curtime ); 
		sprintf(ltime,"%d-%d-%d %02d:%02d:%02d",tim.tm_year+1900,tim.tm_mon+1,tim.tm_mday,tim.tm_hour,tim.tm_min,tim.tm_sec);

        CSdescs.clear();
		ContentSessionDesc csdes;
		for (iter = listInfo.begin(); iter != listInfo.end();iter++)
		{
		//	glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "Store data in CSdescs"));
		//	memset(&csdes,0,sizeof(csdes));
		//	proInfo = *iter;	
			csdes.contentname = (*iter).contentKey.content;
			csdes.netId =  (*iter).contentKey.contentStoreNetId;	
			params =  (*iter).params;
			switch( (*iter).state)
			{
			case 0:
				csdes.status = "Created";
				createcount++;
				break;
			case 1:
				csdes.status = "Accepted";
				acceptcount++;
				break;
			case 2:
				csdes.status = "Wait";
				waitcount++;
				break;
			case 3:
				csdes.status = "Ready";
				readycount++;
				break;
			case 4:
				csdes.status = "Provisioning";
				privisioncout++;
				break;
			case 5:
				csdes.status = "Stopped";
				stoppedcount++;
				break;		
			}

			csdes.methodType = (*params.find(SYS_PROP(contentMethod))).second;
			csdes.schebegtime = (*params.find(SYS_PROP(scheduledStart))).second;
			csdes.scheendtime =(*params.find(SYS_PROP(scheduledEnd))).second;
		   // csdes.pb = (*params.find(SYS_PROP(processedBytes))).second;

#ifdef ZQ_OS_MSWIN
			csdes.processedBytes = _atoi64((*params.find(SYS_PROP(processedBytes))).second.c_str());
			csdes.totalBytes = _atoi64((*params.find(SYS_PROP(totalBytes))).second.c_str());
#else
			csdes.processedBytes = atoll((*params.find(SYS_PROP(processedBytes))).second.c_str());
			csdes.totalBytes = atoll((*params.find(SYS_PROP(totalBytes))).second.c_str());
#endif
		
			if((*iter).state == 4)
			{
				  glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "contentName-%s,totalbytes-%lld,prob-%lld"),csdes.contentname.c_str(),
					  csdes.totalBytes,csdes.processedBytes);
			}
			
			CSdescs.push_back(csdes);
			//sesscount++;
		}

		sesscount = CSdescs.size();
        /*
		if (sesscount <= 0)
		{
			    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "There is no session."));
				sprintf(buff, "There is no sessions  in current snapshot");
				responser<<buff;
				url.clear();
				return true;
		}
        */
		glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "%d sessions exist"),sesscount);

		
		// 计算分页数量
		_pageCount = sesscount / _sessionNumberPerPage;
		if (sesscount % _sessionNumberPerPage != 0)
			_pageCount ++;

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
	sort(CSdescs.begin(),CSdescs.end(),rule);    

	//display statistic data
	responser<<"<H2>Total Sessions by State</H2>";
	responser<<"<table class='listTable'>";

	responser<<"<tr>";
	responser<<"	<th width='9%'> Total</th>";
	responser<<"	<th width='9%'>     Created</th>";
	responser<<"	<th width='9%'>    Accepted</th>";
	responser<<"	<th width='9%'>        Wait</th>";
	responser<<"	<th width='9%'>       Ready</th>";
	responser<<"	<th width='9%'>Provisioning</th>";
	responser<<"	<th width='9%'>     Stopped</th>";
	responser<<"	<th width='14%'> Update time</th>";
	responser<<"</tr>";

	responser<<"<tr>";
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", sesscount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", createcount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", acceptcount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", waitcount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", readycount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", privisioncout);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", stoppedcount);
	responser<<szBuf;
	snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", ltime);
	responser<<szBuf;
	responser<<"</tr>";
	responser<<"</table>";
	responser<<"<br>";

	responser<<"<script type=text/javascript>";
	snprintf(szBuf, sizeof(szBuf) - 1, "var _curPageId = 'tb0';");
	responser<<szBuf;
	responser<<"function showPage(idx)";
	responser<<"{";
	responser<<"if(idx < 1) idx = 1;";
	responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
	responser<<"var pgId = 'tb' + (idx-1);";
	responser<<"document.getElementById(_curPageId).style.display='none';";
	responser<<"document.getElementById(pgId).style.display='block';";
	responser<<"_curPageId = pgId; return idx;";
	responser<<"}";
	responser
		<<"function updateStatus(){"
		<<"document.getElementById('cur-page').innerHTML=document.getElementById('page-idx').value;}\n";
	responser<<" </script>";
	

	ContentSessionDescs::iterator it = CSdescs.begin();
	responser<<"<H2>Active Sessions List</H2>";
    if(0 == _pageCount) {
        responser<<"<table class='listTable' style='display:block' id=tb0>";
        responser<<"<tr>";
        responser<<"	<th>Content Name</th>";
        responser<<"	<th>Method Type</th>";
        responser<<"	<th>Start Time</th>";
        responser<<"	<th>End Time</th>";
        responser<<"	<th>Status</th>";
        responser<<"	<th>Progress (MB): Current/Total</th>";
        responser<<"	<th>Percent</th>";
        responser<<"</tr>";
        // write end table flag
        responser<<"</table>";
    } else {
	    for(size_t i = 0; i < _pageCount; i++)
	    {
		    if(i == 0)
		    {
			    snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:block' id=tb%ld>", i);
			    responser<<szBuf;
		    }
		    else
		    {
			    snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:none' id=tb%ld>", i);
			    responser<<szBuf;
		    }

		    // write page's functionality
	    //	responser<<"<H2>Sessions List</H2>";
	    //	responser<<"<table id='' class='listTable'>";

		    // write table header line
		    responser<<"<tr>";
		    responser<<"	<th>Content Name</th>";
		    responser<<"	<th>Method Type</th>";
    //		responser<<"	<th>netId</th>";
		    responser<<"	<th>Start Time</th>";
		    responser<<"	<th>End Time</th>";
		    responser<<"	<th>Status</th>";
		    responser<<"	<th>Progress (MB): Current/Total</th>";
		    responser<<"	<th>Percent</th>";
		    responser<<"</tr>";


		    // write table data line
		    for(size_t j = 0; j < _sessionNumberPerPage && it < CSdescs.end(); j++)
		    {
			    responser<<"<tr>";

			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", it->contentname.c_str());
			    responser<<szBuf;

			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", it->methodType.c_str());
			    responser<<szBuf;

	    //		snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>",it->netId.c_str());
	    //		responser<<szBuf;

			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", it->schebegtime.c_str());
			    responser<<szBuf;

			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", it->scheendtime.c_str());
			    responser<<szBuf;

			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", it->status.c_str());
			    responser<<szBuf;

			    long pb = (it->processedBytes)/(1024*1024);
			    long tb = (it->totalBytes)/(1024*1024);
			    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"%s"FMT64" </td>", pb,"/",tb);
			    responser<<szBuf;

			    if(tb)
			    {
				    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>"FMT64"%%</td>",pb*100/tb);
				    responser<<szBuf;
			    }
			    else
			    {
				    snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d%%</td>",0);
				    responser<<szBuf;
			    }

			    responser<<"</tr>";

			    it++;
		    }	
		     // write end table flag
		    responser<<"</table>";
	    }
    }

//for(int cur = 0; cur< _pageCount; cur++)
//		{
//			snprintf(szBuf, sizeof(szBuf) - 1, "<a href='#' onclick=\"showPage('tb%d');return false\">%d</a> ", cur, cur+1);
//			responser<<szBuf;
//		}
	responser << "<br>";

	responser
		<< "<span>[<span id='cur-page'>1</span> / " << (int)_pageCount << "]</span>"
		<< "<form style='display:inline;position:absolute;right:10%'>";
	responser << "<input id=\"page-idx\" value='1' type='hidden'>";
	responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(1); updateStatus();\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
    responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1);updateStatus();\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
	responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1);updateStatus();\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
	responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(" << (int)_pageCount << ");updateStatus();\"><img src='img/last_page.gif' alt='last'></span>";
    responser << "</form>";

	responser << "<br><br>";
	responser << "<br><br>";

	return true;

}

bool SessionController::post()
{
	IHttpResponse& responser = _reqCtx->Response();

	url.clear();
	url.setPath(SessionPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(contentprovisonAddressKey, _varMap[contentprovisonAddressKey].c_str());
	
	responser<<"<script language=\"javascript\">";
	snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	responser<<"</script>";
	return true;
}

