// PathController.cpp: implementation of the PathController class.
//
//////////////////////////////////////////////////////////////////////

#include "SystemUtils.h"
#include "StdAfx.h"
#include "AdminCtrlUtil.h"
#include "PathController.h"
#include <Ice/Ice.h>
#include <TsStreamer.h>
#include <TsStorage.h>
#include <fstream>
#include <urlstr.h>

using namespace ZQ::common;
using namespace TianShanIce;

static bool extractPrivateData(const std::string &keyPrefix, IHttpRequestCtx *pRqstCtx, ValueMap &privateData)
{
	privateData.clear();
	if (NULL == pRqstCtx)
	{
		return false;
	}
	AdminCtrlUtil::KVData_t kvs;
	if (!AdminCtrlUtil::extractKVData(keyPrefix, pRqstCtx, kvs))
	{
		glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to get all vars with prefix [%s]."), keyPrefix.c_str());
		return false;
	}
	for (AdminCtrlUtil::KVData_t::const_iterator cit_kv = kvs.begin(); cit_kv != kvs.end(); ++cit_kv)
	{
		if (cit_kv->second.size() < 2)
		{
			glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad variant format. [%s]"), cit_kv->second.c_str());
			return false;
		}
		std::string pdType = cit_kv->second.substr(0, 2);
		std::string pdValue = cit_kv->second.substr(2);
		Variant var;
		if (AdminCtrlUtil::str2var(pdValue, pdType, var))
		{
			privateData[cit_kv->first] = var;
		}
		else
		{
			glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad variant format. [%s]"), cit_kv->second.c_str());
			return false;
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////
#define PATHCTRL_CATCH_COMMON_CASE \
catch (const TianShanIce::BaseException& ex)\
{\
	glog(Log::L_ERROR, CLOGFMT(PathCtrl, "caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str()); \
	std::string errmsg = std::string("caught exception[") + ex.ice_name() + "]"; \
	_pHttpRequestCtx->Response().SetLastError(errmsg.c_str()); \
	return false; \
}\
catch (const Ice::Exception& ex)\
{\
	glog(Log::L_ERROR, CLOGFMT(PathCtrl, "caught [%s]"), ex.ice_name().c_str()); \
	std::string errmsg = std::string("caught exception[") + ex.ice_name() + "]"; \
	_pHttpRequestCtx->Response().SetLastError(errmsg.c_str()); \
	return false; \
}\
catch (const char* msg)\
{\
	glog(Log::L_ERROR, CLOGFMT(PathCtrl, "caught exception [%s]"), msg); \
	std::string errmsg = std::string("caught exception[") + msg + "]"; \
	_pHttpRequestCtx->Response().SetLastError(errmsg.c_str()); \
	return false; \
}\
catch (...)\
{\
	glog(Log::L_ERROR, CLOGFMT(PathCtrl, "caught exception")); \
	_pHttpRequestCtx->Response().SetLastError("caught exception"); \
	return false; \
}

#define PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG \
	_pHttpRequestCtx->Response().SetLastError("Bad program logic or incorrect config was detected!"); \
	return false

//////////////////////////////////////////////////////////////////////

PathController::PathController(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{

}

PathController::~PathController()
{
	uninit();
}

bool PathController::init()
{
	_rootDir = _pHttpRequestCtx->GetRootDir();
	if (NULL == _rootDir)
	{
		glog(Log::L_ERROR, CLOGFMT(PathCtrl, "web root directory is required in the request."));
		PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
	}
	// gather required variables
	_template = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_TEMPLATE); // template may not need for some page
	_endpoint = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ENDPOINT);
	if (NULL == _endpoint)
	{
		glog(Log::L_ERROR, CLOGFMT(PathCtrl, "endpoint is missed in the request."));
		PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
	}
	std::string prxstr = std::string(SERVICE_NAME_PathManager":") + _endpoint;
	try
	{
		int argc = 0;
		_ic = Ice::initialize(argc, NULL);
		glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "connecting PathManager [%s]."), prxstr.c_str());
		_pa = Transport::PathAdminPrx::checkedCast(_ic->stringToProxy(prxstr));
	}
	PATHCTRL_CATCH_COMMON_CASE;
	return true;
}

void PathController::uninit()
{
	try
	{
		_pa = NULL;
		if (_ic)
		{
			_ic->destroy();
			_ic = NULL;
		}
	}
	catch (...)
	{
		glog(Log::L_ERROR, CLOGFMT(PathCtrl, "caught exception during uninit()."));
	}
}

//////////////////////////////////////////////////////////////////////////
#define PATHCTRL_VARPREFIX_PD       "path.pd#"
//////////////////////////////////////////////////////////////////////
#define PATHCTRL_ACTION_UPDATESG    'u'
#define PATHCTRL_ACTION_REMOVESG    'r'
#define PATHCTRL_ACTIONSET_SG       "ur"

#define PATHCTRL_VAR_SGID           "path.sg#id"
#define PATHCTRL_VAR_SGDESC         "path.sg#desc"

struct LessSvcGrp
{
     bool operator()(const ::TianShanIce::Transport::ServiceGroup& A, const ::TianShanIce::Transport::ServiceGroup& B)
     {
          return A.id < B.id;
     }
};

// this function work together with AdminCtrl_ServiceGroup.html
bool PathController::ServiceGroupPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "ServiceGroupPage() requested"));
	if (!init())
		return false;

	bool bSGUpdated = false;
	try{
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "ServiceGroupPage() with action[%s]"), action);
			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_SG) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "ServiceGroupPage() bad action[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}

			const char* sgIdStr = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_SGID);
			if (NULL == sgIdStr)
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "ServiceGroupPage() service group id is missed in the request."));
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}

			::Ice::Int sgId = atoi(sgIdStr);
			if (strchr(action, PATHCTRL_ACTION_REMOVESG))
			{
				// remove service group
				_pa->removeServiceGroup(sgId);
				bSGUpdated = true;
			}

			if (strchr(action, PATHCTRL_ACTION_UPDATESG))
			{
				// update service group
				const char* sgDesc = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_SGDESC);
				if (NULL == sgDesc)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "ServiceGroupPage() service group's desc is missed in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				_pa->updateServiceGroup(sgId, sgDesc);
				bSGUpdated = true;
			}
		} // end if(action)

		IHttpResponse &out = _pHttpRequestCtx->Response();
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_ServiceGroup.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		// list all service groups
		Transport::ServiceGroups sgs = _pa->listServiceGroups();
		// sort by service group Id
		std::sort(sgs.begin(), sgs.end(), LessSvcGrp());

		if (bSGUpdated)
		{ // need update the serverload template file
			const char* srvrloadFilePath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
			if (srvrloadFilePath && (*srvrloadFilePath) != '\0')
			{
				// construct the content to be updated
				AdminCtrlUtil::StringVector content;
				content.reserve(sgs.size());
				for (size_t iSG = 0; iSG < sgs.size(); ++iSG)
					content.push_back(AdminCtrlUtil::int2str(sgs[iSG].id));

				int nTried = 0;
				do
				{
					++nTried;
					if (AdminCtrlUtil::updateXML(srvrloadFilePath, "SrvrList/CMGroup/NodeGroup", content))
					{
						glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "Updated server load template file [%s] successfully."), srvrloadFilePath);
						break;
					}

					if (nTried < 2)
					{ // retry
						glog(Log::L_WARNING, CLOGFMT(PathCtrl, "Failed to update server load template file [%s]. Retry after 500 msec"), srvrloadFilePath);
						//Sleep(500);
						SYS::sleep(500);
						continue;
					}

					// failed
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "Failed to update server load template file [%s] after 2 tries."), srvrloadFilePath);
					break;

				} while (true);
			}
			else glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "ServiceGroupPage() no server load template file path provide."));
		}

		out << "<script type='text/javascript'>\n";
		for (int i = 0; i < sgs.size(); ++i)
		{
			out << "_sgs.push(new TSServiceGroup("
				<< sgs[i].id << ",\""
				<< sgs[i].desc << "\",\""
				<< sgs[i].type << "\"));\n";
		}
		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		{
			const char* srvrloadPath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
			out << "_srvrloadpath=\"";
			if (srvrloadPath)
			{ // escape the '\'
				out << AdminCtrlUtil::escapeString(srvrloadPath);
			}
			out << "\";\n";
		}

		out << "display();\n";
		out << "</script>\n";
		return true;
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return false;
}

#define PATHCTRL_ACTION_UPDATESTORAGE       'u'
#define PATHCTRL_ACTION_REMOVESTORAGE       'r'
#define PATHCTRL_ACTION_SETSTORAGEPRIVATE   'p'
#define PATHCTRL_ACTION_CONNECTCONTENTSTORE 'c'
#define PATHCTRL_ACTIONSET_STORAGE          "urpc"

#define PATHCTRL_VAR_STORAGE_NETID          "path.strg#netid"
#define PATHCTRL_VAR_STORAGE_TYPE           "path.strg#type"
#define PATHCTRL_VAR_STORAGE_DESC           "path.strg#desc"
#define PATHCTRL_VAR_STORAGE_IFEP           "path.strg#ifep"

// this function work together with AdminCtrl_Storage.html
bool PathController::StoragePage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "StoragePage() requested"));
	if (!init())
		return false;

	try{
		Storage::ContentStorePrx storageSvc;
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		char actCode = '-';
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "StoragePage() action[%s]"), action);
			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_STORAGE) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() bad action[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}

			if (strchr(action, PATHCTRL_ACTION_CONNECTCONTENTSTORE))
			{
				actCode = PATHCTRL_ACTION_CONNECTCONTENTSTORE;
				// connect ContentStore
				const char* strgEndpoint = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_IFEP);
				if (NULL == strgEndpoint)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed endpoint of storage"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				storageSvc = Storage::ContentStorePrx::checkedCast(_ic->stringToProxy(strgEndpoint));
			}

			if (strchr(action, PATHCTRL_ACTION_REMOVESTORAGE))
			{
				actCode = PATHCTRL_ACTION_REMOVESTORAGE;
				// remove storage
				const char* strgNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_NETID);
				if (NULL == strgNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's net id"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				_pa->removeStorage(strgNetId);
			}

			if (strchr(action, PATHCTRL_ACTION_UPDATESTORAGE))
			{
				actCode = PATHCTRL_ACTION_UPDATESTORAGE;
				// update storage
				const char* strgNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_NETID);
				if (NULL == strgNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's net id"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* strgType = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_TYPE);
				if (NULL == strgType)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's type"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* strgEndpoint = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_IFEP);
				if (NULL == strgEndpoint)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's endpoint"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* strgDesc = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_DESC);
				if (NULL == strgDesc)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's description"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				_pa->updateStorage(strgNetId, strgType, strgEndpoint, strgDesc);
			}

			if (strchr(action, PATHCTRL_ACTION_SETSTORAGEPRIVATE))
			{
				actCode = PATHCTRL_ACTION_UPDATESTORAGE;
				// set storage private data.
				const char* strgNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_NETID);
				if (NULL == strgNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] missed storage's net id"), actCode);
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				ValueMap privateData;
				// extract private data
				if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() action[%c] failed to extract private data."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				for (ValueMap::const_iterator cit_pd = privateData.begin(); cit_pd != privateData.end(); ++cit_pd)
				{
					_pa->setStoragePrivateData(strgNetId, cit_pd->first, cit_pd->second);
				}
			}

		} // end if(action)

		IHttpResponse &out = _pHttpRequestCtx->Response();
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_Storage.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "StoragePage() failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		out << "<script type='text/javascript'>\n";
		// new storage
		if (storageSvc)
		{
			out << "_newStorage = new TSStorage(\""
				<< storageSvc->getNetId() << "\",\""
				<< storageSvc->type() << "\",\"\",\""
				<< _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_IFEP)
				<< "\");\n";
		}

		// list all streamers
		Transport::Storages strgs = _pa->listStorages();
		for (int i = 0; i < strgs.size(); ++i)
		{
			out << "_strgs[" << i << "]=new TSStorage(\""
				<< strgs[i].netId << "\",\""
				<< strgs[i].type << "\",\""
				<< strgs[i].desc << "\",\""
				<< strgs[i].ifep << "\");\n";
			for (ValueMap::const_iterator cit_val = strgs[i].privateData.begin(); cit_val != strgs[i].privateData.end(); ++cit_val)
			{
				out << "_strgs[" << i << "].privateData[\"" << cit_val->first << "\"]=\"";
				out << AdminCtrlUtil::vartype(cit_val->second) << AdminCtrlUtil::var2str(cit_val->second);
				out << "\";\n";
			}
		}
		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		out << "display();\n";
		out << "</script>\n";

		return true;
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return false;
}

#define PATHCTRL_ACTION_UPDATESTREAMER      'u'
#define PATHCTRL_ACTION_REMOVESTREAMER      'r'
#define PATHCTRL_ACTION_SETSTREAMERPRIVATE  'p'
#define PATHCTRL_ACTION_CONNECTSTREAMSVC    'c'
#define PATHCTRL_ACTIONSET_STREAMER         "urpc"

#define PATHCTRL_VAR_STREAMER_NETID         "path.stmr#netid"
#define PATHCTRL_VAR_STREAMER_TYPE          "path.stmr#type"
#define PATHCTRL_VAR_STREAMER_DESC          "path.stmr#desc"
#define PATHCTRL_VAR_STREAMER_IFEP          "path.stmr#ifep"

// this function work together with AdminCtrl_Streamer.html
bool PathController::StreamerPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request streamer page."));
	if (!init())
	{
		return false;
	}

	try{
		Streamer::StreamServicePrx activeStreamSvc;
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request StreamerPage with action [%s]"), action);
			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_STREAMER) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad action type.[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}

			if (strchr(action, PATHCTRL_ACTION_REMOVESTREAMER))
			{
				// remove streamer
				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				_pa->removeStreamer(stmrNetId);
			}
			if (strchr(action, PATHCTRL_ACTION_CONNECTSTREAMSVC))
			{
				// connect stream service
				const char* stmrEndpoint = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_IFEP);
				if (NULL == stmrEndpoint)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's endpoint is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "connect StreamService [%s]."), stmrEndpoint);
				activeStreamSvc = Streamer::StreamServicePrx::checkedCast(_ic->stringToProxy(stmrEndpoint));
			}
			if (strchr(action, PATHCTRL_ACTION_UPDATESTREAMER))
			{
				// update streamer
				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* stmrType = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_TYPE);
				if (NULL == stmrType)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's type is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* stmrEndpoint = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_IFEP);
				if (NULL == stmrEndpoint)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's endpoint is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* stmrDesc = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_DESC);
				if (NULL == stmrDesc)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's description is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				_pa->updateStreamer(stmrNetId, stmrType, stmrEndpoint, stmrDesc);
			}
			if (strchr(action, PATHCTRL_ACTION_SETSTREAMERPRIVATE))
			{
				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				ValueMap privateData;
				// extract private data
				if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				for (ValueMap::const_iterator cit_pd = privateData.begin(); cit_pd != privateData.end(); ++cit_pd)
				{
					_pa->setStreamerPrivateData(stmrNetId, cit_pd->first, cit_pd->second);
				}
			}

		} // end if(action)

		IHttpResponse &out = _pHttpRequestCtx->Response();
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_Streamer.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		out << "<script type='text/javascript'>\n";
		// list stream service info
		if (activeStreamSvc)
		{
			out << "_activeStreamSvc = new TSStreamService(\""
				<< _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_IFEP) << "\",\""
				<< activeStreamSvc->getNetId() << "\"";
			Streamer::StreamerDescriptors sds = activeStreamSvc->listStreamers();
			for (Streamer::StreamerDescriptors::const_iterator cit_sd = sds.begin(); cit_sd != sds.end(); ++cit_sd)
			{
				out << ",new TSStreamerDescriptor(\""
					<< cit_sd->deviceId << "\",\""
					<< cit_sd->type << "\")";
			}
			out << ");\n";
		}
		// list all streamers
		Transport::Streamers stmrs = _pa->listStreamers();
		for (int i = 0; i < stmrs.size(); ++i)
		{
			out << "_stmrs[" << i << "]=new TSStreamer(\""
				<< stmrs[i].netId << "\",\""
				<< stmrs[i].type << "\",\""
				<< stmrs[i].desc << "\",\""
				<< stmrs[i].ifep << "\");\n";
			for (ValueMap::const_iterator cit_val = stmrs[i].privateData.begin(); cit_val != stmrs[i].privateData.end(); ++cit_val)
			{
				out << "_stmrs[" << i << "].privateData[\"" << cit_val->first << "\"]=\"";
				out << AdminCtrlUtil::vartype(cit_val->second) << AdminCtrlUtil::var2str(cit_val->second);
				out << "\";\n";
			}
		}
		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		out << "display();\n";
		out << "</script>\n";
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}

#define PATHCTRL_ACTION_LINK        'l'
#define PATHCTRL_ACTION_UNLINK      'u'
#define PATHCTRL_ACTION_SETPRIVATE  'p'
#define PATHCTRL_ACTION_ENABLE		'e'
#define PATHCTRL_ACTION_SEARCH		's'
#define PATHCTRL_ACTION_DISABLE		'd'
#define PATHCTRL_ACTIONSET_LINK     "lupeds"

#define PATHCTRL_VAR_LINKTYPE       "path.lnk#type"
#define PATHCTRL_VAR_LINKIDENT      "path.lnk#ident"
#define PATHCTRL_VAR_LINKIDENTS		"path.lnk#idents"
// this function work together with AdminCtrl_StorageLink.html
bool PathController::StorageLinkPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request storage link page."));
	if (!init())
	{
		return false;
	}

	try{
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request StorageLinkPage with action [%s]"), action);

			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_LINK) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad action type.[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}

			if (strspn(action, "ed") == nAction)
			{
				////////////////////////////add disable and enable actions
				if (strchr(action, PATHCTRL_ACTION_DISABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}

					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						//find storage link proxy
						if (lnkIdent.length() == 0)
							break;
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StorageLinkExPrx lnkEx = Transport::StorageLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (1 == lnkEx->status())
							continue;
						lnkEx->enableLink(false);
					}
				}
				/*
				if(strchr(action,PATHCTRL_ACTION_ENABLE))
				{
				char ident[100] = "";
				int i = 0;
				while(1)
				{
				memset(ident,0,sizeof(ident));
				sprintf(ident,"%s[%d]",PATHCTRL_VAR_LINKIDENTS,i);
				i++;
				const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(ident);
				if(NULL == lnkIdent)
				break;
				//find stream link proxy
				std::string lnkProxyStr = std::string(lnkIdent) + ":" + _endpoint;
				Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkProxyStr));
				//continue if link status is same as before,else enable the link
				if(1 == lnkEx->status())
				continue;
				lnkEx->enableLink(true);
				}
				}
				*/
				if (strchr(action, PATHCTRL_ACTION_ENABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}

					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						if (lnkIdent.length() == 0)
							break;
						//find storage link proxy
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StorageLinkExPrx lnkEx = Transport::StorageLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (0 == lnkEx->status())
							continue;
						lnkEx->enableLink(true);
					}
				}
			}
			else
			{
				const char* strgNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STORAGE_NETID);
				if (NULL == strgNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "storage's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				const char* lnkType = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKTYPE);
				if (NULL == lnkType)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "storage link's type is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				if (strchr(action, PATHCTRL_ACTION_UNLINK))
				{
					// unlink storage
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "storage link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find storage link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StorageLinkExPrx lnkEx = Transport::StorageLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));
					lnkEx->destroy();
				}

				if (strchr(action, PATHCTRL_ACTION_SETPRIVATE))
				{
					// set link private data
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "storage link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}

					ValueMap privateData;
					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find storage link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StorageLinkExPrx lnkEx = Transport::StorageLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));

					if (lnkEx->updatePrivateData(privateData))
					{
						glog(Log::L_INFO, CLOGFMT(PathCtrl, "Update private data successfully."));
					}
					else
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "Failed to update private data."));
						_pHttpRequestCtx->Response().SetLastError("Failed to update private data.");
						return false;
					}
				}

				if (strchr(action, PATHCTRL_ACTION_LINK))
				{
					// link storage
					ValueMap privateData;

					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					_pa->linkStorage(strgNetId, stmrNetId, lnkType, privateData);
				}
			} // end if(action)
		}

		IHttpResponse &out = _pHttpRequestCtx->Response();
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_StorageLink.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		out << "<script type='text/javascript'>\n";
		// list all link types
		StrValues types = _pa->listSupportedStorageLinkTypes();
		for (StrValues::const_iterator cit_type = types.begin(); cit_type != types.end(); ++cit_type)
		{
			out << "_types.push(new TSLinkType(\"" << (*cit_type) << "\"";
			TianShanIce::PDSchema schema = _pa->getStorageLinkSchema(*cit_type);
			for (TianShanIce::PDSchema::const_iterator cit_schema = schema.begin(); cit_schema != schema.end(); ++cit_schema)
			{
				//out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional);
				out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional2);
				out << ", \"";
				out << AdminCtrlUtil::vartype(cit_schema->defaultvalue) << AdminCtrlUtil::var2str(cit_schema->defaultvalue);
				out << "\")";
			}
			out << "));\n";
		}

		// list all streamers
		Transport::Streamers stmrs = _pa->listStreamers();
		for (Transport::Streamers::const_iterator cit_stmr = stmrs.begin(); cit_stmr != stmrs.end(); ++cit_stmr)
		{
			out << "_stmrs.push(\"" << cit_stmr->netId << "\");\n";
		}

		// list all storages
		int iLnk = 0;
		Transport::Storages strgs = _pa->listStorages();
		for (Transport::Storages::const_iterator cit_strg = strgs.begin(); cit_strg != strgs.end(); ++cit_strg)
		{
			out << "_strgs.push(\"" << cit_strg->netId << "\");\n";

			// list all links of this storage
			Transport::StorageLinks lnks = _pa->listStorageLinksByStorage(cit_strg->netId);
			for (Transport::StorageLinks::const_iterator cit_lnk = lnks.begin(); cit_lnk != lnks.end(); ++cit_lnk)
			{
				int status = 0;//0 if disable,1 if enable
				std::string ident = _ic->identityToString((*cit_lnk)->getIdent());
				std::string lnkPrx = ident + ":" + _endpoint;
				Transport::StorageLinkExPrx lnkEx;
				try
				{
					lnkEx = Transport::StorageLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkPrx));
				}
				catch (::Ice::Exception& e){}

				status = lnkEx->status();
				if (status & 1)//bit0 - 1 if disable
					status = 0;
				else
					status = 1;

				out << "_lnks[" << iLnk << "]=new TSStorageLink(\""
					<< _ic->identityToString((*cit_lnk)->getIdent()) << "\",\""
					<< (*cit_lnk)->getStorageId() << "\",\""
					<< (*cit_lnk)->getStreamerId() << "\",\""
					<< (*cit_lnk)->getType() << "\","
					<< status << ");\n";

				ValueMap pds = (*cit_lnk)->getPrivateData();
				for (ValueMap::const_iterator cit_pd = pds.begin(); cit_pd != pds.end(); ++cit_pd)
				{
					out << "_lnks[" << iLnk << "].privateData[\"" << cit_pd->first << "\"]=\"";
					out << AdminCtrlUtil::vartype(cit_pd->second) << AdminCtrlUtil::var2str(cit_pd->second);
					out << "\";\n";
				}
				++iLnk;
			}
		}
		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		out << "display();\n";
		out << "</script>\n";
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}

bool lessSort(::TianShanIce::Transport::StreamLinkPrx& lnk1, ::TianShanIce::Transport::StreamLinkPrx& lnk2)
{
	if (lnk1->getServiceGroupId() < lnk2->getServiceGroupId())
		return true;
	if (lnk1->getServiceGroupId() > lnk2->getServiceGroupId())
		return false;

	//same group id
	::TianShanIce::ValueMap private1 = lnk1->getPrivateData();
	::TianShanIce::ValueMap private2 = lnk2->getPrivateData();
	::TianShanIce::ValueMap::iterator iterQamIP1 = private1.find("Qam.IP");
	::TianShanIce::ValueMap::iterator iterQamIP2 = private2.find("Qam.IP");
	if (iterQamIP1 == private1.end())
		return false;
	if (iterQamIP2 == private2.end())
		return true;
	if (iterQamIP1->second.strs < iterQamIP2->second.strs)
		return true;
	if (iterQamIP1->second.strs > iterQamIP2->second.strs)
		return false;

	//same qamIp
	::TianShanIce::ValueMap::iterator iterQamReq1 = private1.find("Qam.frequency");
	::TianShanIce::ValueMap::iterator iterQamReq2 = private2.find("Qam.frequency");
	if (iterQamReq1 == private1.end())
		return false;
	if (iterQamReq2 == private2.end())
		return true;
	if (iterQamReq1->second.ints < iterQamReq2->second.ints)
		return true;
	else
		return false;
}

// this function work together with AdminCtrl_StreamLink.html
bool PathController::StreamLinkPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request stream link page."));
	if (!init())
	{
		return false;
	}

	try{
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request StreamLinkPage with action [%s]"), action);
			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_LINK) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad action type.[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
			if (strspn(action, "ed") == nAction)
			{
				////////////////////////////add disable and enable actions
				if (strchr(action, PATHCTRL_ACTION_DISABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						//find stream link proxy
						if (lnkIdent.length() == 0)
							break;
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (1 == lnkEx->status())
							continue;
						lnkEx->enableLink(false);
					}
				}
				/*
				if(strchr(action,PATHCTRL_ACTION_ENABLE))
				{
				char ident[100] = "";
				int i = 0;
				while(1)
				{
				memset(ident,0,sizeof(ident));
				sprintf(ident,"%s[%d]",PATHCTRL_VAR_LINKIDENTS,i);
				i++;
				const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(ident);
				if(NULL == lnkIdent)
				break;
				//find stream link proxy
				std::string lnkProxyStr = std::string(lnkIdent) + ":" + _endpoint;
				Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkProxyStr));
				//continue if link status is same as before,else enable the link
				if(1 == lnkEx->status())
				continue;
				lnkEx->enableLink(true);
				}
				}
				*/
				if (strchr(action, PATHCTRL_ACTION_ENABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						if (lnkIdent.length() == 0)
							break;
						//find stream link proxy
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (0 == lnkEx->status())
							continue;
						lnkEx->enableLink(true);
					}
				}
			}
			else{
				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* sgIdStr = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_SGID);
				if (NULL == sgIdStr)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "service group id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				::Ice::Int sgId = atoi(sgIdStr);
				const char* lnkType = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKTYPE);
				if (NULL == lnkType)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's type is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				if (strchr(action, PATHCTRL_ACTION_UNLINK))
				{
					// unlink streamer
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find stream link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));
					lnkEx->destroy();
				}

				if (strchr(action, PATHCTRL_ACTION_SETPRIVATE))
				{
					// set link private data
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					ValueMap privateData;
					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find stream link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));

					if (lnkEx->updatePrivateData(privateData))
					{
						glog(Log::L_INFO, CLOGFMT(PathCtrl, "Update private data successfully."));
					}
					else
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "Failed to update private data."));
						_pHttpRequestCtx->Response().SetLastError("Failed to update private data.");
						return false;
					}
				}

				if (strchr(action, PATHCTRL_ACTION_LINK))
				{
					// link streamer
					ValueMap privateData;

					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					_pa->linkStreamer(sgId, stmrNetId, lnkType, privateData);
				}
			} // end if(action)
		}

		IHttpResponse &out = _pHttpRequestCtx->Response();
		int lnksCount = 0;
		int strmrCount = 0;
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_StreamLink.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		//////////////////////////////
		//std::cout<<"start output data"<<std::endl;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(StreamLinkPage, "start output data..."));
		//////////////////////////////
		out << "<script type='text/javascript'>\n";
		// list all link types
		StrValues types = _pa->listSupportedStreamLinkTypes();
		int64 stampNow = ZQ::common::TimeUtil::now();
		for (StrValues::const_iterator cit_type = types.begin(); cit_type != types.end(); ++cit_type)
		{
			//////////////////////////////////////////////////////
			out << "_types.push(new TSLinkType(\"" << (*cit_type) << "\"";
			TianShanIce::PDSchema schema = _pa->getStreamLinkSchema(*cit_type);
			for (TianShanIce::PDSchema::const_iterator cit_schema = schema.begin(); cit_schema != schema.end(); ++cit_schema)
			{
				//out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional);
				out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional2);
				out << ", \"";
				out << AdminCtrlUtil::vartype(cit_schema->defaultvalue) << AdminCtrlUtil::var2str(cit_schema->defaultvalue);
				out << "\")";
			}
			out << "));\n";
		}

		///////////////
		//std::cout<<"list all link type spend:"<<ZQ::common::TimeUtil::now() - stampNow<<"ms"<<std::endl;
		///////////////

		// list all service group
		Transport::ServiceGroups sgs = _pa->listServiceGroups();
		for (Transport::ServiceGroups::const_iterator cit_sg = sgs.begin(); cit_sg != sgs.end(); ++cit_sg)
		{
			out << "_sgs.push(" << cit_sg->id << ");\n";
		}

		/////////////
		//std::cout<<"list all service group spend:"<<ZQ::common::TimeUtil::now()-stampNow<<"ms"<<std::endl;
		/////////////

		// list all streamers
		int iLnk = 0;
		Transport::Streamers strmrs = _pa->listStreamers();
		strmrCount += strmrs.size();
		int numDelay = 0;
		int num30 = 0, num100 = 0, num200 = 0;
		for (Transport::Streamers::const_iterator cit_strmr = strmrs.begin(); cit_strmr != strmrs.end(); ++cit_strmr)
		{
			out << "_stmrs.push(\"" << cit_strmr->netId << "\");\n";

			// list all links of this streamer
			int64 stampTemp = ZQ::common::TimeUtil::now();
			Transport::StreamLinks lnks = _pa->listStreamLinksByStreamer(cit_strmr->netId);
			lnksCount += lnks.size();
			//std::sort(lnks.begin(),lnks.end(),lessSort);
			stampTemp = ZQ::common::TimeUtil::now();
			for (Transport::StreamLinks::const_iterator cit_lnk = lnks.begin(); cit_lnk != lnks.end(); ++cit_lnk)
			{
				int status = 0;//0 if disable,1 if enable
				int64 stampGetid = ZQ::common::TimeUtil::now();
				std::string ident = _ic->identityToString((*cit_lnk)->getIdent());
				int64 getidSpend = ZQ::common::TimeUtil::now() - stampGetid;
				if (getidSpend > 0)
					numDelay++;
				if (getidSpend > 30)
					num30++;
				else if (getidSpend > 100)
					num100++;
				else if (getidSpend > 200)
					num200++;
				std::string lnkPrx = ident + ":" + _endpoint;

				Transport::StreamLinkExPrx lnkEx;
				try
				{
					lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkPrx));
				}
				catch (::Ice::Exception& e){}

				status = lnkEx->status();
				if (status & 1)//bit0 - 1 if disable
					status = 0;
				else
					status = 1;

				out << "_lnks[" << iLnk << "]=new TSStreamLink(\""
					<< ident << "\","
					<< (*cit_lnk)->getServiceGroupId() << ",\""
					<< (*cit_lnk)->getStreamerId() << "\",\""
					<< (*cit_lnk)->getType() << "\","
					<< status << ");\n";
				ValueMap pds = (*cit_lnk)->getPrivateData();
				for (ValueMap::const_iterator cit_pd = pds.begin(); cit_pd != pds.end(); ++cit_pd)
				{
					out << "_lnks[" << iLnk << "].privateData[\"" << cit_pd->first << "\"]=\"";
					out << AdminCtrlUtil::vartype(cit_pd->second) << AdminCtrlUtil::var2str(cit_pd->second);
					out << "\";\n";
				}

				++iLnk;
				//printf("output link [%s] spend %lld\n",ident.c_str(),ZQ::common::TimeUtil::now()-stampTemp);
				//stampTemp = ZQ::common::TimeUtil::now();
			}

			int64 stamp = ZQ::common::TimeUtil::now() - stampTemp;
			//std::cout<<"list "<<lnksCount<<" links spend "<<stamp<<" ms"<<std::endl;
		}
		//////////////
		/*
		std::cout<<numDelay<<" links take ice call delay"<<std::endl;
		std::cout<<num30<<" links delay more than 30ms\n"
		<<num100<<" links delay more than 100ms\n"
		<<num200<<" links delay more than 200ms"
		<<std::endl;
		std::cout<<"list all streamers spend:"<<ZQ::common::TimeUtil::now()-stampNow<<"ms"<<std::endl;
		*/
		//////////////

		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		out << "display();\n";
		out << "</script>\n";
		int64 stampSpend = ZQ::common::TimeUtil::now() - stampNow;

		//std::cout<<"output data in javaScript spend "<<stampSpend<<"ms"<<std::endl;
		//std::cout<<"output "<<strmrCount<<" streamers and "<<lnksCount<<" links"<<std::endl;
		glog(Log::L_INFO, CLOGFMT(StreamLinkPage, "output %d links in javaScript spend %lldms"), lnksCount, stampSpend);
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}

bool PathController::StreamLinkBySGIdPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request stream link By SG page."));
	if (!init())
	{
		return false;
	}

	Ice::Int serviceGroupId = -1;

	try{
		const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
		if (action)
		{
			glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request StreamLinkBySGPage with action [%s]"), action);
			size_t nAction = strlen(action);
			// validity check
			if ((0 == nAction) || (strspn(action, PATHCTRL_ACTIONSET_LINK) != nAction))
			{
				// bad action type
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "bad action type.[%s]"), action);
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
			if (strspn(action, "eds") == nAction)
			{
				const char* sgIdStr = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_SERVICEGROUPID);
				if( NULL != sgIdStr)
				{
					serviceGroupId = atoi(sgIdStr);
				}
				//disable action
				if (strchr(action, PATHCTRL_ACTION_DISABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						//find stream link proxy
						if (lnkIdent.length() == 0)
							break;
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (1 == lnkEx->status())
							continue;
						lnkEx->enableLink(false);
					}
				}
				if (strchr(action, PATHCTRL_ACTION_ENABLE))
				{
					char idents[1024] = "";
					memset(idents, 0, sizeof(idents));
					sprintf(idents, "%s", PATHCTRL_VAR_LINKIDENTS);
					const char* lnkIdents = _pHttpRequestCtx->GetRequestVar(idents);
					if (NULL == lnkIdents)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream links are missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					std::string identsStr(lnkIdents);
					int startPos = 0;
					int endPos = 0;
					while (startPos <= identsStr.length())
					{
						endPos = identsStr.find(',', startPos + 1);
						if (endPos == std::string::npos)
							endPos = identsStr.length();
						std::string lnkIdent = identsStr.substr(startPos, endPos - startPos);
						if (lnkIdent.length() == 0)
							break;
						//find stream link proxy
						std::string lnkProxyStr = lnkIdent + ":" + _endpoint;
						Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkProxyStr));
						startPos = endPos + 1;
						//continue if link status is same as before,else enable the link
						if (0 == lnkEx->status())
							continue;
						lnkEx->enableLink(true);
					}
				}
				//search action
				if(strchr(action,PATHCTRL_ACTION_SEARCH))
				{
					glog(Log::L_NOTICE, CLOGFMT(PathCtrl, "this is the search action."));
				}
			}
			else{
				const char* stmrNetId = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_STREAMER_NETID);
				if (NULL == stmrNetId)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "streamer's net id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				const char* sgIdStr = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_SGID);
				if (NULL == sgIdStr)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "service group id is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}
				::Ice::Int sgId = atoi(sgIdStr);
				serviceGroupId = sgId;
				const char* lnkType = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKTYPE);
				if (NULL == lnkType)
				{
					glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's type is missing in the request."));
					PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
				}

				if (strchr(action, PATHCTRL_ACTION_UNLINK))
				{
					// unlink streamer
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find stream link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));
					lnkEx->destroy();
				}

				if (strchr(action, PATHCTRL_ACTION_SETPRIVATE))
				{
					// set link private data
					const char* lnkIdent = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_LINKIDENT);
					if (NULL == lnkIdent)
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "stream link's ident is missing in the request."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					ValueMap privateData;
					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					// find stream link proxy
					std::string lnkPrxstr = std::string(lnkIdent) + ":" + _endpoint;
					Transport::StreamLinkExPrx lnkEx = Transport::StreamLinkExPrx::checkedCast(_ic->stringToProxy(lnkPrxstr));

					if (lnkEx->updatePrivateData(privateData))
					{
						glog(Log::L_INFO, CLOGFMT(PathCtrl, "Update private data successfully."));
					}
					else
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "Failed to update private data."));
						_pHttpRequestCtx->Response().SetLastError("Failed to update private data.");
						return false;
					}
				}

				if (strchr(action, PATHCTRL_ACTION_LINK))
				{
					// link streamer
					ValueMap privateData;

					// extract private data
					if (!extractPrivateData(PATHCTRL_VARPREFIX_PD, _pHttpRequestCtx, privateData))
					{
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to extract private data."));
						PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
					}
					_pa->linkStreamer(sgId, stmrNetId, lnkType, privateData);
				}
			} // end if(action)
		}//end if(try)

		IHttpResponse &out = _pHttpRequestCtx->Response();
		int lnksCount = 0;
		int strmrCount = 0;
		{
			// import display code
			std::string dispFile = std::string(_rootDir) + "AdminCtrl_StreamLinkBySGId.html";
			if (!AdminCtrlUtil::importFile(out, dispFile.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), dispFile.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		//////////////////////////////
		//std::cout<<"start output data"<<std::endl;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(StreamLinkBySGPage, "start output data..."));
		//////////////////////////////
		out << "<script type='text/javascript'>\n";
		// list all link types
		StrValues types = _pa->listSupportedStreamLinkTypes();
		int64 stampNow = ZQ::common::TimeUtil::now();
		for (StrValues::const_iterator cit_type = types.begin(); cit_type != types.end(); ++cit_type)
		{
			//////////////////////////////////////////////////////
			out << "_types.push(new TSLinkType(\"" << (*cit_type) << "\"";
			TianShanIce::PDSchema schema = _pa->getStreamLinkSchema(*cit_type);
			for (TianShanIce::PDSchema::const_iterator cit_schema = schema.begin(); cit_schema != schema.end(); ++cit_schema)
			{
				//out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional);
				out << ", new TSPDElement(\"" << cit_schema->keyname << "\", " << AdminCtrlUtil::bool2str(cit_schema->optional2);
				out << ", \"";
				out << AdminCtrlUtil::vartype(cit_schema->defaultvalue) << AdminCtrlUtil::var2str(cit_schema->defaultvalue);
				out << "\")";
			}
			out << "));\n";
		}

		///////////////
		//std::cout<<"list all link type spend:"<<ZQ::common::TimeUtil::now() - stampNow<<"ms"<<std::endl;
		///////////////
	
		// list all service group
		Transport::ServiceGroups sgs = _pa->listServiceGroups();
		for (Transport::ServiceGroups::const_iterator cit_sg = sgs.begin(); cit_sg != sgs.end(); ++cit_sg)
		{
			out << "_sgs.push(" << cit_sg->id << ");\n";
		}

		/////////////
		//std::cout<<"list all service group spend:"<<ZQ::common::TimeUtil::now()-stampNow<<"ms"<<std::endl;
		/////////////

		// list all streamers
		int iLnk = 0;
		Transport::Streamers strmrs = _pa->listStreamers();
	//	strmrCount += strmrs.size();
		int numDelay = 0;
		int num30 = 0, num100 = 0, num200 = 0;
		for (Transport::Streamers::const_iterator cit_strmr = strmrs.begin(); cit_strmr != strmrs.end(); ++cit_strmr)
		{
			out << "_stmrs.push(\"" << cit_strmr->netId << "\");\n";
		}
			// list all links of this streamer
			int64 stampTemp = ZQ::common::TimeUtil::now();
	//		const char* sgIdStr1 = _pHttpRequestCtx->GetRequestVar(PATHCTRL_VAR_SGID);

			if(serviceGroupId < 0)
			{
				const char* sgIdStr = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_SERVICEGROUPID);
				if( NULL != sgIdStr)
				{
					serviceGroupId = atoi(sgIdStr);
				}
				else if(!sgs.empty())
				{
					//glog(Log::L_WARNING, CLOGFMT(PathCtrl, "ServiceGroupPage() service group id is missed in the request."));
					serviceGroupId = sgs[0].id;
				}
			}

			Transport::StreamLinks lnks;
			if(serviceGroupId > 0)
				lnks = _pa->listStreamLinksByServiceGroup(serviceGroupId);

			lnksCount += lnks.size();
			//std::sort(lnks.begin(),lnks.end(),lessSort);
			stampTemp = ZQ::common::TimeUtil::now();
			for (Transport::StreamLinks::const_iterator cit_lnk = lnks.begin(); cit_lnk != lnks.end(); ++cit_lnk)
			{
				int status = 0;//0 if disable,1 if enable
				int64 stampGetid = ZQ::common::TimeUtil::now();
				std::string ident = _ic->identityToString((*cit_lnk)->getIdent());
				int64 getidSpend = ZQ::common::TimeUtil::now() - stampGetid;
				if (getidSpend > 0)
					numDelay++;
				if (getidSpend > 30)
					num30++;
				else if (getidSpend > 100)
					num100++;
				else if (getidSpend > 200)
					num200++;
				std::string lnkPrx = ident + ":" + _endpoint;

				Transport::StreamLinkExPrx lnkEx;
				try
				{
					lnkEx = Transport::StreamLinkExPrx::uncheckedCast(_ic->stringToProxy(lnkPrx));
				}
				catch (::Ice::Exception& e){}

				status = lnkEx->status();
				if (status & 1)//bit0 - 1 if disable
					status = 0;
				else
					status = 1;

				out << "_lnks[" << iLnk << "]=new TSStreamLinkBySGId(\""
					<< ident << "\","
					<< (*cit_lnk)->getServiceGroupId() << ",\""
					<< (*cit_lnk)->getStreamerId() << "\",\""
					<< (*cit_lnk)->getType() << "\","
					<< status << ");\n";
				ValueMap pds = (*cit_lnk)->getPrivateData();
				for (ValueMap::const_iterator cit_pd = pds.begin(); cit_pd != pds.end(); ++cit_pd)
				{
					out << "_lnks[" << iLnk << "].privateData[\"" << cit_pd->first << "\"]=\"";
					out << AdminCtrlUtil::vartype(cit_pd->second) << AdminCtrlUtil::var2str(cit_pd->second);
					out << "\";\n";
				}

				++iLnk;
				//printf("output link [%s] spend %lld\n",ident.c_str(),ZQ::common::TimeUtil::now()-stampTemp);
				//stampTemp = ZQ::common::TimeUtil::now();
			}

			int64 stamp = ZQ::common::TimeUtil::now() - stampTemp;
			//std::cout<<"list "<<lnksCount<<" links spend "<<stamp<<" ms"<<std::endl;
	//	}
		//////////////
		/*
		std::cout<<numDelay<<" links take ice call delay"<<std::endl;
		std::cout<<num30<<" links delay more than 30ms\n"
		<<num100<<" links delay more than 100ms\n"
		<<num200<<" links delay more than 200ms"
		<<std::endl;
		std::cout<<"list all streamers spend:"<<ZQ::common::TimeUtil::now()-stampNow<<"ms"<<std::endl;
		*/
		//////////////

		out << "_template=\"" << _template << "\";\n";
		out << "_endpoint=\"" << _endpoint << "\";\n";
		out << "display();\n";
		out << "</script>\n";
		int64 stampSpend = ZQ::common::TimeUtil::now() - stampNow;

		//std::cout<<"output data in javaScript spend "<<stampSpend<<"ms"<<std::endl;
		//std::cout<<"output "<<strmrCount<<" streamers and "<<lnksCount<<" links"<<std::endl;
		glog(Log::L_INFO, CLOGFMT(StreamLinkPage, "output %d links in javaScript spend %lldms"), lnksCount, stampSpend);
	}
	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}


bool PathController::TransportMapPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request transport map page."));
	if (!init())
		return false;

	try
	{
		IHttpResponse &out = _pHttpRequestCtx->Response();
		// the image view
		std::string dotfilepath = std::string(_rootDir) + "transport.dot";
		std::string storageGraph, streamGraph, sgGraph, storageLinksGraph, streamLinksGraph;
		_pa->dumpDot(ValueMap(), storageGraph, streamGraph, sgGraph, storageLinksGraph, streamLinksGraph);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportMapPage(): successful to get transport data."));

		{
			// generate dot file
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportMapPage(): generage dot file : %s"), dotfilepath.c_str());
			std::ofstream dotfile(dotfilepath.c_str());
			dotfile << "digraph G {\n size=\"6,100\";\nnode[style=filled];\nrankdir=LR;\ncolor=white;\n";

			dotfile << "   subgraph clusterStorages {\nnode[shape=octagon,color=green];\n" << storageGraph << "\n}\n";
			dotfile << "   subgraph clusterStreamers {\nnode[shape=record,fillcolor=red];\n" << streamGraph << "\n}\n";
			dotfile << "   subgraph clusterServicegroups {\nnode[shape=house,color=blue];\n" << sgGraph << "\n}\n";
			dotfile << "   subgraph storagelinks {\nedge[style=dashed,color=red,minlen=2];\n" << storageLinksGraph << "\n}\n";
			dotfile << "   subgraph streamlinks {\nedge[style=solid,minlen=2];\n" << streamLinksGraph << "\n}\n";
			dotfile << "}\n";

		}

		std::string pngfilepath = std::string(_rootDir) + "transport.png";
		std::string mapfilepath = std::string(_rootDir) + "transport.map";
		{
			// generate map files
			std::string cmd = std::string("dot -Tpng \"") + dotfilepath + "\" -o\"" + pngfilepath + "\"";
			if (0 != ::system(cmd.c_str()))
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(PathCtrl, "error occured when execute command: %s"), cmd.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
			cmd = std::string("dot -Tcmap \"") + dotfilepath + "\" -o\"" + mapfilepath + "\"";
			if (0 != ::system(cmd.c_str()))
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(PathCtrl, "error occured when execute command: %s"), cmd.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
		}

		{
			// display image
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathCtrl, "importing map content: %s"), mapfilepath.c_str());
			out << "<IMG SRC='transport.png' usemap='#transport__map' border=0><map name='transport__map'>";
			if (!AdminCtrlUtil::importFile(out, mapfilepath.c_str()))
			{
				glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to import file [%s]."), mapfilepath.c_str());
				PATHCTRL_REPORT_BAD_LOGIC_OR_CONFIG;
			}
			out << "\n</map>";
		}
	}

	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}

static bool exportFile(const char* data, const char* to)
{
	if (NULL == data || NULL == to)
		return false;

	std::ofstream fl(to);
	if (!fl.good())
	{
		glog(Log::L_ERROR, CLOGFMT(PathCtrl, "failed to open file [%s]."), to);
		return false;
	}

	fl.write(data, strlen(data));
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "export to file [%s] successfully."), to);
	return true;
}

// convert the local file path to the network path
// d:\aa\ccc  -->  \\xxx.xxx.x.xx\d$\aa\ccc
// for windows only
static std::string localPathToNetworkPath(const std::string& host, const std::string& path)
{
#ifndef ZQ_OS_MSWIN
	// bug#19016 only allow local Sentry to import/export Transport XML
	return path;
#else
	std::string netPath;
	netPath.reserve(host.size() + path.size() + 3);
	netPath = std::string("\\\\") + host + "\\";
	// replace the ':' in the path to '$'
	for (size_t i = 0; i < path.size(); ++i)
	{
		if (path[i] == ':')
			netPath.push_back('$');
		else
			netPath.push_back(path[i]);
	}

	return netPath;
#endif // ZQ_OS_MSWIN
}

bool PathController::TransportConfPage()
{
	glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "request transport configuration page."));
	if (!init())
		return false;

	try
	{ // download the current transport configuration
		const char* download = _pHttpRequestCtx->GetRequestVar("dl");
		if (download)
		{
			// check the file type to download
#pragma message(__MSGLOC__"TODO: support download as compressed file")
			IHttpResponse &out = _pHttpRequestCtx->Response();
			//out << _pa->dumpXml();
			// new interface

			// get the network dir first
			std::string host = ZQ::common::URLStr(_pHttpRequestCtx->GetRootURL(), true).getHost();
			std::string tempTransportConfig = localPathToNetworkPath(host, _rootDir) + "transport.conf.o." + AdminCtrlUtil::long2str(ZQTianShan::now());

			_pa->dumpToXmlFile(tempTransportConfig);

			// set the necessary meta data
			out.setContentType("text/xml");

			if (0 == strcmp(download, "v")) // with viewing info
			{
				out << "<?xml-stylesheet type=\"text/xsl\" href=\"transport.xslt\"?>";
			}
			else if (0 == strcmp(download, "v2")) // viewing sort by streamer
			{
				out << "<?xml-stylesheet type=\"text/xsl\" href=\"transport2.xslt\"?>";
			}
			else
			{
				// download only
				out.setHeader("Content-Disposition", "attachment; filename=transport.conf.xml");
			}

			AdminCtrlUtil::importFile(out, tempTransportConfig.c_str());

			// delete the temp file
			if (0 == remove(tempTransportConfig.c_str())) // successful
			{
				glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() Removed the temporary file %s."), tempTransportConfig.c_str());
			}
			else
			{
#ifdef ZQ_OS_MSWIN
				DWORD err = ::GetLastError();
				glog(Log::L_WARNING, CLOGFMT(PathCtrl, "TransportConfPage() Failed to delete the temporary file %s, error code=%d."), tempTransportConfig.c_str(), err);
#else
				glog(Log::L_WARNING, CLOGFMT(PathCtrl, "TransportConfPage() Failed to delete the temporary file %s, error code=%d."), tempTransportConfig.c_str(), errno);
#endif           
			}

			return true;
		} // else continue the other 
	}
	PATHCTRL_CATCH_COMMON_CASE;

	try
	{
		bool bPathChanged = false;
		IHttpResponse &out = _pHttpRequestCtx->Response();
		const char* config = _pHttpRequestCtx->GetRequestVar("config");
		if (config && '\0' != *config)
		{
			// get the network dir first
			std::string host = ZQ::common::URLStr(_pHttpRequestCtx->GetRootURL(), true).getHost();
			// save the config as xml file
			std::string tempTransportConfig = localPathToNetworkPath(host, _rootDir) + "transport.conf.i." + AdminCtrlUtil::long2str(ZQTianShan::now());
			if (!exportFile(config, tempTransportConfig.c_str()))
			{ // failed to save the config as file
				out << "<div class='message error'>Error occur during the importing.</div><hr>";
			}
			else
			{
				glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() saved the posted configuration as %s."), tempTransportConfig.c_str());
				::TianShanIce::Transport::PathAdminPrx pathAdm;

				try {
					std::string prxStr = _ic->proxyToString(_pa);
					size_t pos = prxStr.find("-t ");
					if (std::string::npos != pos)
					{
						size_t epos = prxStr.find_first_not_of(" \t\r\n", pos +sizeof("-t ")-1);
						epos = prxStr.find_first_of(" \t\r\n", epos);
						if (std::string::npos != epos)
							prxStr.erase(pos, epos-pos);
						else prxStr.erase(pos);
					}

					prxStr += " -t 60000";
					pathAdm = Transport::PathAdminPrx::checkedCast(_ic->stringToProxy(prxStr));
					glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() taking ep[%s] to importXML"), prxStr.c_str());
				}
				catch (const ::Ice::UserException& e)
				{
				}
				catch (...)
				{
				}

				if (!pathAdm)
					pathAdm = _pa;

				try
				{
					/*
					// inject the operation to the conf
					const char* op = _pHttpRequestCtx->GetRequestVar("op");
					if(NULL == op || '\0' == *op)
					{
					op = "modify";
					}
					AdminCtrlUtil::StringVector opList;
					opList.push_back(op);
					AdminCtrlUtil::updateXML(tempTransportConfig, "TianShanTransport/Operation", opList);
					*/
					// never clean the data
					pathAdm->importXml(tempTransportConfig, false);
					glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() Update the transport configuration successfully"));
					// update the template file
					bPathChanged = true;

					// display success message
					out << "<div class='message info'>Update the transport configuration successfully.</div><hr>";
				}
				catch (TianShanIce::InvalidParameter& e)
				{ // display failure message
					out << "<div class='message error'>Invalid configuraton. Detail: " << e.message.c_str() << "</div><hr>";
				}
				catch (TianShanIce::ServerError& e)
				{ // display failure message
					out << "<div class='message error'>Error occur during the importing. Detail: " << e.message.c_str() << "</div><hr>";
				}
			}

			// delete the temp file
			if (0 == remove(tempTransportConfig.c_str())) // successful
			{
				glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() Removed the temporary file %s."), tempTransportConfig.c_str());
			}
			else
			{
#ifdef ZQ_OS_MSWIN
				DWORD err = ::GetLastError();
				glog(Log::L_WARNING, CLOGFMT(PathCtrl, "TransportConfPage() Failed to delete the temporary file %s, error code=%d."), tempTransportConfig.c_str(), err);
#else
				glog(Log::L_WARNING, CLOGFMT(PathCtrl, "TransportConfPage() Failed to delete the temporary file %s, error code=%d."), tempTransportConfig.c_str(), errno);
#endif           
			}
		}

		if (bPathChanged)
		{ // the service group may changed or not,
			// need update the serverload template file
			const char* srvrloadFilePath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
			if (srvrloadFilePath && (*srvrloadFilePath) != '\0')
			{
				// construct the content to be updated
				AdminCtrlUtil::StringVector content;
				Transport::ServiceGroups sgs = _pa->listServiceGroups();

				content.reserve(sgs.size());
				for (size_t iSG = 0; iSG < sgs.size(); ++iSG)
					content.push_back(AdminCtrlUtil::int2str(sgs[iSG].id));

				int nTried = 0;
				do
				{
					++nTried;
					if (AdminCtrlUtil::updateXML(srvrloadFilePath, "SrvrList/CMGroup/NodeGroup", content))
					{
						glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() Updated server load template file [%s] successfully."), srvrloadFilePath);
						break;
					}
					else if (nTried < 2)
					{ // retry
						glog(Log::L_WARNING, CLOGFMT(PathCtrl, "TransportConfPage() Failed to update server load template file [%s]. Retry after 500 msec"), srvrloadFilePath);
						// Sleep(500);
						::SYS::sleep(500);
						continue;
					}
					else
					{ // failed
						glog(Log::L_ERROR, CLOGFMT(PathCtrl, "TransportConfPage() Failed to update server load template file [%s] after 2 tries."), srvrloadFilePath);
						break;
					}
				} while (true);
			}
			else
			{
				glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() no server load template file path provide."));
			}
		}

		// display the page
#ifndef ZQ_OS_MSWIN
		// bug#19016 only allow local Sentry to import/export Transport XML
		const char* envLocalWeiwoo = ::getenv("sentry_localWeiwoo");
		glog(Log::L_DEBUG, CLOGFMT(PathCtrl, "TransportConfPage() getenv() sentry_localWeiwoo=%s"), envLocalWeiwoo ? envLocalWeiwoo:"null");
		if (NULL == envLocalWeiwoo || atoi(envLocalWeiwoo) <= 0)
		{
			out << "<div>The Transport Advanced Configuration Page is only accessible by the Sentry instance on the same machine where Weiwoo service is running on</div>";
			out << "<div>Please enter the correct Sentry URL on your web browser</div>";
			return true;
		}
#endif // ZQ_OS_MSWIN

		out << "<div>In addition to the previous pages that can configure Transport entities and links. It is allowed to backup and import the full configurations via files.</div>";
#pragma message(__MSGLOC__"TODO: Backup the transport configuration as log data.")
		{
			// generate the download url
			ZQ::common::URLStr dlUrl(_pHttpRequestCtx->GetRootURL(), true); // case sensitive
			dlUrl.setPath("TransportConfPage.ac.tswl");
			dlUrl.setVar(ADMINCTRL_VAR_ENDPOINT, _endpoint);
			dlUrl.setVar("dl", "");
			out << "<div>Please click to ";
			out << "<a href='" << dlUrl.generate() << "'><input type='button' value='export' onclick='this.parentNode.click()'></a>";
			out << " the current Transport settings, or view them online ";
			dlUrl.setVar("dl", "v");
			out << "<a href='" << dlUrl.generate() << "' target=_blank><input type='button' value='by ServiceGroup' onclick='this.parentNode.click()' style='width:9em'></a>";
			dlUrl.setVar("dl", "v2");
			out << " <a href='" << dlUrl.generate() << "' target=_blank><input type='button' value='by Streamer' onclick='this.parentNode.click()' style='width:7em'></a>";
			out << ".</div>\n";
		}

		out << "<br>The Transport settings can be changed by importing a file:\n"
			<< "<ul>\n"
			<< "<li>If the matched entities/links have existed already, their attributes will be modified to the new values in the imported file.</li>\n"
			<< "<li>If no matched entities(ServiceGroup, Storage or Streamer) exist, new entities would be added.</li>\n"
			<< "<li>If empty linkIds are given, new links would be added with new unique IDs assigned by the system automatically.</li>\n"
			<< "<li>If some existing entities/links are not present in the imported file, the system would cleanup them.</li>\n"
			<< "</ul>\n"
			<< "It is recommended to always make a backup and edit based on the current settings that can be exported above.\n"
			<< "<div><br>To import the new Transport settings, please locate the file and click button \"import\"</div>\n";

		out << "<form method='post' action='" << _pHttpRequestCtx->GetRootURL()
			<< "TransportConfPage.ac.tswl' enctype='multipart/form-data'>\n"
			<< "<input type='file' name='config'>"
			<< "<input type='submit' value='import'>\n";

		out << "<input type='hidden' name='" ADMINCTRL_VAR_TEMPLATE "' value='" << _template << "'>";
		out << "<input type='hidden' name='" ADMINCTRL_VAR_ENDPOINT "' value='" << _endpoint << "'>";
		{
			const char* srvrloadFilePath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
			if (srvrloadFilePath && (*srvrloadFilePath) != '\0')
			{
				out << "<input type='hidden' name='srvrloadpath' value='" << srvrloadFilePath << "'>";
			}
		}
		out << "</form>\n";

	}
	PATHCTRL_CATCH_COMMON_CASE;

	return true;
}
