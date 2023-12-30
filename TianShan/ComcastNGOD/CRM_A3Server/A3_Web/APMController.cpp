// FileName : APMController.cpp
// Author   : Zheng Junming
// Date     : 2009-06
// Desc     : handler of asset propagate web page

#include <Log.h>
#include <httpdInterface.h>
#include "urlstr.h"
#include "StringUtil.h"
#include "APMController.h"

using namespace ZQ::common;

namespace APMWeb
{

const char* const Controller::CS_VAR_MODULE_NAME = "APMWeb";
const char* const Controller::CS_VAR_TEMPLATE = "#template";
const char* const Controller::CS_VAR_ENDPOINT = "#endpoint";
const char* const Controller::CS_VAR_MAXCOUNT = "#maxcount";
const char* const Controller::CS_VAR_COMMON_METADATA = "#commonMetaData";
const char* const Controller::CS_VAR_REPLIC_METADATA = "#replicaMetaData";

const char* const Controller::CS_VAR_ASSET_PAID = "AssetPAID";
const char* const Controller::CS_VAR_ASSET_PID = "AssetPID";
const char* const Controller::CS_VAR_ASSET_PRE_PAID = "AssetPrePAID";
const char* const Controller::CS_VAR_ASSET_PRE_PID = "AssetPrePID";
const char* const Controller::CS_VAR_SEARCH_PAID = "SearchPAID";
const char* const Controller::CS_VAR_SEARCH_PID = "SearchPID";
const char* const Controller::CS_VAR_REPLICA_PAID = "ReplicaPAID";
const char* const Controller::CS_VAR_REPLICA_PID = "ReplicaPID";
const char* const Controller::CS_VAR_REPLICA_VOLUME = "ReplicaVolume";
const char* const Controller::CS_VAR_REPLICA_NETID = "ReplicaNetId";

Controller::Controller(IHttpRequestCtx *pHttpRequestCtx)
: _pHttpRequestCtx(pHttpRequestCtx)
{

}

Controller::~Controller()
{
	finalize();
}

bool Controller::init()
{
	_template = _pHttpRequestCtx->GetRequestVar(CS_VAR_TEMPLATE);
	if (NULL == _template)
	{
		_pHttpRequestCtx->Response().SetLastError("404 Internal Server Errors(Missing web template)");
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Web template is missing in the request."));
		return false;
	}

	// get server address 
	_endpoint = _pHttpRequestCtx->GetRequestVar(CS_VAR_ENDPOINT);
	if (NULL == _endpoint)
	{
		_pHttpRequestCtx->Response().SetLastError("404 Internal Server Errors(Missing APM Server address)");
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "APM Server address is missing in the request."));
		return false;
	}

	_commonMetaData = _pHttpRequestCtx->GetRequestVar(CS_VAR_COMMON_METADATA);
	if (NULL == _commonMetaData)
	{
		_pHttpRequestCtx->Response().SetLastError("404 Internal Server Errors(Missing Asset's common metaData)");
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Asset common metaData is missing in the request."));
		return false;
	}

	_replicaMetaData = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLIC_METADATA);
	if (NULL == _replicaMetaData)
	{
		_pHttpRequestCtx->Response().SetLastError("404 Internal Server Errors(Missing Asset's replica metaData)");
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Asset replica metaData is missing in the request."));
		return false;
	}

	try
	{
		int argc = 0;
		_ic = Ice::initialize(argc, NULL);
		Ice::ObjectPrx base = _ic->stringToProxy(_endpoint);
		_a3FacedeProxy = A3Module::A3FacedePrx::checkedCast(base);
		if (!_a3FacedeProxy)
		{
			_pHttpRequestCtx->Response().SetLastError("404 Internal Server Errors(APM Server address is wrong or APM Server isn't running)");
			glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "APM Server address is wrong or APM Server isn't running"));
			return false;
		}
	}
	catch (const Ice::ConnectFailedException& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"404 Internal Server Errors(Fail To Connect APM Server [%s])", ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Connect APM server fail[%s]"), ex.ice_name().c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		char errorMsg[255];
		sprintf(errorMsg,"404 Internal Server Errors([%s])", ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "Ice Exception [%s]"),ex.ice_name().c_str());
		return false;
	} 
	return true;
}

bool Controller::listAssets()
{
	// get request parameters and connect APM Server;
	if (!init())
	{
		return false;
	}
	const char* maxCount = _pHttpRequestCtx->GetRequestVar(CS_VAR_MAXCOUNT);
	int nMaxCount = maxCount ? atoi(maxCount) : 15;
	if (nMaxCount < 15)
	{
		nMaxCount = 15;
	}
	const char* assetPAID = _pHttpRequestCtx->GetRequestVar(CS_VAR_ASSET_PAID);
	std::string strAssetPAID = assetPAID ? assetPAID : "";
	const char* assetPID  = _pHttpRequestCtx->GetRequestVar(CS_VAR_ASSET_PID);
	std::string strAssetPID = assetPID ? assetPID : "";

	const char* preAssetPAID = _pHttpRequestCtx->GetRequestVar(CS_VAR_ASSET_PRE_PAID);
	std::string strPreAssetPAID = preAssetPAID ? preAssetPAID: "";
	const char* preAssetPID  = _pHttpRequestCtx->GetRequestVar(CS_VAR_ASSET_PRE_PID);
	std::string strPreAssetPID = preAssetPID ? preAssetPID : "";

	// get search condition
	const char* PAID = _pHttpRequestCtx->GetRequestVar(CS_VAR_SEARCH_PAID);
	std::string strPAID = PAID ? PAID : "";
	if (strPAID != "") // erase empty space from strSearchName 
	{
		size_t nFirst = strPAID.find_first_not_of(' ');
		strPAID = strPAID.substr(nFirst, strPAID.find_last_not_of(' ') - nFirst + 1);
	}
	const char* PID  = _pHttpRequestCtx->GetRequestVar(CS_VAR_SEARCH_PID);
	std::string strPID = PID ? PID : "";
	if (strPID != "") // erase empty space from strSearchName 
	{
		size_t nFirst = strPID.find_first_not_of(' ');
		strPID = strPID.substr(nFirst, strPID.find_last_not_of(' ') - nFirst + 1);
	}

	// get page index
	const char * strCurPage = _pHttpRequestCtx->GetRequestVar("pageindex");
	int iCurPage = strCurPage ? atoi(strCurPage) : 1;
	if (iCurPage <= 0)
	{
		iCurPage = 1;
	}

	A3Module::A3Assets a3Assets;
	try
	{
		if (strPID.empty() || strPAID.empty())
		{
			if (iCurPage == 1) // reset list condition
			{
				strAssetPAID = strAssetPID = strPreAssetPAID = strPreAssetPID = "";
			}
			a3Assets = _a3FacedeProxy->listAssets(strAssetPID, strAssetPAID, nMaxCount, false);
			if (a3Assets.empty() && iCurPage > 1) // back to previous page
			{
				a3Assets = _a3FacedeProxy->listAssets(strPreAssetPID, strPreAssetPAID, nMaxCount, false);
				iCurPage--;
			}
			else
			{
				// set list condition
				strPreAssetPAID = strAssetPAID;
				strPreAssetPID = strAssetPID;
				A3Module::A3Assets::reverse_iterator revIter = a3Assets.rbegin();
				size_t npos = revIter->first.find("_");
				strAssetPAID =  revIter->first.substr(0, npos);
				strAssetPID = revIter->first.substr(npos + 1);
			}
		}
		else
		{
			a3Assets = _a3FacedeProxy->listAssets(strPID, strPAID, 1, true);
			strAssetPAID = strAssetPID = strPreAssetPAID = strPreAssetPID = "";
			iCurPage = 1;
		}
	}
	catch (const Ice::Exception& ex)
	{
		return false;
	}
	// output html page
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
	std::string strRequestURL = _pHttpRequestCtx->GetUri();
	out << "<form id=\"searchform\" method=\"get\" action=\"" << strRequestURL << "\" style=\"display:inline\">\n"
	    << "<div>&nbsp&nbsp\n"
		<< "PAID:&nbsp<input type=\"text\" name=\"" << CS_VAR_SEARCH_PAID << "\" id=\"" << CS_VAR_SEARCH_PAID << "\" value =\""<< strPAID <<"\"size=\"30\" />&nbsp&nbsp\n"
		<< "PID:&nbsp<input type=\"text\" name=\"" << CS_VAR_SEARCH_PID << "\" id=\"" << CS_VAR_SEARCH_PID <<"\" value =\""<< strPID <<"\"size=\"30\" />&nbsp&nbsp\n"
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
		<< "<tr width=\"100%\">\n"
		<< "<th width=\"35%\">PAID</th><th width=\"35%\">PID</th><th width=\"30%\">Replica</th>"
		<< "</tr>\n";
	std::string strRootURL = "ListAssetReplica" + strRequestURL.substr(strRequestURL.find("."));
	ZQ::common::URLStr urlStr;
	urlStr.setPath(strRootURL.c_str());
	urlStr.setVar(CS_VAR_TEMPLATE, _template);
	urlStr.setVar(CS_VAR_ENDPOINT, _endpoint);
	urlStr.setVar(CS_VAR_COMMON_METADATA, _commonMetaData);
	urlStr.setVar(CS_VAR_REPLIC_METADATA, _replicaMetaData);
	urlStr.setVar(CS_VAR_MAXCOUNT, maxCount);
	std::string strUrl;
	std::string strTempPAID;
	std::string strTempPID;
	for (A3Module::A3Assets::iterator iter = a3Assets.begin(); iter != a3Assets.end(); iter++)
	{
		size_t npos = iter->first.find("_");
		strTempPAID = iter->first.substr(0, npos);
		strTempPID = iter->first.substr(npos + 1);
		urlStr.setVar(CS_VAR_REPLICA_PAID, strTempPAID.c_str());
		urlStr.setVar(CS_VAR_REPLICA_PID, strTempPID.c_str());
		strUrl = urlStr.generate();
		strUrl = strUrl.substr(strUrl.find(strRootURL));
		out << "<tr>";
		out << "<a href=\"" << strUrl << "\">";
		out << "<td>" << strTempPAID  << "</td>\n";
		out << "<td>" << strTempPID   << "</td>\n";
		out << "<td>" << iter->second << "</td>\n";
        out << "</a>";
		out << "</tr>";
	}
	out	<< "</table>\n" // end table 3
		<< "</td></tr>\n" 
		<< "</table>\n"; // end table 1
	out << "<input type=\"hidden\" name=\"" << CS_VAR_TEMPLATE << "\" id=\"" << CS_VAR_TEMPLATE << "\" value=\""<< _template <<"\">\n" 
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ENDPOINT << "\" id=\"" << CS_VAR_ENDPOINT << "\" value=\""<< _endpoint <<"\">\n"
	    << "<input type=\"hidden\" name=\"" << CS_VAR_MAXCOUNT << "\" id=\"" << CS_VAR_MAXCOUNT << "\" value=\""<< nMaxCount <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_COMMON_METADATA << "\" id=\"" << CS_VAR_COMMON_METADATA << "\" value=\""<< _commonMetaData <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_REPLIC_METADATA << "\" id=\"" << CS_VAR_REPLIC_METADATA << "\" value=\""<< _replicaMetaData <<"\">\n";

	out << "<input type=\"hidden\" name=\"" << CS_VAR_ASSET_PAID << "\" id=\"" << CS_VAR_ASSET_PAID << "\" value=\""<< strAssetPAID<<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ASSET_PID<< "\" id=\"" << CS_VAR_ASSET_PID << "\" value=\""<< strAssetPID <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ASSET_PRE_PAID<< "\" id=\"" << CS_VAR_ASSET_PRE_PAID << "\" value=\""<< strPreAssetPAID <<"\">\n"
		<< "<input type=\"hidden\" name=\"" << CS_VAR_ASSET_PRE_PID << "\" id=\"" << CS_VAR_ASSET_PRE_PID  << "\" value=\""<< strPreAssetPID <<"\">\n";

	out << "</form>\n";
	return true;
}

void Controller::getMetaData(const char *paramList, 
							 TianShanIce::StrValues &metaDataNames, 
							 TianShanIce::StrValues &tableHeader)
{
	TianShanIce::StrValues params;
	StorageWeb::StringUtil::splitString(params, paramList, ";");
	for (int i = 0; i < params.size(); i++)
	{
		TianShanIce::StrValues param;
		StorageWeb::StringUtil::splitString(param, params[i], ":");
		std::transform(param.begin(), param.end(), param.begin(), StorageWeb::StringUtil::trimWS); // erase empty space
		if (param.size() > 0 && (param[0].find("sys.") != std::string::npos))
		{
			metaDataNames.push_back(param[0]); // exclusive string no include "sys."
		}
		switch(param.size())
		{
		case 1: // use param name as display title
			tableHeader.push_back(param[0]);
			break;
		case 2:
			tableHeader.push_back(param[1]);
			break;
		default:
			break;
		}
	}
}

bool Controller::listAssetReplica()
{
	// get request parameters and connect APM Server;
	if (!init())
	{
		return false;
	}
	const char* assetPAID = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_PAID);
	const char* assetPID = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_PID);
	if (!assetPAID || !assetPID)
	{
		_pHttpRequestCtx->Response().SetLastError("404 Internal Server Error(Missing Asset PID or PAID");
		glog(ZQ::common::Log::L_ERROR, "Missing Asset PID or PAID");
		return false;
	}


	A3Module::A3Contents a3Contents;
	try
	{
		a3Contents = _a3FacedeProxy->findContentsByAsset(assetPID, assetPAID);
	}
	catch (const Ice::Exception& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"404 Internal Server Errors[%s]", ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		glog(ZQ::common::Log::L_ERROR,  CLOGFMT(CS_VAR_MODULE_NAME, "Catch an exception[%s]"), ex.ice_name().c_str());
		return false;
	}
	IHttpResponse &out = _pHttpRequestCtx->Response();
	if (a3Contents.empty())
	{
		std::string strRequestURL = _pHttpRequestCtx->GetUri();
		std::string strRootURL = "ListAssets" + strRequestURL.substr(strRequestURL.find("."));
		ZQ::common::URLStr urlStr;
		urlStr.setPath(strRootURL.c_str());
		urlStr.setVar(CS_VAR_TEMPLATE, _template);
		urlStr.setVar(CS_VAR_ENDPOINT, _endpoint);
		urlStr.setVar(CS_VAR_COMMON_METADATA, _commonMetaData);
		urlStr.setVar(CS_VAR_REPLIC_METADATA, _replicaMetaData);
		urlStr.setVar(CS_VAR_MAXCOUNT, _pHttpRequestCtx->GetRequestVar(CS_VAR_MAXCOUNT));
		std::string strUrl = urlStr.generate();
		strUrl = strUrl.substr(strUrl.find(strRootURL));
		out << "<head>\n"
			<< "<meta http-equiv=\"refresh\" content=\"1;url=" << strUrl << "\">"
			<< "</head>";
	}


	TianShanIce::StrValues commonMetaDataNames;
	TianShanIce::StrValues commonTableHeader;
	getMetaData(_commonMetaData, commonMetaDataNames, commonTableHeader);

	TianShanIce::StrValues replicaMetaDataNames;
	TianShanIce::StrValues replicaTableHeader;
	getMetaData(_replicaMetaData, replicaMetaDataNames, replicaTableHeader);
	
	int replicaId = 1;
	std::string strVolume;
	std::string strNetId;
	TianShanIce::Properties metaDatas;
	A3Module::A3Contents::iterator contentIter = a3Contents.begin();
	for (; contentIter != a3Contents.end(); contentIter++)
	{
		try
		{
			metaDatas = (*contentIter)->getMetaData();
			(*contentIter)->getVolumeInfo(strNetId, strVolume);
		}
		catch (const Ice::Exception& ex)
		{
			continue;
		}
		if (1 == replicaId)
		{
			out << "<h2>Common Properties</h2>\n";
			out << "<form id=\"listAssets\" style=\"display:inline\">\n";
			out << "<table id=\"common-tbl\" class=\"chunk listTable\">\n";
			out << "<tr width=\"100%\">\n"
				<< "<th width=\"30%\"> PAID</th>\n"
				<< "<td width=\"70%\">" << assetPAID <<"</td>\n"
				<< "</tr>\n";
			out << "<tr width=\"100%\">\n"
				<< "<th width=\"30%\"> PID</th>\n"
				<< "<td width=\"70%\">" << assetPID <<"</td>\n"
				<< "</tr>\n";
			for (size_t i = 0; i < commonTableHeader.size(); i++)
			{
				out << "<tr width=\"100%\">\n"
				    << "<th width=\"30%\">" << commonTableHeader[i] <<"</th>\n"
					<< "<td width=\"70%\">" << metaDatas[commonMetaDataNames[i]] <<"</td>\n"
					<< "</tr>\n";
			}
			out << "</table><br /><hr />\n";

			out << "<h2>Replica Properties</h2>\n";
			out << "<table id=\"replica-tbl\" class=\"chunk listTable\">\n";
			out << "<tr width=\"100%\">\n"
				<< "<th>Replica#</th>\n"
				<< "<th>Volume</th>\n"
				<< "<th>NetId</th>\n";
			for (size_t i = 0; i < replicaTableHeader.size(); i++)
			{
				out << "<th>" << replicaTableHeader[i] << "</th>\n";
			}
			out	<< "</tr>\n";
		}
		std::string strRequestURL = _pHttpRequestCtx->GetUri();
		std::string strRootURL = "DeleteReplica" + strRequestURL.substr(strRequestURL.find("."));
		ZQ::common::URLStr urlStr;
		urlStr.setPath(strRootURL.c_str());
		urlStr.setVar(CS_VAR_TEMPLATE, _template);
		urlStr.setVar(CS_VAR_ENDPOINT, _endpoint);
		urlStr.setVar(CS_VAR_COMMON_METADATA, _commonMetaData);
		urlStr.setVar(CS_VAR_REPLIC_METADATA, _replicaMetaData);
		urlStr.setVar(CS_VAR_MAXCOUNT, _pHttpRequestCtx->GetRequestVar(CS_VAR_MAXCOUNT));

		urlStr.setVar(CS_VAR_REPLICA_PAID, assetPAID);
		urlStr.setVar(CS_VAR_REPLICA_PID, assetPID);
		urlStr.setVar(CS_VAR_REPLICA_VOLUME, strVolume.c_str());
		urlStr.setVar(CS_VAR_REPLICA_NETID, strNetId.c_str());

		std::string strUrl = urlStr.generate();
		strUrl = strUrl.substr(strUrl.find(strRootURL));
		out << "<tr width=\"100%\">\n"
			<< "<td width=\"10%\">" << replicaId << "<a href=\"" << strUrl << "\">" <<"&nbsp[delete]</a></td>\n"
			<< "<td width=\"10%\">" << strVolume << "</td>\n"
			<< "<td width=\"10%\">" << strNetId  << "</td>\n";
		for (size_t i = 0; i < replicaMetaDataNames.size(); i++)
		{
			out << "<td>" << metaDatas[replicaMetaDataNames[i]] << "</td>\n";
		}
		out	<< "</tr>\n";
		replicaId++;
	}
	out << "</table>\n";
	out << "</form>\n";
	return true;
}

bool Controller::deleteReplica()
{
	if (!init())
	{
		return false;
	}
	const char* PAID = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_PAID);
	const char* PID = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_PID);
	const char* volume = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_VOLUME);
	const char* netId = _pHttpRequestCtx->GetRequestVar(CS_VAR_REPLICA_NETID);
	if (!PAID || !PID || !volume || !netId)
	{
		_pHttpRequestCtx->Response().SetLastError("Invalid replica name");
		return false;
	}
	std::string strRequestURL = _pHttpRequestCtx->GetUri();
	std::string strRootURL = "ListAssetReplica" + strRequestURL.substr(strRequestURL.find("."));
	ZQ::common::URLStr urlStr;
	urlStr.setPath(strRootURL.c_str());
	urlStr.setVar(CS_VAR_TEMPLATE, _template);
	urlStr.setVar(CS_VAR_ENDPOINT, _endpoint);
	urlStr.setVar(CS_VAR_COMMON_METADATA, _commonMetaData);
	urlStr.setVar(CS_VAR_REPLIC_METADATA, _replicaMetaData);
	urlStr.setVar(CS_VAR_MAXCOUNT, _pHttpRequestCtx->GetRequestVar(CS_VAR_MAXCOUNT));
	urlStr.setVar(CS_VAR_REPLICA_PAID, PAID);
	urlStr.setVar(CS_VAR_REPLICA_PID, PID);
	std::string strUrl = urlStr.generate();
	strUrl = strUrl.substr(strUrl.find(strRootURL));
	IHttpResponse &out = _pHttpRequestCtx->Response();
	out << "<head>\n"
		<< "<meta http-equiv=\"refresh\" content=\"3;url=" << strUrl << "\">"
		<< "</head>";
	
	try
	{
		if (_a3FacedeProxy->deleteA3Content(PID, PAID, netId, volume))
		{
			out << "<h2><a href=\"" << strUrl << "\">Success to delete replica, return previous page</a></h2>\n";
		}
		else
		{
			out << "<h2><a href=\"" << strUrl << "\">Fail to delete replica, return previous page</a></h2>\n";
		}
	}
	catch (const Ice::Exception& ex)
	{
		char errorMsg[255];
		sprintf(errorMsg,"Catch an exceptin [%s] when destroy replica)", ex.ice_name().c_str());
		_pHttpRequestCtx->Response().SetLastError(errorMsg);
		glog(Log::L_ERROR, CLOGFMT(CS_VAR_MODULE_NAME, "[%s]"), errorMsg);
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
			_a3FacedeProxy = NULL;
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

} // end for APMWeb