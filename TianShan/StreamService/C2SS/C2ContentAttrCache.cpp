#include "C2ContentAttrCache.h"
#include "C2SSCfg.h"

#include <TianShanIceHelper.h>


//#include "C2SSTest.h"

extern ZQTianShan::C2SS::ST_C2SS::C2SSHolder	*pC2SSBaseConfig;


namespace ZQTianShan{
namespace C2SS{
// -----------------------------
// interface C2LocateQuery
// -----------------------------
// the callback interface to receive obtained c2Client
/*C2LocateQuery::C2LocateQuery(C2ContentAttrCache& owner, const std::string& contentName, ZQ::common::Log& log, std::string& url) */

bool C2LocateQuery::issueLocateRequest()
{
	try{
	  	_locateClientPtr = new ZQ::StreamService::LocateRequest(_log, this, _params);
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery,"issueLocateRequest() create C2ClientAsync to send locate request use content[%s] failed."), _params.contentName.c_str());
	}
	if (NULL == _locateClientPtr)
		return false;

	MAPSET(AttrMap, _params.props, C2CLIENT_ClientTransfer, pC2SSBaseConfig->_httpCRGServerIP);
	try
	{
		if (!_locateClientPtr->process())
		{
			  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery,"issueLocateRequest() send request use content[%s], url[%s] failed."), _params.contentName.c_str(), _params.url.c_str());
			  return false;
		}
		_log(ZQ::common::Log::L_INFO, CLOGFMT(C2LocateQuery,"issueLocateRequest() send request use content[%s], url[%s] successful."), _params.contentName.c_str(), _params.url.c_str());
		return true;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery, "issueLocateRequest() send request use content[%s], url[%s] failed."), _params.contentName.c_str(), _params.url.c_str());
	}
	return false;
}

void C2LocateQuery::OnC2LocateResponse(const std::string& contentName, const AttrMap& locRespParamters)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocateQuery," OnC2LocateResponse() enter with content[%s]."), _params.contentName.c_str());
	//1, dispose the locate response data
	C2ContentQueryBind::AttrMap  attrMap = locRespParamters;
	C2ContentQueryBind::AttrMap::iterator findIter = attrMap.find(C2CLIENT_AssetID);
	if ( attrMap.end() != findIter )
	{
		_ca.assetId = findIter->second;
		attrMap.erase(findIter);
	}

	findIter = attrMap.find(C2CLIENT_ProviderID);
	if (attrMap.end() != findIter)
	{
		_ca.providerId = findIter->second;
		attrMap.erase(findIter);
	}

	findIter = attrMap.find(C2CLIENT_ExtName);
	if (attrMap.end() != findIter)
	{
		_ca.mainFileExtName = findIter->second;
		attrMap.erase(findIter);
	}

	findIter = attrMap.find(C2CLIENT_MuxBitrate);
	if (attrMap.end() != findIter)
	{
		_ca.muxBitrate = atoi(findIter->second.c_str());
		attrMap.erase(findIter);
	}

	findIter = attrMap.find(C2CLIENT_PlayTime);
	if (attrMap.end() != findIter)
	{
		_ca.playTime_msec = atoi(findIter->second.c_str());
		attrMap.erase(findIter);
	}

	findIter = attrMap.find(C2CLIENT_OpenForWrite);
	if (attrMap.end() != findIter)
	{
		std::string pwe = findIter->second;
		if( 0 == pwe.compare("yes"))
			_ca.isPWE = true;
		else
			_ca.isPWE = false;
		attrMap.erase(findIter);
	}

	_ca.stampAsOf = ZQ::common::TimeUtil::now();
	_ca.attrs = attrMap;

	//2, send get request
	std::string transferPort = "";
	findIter = attrMap.find(C2CLIENT_TransferPort);
	if (attrMap.end() != findIter)
		  transferPort = findIter->second.c_str();
	std::string transferID = "";
	findIter = attrMap.find(C2CLIENT_TransferID);
	if (attrMap.end() != findIter)
		  transferID = findIter->second.c_str();
	unsigned int transferNum = 0;
	findIter = attrMap.find(C2CLIENT_PortNum);
	if (attrMap.end() != findIter){
		  transferNum = atoi(findIter->second.c_str());
	}else{
		  transferNum = pC2SSBaseConfig->_httpCRGDefaultGetPort;
	}

	if ( transferPort.empty() || transferNum <= 0)
	{
		  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery," OnC2LocateResponse() locate get the empyt C2CLIENT_TransferPort or error C2CLIENT_PortNum with content[%s]."),_params.contentName.c_str());
		  return;
	}
	try{

        _params.transferID = transferID;
        _params.getAddr = transferPort;
        _params.getPort = transferNum;

		  _getClientPtr = new ZQ::StreamService::GetRequest(_log, this, _params);
	}
	catch(...)
	{
		  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery,"OnC2LocateResponse() create C2ClientAsync to send get request content[%s] failed."), _params.contentName.c_str());
	}
	if (NULL == _getClientPtr)
	{
		  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery,"OnC2LocateResponse() create a NULL C2ClientAsync to send get request content[%s]."), _params.contentName.c_str());
		  return ;
	}

	try
	{
		  if (! _getClientPtr->process())
				  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery,"OnC2LocateResponse() send request content[%s], url[%s] failed."), _params.contentName.c_str(), _params.url.c_str());
		  _log(ZQ::common::Log::L_INFO, CLOGFMT(C2LocateQuery,"OnC2LocateResponse() send request content[%s], url[%s] successful."), _params.contentName.c_str(), _params.url.c_str());
	}
	catch (...)
	{
		  _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery, "issueLocateRequest() send request content[%s], url[%s] failed."), _params.contentName.c_str(), _params.url.c_str());
	}
}

void C2LocateQuery::OnC2GetResponse(const std::string& contentName, const AttrMap& locRespParamters)
{
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocateQuery," OnC2GetResponse() enter with content[%s]."), _params.contentName.c_str());
	  //1, dispose the locate response data
	  C2ContentQueryBind::AttrMap  attrMap = locRespParamters;
	  C2ContentQueryBind::AttrMap::iterator findIter = attrMap.find(C2CLIENT_AssetID);
	  if ( attrMap.end() != findIter )
	  {
			_ca.assetId = findIter->second;
			attrMap.erase(findIter);
	  }

	  findIter = attrMap.find(C2CLIENT_ProviderID);
	  if (attrMap.end() != findIter)
	  {
			_ca.providerId = findIter->second;
			attrMap.erase(findIter);
	  }

	  findIter = attrMap.find(C2CLIENT_ExtName);
	  if (attrMap.end() != findIter)
	  {
			_ca.mainFileExtName = findIter->second;
			attrMap.erase(findIter);
	  }

	  findIter = attrMap.find(C2CLIENT_MuxBitrate);
	  if (attrMap.end() != findIter)
	  {
			_ca.muxBitrate = atoi(findIter->second.c_str());
			attrMap.erase(findIter);
	  }

	  findIter = attrMap.find(C2CLIENT_PlayTime);
	  if (attrMap.end() != findIter)
	  {
			_ca.playTime_msec = atoi(findIter->second.c_str());
			attrMap.erase(findIter);
	  }

	  findIter = attrMap.find(C2CLIENT_OpenForWrite);
	  if (attrMap.end() != findIter)
	  {
			std::string pwe = findIter->second;
			if( 0 == pwe.compare("yes"))
				  _ca.isPWE = true;
			else
				  _ca.isPWE = false;
			attrMap.erase(findIter);
	  }

	  _ca.stampAsOf = ZQ::common::TimeUtil::now();
	  //_ca.attrs = attrMap;
	  ZQTianShan::Util::mergeProperty(_ca.attrs, attrMap, false);
	  {
			ZQ::common::MutexGuard g(_owner._lkAttrCache);
			//set to <contentName, AttrCache> to LRUMap _attrCache 
			_owner._attrCache[contentName] = _ca;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2LocateQuery," OnC2GetResponse() insert content[%s] attr into map[%d] ."), contentName.c_str(), _owner._attrCache.size());
	  }
	  {
			ZQ::common::MutexGuard g(_owner._lkLocateAwaitMap);
			_owner._locateAwaitMap.erase(contentName);
	  }
	  (new C2ContentResultDispatcher(_owner, contentName, _owner._pool))->start();
}

void C2LocateQuery::OnError(int errCode, const std::string& errMsg)
{
	_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocateQuery," OnError() enter with errcode[%d], errmsg[%s], content[%s]."), errCode, errMsg.c_str(), _params.contentName.c_str());
	
	{
		ZQ::common::MutexGuard g(_owner._lkLocateAwaitMap);
		_owner._locateAwaitMap.erase(_params.contentName);
	}

}

/*
void C2LocateQuery::OnC2IndexHeader(std::string& contentName, const uint8* buf, const int len)
{
	// TODO:
	// 1. parse the buffer and build up ContentAttr, then
	// 2 insert it into _attrCache
	//	{
	//	ZQ::common::MutexGuard g(_cache._lkAttrCache);
	//	MAPSET(..., _attrCache, contentName, _ca);
	//	}
	// 3. (new C2ContentResultDispatcher(*this, contentName))->start();
}*/


// -----------------------------
// class C2ContentResultDispatcher
// -----------------------------
// the command module to dispatch associated content attribute to the subscribers
C2ContentResultDispatcher::C2ContentResultDispatcher(C2ContentAttrCache& cache, std::string contentName, ZQ::common::NativeThreadPool& pool)
: _c2ContentCache(cache), _c2ContentName(contentName), ThreadRequest(pool)
{

}

C2ContentResultDispatcher::~C2ContentResultDispatcher()
{

}

int C2ContentResultDispatcher::run()
{
	// step 1. read the await subscribers who interests this content
	C2ContentAttrCache::QueryAwaitList slist;
	{
		ZQ::common::MutexGuard g(_c2ContentCache._lkAwaitMap);
		C2ContentAttrCache::QueryAwaitMap::iterator it = _c2ContentCache._awaitMap.find(_c2ContentName);
		if ( _c2ContentCache._awaitMap.end() == it)
			return 0;

		slist = it->second;
		_c2ContentCache._awaitMap.erase(_c2ContentName);
	}

	// step 2. read the attribute of content from the cache
	C2ContentQueryBind::ContentAttr ca;
	{
		ZQ::common::MutexGuard g(_c2ContentCache._lkAttrCache);
		C2ContentAttrCache::AttrCache::iterator it = _c2ContentCache._attrCache.find(_c2ContentName);
		if (_c2ContentCache._attrCache.end() == it)
			return 1;
		ca = it->second;
	}

	// step 3. call each subsribers
	for (C2ContentAttrCache::QueryAwaitList::iterator it = slist.begin(); it< slist.end(); it++)
	{
		if (NULL == *it)
			continue;

		(*it)->OnContentAttributes(_c2ContentName, ca);
	}

	return 0;
}
// -----------------------------
// class C2ContentAttrCache
// -----------------------------
// the container of C2 content attributes

C2ContentAttrCache::C2ContentAttrCache(ZQ::common::Log& logger,  ZQ::common::NativeThreadPool& pool, const std::string& url)
:_log(logger), ThreadRequest(pool), _url(url)
{
	_attrCache.resize(pC2SSBaseConfig->_c2ContentCacheSize);
}

C2ContentAttrCache::~C2ContentAttrCache()
{
}

bool C2ContentAttrCache::getAttr(std::string& contentName, C2ContentQueryBind::ContentAttr& attr)
{
	ZQ::common::MutexGuard g(_lkAttrCache);
	AttrCache::iterator it = _attrCache.find(contentName);
	if (_attrCache.end() == it)
		return false;
	attr = it->second;
	return true;
}

void C2ContentAttrCache::lookupContent_async(C2ContentQueryBind& cb, const std::string& contentName)
{
	C2ContentQueryBind::ContentAttr ca;
	if(_lookupCacheAndLocate(contentName, ca, &cb))
	{
		cb.OnContentAttributes(contentName, ca);
		return;
	}
	return;

}

bool C2ContentAttrCache::lookupContent_sync(C2ContentQueryBind::ContentAttr& ca, std::string& contentName, int timeout)
{
	if (_lookupCacheAndLocate(contentName, ca, NULL))
		return true;
	_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ContentAttrCache,"lookupContent_sync() look up cache and locate with content[%s] failed."), contentName.c_str());
	return false;
}

//@return true if hit cache
bool C2ContentAttrCache::_lookupCacheAndLocate(const std::string& contentName,  C2ContentQueryBind::ContentAttr& ca, C2ContentQueryBind* pCb/*=NULL*/)
{
	bool bHit=false;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ContentAttrCache,"_lookupCacheAndLocate() enter content[%s]."), contentName.c_str());		
	// step 1. check if the content already cached
	ca.stampAsOf= -1;
	{
		ZQ::common::MutexGuard g(_lkAttrCache);
		AttrCache::iterator it = _attrCache.find(contentName);
		if ( _attrCache.end() != it )
		{
			ca = it->second;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ContentAttrCache,"content[%s] is already cached."), contentName.c_str());
			bHit = true;
		}
	}

	int64 stampNow = ZQ::common::now();
	//if (!bHit || (ca.isPWE || ca.isDirty) && ca.stampAsOf <(stampNow -5000))
	if (( !bHit || ca.isPWE ) && ca.stampAsOf <(stampNow -5000))
	{
		// TODO : create a c2client and issue the LocateRequest
		if (NULL != pCb)//async call
		{
			C2LocateQuery::Ptr pQuery;
			{
				ZQ::common::MutexGuard g(_lkLocateAwaitMap);
				if (_locateAwaitMap.end() != _locateAwaitMap.find(contentName))
					return  false;

                ZQ::StreamService::RequestParams params;
                params.contentName  = contentName;
                params.subType      = "index";
                params.filename     = params.contentName + params.subType;
                params.url = _url;
                params.locateIP     = pC2SSBaseConfig->_httpCRGAddr;
                params.locatePort   = pC2SSBaseConfig->_httpCRGPort;
                params.upstreamIP   = pC2SSBaseConfig->_httpCRGUpStreamIP;
                params.clientTransfer   = pC2SSBaseConfig->_httpCRGServerIP;
                
				pQuery = new C2LocateQuery(*this, _log, params);
				if (pQuery)
					MAPSET(LocateAwaitMap, _locateAwaitMap, contentName, pQuery);
			}
			if (pQuery)
				if (pQuery->issueLocateRequest())
					bHit = false;

		}
		else //sync call
		{
            ZQ::StreamService::C2ClientSync::AttrMap reqMap;
			ZQ::StreamService::C2ClientSync::AttrMap respMap;
			try
			{
				bool locateRet = false;
				ZQ::StreamService::C2ClientSync::Ptr syncC2Client= new ZQ::StreamService::C2ClientSync(_log, pC2SSBaseConfig->_httpCRGAddr, pC2SSBaseConfig->_httpCRGPort);
				if (NULL != syncC2Client)
					locateRet = syncC2Client->sendLocateRequest(_url, contentName, "index",reqMap, respMap);
				if (locateRet)
				{
					std::string transferId, getReqIp;
					int getReqPort = 12000; 
					C2ContentQueryBind::AttrMap::iterator locResIter = respMap.find(C2CLIENT_TransferID);
					if ( respMap.end() != locResIter)
						transferId = locResIter->second;

					locResIter = respMap.find(C2CLIENT_TransferPort);
					if ( respMap.end() != locResIter)
						getReqIp = locResIter->second;

					locResIter = respMap.find(C2CLIENT_PortNum);
					if ( respMap.end() != locResIter && !locResIter->second.empty())
						getReqPort = atoi(locResIter->second.c_str());
					if ( !transferId.empty() && !getReqIp.empty() )
					{
						ZQ::StreamService::C2ClientSync::Ptr getC2Client= new ZQ::StreamService::C2ClientSync(_log, getReqIp, getReqPort);
						if (NULL != getC2Client)
						{
							getC2Client->sendGetRequest(transferId, contentName, reqMap, respMap);
						}
					}
					
				}
				else
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(C2ContentAttrCache,"content[%s] Locate response failed."), contentName.c_str());
				}
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ContentAttrCache,"content[%s] get attr failed."), contentName.c_str());
			}

			if ( !respMap.empty() )
			{
				bool respMapRet = true;
				C2ContentQueryBind::AttrMap::iterator findIter = respMap.find(C2CLIENT_AssetID);
				if ( respMap.end() != findIter )
				{
					ca.assetId = findIter->second;
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				findIter = respMap.find(C2CLIENT_ProviderID);
				if (respMap.end() != findIter)
				{
					ca.providerId = findIter->second;
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				findIter = respMap.find(C2CLIENT_ExtName);
				if (respMap.end() != findIter)
				{
					ca.mainFileExtName = findIter->second;
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				findIter = respMap.find(C2CLIENT_MuxBitrate);
				if (respMap.end() != findIter)
				{
					ca.muxBitrate = atoi(findIter->second.c_str());
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				findIter = respMap.find(C2CLIENT_PlayTime);
				if (respMap.end() != findIter)
				{
					ca.playTime_msec = atoi(findIter->second.c_str());
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				findIter = respMap.find(C2CLIENT_OpenForWrite);
				if (respMap.end() != findIter)
				{
					std::string pwe = findIter->second;
					if( 0 == pwe.compare("yes") )
						ca.isPWE = true;
					else
						ca.isPWE = false;
					respMap.erase(findIter);
				}
				else{
					respMapRet =false;
				}

				ca.stampAsOf = ZQ::common::TimeUtil::now();
				ca.attrs = respMap;
				if(respMapRet)
				{
					ZQ::common::MutexGuard g(_lkAttrCache);
					//set to <contentName, AttrCache> to LRUMap _attrCache 
					_attrCache[contentName] = ca;
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ContentAttrCache," sync locate insert content[%s] attr into cache size[%d] ."), contentName.c_str(), _attrCache.size());
					bHit = true;
				}
			}
		} //sync call
	}


	// put the cb into await list
	if (NULL != pCb && !bHit)
	{
		ZQ::common::MutexGuard g(_lkAwaitMap);
		QueryAwaitMap::iterator it = _awaitMap.find(contentName);
		if (_awaitMap.end() != it)
			it->second.push_back(pCb);
		else
		{
			QueryAwaitList slist;
			slist.push_back(pCb);
			//MAPSET(QueryAwaitMap, _awaitMap, contentName, slist);
			_awaitMap[contentName] = slist;
		}
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ContentAttrCache,"cache the awaitmap for content[%s]."), contentName.c_str());
		bHit = false;
	}

	return bHit;
}

	}//namespace C2SS 
}//namespace ZQTianShan