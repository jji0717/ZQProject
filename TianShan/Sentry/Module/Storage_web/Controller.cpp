/* File Name: Controller.cpp
   Date     : 26th Nov
   Purpose  : Implemtation of Controller class
**/

#include <stdlib.h>
#include <algorithm>
#include <StringUtil.h>
#include "Controller.h"

using namespace ZQ::common;
using StorageWeb::Controller;

const char* const Controller::CS_VAR_TEMPLATE = "#template";
const char* const Controller::CS_VAR_ENDPOINT = "#endpoint";
const char* const Controller::CS_VAR_MAXCOUNT = "#maxcount";
const char* const Controller::CS_VAR_METADATA_NAMES = "#metaDataNames";
const char* const Controller::CS_VAR_VOLOUMEINFO_STRUCT = "#volumeinfostruct";
const char* const Controller::CS_VAR_MODULE_NAME = "Contents Storage";

Controller::Controller(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{
}

Controller::~Controller(void)
{
	finalize();
}

bool Controller::init()
{
	// get page template 
	_template = _pHttpRequestCtx->GetRequestVar(CS_VAR_TEMPLATE);
	if (NULL == _template)
	{
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Template is missing in the request."));
		_pHttpRequestCtx->Response().SetLastError("Missing Template Errors");
		return false;
	}

	// get server address 
	_endpoint = _pHttpRequestCtx->GetRequestVar(CS_VAR_ENDPOINT);
	if (NULL == _endpoint)
	{
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Endpoint is missing in the request."));
		_pHttpRequestCtx->Response().SetLastError("Missing Server Address Errors");
		return false;
	}

	// get max rows of table of each page, this attribute is not must .
	const char* maxCount = _pHttpRequestCtx->GetRequestVar(CS_VAR_MAXCOUNT); 
	_nMaxCount = maxCount ? atoi(maxCount) : 15;
	if (_nMaxCount < 15)
	{
		_nMaxCount = 15;
	}

	// get contentStore's proxy string 
	std::string strProxy = std::string("ContentStore:") + _endpoint; 
	try
	{
		int argc = 0;
		_ic = Ice::initialize(argc, NULL);
		Ice::ObjectPrx base = _ic->stringToProxy(strProxy);
		_contentStorePrx = ContentStorePrx::checkedCast(base);
		if (!_contentStorePrx)
		{
			glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Invalide Proxy"));
			_pHttpRequestCtx->Response().SetLastError("Invalide Proxy");
			return false;
		}
	}
	catch (const Ice::ConnectFailedException& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"Fail To Connect Server [%s]", ex.ice_name().c_str());
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Connect server fail exception [%s]"), ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		char errorMsg[255];
		sprintf(errorMsg,"Ice Run Time Errors [%s]", ex.ice_name().c_str());
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Ice Exception [%s]"),ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		return false;
	} 
	return true;
}

void Controller::finalize()
{
	if (_ic)
	{
		try
		{
			_contentStorePrx = NULL;
			_ic->destroy();
			_ic = NULL;
		}
		catch(...)
		{
			glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "encounter unknown exception during finalize()."));
			_pHttpRequestCtx->Response().SetLastError("Unknown Errors when ");
		} // end try 
	} // end if
}

// get all result, then list special page
bool Controller::listVolumes()
{
	glog(Log::L_DEBUG,CLOGFMT(CS_VAR_MODULE_NAME, "Request Volumes page."));
	if (!init())
	{
		return false;
	}
	VolumeInfos volumeInfos;
	try
	{
		volumeInfos = _contentStorePrx->listVolumes("", true); // get all volumes infos 
	}
	catch (const Ice::Exception& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg, "Fail To List Volumes From Server [%s]", ex.ice_name().c_str());
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Fail to list Volumes from server [%s]"), ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		return false;
	}

	// computer last page
	int iLastPage = volumeInfos.size() / _nMaxCount;
	if ((volumeInfos.size() % _nMaxCount) || (iLastPage <= 0))
	{
		iLastPage++;
	}
	// computer current page
	int iCurPage = _pHttpRequestCtx->GetRequestVar("pageindex") ? atoi(_pHttpRequestCtx->GetRequestVar("pageindex")) : 1;
	if ((iCurPage <= 0) || (iCurPage > iLastPage))
	{
		iCurPage = 1;
	}

	IHttpResponse &out = _pHttpRequestCtx->Response();
	
	// get table header
	const char* volumeInfoStruct = _pHttpRequestCtx->GetRequestVar(CS_VAR_VOLOUMEINFO_STRUCT);
	StringUtil::splitString(_strTableHeaders, volumeInfoStruct, ";");
	int prefixLen = strlen("sys."); // May be need to improve here

	TianShanIce::Properties::const_iterator p = volumeInfos[0].metaData.begin();
	while (p != volumeInfos[0].metaData.end())
	{
		_strTableHeaders.push_back(p->first.substr(prefixLen));
		p++;
	}

	out << "<script type=\"text/javascript\">\n"
		<<"<!--\n"
		<< "function toPage(pgidx)\n"
		<< "{\n"
		<<    "document.getElementById(\"pageindex\").value = pgidx;\n"
		<<    "document.getElementById(\"volumesform\").submit();\n"
		<< "}\n"
		<< "//-->\n"
		<<"</script>\n";
	out << "<form id=\"volumesform\" method=\"get\" action=\"ListVolumes.storage.tswl\" style=\"display:inline\">\n"
		<< "<table>\n" // table 1
		<< "<tr>\n"
		<< "<td>\n"
		<< "<table>\n" // table 2
		<< "<tr>\n"
		<< "<td width=\"400\" align=\"right\">\n";
	if (iCurPage > 1)
	{
		//previous page
		out << "<span class=\"lnk\" onclick=\"history.back(-1)\">\n"
	        << "<img src=\"img/previous_page.gif\" alt=\"previous\">\n"
		    << "</span>\n";
	}
		//current page index
	out	<< "<input type=\"text\" id=\"pageindex\" name=\"pageindex\" readonly=\"readonly\" style=\"text-align:center\" style=\"border:none \" size=2 value=\"" << iCurPage << "\">\n"
	    //next page
	    << "<span class=\"lnk\" onclick=\"toPage(" << iCurPage + 1 << ")\">\n"
	    << "<img src=\"img/next_page.gif\" alt=\"next\">\n"
		<< "</span>\n"
		<< "</td>\n"
		<< "</tr>\n"
		<< "</table>\n" // table 2
		<< "</td>\n"
		<< "</tr>\n"
		<< "<tr>\n"
		<< "<td>\n"
		<< "<table id=\"contents-tbl\" class=\"chunk listTable\">\n" // table 3
	    << "<tr>\n";
	StrValues::const_iterator q = _strTableHeaders.begin();
	while (q != _strTableHeaders.end())
	{
		out << "<th>" << *q <<"</th>\n";
		q++;
	}
	out << "</tr>\n";
	size_t nHighRow = (iCurPage * _nMaxCount);
    size_t tmp = ((iCurPage-1) * _nMaxCount);
	for (size_t i = tmp; (i < volumeInfos.size()) && (i < nHighRow); i++)
	{
		out << "<tr>\n";
		out	<< "<td>" << volumeInfos[i].name << "</td>\n";
		out	<< "<td>" << (volumeInfos[i].isVirtual ? "True" : "False") << "</td>\n";
		out << "<td>" << volumeInfos[i].quotaSpaceMB << "</td>\n";
	    p = volumeInfos[i].metaData.begin();
		for (; p != volumeInfos[i].metaData.end(); p++)
		{
			out << "<td>\n" << p->second << "</td>\n";
		}
		out	<< "</tr>\n";
	}
	out	<< "</table>\n" // table 3
		<< "</td>\n"
		<< "</tr>\n"
		<< "</table>\n" // table 1
		<< "<input type=\"hidden\" name=\"" << CS_VAR_TEMPLATE << "\" id=\"" << CS_VAR_TEMPLATE << "\" value=\""<< _template <<"\">\n" 
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ENDPOINT << "\" id=\"" << CS_VAR_ENDPOINT << "\" value=\""<< _endpoint <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_MAXCOUNT << "\" id=\"" << CS_VAR_MAXCOUNT << "\" value=\""<< _nMaxCount <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_VOLOUMEINFO_STRUCT << "\" id=\"" << CS_VAR_VOLOUMEINFO_STRUCT << "\" value=\""<< volumeInfoStruct <<"\">\n" 
	    << "</form>\n";
	return true;
}

bool Controller::listContents()
{
	glog(Log::L_DEBUG,CLOGFMT(CS_VAR_MODULE_NAME, "Request contents page."));
	if (!init())
	{
		return false;
	}

	// get metaDataNames
	const char* paramList = _pHttpRequestCtx->GetRequestVar(CS_VAR_METADATA_NAMES);
	if (NULL == paramList)
	{
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Missing Metadata names Errors."));
		_pHttpRequestCtx->Response().SetLastError("Missing Metadata names Errors.");
		return false;
	}
	TianShanIce::StrValues params;
	StringUtil::splitString(params, paramList, ";");
	for (size_t i = 0; i < params.size(); i++)
	{
		TianShanIce::StrValues param;
		StringUtil::splitString(param, params[i], ":");
		std::transform(param.begin(), param.end(), param.begin(), StringUtil::trimWS); // erase empty space
		if (param.size() > 0 && (param[0].find("sys.") != std::string::npos))
		{
			_strMetaDataNames.push_back(param[0]); // exclusive string no include "sys."
		}
        switch(param.size())
        {
        case 1: // use param name as display title
			_strTableHeaders.push_back(param[0]);
            break;
        case 2:
			_strTableHeaders.push_back(param[1]);
            break;
        default:
			break;
        }
	}

	// get search category
	const char* category = _pHttpRequestCtx->GetRequestVar("category");
	category = category ? category : "volume";

	// get current search name
	const char* searchName = _pHttpRequestCtx->GetRequestVar("searchname");
	std::string strSearchName = searchName ? searchName : "";
	if (strSearchName != "") // erase empty space from strSearchName 
	{
		size_t nFirst = strSearchName.find_first_not_of(' ');
		strSearchName = strSearchName.substr(nFirst, strSearchName.find_last_not_of(' ')-nFirst+1);
	}

	// get previous search name, to modify index page 
	const char* preSearchName = _pHttpRequestCtx->GetRequestVar("presearchname");
	std::string strPreSearchName = preSearchName ? preSearchName : strSearchName;

	// computer current page
	const char * strCurPage = _pHttpRequestCtx->GetRequestVar("pageindex");
	int iCurPage = strCurPage ? atoi(strCurPage) : 1;
	if (iCurPage <= 0 || strPreSearchName != strSearchName)
	{
		iCurPage = 1;
	}
	// display page
	IHttpResponse &out = _pHttpRequestCtx->Response();
	out << "<script type=\"text/javascript\">\n"
		<< "<!--\n"
		<< "function toPage(pgidx)\n"
		<< "{\n"
		<<    "document.getElementById(\"pageindex\").value = pgidx;\n"
		<<    "document.getElementById(\"searchform\").submit();\n"
		<< "}\n"
		<< "//-->\n"
		<< "</script>\n";
	out << "<form id=\"searchform\" method=\"get\" action=\"ListContents.storage.tswl\" style=\"display:inline\">\n";
	// Search according category
	ContentInfos contentInfos;
	if (0 == strcmp(category, "volume")) // search by volume
	{
		const char* lastpage = _pHttpRequestCtx->GetRequestVar("lastpage");
		int nLastPage = lastpage ? atoi(lastpage) : 1; 
		if ( iCurPage > nLastPage)
		{
			iCurPage = nLastPage;
		};
		listVolume(strSearchName, iCurPage, contentInfos);

	}
	else // search by content
	{
		listContent(strSearchName, iCurPage, contentInfos);
		if (contentInfos.empty() && (iCurPage > 1))
		{
			listContent(strSearchName, --iCurPage, contentInfos); 
		} // end if 
	} // end if 

	
	out << "<div>&nbsp&nbsp\n"
		<< "<select name=\"category\" id=\"category\">\n"
		<< "<option value=\"volume\" ";
	if (0 == strcmp(category, "volume"))
	{
		out << "selected=\"selected\"";
	}
	out	<< ">directory</option>\n"
		<< "<option value=\"content\" ";
	if (0 == strcmp(category, "content"))
	{
		out << "selected=\"selected\"";
	}
    out << ">content</option>\n"
		<< "</select>\n"
		<< "<input type=\"text\" name=\"searchname\" id=\"searchname\" value =\""<<strSearchName<<"\"size=\"70\" />&nbsp&nbsp\n"
		<< "<input type=\"submit\" name=\"search\" id=\"search\" onclick=\"toPage(1)\" value=\"search\"/><br /><br />\n"
		<< "</div>\n";
	out << "<table>\n"  // begin table 1
		<< "<tr><td>\n" 
		<< "<table>\n" // begin table 2 
		<< "<tr>\n"
		<< "<td width=\"500\" align=\"right\">\n";
	if (iCurPage > 1)
	{
		//previous page
		out << "<span class=\"lnk\" onclick=\"history.back(-1)\">\n"
	        << "<img src=\"img/previous_page.gif\" alt=\"previous\">\n"
		    << "</span>\n";
	}
		//current page index
	out	<< "<input type=\"text\" id=\"pageindex\" name=\"pageindex\" readonly=\"readonly\" style=\"text-align:center\" size=2 style=\"border:none\" value=\""<< iCurPage << "\">\n"
		//next page
	    << "<span class=\"lnk\" onclick=\"toPage(" << iCurPage + 1 << ")\">\n"
	    << "<img src=\"img/next_page.gif\" alt=\"next\">\n"
		<< "</span>\n"
		<< "</td>\n"
		<< "</tr>\n"
		<< "</table>\n" // end table 2
	    << "</td></tr>\n"
		<< "<tr><td width=\"600\">\n"
	    << "<table id=\"contents-tbl\" class=\"chunk listTable\">\n" // begin table 3
	    << "<tr width=\"100%\">\n";
	StrValues::const_iterator p = _strTableHeaders.begin();
	while(p != _strTableHeaders.end())
	{
		out << "<th>" << *p << "</th>\n";
		p++;
	}
	out << "</tr>\n";
	for (size_t i = 0; i < contentInfos.size(); i++)
	{
		std::string volname = contentInfos[i].fullname.substr(0, contentInfos[i].fullname.length() - contentInfos[i].name.length() - 1);
		out << "<tr>";
		out << "<td>" << contentInfos[i].name << "</td>\n";
		out << "<td>" << volname << "</td>\n";
		p = _strMetaDataNames.begin();
		while(p != _strMetaDataNames.end())
		{
			out << "<td>" << contentInfos[i].metaData[*p] << "</td>\n";
			p++;
		}
		out<<"</tr>";
	}
	out	<< "</table>\n" // end table 3
	    << "</td></tr>\n" 
	    << "</table>\n"; // end table 1
	out << "<input type=\"hidden\" name=\"" << CS_VAR_TEMPLATE << "\" id=\"" << CS_VAR_TEMPLATE << "\" value=\""<< _template <<"\">\n" 
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ENDPOINT << "\" id=\"" << CS_VAR_ENDPOINT << "\" value=\""<< _endpoint <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_MAXCOUNT << "\" id=\"" << CS_VAR_MAXCOUNT << "\" value=\""<< _nMaxCount <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_METADATA_NAMES << "\" id=\"" << CS_VAR_METADATA_NAMES << "\" value=\""<< _pHttpRequestCtx->GetRequestVar(CS_VAR_METADATA_NAMES) <<"\">\n";
	out << "</form>\n";
	return true;
}


bool Controller::listVolume(std::string& strVolumeName, int nIndexPage, ContentInfos& contentInfos)
{
	// get default volume name 
	if ("" == strVolumeName)
	{
		strVolumeName = _contentStorePrx->getVolumeName();
	}

	// open volume
	VolumePrx volPrx;
	try
	{
		volPrx = _contentStorePrx->openVolume(strVolumeName);
	}
	catch (const Ice::Exception& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"Fail To List Contents Of Special Volume From Server [%s]", ex.ice_name().c_str());
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Fail To List Contents Of Special Volume From Server [%s]"), ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		return false;
	}
	std::string  strStartName;
	int nLast = nIndexPage + 1;
	if (volPrx)
	{	
		const char* startName = _pHttpRequestCtx->GetRequestVar("startname");
		strStartName = startName && (nIndexPage > 1) ? startName : "";
		contentInfos = volPrx->listContents(_strMetaDataNames, strStartName, _nMaxCount+1);

		if (contentInfos.size() == (size_t)(_nMaxCount + 1))
		{
			strStartName = contentInfos[_nMaxCount].name;
			contentInfos.pop_back(); // delete last elements from contentInfos
		}
		else
		{
			nLast = nIndexPage;  // found last page 
		}
	} // end if(volPrx)
	
	IHttpResponse &out = _pHttpRequestCtx->Response();
	out << "<input type=\"hidden\" name=\"startname\" id=\"startname\" value=\""<< strStartName << "\">\n";
	out << "<input type=\"hidden\" name=\"presearchname\" id=\"presearchname\" value=\""<< strVolumeName << "\">\n";
	out << "<input type=\"hidden\" name=\"lastpage\" id=\"lastpage\" value=\""<< nLast << "\">\n";
	return true;
}

bool Controller::listContent(std::string& strContentName, int nIndexPage, ContentInfos& contentInfos)
{
	VolumeInfos volumeInfos;
	try
	{
		volumeInfos = _contentStorePrx->listVolumes("", true);
	}
	catch (const Ice::Exception& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"Fail To List Volumes Of Special Content From Server [%s]", ex.ice_name().c_str());
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Fail To List Volumes Of Special Content From Server [%s]"), ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		return false;
	}
	int count = 0;
	int nLowLimit = (nIndexPage -1) * _nMaxCount;
	int nHighLimit = nIndexPage * _nMaxCount;
	VolumeInfos::const_iterator p = volumeInfos.begin();
	ContentInfos tempInfos;
	for (; p != volumeInfos.end(); p++)
	{
		VolumePrx volPrx = _contentStorePrx->openVolume(p->name);
		if (volPrx)
		{
			tempInfos.clear();
			tempInfos = volPrx->listContents(_strMetaDataNames, strContentName, 1);
			if ((!tempInfos.empty()) && (tempInfos[0].name == strContentName))
			{
				count++;
				if ((count >= nLowLimit) && (count < nHighLimit))
				{
					contentInfos.push_back(tempInfos[0]);
				};
			} // end if
		} // end if(volPrx)
	} // end for
	return true;
}

