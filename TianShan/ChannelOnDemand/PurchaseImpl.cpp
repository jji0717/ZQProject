// PurchaseImpl.cpp: implementation of the PurchaseImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PurchaseImpl.h"
#include "PurchaseItemImpl.h"
#include <functional>
#include <algorithm>
#include "stroprt.h"
#include "CODConfig.h"
#include "IceAsyncSub.h"
#include "PurchaseRequest.h"
#include <boost/regex.hpp>
#include "SiteDefines.h"


#define LOG_MODULE_NAME			"Purchase"

#define PurchaseLog(_C, _X) CLOGFMT(_C, "[%s] " _X), ident.name.c_str()

#define ClientRequestPrefix "ClientRequest#"

bool localTime2TianShanTimeB(const char* szTime, __int64& lTime)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	char cT;
	if (sscanf(szTime, "%4d%2d%2d%c%2d%2d%2d", &nYear, &nMon, &nDay, &cT, &nHour, &nMin, &nSec)<6)
		return false;
	
	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;
	
	// convert to system time
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = nYear;
	st.wMonth = nMon;
	st.wDay = nDay;
	st.wHour = nHour;
	st.wMinute = nMin;
	st.wSecond = nSec;
	
	FILETIME ft_local, ft_utc;
	SystemTimeToFileTime(&st, &ft_local);
	LocalFileTimeToFileTime(&ft_local, &ft_utc);
	
	memcpy(&lTime, &ft_utc, sizeof(lTime));
	lTime = lTime / 10000;  

	return true;
}

bool systemTime2TianShanTimeB(const char* szTime, __int64& lTime)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	char cT;
	if (sscanf(szTime, "%4d%2d%2d%c%2d%2d%2d", &nYear, &nMon, &nDay, &cT, &nHour, &nMin, &nSec)<6)
		return false;
	
	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;
	
	// convert to system time
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = nYear;
	st.wMonth = nMon;
	st.wDay = nDay;
	st.wHour = nHour;
	st.wMinute = nMin;
	st.wSecond = nSec;
	
	FILETIME ft_utc;
	SystemTimeToFileTime(&st, &ft_utc);
	
	memcpy(&lTime, &ft_utc, sizeof(lTime));
	lTime = lTime / 10000;

	return true;
}

namespace ZQChannelOnDemand {

//////////////////////////////////////////////////////////////////////////
// constructor and destructor
//////////////////////////////////////////////////////////////////////////

PurchaseImpl::PurchaseImpl(ChODSvcEnv& env)
	: _env(env)
{
	weiwoo = NULL;
	playlist = NULL;
	bAuthorize = true;
	bNeedSyncChannel = false;
	nNodeGroupId = 0;
	nCreateTime = 0;
	properties.clear();
}

PurchaseImpl::~PurchaseImpl()
{
}

//////////////////////////////////////////////////////////////////////////
// implement the functions defined in TianShanIce::Application::Purchase
//////////////////////////////////////////////////////////////////////////

// available error code [100, 200)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
::TianShanIce::SRM::SessionPrx PurchaseImpl::getSession(const ::Ice::Current&) const
{
    Lock lock(*this);
	return weiwoo;
}

// available error code [200, 300)
// throw exception list NotSupported, InvalidParameter, ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::provision(const ::Ice::Current&)
{
	Lock lock(*this);
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "purchase provision() enter"));
}

// available error code [300, 400)
// throw exception list NotSupported, InvalidParameter, ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::render(const ::TianShanIce::Streamer::StreamPrx& sPrx, const ::TianShanIce::SRM::SessionPrx& sessionPrx, const ::Ice::Current&)
{
    Lock lock(*this);
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase render() enter"));

	// get playlist proxy
	try
	{
		playlist = ::TianShanIce::Streamer::PlaylistPrx::checkedCast(sPrx);
		playlistId = playlist->getId();
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 300, PurchaseLog("Purchase", "get playlist caught %s"), ex.ice_name().c_str());
	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase's associated playlist [%s]"), playlistId.c_str());

	// get channel publish point
	// becuase open function can throw TianShanIce::InvalidParameter, TianShanIce::ServerError exception and
	// both of them are in the exception list of render function
	//::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx  publishPointPrx = NULL;
	::TianShanIce::Application::PublishPointPrx  publishPointPrx = NULL;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx)  chlPublishPointPrx = NULL;
	try 
	{
#ifdef USE_OLD_NS
		chlPublishPointPrx = _env._publisher->open(chlPubName);
#else
		publishPointPrx = _env._publisher->open(chlPubName);
		chlPublishPointPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(publishPointPrx);
#endif //USE_OLD_NS	
	}
	catch( const TianShanIce::InvalidParameter& ex)
	{
		ex.ice_throw(); // forword throw
	}
	catch (const TianShanIce::ServerError& ex)
	{
		ex.ice_throw(); // forword throw
	}
	catch (...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 301, PurchaseLog("Purchase", "open channel [%s] caught unexpect exception"), chlPubName.c_str());
	}

	// get channel items
	TianShanIce::StrValues items;
	try
	{
		items = chlPublishPointPrx->getItemSequence();
		// items.size() == 0 is allowed
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 302, PurchaseLog("Purchase", "get channel sub items caught %s"), ex.ice_name().c_str());
	}

	::ChannelOnDemand::ChannelItemDict::const_iterator dictIt;
	::TianShanIce::StrValues::iterator sit;
	for(sit= items.begin(); sit != items.end(); sit ++)
	{
		std::string itemKey = chlPubName + CHANNELITEM_KEY_SEPARATOR + *sit;
		STRTOLOWER(itemKey);

		// find channel item according to the item key.
		NS_PREFIX(ChannelOnDemand::ChannelItemEx) tmpItem;
		try
		{
			LockT<RecMutex> lk(_env._dictLock);
			dictIt = _env._pChannelItemDict->find(itemKey); 
			// if channel item not found in safestore, should throw server error
			if (dictIt == _env._pChannelItemDict->end())
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 305, PurchaseLog(LOG_MODULE_NAME, "channel item [%s] not found in dict"), itemKey.c_str());
			}
			else
			{
				tmpItem = dictIt->second;
			}
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 303, PurchaseLog("Purchase", "find channel item [%s] caught %s:%s"), itemKey.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const TianShanIce::ServerError& ex)
		{
			ex.ice_throw();
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 304, PurchaseLog("Purchase", "find channel item [%s] caught %s"), itemKey.c_str(), ex.ice_name().c_str());
		}

		// initialize playlist item information according to channel item.
		TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
		copyChannelItemToSetupInfo(tmpItem, newItemInfo);
		int userCtrlNum = _gUserCtrlNumGen.Generate();

		// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
		try
		{
			playlist->pushBack(userCtrlNum, newItemInfo);
			glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "channel item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "append channel item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			continue;
		}
		catch(const ::Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "append channel item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
			continue;
		}

		// add an item to PurchaseItemAssoc
		PurchaseItemImplPtr pia = new PurchaseItemImpl(_env);
		pia->ident.name = IceUtil::generateUUID();
		pia->ident.category = ICE_PurchaseItemAssoc;
		pia->purchaseIdent = ident;
		pia->channelItemKey = itemKey;
		pia->playlistCtrlNum = userCtrlNum;
		pia->lastModified = tmpItem.setupInfo.lastModified;
		try
		{
//			LockT<RecMutex> lk(_env._evitPITLock);
			_env._evitPurchaseItemAssoc->add(pia, pia->ident);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 306, PurchaseLog("Purchase", "append purchase item [%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 307, PurchaseLog("Purchase", "append purchase item [%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
		}
		glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "purchase item [%s] appended"), pia->ident.name.c_str());
	}

	bInService = true;
}

// available error code [400, 500)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::detach(const ::std::string& sessId, const ::TianShanIce::Properties& params, const ::Ice::Current&c)
{
	Lock lock(*this);
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "purchase detach() enter"));

	Authorization::AuthorizationParams::iterator itor = _config.authInfo.authorizationParams.find("bookmarkOnLeave");
	if (itor ==  _config.authInfo.authorizationParams.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, 6500, "Authorization [%s] has no [%s] parameter", 
			_config.authInfo.entry.c_str(), "bookmarkOnLeave");
	}

	int BookmarkLastView = 0;
	BookmarkLastView = atoi(itor->second.value.c_str());
/*
	::TianShanIce::ValueMap privData;
	try
	{
		privData = weiwoo->getPrivateData();
	}
	catch (Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 6600, "[%s] getPrivateData() caught %s", ident.name.c_str(), ex.ice_name().c_str());
	}
*/
	/*add here*/
	for(AppDataPatternMAP::iterator it_authappdata = _config.authAppDataMap.begin();
		it_authappdata != _config.authAppDataMap.end(); ++it_authappdata)
	{
// 		::TianShanIce::ValueMap::iterator privateItor;
// 
// 		string AppKey = ClientRequestPrefix + it_authappdata->second.param;
// 		privateItor =  privData.find(AppKey);
// 
// 		if(privateItor != privData.end())
// 		{
// 			::TianShanIce::Variant var = privateItor->second;
// 			string MatchStr = var.strs[0];
// 
// 			boost::regex AppDataRegex(it_authappdata->second.pattern);
// 			boost::cmatch result;
// 
// 			if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
// 			{
// 				continue;
// 			}

			PARAMMAP::iterator itor = it_authappdata->second.appDataParammap.find("bookmarkOnLeave");
			if (itor != it_authappdata->second.appDataParammap.end())
			{
				BookmarkLastView = atoi(itor->second.value.c_str());		
				break;
			}	
// 		}
	}

	try
	{
/*
		map<std::string, ZQ::common::Config::Holder< AuthorizationParam > >::const_iterator iter;
		iter = _config.authInfo.authorizationParams.find("bookmarkOnLeave");
		if(iter != _config.authInfo.authorizationParams.end())
		{
			int BookmarkLastView = 0;
			BookmarkLastView = atoi(iter->second.value.c_str());
			if (BookmarkLastView != 0)
				bookmark("lastview",NULL,c);
		}
*/
		if (BookmarkLastView != 0)
			bookmark("lastview",NULL,c);
	}
	catch(...)
	{
	}

	try {todasTeardown(params);} catch (...){}

	try	{destroy(params, c);} catch (...){}
}

// available error code [500, 600)
// throw exception list NotSupported, ServerError;;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::bookmark(const ::std::string& title, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	Lock lock(*this);
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase bookmark() enter"));

	if (!_config.authInfo.enable || !bAuthorize) // don't save bookmark if the purchase didn't authorize on ia component when created
		return;

	Authorization::AuthorizationParams::iterator itor = _config.authInfo.authorizationParams.find(ENDPOINT);
	if (itor ==  _config.authInfo.authorizationParams.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, 6500, "Authorization [%s] has no [%s] parameter", 
			_config.authInfo.entry.c_str(), ENDPOINT);
	}

	std::string TODASendpoint = itor->second.value;
	TianShanIce::Properties::iterator pit = properties.find(TODAS_ENDPOINT);
	if(pit != properties.end())
	{
		TODASendpoint = pit->second;
	}
/*
	::TianShanIce::ValueMap privData;
	try
	{
		privData = weiwoo->getPrivateData();
	}
	catch (Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 6600, "[%s] getPrivateData() caught %s", ident.name.c_str(), ex.ice_name().c_str());
	}
*/
	/*add here*/
// 	for(AppDataPatternMAP::iterator it_authappdata = _config.authAppDataMap.begin();
// 		it_authappdata != _config.authAppDataMap.end(); ++it_authappdata)
// 	{
// 		::TianShanIce::ValueMap::iterator privateItor;
// 
// 		string AppKey = ClientRequestPrefix + it_authappdata->second.param;
// 		privateItor =  privData.find(AppKey);
// 
// 		if(privateItor != privData.end())
// 		{
// 			::TianShanIce::Variant var = privateItor->second;
// 			string MatchStr = var.strs[0];
// 
// 			boost::regex AppDataRegex(it_authappdata->second.pattern);
// 			boost::cmatch result;
// 
// 			if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
// 			{
// 				continue;
// 			}
// 
// 			PARAMMAP::iterator itor = it_authappdata->second.appDataParammap.find(ENDPOINT);
// 			if (itor != it_authappdata->second.appDataParammap.end())
// 			{
// 				TODASendpoint = itor->second.value;		
// 				break;
// 			}	
// 		}
// 	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, 
		"Get authorize  endpoint is  [%s]"), TODASendpoint.c_str());

	TianShanIce::ValueMap vMap;
	bool bRet = false;
	try
	{
		// getInfo throw non exception from TsStreamer.ICE. version: 42
		bRet = playlist->getInfo(::TianShanIce::Streamer::infoPLAYPOSITION, vMap);
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 500, PurchaseLog(LOG_MODULE_NAME, "getInfo() on playlist [%s] caught %s"), playlistId.c_str(), ex.ice_name().c_str());
	}

	// get current play position information
	int nCtrlNum = -1;
	int nPlayPos = -1;
	TianShanIce::ValueMap::iterator it = vMap.find("ctrlnumber");
	if (it != vMap.end() && it->second.type == TianShanIce::vtInts && it->second.ints.size() > 0)
		nCtrlNum = it->second.ints[0];
	it = vMap.find("playposition");
	if (it != vMap.end() && it->second.type == TianShanIce::vtInts && it->second.ints.size() > 0)
		nPlayPos = it->second.ints[0];

	std::vector<Ice::Identity> idents1, idents2;
	try
	{
		idents1 = _env._idxCtrlNum2ItemAssoc->find(nCtrlNum);
		idents2 = _env._idxPurchase2ItemAssoc->find(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 501, PurchaseLog(LOG_MODULE_NAME, "find index caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 502, PurchaseLog(LOG_MODULE_NAME, "find index caught %s"), ex.ice_name().c_str());
	}

	Ice::Identity identFound;
	bool bFound = false;
	for(unsigned int i = 0;i < idents1.size() && !bFound; i ++)
	{
		for(unsigned int j = 0; j < idents2.size(); j ++)
		{
			if (idents1[i] == idents2[j])
			{
				identFound = idents1[i];
				bFound = true;
				break;
			}
		}
	}
	if (!bFound)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 503, PurchaseLog(LOG_MODULE_NAME, "no purchase item associated with userctrlnum [%d]"), nCtrlNum);
	}

	NS_PREFIX(ChannelOnDemand::ChannelItemEx) cit;
	NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) pPurcharseItemPrx = NULL;
	try
	{
		pPurcharseItemPrx = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(identFound));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 504, PurchaseLog(LOG_MODULE_NAME, "get purchase item [%s] proxy caught %s"), identFound.name.c_str(), ex.ice_name().c_str());
	}
	try
	{
		cit = pPurcharseItemPrx->getChannelItem();
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 505, PurchaseLog(LOG_MODULE_NAME, "get channel item context caught %s"), ex.ice_name().c_str());
	}

	// get item name and utc time
	std::string strElementId = cit.setupInfo.contentName.c_str();
	std::string strBroadcastTime;
	// nPlayPos is ms, convert to filetime(100 ns)
	LONGLONG llBrocastTime = (cit.broadcastStart + nPlayPos)*10000;
	SYSTEMTIME st;
	FILETIME ft;
	FileTimeToLocalFileTime((FILETIME*)&llBrocastTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	char szBuf[50];
	memset(szBuf, 0, sizeof(szBuf));
	snprintf(szBuf, sizeof(szBuf) - 1, "%04d%02d%02dT%02d%02d%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	strBroadcastTime = szBuf;

	::com::izq::todas::integration::cod::BookmarkResultData brd;
	try
	{
		::com::izq::todas::integration::cod::BookmarkData bd;
		//assetid is changed to string type
		bd.assetId = "";
		bd.broadcastTime = strBroadcastTime;
		bd.channelId = chlPubName;
		bd.device.macAddress = macAddress;
		bd.device.smardcardId = smardcardId;
		
		// *******   in current interface, there is a element id, is int and so I need to convert the content name to it ********
		bd.elementId = strElementId;
		bd.homeId = homeId;
		bd.npt = 0.0;
		::com::izq::todas::integration::cod::TodasForCodPrx		_todasPrx;
		_todasPrx = ::com::izq::todas::integration::cod::TodasForCodPrx::checkedCast(_env._communicator->stringToProxy(TODASendpoint));
		brd = _todasPrx->saveBookmark(bd);
	}
	catch(::com::izq::todas::integration::cod::TodasException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 506, PurchaseLog(LOG_MODULE_NAME, "todas saveBookmark() caught %s:%s"), ex.errorCode.c_str(), ex.errorDescription.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 507, PurchaseLog(LOG_MODULE_NAME, "todas saveBookmark() caught %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 509, PurchaseLog(LOG_MODULE_NAME, "todas saveBookmark() caught unknown exception"));
	}
	if (brd.status == 2) // if equals to 2 means a failure return from todas.
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 508, PurchaseLog(LOG_MODULE_NAME, "todas saveBookmark() failed with error code: %s"), brd.errorCode.c_str());
	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase bookmark() OK."));
}

// available error code [600, 700)
// throw exception list InvalidParameter, NotSupported, ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
::Ice::Int PurchaseImpl::getParameters(const ::TianShanIce::StrValues& params, const ::TianShanIce::ValueMap& vMap_in, ::TianShanIce::ValueMap& vMap_out, const ::Ice::Current& c) const
{
	// input parameter "params" representaing the parameters are requested by the caller.
	// "UserCtrlNum", "Offset", "StartPos" and "BcastPos" can be gained now.
	Lock lock(*this);
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase getParameters() enter."));
	
	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';
	snprintf(szBuf, sizeof(szBuf) - 1, PurchaseLog(LOG_MODULE_NAME, "vMap_in."));
	::ZQTianShan::dumpValueMap(vMap_in, szBuf, dumpLine);

	// renew purchase
//	_env._pWatchDog->watch(ident.name, _gCODCfg._dwPurchaseTimeout * 1000);

	// clear out map.
	vMap_out.clear();

	// gain all possible input parameters, such as "UserCtrlNum", "Offset", "BcastPos" and "Position.clock"
	// 1. if the client wants to get utc time, he must specify "UserCtrlNum" and "Offset"
	// "UserCtrlNum" and "Offset" will be copied to variables "ctrlNum" and "offSet"
	// 2. if the client wants to get "UserCtrlNum" and "Offset", he must specify "BcastPos", the value
	// of "BcastPos" will be copied to variable "utcTime"
	std::string utcTime; // format: YYYY:MM:DDThh:mm:ss
	Ice::Int ctrlNum = -1, offSet = -1, startPos = -1;
	// initialize the foregoing variables from the input map.
	TianShanIce::ValueMap::const_iterator inMap_itor;
	inMap_itor = vMap_in.find("UserCtrlNum");
	if (vMap_in.end() != inMap_itor && TianShanIce::vtInts == inMap_itor->second.type && inMap_itor->second.ints.size() > 0)
		ctrlNum = inMap_itor->second.ints[0];
	inMap_itor = vMap_in.find("Offset");
	if (vMap_in.end() != inMap_itor && ::TianShanIce::vtInts == inMap_itor->second.type && inMap_itor->second.ints.size() >0)
		offSet = inMap_itor->second.ints[0];
	inMap_itor = vMap_in.find("BcastPos");
	if (vMap_in.end() != inMap_itor && ::TianShanIce::vtStrings == inMap_itor->second.type && inMap_itor->second.strs.size() >0)
		utcTime = inMap_itor->second.strs[0];
	inMap_itor = vMap_in.find("Position.clock");
	if (vMap_in.end() != inMap_itor && ::TianShanIce::vtStrings == inMap_itor->second.type && inMap_itor->second.strs.size() >0)
		utcTime = inMap_itor->second.strs[0];

	uint cur = 0;
	for (cur = 0; cur < params.size(); cur ++)
	{
		if (vMap_out.end() != vMap_out.find(params[cur]))
		{
			continue; // ignore if the result already set into "vMap_out".
			// because if you want to get UserCtrlNum, Offset, StartPosition by a given utc time, all of them will
			// be pushed in "vMap_out" by a call
		}

		// if the equry parameter equals to "BcastPos" incase-sensitive
		// use the input "UserCtrlNum" and "Offset" stored in "vMap_in" to calculate corresponding utc time.
		if (0 == stricmp(params[cur].c_str(), "BcastPos") || 0 == stricmp(params[cur].c_str(), "Position.clock"))
		{
			if (offSet == -1 || ctrlNum == -1) // if offSet or ctrlNum not specified, get current offset and ctrlNum
			{
				try
				{
					getCurStreamPos(ctrlNum, offSet);
				}
				catch (const TianShanIce::ServerError& ex)
				{
					ex.ice_throw();
				}
				catch (const Ice::Exception& ex)
				{
					ex.ice_throw();
				}
				catch (...)
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 600, PurchaseLog(LOG_MODULE_NAME, "getCurStreamPos() caught unexpect exception"));
				}
			}

			// getUtcTime() only throw TianShanIce::ServerError, TianShanIce::InvalidParameter, and both of them are lie 
			// in the list of caller function, so we needn't transfer exception by try catch.
			try
			{
				getUtcTime(ctrlNum, offSet, utcTime);
			}
			catch (const TianShanIce::InvalidParameter& ex)
			{
				ex.ice_throw();
			}
			catch (const TianShanIce::ServerError& ex)
			{
				ex.ice_throw();
			}
			catch (...)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 601, PurchaseLog(LOG_MODULE_NAME, "getUtcTime() caught unexpect exception"));
			}

			TianShanIce::Variant vrtBcastPos;
			vrtBcastPos.type = TianShanIce::vtStrings;
			vrtBcastPos.bRange = false;
			vrtBcastPos.strs.clear();
			vrtBcastPos.strs.push_back(utcTime);
			vMap_out["BcastPos"] = vrtBcastPos;
			continue;
		}

		// if the current param equals to any of "UserCtrlNum", "Offset" or "StartPos", use the utc time gained from the input parameters.
		if (0 == stricmp(params[cur].c_str(), "UserCtrlNum") || 0 == stricmp(params[cur].c_str(), "Offset") || 0 == stricmp(params[cur].c_str(), "StartPos"))
		{
			// getStreamPos() only throw TianShanIce::ServerError, TianShanIce::InvalidParameter, and both of them are lie 
			// in the list of caller function, so we needn't transfer exception by try catch.
			try
			{
				getStreamPos(utcTime, ctrlNum, offSet, startPos);
			}
			catch (const TianShanIce::InvalidParameter& ex)
			{
				ex.ice_throw();
			}
			catch (const TianShanIce::ServerError& ex)
			{
				ex.ice_throw();
			}
			catch (...)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 602, PurchaseLog(LOG_MODULE_NAME, "getStreamPos() caught unexpect exception"));
			}

			TianShanIce::Variant vrtCtrlNum, vrtOffset, vrtStartPos;
			vrtCtrlNum.type = TianShanIce::vtInts;
			vrtCtrlNum.bRange = false;
			vrtCtrlNum.ints.clear();
			vrtCtrlNum.ints.push_back(ctrlNum);
			vMap_out["UserCtrlNum"] = vrtCtrlNum;

			vrtOffset.type = TianShanIce::vtInts;
			vrtOffset.bRange = false;
			vrtOffset.ints.clear();
			vrtOffset.ints.push_back(offSet);
			vMap_out["Offset"] = vrtOffset;

			vrtStartPos.type = TianShanIce::vtInts;
			vrtStartPos.bRange = false;
			vrtStartPos.ints.clear();
			vrtStartPos.ints.push_back(startPos);
			vMap_out["StartPos"] = vrtStartPos;
			continue;
		}

		// not supported parameters requested.
		ZQTianShan::_IceThrow<TianShanIce::NotSupported>(glog, "Purchase", 603, PurchaseLog(LOG_MODULE_NAME, "[%s] not supported"), params[cur].c_str());
	} // for

	snprintf(szBuf, sizeof(szBuf) - 1, PurchaseLog(LOG_MODULE_NAME, "vMap_out."));
	::ZQTianShan::dumpValueMap(vMap_out, szBuf, dumpLine);
	return vMap_out.size();
}

//////////////////////////////////////////////////////////////////////////
// implement the functions defined in ChannelOnDemand::ChannelPurchase
//////////////////////////////////////////////////////////////////////////

::TianShanIce::Streamer::PlaylistPrx PurchaseImpl::getPlaylist(const ::Ice::Current& c)
{
    Lock lock(*this);
	return playlist;
}

::std::string PurchaseImpl::getPlaylistId(const ::Ice::Current& c)
{
    Lock lock(*this);
	return playlistId;
}

::std::string PurchaseImpl::getChannelName(const ::Ice::Current& c)
{
    Lock lock(*this);
	return chlPubName;
}

bool PurchaseImpl::getIfNeedSyncChannel(const ::Ice::Current& c)
{
    Lock lock(*this);
	return bNeedSyncChannel;
}

bool PurchaseImpl::getIfNeedAuthorize(const ::Ice::Current& c)
{
    Lock lock(*this);
	return bAuthorize;
}

// available error code [2000, 2100)
// throw exception list InvalidParameter, NotSupported, ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::destroy(const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
    Lock lock(*this);
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase destroy() enter"));

	SYSTEMTIME timeLocal;
	GetLocalTime(&timeLocal);
	char reqArriveTime[50];
	sprintf(reqArriveTime,"%04d%02d%02dT%02d%02d%02d.%d",timeLocal.wYear,timeLocal.wMonth,timeLocal.wDay,
		timeLocal.wHour,timeLocal.wMinute,timeLocal.wSecond,timeLocal.wMilliseconds);

	// get purchase items
	std::vector<Ice::Identity> idents;
	try
	{
		idents = _env._idxPurchase2ItemAssoc->find(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "find purchase items caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "find purchase items caught %s"), ex.ice_name().c_str());
	}

	for(unsigned int i = 0; i < idents.size(); i ++)
	{
		try
		{
//			LockT<RecMutex> lk(_env._evitPITLock);
			_env._evitPurchaseItemAssoc->remove(idents[i]);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "remove purchase item [%s] caught %s:%s"), idents[i].name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "remove purchase item [%s] caught %s"), idents[i].name.c_str(), ex.ice_name().c_str());
		}
		glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "purchase item [%s] removed"), idents[i].name.c_str());
	}

	try
	{
//		LockT<RecMutex> lk(_env._evitPurLock);
		_env._evitPurchase->remove(ident);
	}
	catch(const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "remove purchase caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "remove purchase caught %s"), ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "purchase removed"));

	// unwatch purchase
	_env._pWatchDog->unwatch(ident.name);
}

bool PurchaseImpl::isInService(const ::Ice::Current& c)
{
	Lock lock(*this);
	return bInService;
}

//////////////////////////////////////////////////////////////////////////
// implement the functions defined in ChannelOnDemand::ChannelPurchaseEx
//////////////////////////////////////////////////////////////////////////

::std::string PurchaseImpl::getClientSessionId(const ::Ice::Current& c)
{
    Lock lock(*this);
	return clientSessionId;
}

::std::string PurchaseImpl::getServerSessionId(const ::Ice::Current& c)
{
    Lock lock(*this);
	return serverSessionId;
}

void PurchaseImpl::copyChannelItemToSetupInfo(const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& chnlItemInfo, 
	TianShanIce::Streamer::PlaylistItemSetupInfo& setupInfo)
{
	setupInfo.contentName = chnlItemInfo.setupInfo.contentName;
	setupInfo.criticalStart = 0;
	setupInfo.forceNormal = chnlItemInfo.setupInfo.forceNormalSpeed;
	setupInfo.inTimeOffset = chnlItemInfo.setupInfo.inTimeOffset;
	setupInfo.outTimeOffset = chnlItemInfo.setupInfo.outTimeOffset;
	setupInfo.spliceIn = chnlItemInfo.setupInfo.spliceIn;
	setupInfo.spliceOut = chnlItemInfo.setupInfo.spliceOut;
	setupInfo.flags = chnlItemInfo.flags;
}

bool PurchaseImpl::appendPlaylistItem(
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem, 
		const ::Ice::Current& c)
{
    Lock lock(*this);

	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(appendChnlItem, setupInfo);
	const std::string& appendItemKey = appendChnlItem.key;

	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "appendPlaylistItem(%s) enter"), setupInfo.contentName.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bInService = false, Purchase is not in service mode"));
		return false;
	}

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;
/*
	// if playlist already need sync, just ignore.
	if (bNeedSyncChannel && !isCalledFromSync)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bNeedSyncChannel = true, ignore appendPlaylistItem"));
		return false;
	}
*/
	// set bNeedSyncChannel to true now, suppose the operation will be failed
	bool oldNeedSyncChannel = bNeedSyncChannel;
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	int newCtrlNum = _gUserCtrlNumGen.Generate();
	try
	{
		playlist->pushBack(newCtrlNum, setupInfo);
		glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "item [%s: %d] appended on playlist [%s]"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str());
	}
	catch (const ::TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "append item [%s: %d] on playlist [%s] caught %s: %s"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "append item [%s: %d] on playlist [%s] caught %s"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}

	// 2. record the playlist item information in a purchase item
	PurchaseItemImplPtr pia = new PurchaseItemImpl(_env);
	pia->ident.name = IceUtil::generateUUID();
	pia->ident.category = ICE_PurchaseItemAssoc;
	pia->purchaseIdent = ident;
	pia->channelItemKey = appendItemKey;
	pia->playlistCtrlNum = newCtrlNum;
	pia->lastModified = appendChnlItem.setupInfo.lastModified;
	try
	{
//		LockT<RecMutex> lk(_env._evitPITLock);
		_env._evitPurchaseItemAssoc->add(pia, pia->ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		// if save purchase item caught exception, you should remove purchase here, because no purchase item records
		// the playlist list item information, it will affect the streaming.
		char szBuf[MAX_PATH];
		szBuf[MAX_PATH - 1] = '\0';
		_snprintf(szBuf, MAX_PATH - 1, "save purchase item caught %s: %s, freeze db exception to destroy purchase", 
			ex.ice_name().c_str(), ex.message.c_str());
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "%s"), szBuf);
		RemovePurchaseRequest* pRequest = new RemovePurchaseRequest(_env, ident, clientSessionId, szBuf, true);
		if (pRequest)
			pRequest->start();
		return false;
	}

	// set bNeedSyncChannel = false to indicates operation successfully
	if (!isCalledFromSync)
	{
		if(!oldNeedSyncChannel)
			bNeedSyncChannel = false;
	}
	return true;
}

bool PurchaseImpl::ChnlItem2PItemIdent(const ::std::string& channelItem, Ice::Identity& piIdent) const
{
	// find the purchase item' identity according to position Key
	try
	{
		std::vector<Ice::Identity> idents;
		idents = _env._idxPurchase2ItemAssoc->find(ident);
		bool bFound = false;
		for (unsigned int i = 0; i < idents.size() && !bFound; i ++) {
			NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(idents[i]));
			string ChannelItemKey = purPrx->getChannelItemKey();
			if(channelItem.compare(ChannelItemKey) == 0)
			{
				bFound = true;
				piIdent = idents[i];
				break;
			}
		}
		if (!bFound)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "there is no purchase item relating to the channel item [%s]"), 
				channelItem.c_str());
			return false;
		}
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "find position caught %s:%s"), 
			ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get position caught %s"), 
			ex.ice_name().c_str());
		return false;
	}
	return true;
}

bool PurchaseImpl::insertPlaylistItem(
		const ::std::string& insertPosKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem, 
		const ::Ice::Current& c)
{
    Lock lock(*this);

	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(insertChnlItem, setupInfo);

	const std::string& insertItemKey = insertChnlItem.key;

	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "insertPlaylistItem(%s before %s) enter"), 
		insertItemKey.c_str(), insertPosKey.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bInService = false, Purchase is not in service mode"));
		return false;
	}

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;

	// if playlist already need sync, just ignore.
	if (bNeedSyncChannel && !isCalledFromSync)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bNeedSyncChannel = true, ignore insertPlaylistItem"));
		return false;
	}
	// set bNeedSyncChannel to true now, suppose the operation will be failed
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	// find the purchase item' identity before which will be inserted
	Ice::Identity insertPosIdent;
	if(!ChnlItem2PItemIdent(insertPosKey, insertPosIdent))
		return false;

	int insertPosCtrlNum;
	try
	{
		NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) insertPosPrx = NULL;
		insertPosPrx = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(insertPosIdent));
		insertPosCtrlNum = insertPosPrx->getCtrlNum();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "get insert position caught %s"), 
			ex.ice_name().c_str());
		return false;
	}

	// the ctrlnum for new playlist item
	Ice::Int newCtrlNum = _gUserCtrlNumGen.Generate();
	try
	{
		playlist->insert(newCtrlNum, setupInfo, insertPosCtrlNum);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s:%s"), 
			insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s"), 
			insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "item [%s: %d] inserted before [%s: %d] on playlist [%s]"), 
		insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str());

	// save purchase item
	PurchaseItemImplPtr pia = new PurchaseItemImpl(_env);
	pia->ident.name = IceUtil::generateUUID();
	pia->ident.category = ICE_PurchaseItemAssoc;
	pia->purchaseIdent = ident;
	pia->channelItemKey = insertItemKey;
	pia->playlistCtrlNum = newCtrlNum;
	pia->lastModified = insertChnlItem.setupInfo.lastModified;
	try
	{
//		LockT<RecMutex> lk(_env._evitPITLock);
		_env._evitPurchaseItemAssoc->add(pia, pia->ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		// if save purchase item caught exception, you should remove purchase here, because no purchase item records
		// the playlist list item information, it will affect the streaming.
		char szBuf[MAX_PATH];
		szBuf[MAX_PATH - 1] = '\0';
		_snprintf(szBuf, MAX_PATH - 1, "save purchase item caught %s: %s, freeze db exception to destroy purchase", 
			ex.ice_name().c_str(), ex.message.c_str());
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "%s"), szBuf);
		RemovePurchaseRequest* pRequest = new RemovePurchaseRequest(_env, ident, clientSessionId, szBuf, true);
		pRequest->start();
		return false;
	}

	if (!isCalledFromSync)
		bNeedSyncChannel = false;
	return true;
}

bool PurchaseImpl::removePlaylistItem(
	const ::std::string& removeItemKey, 
	const ::Ice::Current& c)
{
    Lock lock(*this);

	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "removePlaylistItem(%s) enter"), 
		removeItemKey.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bInService = false, Purchase is not in service mode"));
		return false;
	}

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;

	// if playlist already need sync, just ignore.
	if (bNeedSyncChannel && !isCalledFromSync)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bNeedSyncChannel = true, ignore removePlaylistItem"));
		return false;
	}

	// set bNeedSyncChannel to true now, suppose the operation will be failed
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	// find the remove purchase item identity
	Ice::Identity removeItemIdent;
	if(!ChnlItem2PItemIdent(removeItemKey, removeItemIdent))
		return false;

	NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) removeItemPrx = NULL;
	Ice::Int removeCtrlNum;
	try
	{
		removeItemPrx = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(removeItemIdent));
		removeCtrlNum = removeItemPrx->getCtrlNum();
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get remove position caught %s"), 
			ex.ice_name().c_str());
		return false;
	}

	try
	{
		playlist->erase(removeCtrlNum);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove [%d] from playlist [%s] caught %s: %s"), 
			removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove [%d] from playlist [%s] caught %s"), 
			removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "item [%s: %d] removed form playlist [%s]"), 
		removeItemKey.c_str(), removeCtrlNum, playlistId.c_str());

	try
	{
		removeItemPrx->destroy();
	}
	catch (const Ice::Exception& ex)
	{
		// if destroy purchase item caught exception, just ignore with a warning message
		// because a excrescent purchase item in db doesn't affect streaming
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "remove purchase item caught %s"), 
			ex.ice_name().c_str());
	}

	if (!isCalledFromSync)
		bNeedSyncChannel = false;
	return true;
}

bool PurchaseImpl::replacePlaylistItem(
		const ::std::string& oldItemKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem, 
		const ::Ice::Current& c)
{
    Lock lock(*this);

	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(replaceChnlItem, setupInfo);

	const std::string& newItemKey = replaceChnlItem.key;

	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "replacePlaylistItem(%s -> %s) enter"), 
		oldItemKey.c_str(), newItemKey.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bInService = false, Purchase is not in service mode"));
		return false;
	}

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;
/*
	// if playlist already need sync, just ignore.
	if (bNeedSyncChannel && !isCalledFromSync)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bNeedSyncChannel = true, ignore replacePlaylistItem"));
		return false;
	}
*/
	// set bNeedSyncChannel to true now, suppose the operation will be failed
	bool oldNeedSyncChannel = bNeedSyncChannel;
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	Ice::Identity oldPosIdent;
	if(!ChnlItem2PItemIdent(oldItemKey, oldPosIdent))
		return false;

	// 1. to insert a new item before the old item
	NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) oldPurItemPrx = NULL;
	Ice::Int oldCtrlNum;
	try
	{
		oldPurItemPrx = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(oldPosIdent));
		oldCtrlNum = oldPurItemPrx->getCtrlNum();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get old item position information caught %s"), 
			ex.ice_name().c_str());
		return false;
	}

	Ice::Int newCtrlNum = _gUserCtrlNumGen.Generate();
	try
	{
		playlist->insert(newCtrlNum, setupInfo, oldCtrlNum);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s:%s"), 
			newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s"), 
			newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}
	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "item [%s: %d] inserted before [%s: %d] on playlist [%s]"), 
		newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str());

	// 2. to remove the old item
	bool bOldPIRemoved = false;
	try
	{
		playlist->erase(oldCtrlNum);
		bOldPIRemoved = true;
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove [%s: %d] from playlist [%s] caught %s: %s"), 
			oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove [%s: %d] from playlist [%s] caught %s"), 
			oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
	}

	if (bOldPIRemoved)
	{
		glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "item [%s: %d] removed form playlist [%s]"), 
			oldItemKey.c_str(), oldCtrlNum, playlistId.c_str());
		try
		{
			oldPurItemPrx->destroy();
		}
		catch (const Ice::Exception& ex)
		{
			// if destroy purchase item caught exception, just ignore with a warning message
			// because a excrescent purchase item in db doesn't affect streaming
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "remove purchase item caught %s"), 
				ex.ice_name().c_str());
		}

		// save purchase item
		PurchaseItemImplPtr pia = new PurchaseItemImpl(_env);
		pia->ident.name = IceUtil::generateUUID();
		pia->ident.category = ICE_PurchaseItemAssoc;
		pia->purchaseIdent = ident;
		pia->channelItemKey = newItemKey;
		pia->playlistCtrlNum = newCtrlNum;
		pia->lastModified = replaceChnlItem.setupInfo.lastModified;
		try
		{
//			LockT<RecMutex> lk(_env._evitPITLock);
			_env._evitPurchaseItemAssoc->add(pia, pia->ident);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			char szBuf[MAX_PATH];
			szBuf[MAX_PATH - 1] = '\0';
			_snprintf(szBuf, MAX_PATH - 1, "save purchase item caught %s: %s, freeze db exception to destroy purchase", 
				ex.ice_name().c_str(), ex.message.c_str());
			glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "%s"), szBuf);
			RemovePurchaseRequest* pRequest = new RemovePurchaseRequest(_env, ident, clientSessionId, szBuf, true);
			pRequest->start();
			return false;
		}
	}
	else 
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "replacePlaylistItem(), caught remove old item [%s: %d] failed so we have to remove the newly inserted item [%s: %d]"), 
			oldItemKey.c_str(), oldCtrlNum, newItemKey.c_str(), newCtrlNum);
		try
		{
			playlist->erase(newCtrlNum);
			glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "newly inserted item [%s: %d] has been removed"), 
				newItemKey.c_str(), newCtrlNum);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove newly inserted item [%s: %d] from playlist [%s] caught %s: %s"), 
				oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  PurchaseLog(LOG_MODULE_NAME, "remove newly inserted item [%s: %d] from playlist [%s] caught %s"), 
				oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		}
		return false;
	}

	if (!isCalledFromSync)
	{
		if(!oldNeedSyncChannel)
			bNeedSyncChannel = false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// implement the functions just defined in this class PurchaseImpl
//////////////////////////////////////////////////////////////////////////

// available error code [6000, 6100)
void PurchaseImpl::getUtcTime(const ::Ice::Int ctrlNum, const ::Ice::Int offSet, ::std::string& utcTime) const
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "getUtcTime(ctrlNum: %d, offSet: %d) enter"), 
		ctrlNum, offSet);

	// find the ctrl num and get it's broadcast start
	Ice::Identity identPlaylistItem;	
	try
	{
		std::vector<Ice::Identity> identsByCtrlNum, identsByPurchase;
		identsByCtrlNum = _env._idxCtrlNum2ItemAssoc->find(ctrlNum);
		identsByPurchase = _env._idxPurchase2ItemAssoc->find(ident);		
		bool found = false;
		for (size_t i = 0; !found && i < identsByCtrlNum.size(); i++) {
			for (size_t j=0; !found && j < identsByPurchase.size(); j++) {
				if (identsByCtrlNum[i] == identsByPurchase[j]) {
					identPlaylistItem = identsByCtrlNum[i];
					found = true;
					break;
				}
			}
		}
		if (!found) {
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "Purchase", 6000, PurchaseLog(LOG_MODULE_NAME, "userctrlnum can not be found in purchase"));
		}
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 6001, PurchaseLog(LOG_MODULE_NAME, "find index caught %s:%s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}

	// create purchase item proxy
	NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) pia = NULL;
	NS_PREFIX(ChannelOnDemand::ChannelItemEx) cie;
	try
	{
		pia = NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(identPlaylistItem));
		cie = pia->getChannelItem();
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 6002, PurchaseLog(LOG_MODULE_NAME, "get purchase item [%s] proxy caught %s"), identPlaylistItem.name.c_str(), ex.ice_name().c_str());
	}

	::Ice::Long iBcastBos = cie.broadcastStart + offSet;
	__int64 llFileTime = iBcastBos *10000;			
	SYSTEMTIME st;
	if (!_config.InputLocalTime)
	{
		FileTimeToSystemTime((FILETIME*)&llFileTime, &st);
	}
	else
	{
		FILETIME ft;
		FileTimeToLocalFileTime((FILETIME*)&llFileTime, &ft);
		FileTimeToSystemTime(&ft, &st);	
	}
	// copy utc time to buffer
	char szBuf[50];
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "%04d%02d%02dT%02d%02d%02dZ", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);		
	utcTime = szBuf; // assign to return value
}

// available error code [6100, 6200)
void PurchaseImpl::getStreamPos(const ::std::string& utcTime, ::Ice::Int& ctrlNum, ::Ice::Int& offSet, ::Ice::Int& startPos) const
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "getStreamPos(%s) enter"), utcTime.c_str());

	__int64 iBcastPos = -1;// if iBcastPos equals to -1, seek to now.
	if (0 != stricmp(utcTime.c_str(), "now"))// if "utcTime" equals "now", it will mentaion the value of '-1'.
	{
		if (!_config.InputLocalTime)
		{
			if (!systemTime2TianShanTimeB(utcTime.c_str(), iBcastPos))
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "Purchase", 6100, PurchaseLog(LOG_MODULE_NAME, "parse [%s] failed"), utcTime.c_str());
		}
		else
		{
			if (!localTime2TianShanTimeB(utcTime.c_str(), iBcastPos))
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "Purchase", 6101, PurchaseLog(LOG_MODULE_NAME, "parse [%s] failed"), utcTime.c_str());
		}
	}

	// step 1. get the channel item list of this purchase
	TianShanIce::StrValues items;
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx)  chlPublishPointPrx = NULL;
	::TianShanIce::Application::PublishPointPrx publishPointPrx = NULL;
	try
	{
#ifdef USE_OLD_NS
		chlPublishPointPrx = _env._publisher->open(chlPubName);
#else
		publishPointPrx = _env._publisher->open(chlPubName);
		chlPublishPointPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(publishPointPrx);
#endif //USE_OLD_NS	
		items = chlPublishPointPrx->getItemSequence();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 6102, "get channel item list caught %s: %s", 
			ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 6102, "get channel item list caught %s", 
			ex.ice_name().c_str());
	}

	if (items.size() <= 0)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "Purchase", 6103, PurchaseLog(LOG_MODULE_NAME, "channel [%s] has no channel items"), 
			chlPubName.c_str());
	}

	//step 2. locate the channel item where the bcastPos is in
	NS_PREFIX(ChannelOnDemand::ChannelItemEx) item;
	std::string itemKey;
	try
	{
		// find the channel item key for seek and set playoffset
		LockT<RecMutex> lk(_env._dictLock);
		::ChannelOnDemand::ChannelItemDict::iterator dictIt = _env._pChannelItemDict->end();
		::TianShanIce::StrValues::iterator sit;
		::TianShanIce::StrValues::reverse_iterator rit;

		if (-1 == iBcastPos)
		{
			// bcastPos = -1 means seek to now, always take the last item of the channel
			sit = items.end()-1;
			itemKey = chlPubName + CHANNELITEM_KEY_SEPARATOR + *sit;
			dictIt = _env._pChannelItemDict->find(itemKey);
			if (_env._pChannelItemDict->end() != dictIt)
				item = dictIt->second;
			else 
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6104, PurchaseLog(LOG_MODULE_NAME, "channel item [%s] not found in dict"), 
					itemKey.c_str());
			}
		}
		else // seek to a utc time, which looks like "YYYY:MM:DDThh:mm:ssZ"
		{
			bool bFound = false;
			for (rit = items.rbegin(); rit != items.rend(); rit++)
			{
				itemKey = chlPubName + CHANNELITEM_KEY_SEPARATOR + *rit;
				dictIt = _env._pChannelItemDict->find(itemKey);
				if (_env._pChannelItemDict->end() != dictIt && dictIt->second.broadcastStart <=iBcastPos)
				{
					item = dictIt->second;
					bFound = true;
					break;
				}
			}
			if (false == bFound) // if no appropriate item found according to the given utc time, take the first item of the channel
			{
				itemKey = chlPubName + CHANNELITEM_KEY_SEPARATOR + items[0]; // here items.size() must larger than 0, so don't care that.
				dictIt = _env._pChannelItemDict->find(itemKey);
				if (_env._pChannelItemDict->end() != dictIt)
				{
					item = dictIt->second;
					glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "no channel item associated with utc time [%s], take the first item."), utcTime.c_str());
				}
				else 
					ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6105, PurchaseLog(LOG_MODULE_NAME, "invalid ute time [%s]"), utcTime.c_str());
			}
		}
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6106, PurchaseLog(LOG_MODULE_NAME, "find index caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}

	// step 3 find the ctrlnum of this item in the purchase
	Ice::Identity identPlaylistItem;
	if (!ChnlItem2PItemIdent(itemKey, identPlaylistItem))
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6111, PurchaseLog(LOG_MODULE_NAME, "ChnlItem2PItemIdent failed"));
	}

	try
	{
		NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) pPurcharseItemPrx = 
			NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(identPlaylistItem));
		ctrlNum = pPurcharseItemPrx->getCtrlNum();
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6110, PurchaseLog(LOG_MODULE_NAME, "get UserCtrlNum caught %s"), ex.ice_name().c_str());
	}

	// step 4. prepare results
	if (iBcastPos == -1) // bcastPos =="now"
	{
		startPos = 2; // indicating seek from end of the item
		offSet = 0 - _config.ProtectTimeInMs;
	}
	else
	{
		startPos = 1; // indicating seek from the begin of the item
		offSet = (::Ice::Int) (iBcastPos - item.broadcastStart);
		if (offSet < 0)
			offSet = 0;
	}
}

// available error code [6300, 6400)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void PurchaseImpl::todasTeardown(const ::TianShanIce::Properties& params)
{
	if (!_config.authInfo.enable || !bAuthorize)// don't save bookmark if the purchase didn't authorize on a component when created
		return;
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "todasTeardown() enter"));

	Authorization::AuthorizationParams::iterator itor = _config.authInfo.authorizationParams.find(ENDPOINT);
	if (itor ==  _config.authInfo.authorizationParams.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, 6500, "Authorization [%s] has no [%s] parameter", 
			_config.authInfo.entry.c_str(), ENDPOINT);
	}

	std::string TODASendpoint = itor->second.value;
	TianShanIce::Properties::iterator pit = properties.find(TODAS_ENDPOINT);
	if(pit != properties.end())
	{
		TODASendpoint = pit->second;
	}

	try
	{
/*
		::TianShanIce::ValueMap privData;
		try
		{
			privData = weiwoo->getPrivateData();
		}
		catch (Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 6600, "[%s] getPrivateData() caught %s", ident.name.c_str(), ex.ice_name().c_str());
		}
*/
		/*add here*/
// 		for(AppDataPatternMAP::iterator it_authappdata = _config.authAppDataMap.begin();
// 			it_authappdata != _config.authAppDataMap.end(); ++it_authappdata)
// 		{
// 			::TianShanIce::ValueMap::iterator privateItor;
// 
// 			string AppKey = ClientRequestPrefix + it_authappdata->second.param;
// 			privateItor =  privData.find(AppKey);
// 
// 			if(privateItor != privData.end())
// 			{
// 				::TianShanIce::Variant var = privateItor->second;
// 				string MatchStr = var.strs[0];
// 
// 				boost::regex AppDataRegex(it_authappdata->second.pattern);
// 				boost::cmatch result;
// 
// 				if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
// 				{
// 					continue;
// 				}
// 
// 				PARAMMAP::iterator itor = it_authappdata->second.appDataParammap.find(ENDPOINT);
// 				if (itor != it_authappdata->second.appDataParammap.end())
// 				{
// 					TODASendpoint = itor->second.value;		
// 					break;
// 				}	
// 			}
// 		}
		glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, 
			"Get authorize  endpoint is  [%s]"), TODASendpoint.c_str());

		::com::izq::todas::integration::cod::SessionData sd;
		sd.serverSessionId = serverSessionId;
		sd.clientSessionId = clientSessionId;
		SYSTEMTIME st;
		GetLocalTime(&st);
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		sd.params["ViewEndTime"] = strTime;

        // reason code
        {
            ::TianShanIce::Properties::const_iterator cit;
			std::string	strTeardownReason;
			std::string	strTerminateReason;

            cit = params.find(SYS_PROP(teardownReason));
            if(cit != params.end())
			{
				strTeardownReason = cit->second;
				sd.params["teardownReason"] = cit->second;
			}
			else sd.params["teardownReason"] = "";

            cit = params.find(SYS_PROP(terminateReason));
            if(cit != params.end())
			{
				strTerminateReason = cit->second;
				sd.params["terminateReason"] = cit->second;
			}
			else sd.params["terminateReason"] = "";

			sd.params["Reason"] = (!strTeardownReason.empty())?strTeardownReason:strTerminateReason;
        }

		//full URI of the request;AppPath;Virtual Site name
		pit = properties.find(PD_KEY_SiteName);
		if(pit != properties.end())
		{
			sd.params[PD_KEY_SiteName] = pit->second;
		}
		else sd.params[PD_KEY_SiteName] = "";

		pit = properties.find(PD_KEY_Path);
		if(pit != properties.end())
		{
			sd.params[PD_KEY_Path] = pit->second;
		}
		else sd.params[PD_KEY_Path] = "";

		pit = properties.find(PD_KEY_URL);
		if(pit != properties.end())
		{
			sd.params[PD_KEY_URL] = pit->second;
		}
		else sd.params[PD_KEY_URL] = "";

		::com::izq::todas::integration::cod::SessionResultData rd;
		rd.status = 0;

		TodasForCodTeardownCB* pCB = new TodasForCodTeardownCB(clientSessionId);
		::com::izq::todas::integration::cod::TodasForCodPrx		_todasPrx;
		_todasPrx = ::com::izq::todas::integration::cod::TodasForCodPrx::checkedCast(_env._communicator->stringToProxy(TODASendpoint));
		_todasPrx->sessionTeardown_async(pCB, sd);
//		_env._todasPrx->sessionTeardown_async(pCB, sd);
		glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "todasTeardown() leave successfully"));
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6300, PurchaseLog(LOG_MODULE_NAME, "todas teardown session caught %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "Purchase", 6700, PurchaseLog(LOG_MODULE_NAME, "todas teardown session caught unknown exception"));
	}
}

bool PurchaseImpl::syncPlaylist(const ::Ice::Current& c)
{
    Lock lock(*this);

	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() enter"));

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "bInService = false, Purchase is not in service mode"));
		return false;
	}

#if !TestSyncPlaylist
	if (!bNeedSyncChannel)
	{
		glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() found bNeedSyncChannel = false"));
		return true;
	}
#endif
	// 1. get channel item list from channel db
	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chnlPub = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	TianShanIce::StrValues itemsInChannel;
	try
	{
#ifdef USE_OLD_NS
		chnlPub = _env._publisher->open(chlPubName);
#else
		pubPrx = _env._publisher->open(chlPubName);
		chnlPub = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS	
		itemsInChannel = chnlPub->getItemSequence();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get channel publish point item sequence caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get channel publish point item sequence caught %s"), 
			ex.ice_name().c_str());
		return false;
	}

	int i = 0, count = 0;

	// 2. get all channel item's context
	std::vector<NS_PREFIX(ChannelOnDemand::ChannelItemEx)> chnlItemCtxs;
	chnlItemCtxs.reserve(itemsInChannel.size());
	for (i = 0, count = itemsInChannel.size(); i < count; i ++)
	{
		LockT<RecMutex> lk(_env._dictLock);
		std::string chnlItemKey = chlPubName + CHANNELITEM_KEY_SEPARATOR + itemsInChannel[i];
		::ChannelOnDemand::ChannelItemDict::iterator dictIt = _env._pChannelItemDict->find(chnlItemKey);
		if (_env._pChannelItemDict->end() == dictIt)
		{
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "channel item [%s]'s context could not be found"), 
				chnlItemKey.c_str());
		}
		chnlItemCtxs.push_back(dictIt->second);
	}

	// 3. get playlist item ctrl number vector
	TianShanIce::IValues ctrlNums;
	ctrlNums.clear();
	try
	{
		ctrlNums = playlist->getSequence();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get playlist [%s] sequence caught %s"), 
			playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}

	// store all purchase item context
	std::vector<NS_PREFIX(ChannelOnDemand::PurchaseItemData)> purItemDatas;
	purItemDatas.reserve(ctrlNums.size());

	// notice you must add purchase item follow the order of ctrlnum
	for (i = 0, count = ctrlNums.size(); i < count; i ++)
	{
		std::vector<Ice::Identity> identsByCtrlNum;
		identsByCtrlNum = _env._idxCtrlNum2ItemAssoc->find(ctrlNums[i]);
		if (identsByCtrlNum.size() != 1)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "ctrlNum [%d] in playlist [%s] have [%d] corresponding purchase item"), 
				ctrlNums[i], playlistId.c_str(), identsByCtrlNum.size());
			return false;
		}
		NS_PREFIX(ChannelOnDemand::PurchaseItemData) data;
		try
		{
			NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx) pPurcharseItemPrx = 
				NS_PREFIX(ChannelOnDemand::PurchaseItemAssocPrx)::checkedCast(_env._adapter->createProxy(identsByCtrlNum[0]));
			data = pPurcharseItemPrx->getData();
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseLog(LOG_MODULE_NAME, "get purchase item [%s] context caught %s"), 
				identsByCtrlNum[0].name.c_str(), ex.ice_name().c_str());
			return false;
		}
		purItemDatas.push_back(data);
	}

	bool isAllSyncOK = true;

	// start the sync process
	int j = 0, k = 0;
	while (j < (int)chnlItemCtxs.size() && k < (int)purItemDatas.size())
	{
		// if name equal, further check if the item has been replaced
		// 判断当前下标指向的ChannelItem name 和 PlaylistItem name相同，则
		// 继续判断lastModified是否相同
		if (chnlItemCtxs[j].key == purItemDatas[k].channelItemKey)
		{
			if (chnlItemCtxs[j].setupInfo.lastModified != purItemDatas[k].lastModified)
			{
				// lastModified不相同说明ChannelItem的属性被修改了
				if (!replacePlaylistItem(purItemDatas[k].channelItemKey, chnlItemCtxs[j], c))
				{
					glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist(%s -> %s) caught replacePlaylistItem(%s -> %s) error, need further syncPlaylist()"), 
						purItemDatas[k].channelItemKey.c_str(), chnlItemCtxs[j].key.c_str(), purItemDatas[k].channelItemKey.c_str(), chnlItemCtxs[j].key.c_str());
					isAllSyncOK = false;
				}
			}
			j ++;
			k ++;
			continue;
		}

		// 如果Playlist item所对应的channel item name与当前的不符
		int i, count;
		bool plInChannel = false; // to indicates whether or not the playlist item is in channel item list
		for (i = j, count = chnlItemCtxs.size(); i < count; i ++)
		{	// notice here the initail value of i is j
			// means the channel item list is a sub list which after the position of current channel item
			if (purItemDatas[k].channelItemKey == chnlItemCtxs[i].key)
			{
				// 但是Playlist item所对应的channel item name确实存在于channel item list中
				// 则说明当恰所指向的channel item是新添加的
				plInChannel = true;
				break;
			}
		}

		if (plInChannel)
		{
			// current channel item is a new item, so we have to insert it to playlist
			if (!insertPlaylistItem(purItemDatas[k].channelItemKey, chnlItemCtxs[j], c))
			{
				glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() caught insertPlaylistItem(%s before %s) error, need further syncPlaylist()"), 
					chnlItemCtxs[j].key.c_str(), purItemDatas[k].channelItemKey.c_str());
				isAllSyncOK = false;
			}
			j ++;
		}
		else 
		{
			// current playlist item is not in channel item list, so we have to erase it
			if (!removePlaylistItem(purItemDatas[k].channelItemKey, c))
			{
				glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() caught removePlaylistItem(%s) error, need further syncPlaylist()"), 
					purItemDatas[k].channelItemKey.c_str());
				isAllSyncOK = false;
			}
			k ++;
		}
	}

	while (j < (int)chnlItemCtxs.size())
	{
		if (!appendPlaylistItem(chnlItemCtxs[j], c))
		{
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() caught appendPlaylistItem(%s) error, need further syncPlaylist()"), 
				chnlItemCtxs[j].key.c_str());
			isAllSyncOK = false;
		}
		j ++;
	}

	while (k < (int)purItemDatas.size())
	{
		if (!removePlaylistItem(purItemDatas[k].channelItemKey, c))
		{
			glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() caught removePlaylistItem(%s) error, need further syncPlaylist()"), 
				purItemDatas[k].channelItemKey.c_str());
			isAllSyncOK = false;
		}
		k ++;
	}

	if (isAllSyncOK)
	{
		glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "syncPlaylist() OK"));
		bNeedSyncChannel = false;
		return true;
	}
	else 
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseLog(LOG_MODULE_NAME, "some error occured when doing current syncPlaylist()"));
		bNeedSyncChannel = true;
		return false;
	}
}

void PurchaseImpl::getCurStreamPos(::Ice::Int& ctrlNum, ::Ice::Int& offset) const
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseLog(LOG_MODULE_NAME, "playlist[%s] current play position required"), playlistId.c_str());
	ctrlNum = 0;
	offset = 0;
	TianShanIce::ValueMap vMap;
	try
	{
		playlist->getInfo(::TianShanIce::Streamer::infoPLAYPOSITION, vMap);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 6400, PurchaseLog(LOG_MODULE_NAME, "perform playlist[%s] getInfo() caught %s:%s"), 
			playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 6401, PurchaseLog(LOG_MODULE_NAME, "perform playlist[%s] getInfo() caught %s"), 
			playlistId.c_str(), ex.ice_name().c_str());
	}

	TianShanIce::ValueMap::iterator ctrlNum_itor = vMap.find("ctrlnumber");
	TianShanIce::ValueMap::iterator offset_itor = vMap.find("playposition");
	if (ctrlNum_itor == vMap.end() || ctrlNum_itor->second.type != TianShanIce::vtInts || ctrlNum_itor->second.ints.size() <= 0 ||
		offset_itor == vMap.end() || offset_itor->second.type != TianShanIce::vtInts || offset_itor->second.ints.size() <= 0)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "Purchase", 6402, PurchaseLog(LOG_MODULE_NAME, "playlist[%s]'s getInfo() didn't get all wanted informaion"), 
			playlistId.c_str());
	}
	else 
	{
		ctrlNum = ctrlNum_itor->second.ints[0];
		offset = offset_itor->second.ints[0];
	}

	glog(ZQ::common::Log::L_INFO, PurchaseLog(LOG_MODULE_NAME, "playlist[%s] current play position. CtrlNum[%d], Offset[%d]"), 
		playlistId.c_str(), ctrlNum, offset);
}

void PurchaseImpl::setProperties(const std::string& key, const std::string& value)
{
	Lock lock(*this);
	properties[key] = value;
}

::TianShanIce::Properties PurchaseImpl::getProperties()
{
	return properties;
}

::TianShanIce::Application::PlaylistInfo PurchaseImpl::getPlaylistInfo(const ::Ice::Current& c) const
{
	::TianShanIce::Application::PlaylistInfo pli;
#pragma message(__MSGLOC__"TODO: impl getPlaylistInfo() here")
	ZQTianShan::_IceThrow <TianShanIce::NotSupported> (glog, EXPFMT(Purchase, 2001, "getPlaylistInfo() not supported yet"));
	return pli;
}

} // namespace ZQChannelOnDemand

