#include "NGODStorePortal.h"
#include "NSSConfig.h"
#include <set>

extern ::ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig;

namespace ZQTianShan {
namespace NGOD_CS {

// -----------------------------
// class A3Listener
// -----------------------------
// a HTTP listener to receive A3 feedbacks from NGOD StreamingServer

class A3Listener : public SimpleHttpd
{
public:
	A3Listener(const char* localIP, const int& port, ZQ::common::Log* pLog);
	~A3Listener(void);

public:
	int OnHttpMessage(const MsgInfo& info, const std::string& contentBody);
	void setA3Handle(NGODStorePortal* pCtr);

private:
	NGODStorePortal*		_pCtr;
	ZQ::common::Log*		_pLog;
};


// -----------------------------
// class A3Listener implement
// -----------------------------
#define A3RESPLOG  (*_pLog)
A3Listener::A3Listener(const char* localIP, const int& port, ZQ::common::Log* pLog)
:SimpleHttpd(localIP,port,pLog),_pCtr(NULL),_pLog(pLog)
{
}

A3Listener::~A3Listener(void)
{
}

void A3Listener::setA3Handle(NGODStorePortal* pCtr)
{
	_pCtr = pCtr;
}

int A3Listener::OnHttpMessage(const MsgInfo& info, const std::string& contentBody)
{
	assert(_pCtr != NULL);

	if(contentBody.empty()) 
	{
		A3RESPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3Listener, "handleMsg() message is NULL,from [%s:%d]"),info.ip.c_str(), info.port);
		return MSG_INTERNAL_ERR;
	}

	A3RESPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Listener, "OnHttpMessage() Have a  message,uri [%s], from [%s:%d]"), info.uri.c_str(), info.ip.c_str(), info.port);

	std::string strType;
	const char* pType = strrchr(info.uri.c_str(), '/');
	if(pType)
		strType = ++pType;
	else
		strType = info.uri;	

	A3Request::MessageCtx msgData;
	//transfer statues message
	if(stricmp(TYPE_TRANSFER, strType.c_str()) == 0)
	{	
		ContentOprtXml oprtXml(_pLog);
		if(!oprtXml.parseGetTransferStatus(contentBody.c_str(), static_cast<int>(contentBody.size()),msgData)) 
		{
			A3RESPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3Listener, "handleMsg() parse TransferStatus message failed, from [%s:%d]"),info.ip.c_str(), info.port);
			return MSG_INTERNAL_ERR;
		}

		return _pCtr->OnTransferStatus(info, msgData);
	}		
	else if(stricmp(TYPE_CHECKSUM, strType.c_str()) == 0)//content checksum message
	{
		ContentOprtXml oprtXml(_pLog);
		if(!oprtXml.parseContentChecksum(contentBody.c_str(), static_cast<int>(contentBody.size()),msgData)) 
		{
			A3RESPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3Listener, "handleMsg() parse ContentChecksum message failed, from [%s:%d]"), info.ip.c_str(), info.port);
			return MSG_INTERNAL_ERR;
		}
		
		return _pCtr->OnContentChecksum(info, msgData);
	}	
	else
	{
		A3RESPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3Listener, "handleMsg() do not know the message type [%s],from [%s:%d]"),strType.c_str(), info.ip.c_str(), info.port);
		return MSG_INTERNAL_ERR;
	}

}

// -----------------------------
// command class VolumeSyncCmd
// -----------------------------
class VolumeSyncCmd : public ZQ::common::ThreadRequest
{
public:
	VolumeSyncCmd(ZQ::common::NativeThreadPool& thpool,
		NGODStorePortal& csp,
		const std::string& volumeName)
		: ThreadRequest(thpool), _csp(csp), _volName(volumeName)
	{
	}

protected:
	NGODStorePortal&   _csp;
	std::string _volName, _targetName;

protected:
	virtual bool init(void)
	{
		ZQ::common::MutexGuard g(_csp._lockVolumes);
		NGODStorePortal::VolumeMap::iterator it = _csp._volumeMap.find(_volName);
		if (_csp._volumeMap.end() == it)
			return false;

		_targetName = it->second.targetName;
		return true;
	}
	virtual void final(int retcode =0, bool bCancelled =false) { delete this; }
	virtual int run(void)
	{
		int64 stampNow = ZQTianShan::now(), stampLastSync =0;
		int64 stampStart = stampNow;

		{
			// step 1. validate this sync command
			ZQ::common::MutexGuard gd(_csp._lockVolumes); // the locker of _volumeMap
			if (_csp._bQuit)
				return -9;

			NGODStorePortal::VolumeMap::iterator it = _csp._volumeMap.find(_volName);
			if (_csp._volumeMap.end() == it)
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "unknown volume[%s], quit sync"), _volName.c_str());
				return -1;
			}

			stampLastSync = it->second.stampLastSync;

			if (it->second.stampLastTry > it->second.stampLastSync && (stampNow - it->second.stampLastTry) < CONTENT_SYNC_TIMEOUT_MINUTE)
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "volume[%s] sync is recently performed or on-going, quit this sync"), _volName.c_str());
				return 0;
			}

			it->second.stampLastTry = stampNow;
		}

		try 
		{
			// step 2. GetVolumeInfo
			A3Request::MessageCtx  msgData;
			msgData.params["volumeName"] = _targetName;
			_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "quering GetVolumeInfo for volume[%s]->[%s]"), _volName.c_str(), _targetName.c_str());

			A3Request A3Req(_csp);
			int ret = A3Req.request(A3Request::A3_GetVolumeInfo, msgData);
			if (!HTTP_SUCC(ret)) // && atoi(msgData.params["state"].c_str()) == A3Request::OPERATIONAL
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "GetVolumeInfo for volume[%s] failed: %s"), _volName.c_str(), A3Req.getStatusMessage().c_str());
				return -2;
			}
		}
		catch (...)
		{
			_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "GetVolumeInfo() volume[%s]->[%s] caught exception"), _volName.c_str(), _targetName.c_str());
		}

		// step 3. GetContentInfo
		NGODStorePortal::ContentMap oldContents, newContents;
		int64 stampSync=0;
		{
			_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "quering GetContentInfo of volume[%s]->[%s]"), _volName.c_str(), _targetName.c_str());

			A3Request::MessageCtx msgCInfo;
			msgCInfo.params["volumeName"] = _targetName;

			// step 3.1 GetContentInfo query
			{
				A3Request A3Req(_csp);
				int errorCode = A3Req.request(A3Request::A3_GetContentInfo, msgCInfo);
				if( errorCode < 200 || errorCode >= 300)
				{
					_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "GetContentInfo for volume[%s] failed: %s"), _volName.c_str(), A3Req.getStatusMessage().c_str());
					return -3;
				}
			} // free A3Req that may be very large and no more useful

			stampSync = now();

			// step 3.2 build up the newContents
			for(std::vector< ::TianShanIce::Properties>::iterator it = msgCInfo.table.begin(); it < msgCInfo.table.end(); it++)
			{
				// step 3.2.1 construct the NGODStorePortal::ContentInfo record
				NGODStorePortal::ContentInfo newCI;
				newCI.name = (*it)["assetID"] + "_" + (*it)["providerID"];
				newCI.contentSize     = _atoi64((*it)["contentSize"].c_str()) *1024; // A3 ContentInfo message takes KB here, convert it to Byte 
				newCI.supportFileSize = _atoi64((*it)["supportFileSize"].c_str()) *1024; // A3 ContentInfo message takes KB here, convert it to Byte 
				newCI.stampCreated    = ZQTianShan::ISO8601ToTime((*it)["createDate"].c_str());
				newCI.md5Checksum     = (*it)["md5Checksum"];
				newCI.stampMD5        = ZQTianShan::ISO8601ToTime((*it)["md5DateTime"].c_str());
				newCI.contentState    = (*it)["contentState"];
				newCI.stampFromServer = stampSync;

				newContents.insert(NGODStorePortal::ContentMap::value_type(newCI.name, newCI));
			}
		}

		_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "volume[%s] server returned %d contents, merge them into DB"), _volName.c_str(), newContents.size());
		if (newContents.size() <=0 && oldContents.size() >0)
		{  
			// enh for ticket#11870, the connected NGOD server from other vendor may respond an empty content list while startup.
			// this protection is to avoid wipe over the entire volume of NSS
			_csp._store._log(ZQ::common::Log::L_WARNING, CLOGFMT(VolumeSyncCmd, "volume[%s] server returned %d contents but local has %d, ignore this round of sync as it may be a questioning response from the server"), _volName.c_str(), newContents.size(), oldContents.size());
			return -10;
		}

		// step 4. Update the contents of volumes
		{
			ZQ::common::MutexGuard gd(_csp._lockVolumes); // the locker of _volumeMap
			if (_csp._bQuit)
				return -9;

			if (_csp._volumeMap.end() == _csp._volumeMap.find(_volName))
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "failed to lock volume[%s] after receive the result of query, give up"), _volName.c_str());
				return -4;
			}

			NGODStorePortal::VolumeInfo& vi = _csp._volumeMap[_volName];

			// step 4.1 backup the old contents and replace with the new contents
			for (NGODStorePortal::ContentMap::iterator itOld = vi.contents.begin(); itOld != vi.contents.end(); itOld++)
			{
				oldContents.insert(*itOld); // backup contents to oldContents

				if (itOld->second.stampCreated < (stampStart -1000) || newContents.end() != newContents.find(itOld->first))
					continue;

				// content recently added but not shown on the newContents returned from VideoServer
				// append it into tne newContents
				newContents.insert(*itOld);
			}

			vi.contents = newContents; // flush with newContents

			// step 4.2 stamp this sync procedure
			vi.stampLastSync = stampSync;
		}

		if (stampLastSync < _csp._stampStarted) // done if this is the first sync
		{
			_csp._hStartList.signal();
			_csp._store._log(ZQ::common::Log::L_INFO, CLOGFMT(VolumeSyncCmd, "volume[%s] sync completed per this is a fresh after start: new=%d"), _volName.c_str(), newContents.size());
			return 0;
		}

		// step 5. Determine and trigger file events
		_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "volume[%s] comparing the contents: old=%d, new=%d"), _volName.c_str(), oldContents.size(), newContents.size());

		::TianShanIce::Properties params;
		::Ice::Current c = ::Ice::Current();
		c.ctx.insert(::Ice::Context::value_type("src", "NGOD_VolumeSyncCmd"));

		for (NGODStorePortal::ContentMap::iterator itNew = newContents.begin(); itNew != newContents.end(); itNew++)
		{
			std::string contFullName = _volName + FNSEPS + itNew->first;
			NGODStorePortal::ContentMap::iterator itOld = oldContents.find(itNew->first);
			if (oldContents.end() == itOld)
			{
				_csp._store.OnFileEvent(::TianShanIce::Storage::fseFileCreated, contFullName, params, c);
				continue;
			}

			// reach here if the following content exists in oldContents
			if (itNew->second.diff(itOld->second))
			{
				_csp._store.OnFileEvent(::TianShanIce::Storage::fseFileModified, contFullName, params, c);
//				continue;
			}

			oldContents.erase(itNew->first); // to leave a set of "file-deleted"
		}

		for (NGODStorePortal::ContentMap::iterator it = oldContents.begin(); it != oldContents.end(); it++)
		{
			// reach here if the following content ONLY exists in oldContents, appear as deleted
			_csp._store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, _volName + FNSEPS + it->first, params, c);
		}

		_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "volume[%s] complete sync, took [%lld] msec "), _volName.c_str(), ZQTianShan::now() - stampNow);
		return 0;
	}
};

// -----------------------------
// class NGODStorePortal
// -----------------------------
#define cslog (_store._log)

NGODStorePortal::NGODStorePortal(ZQTianShan::ContentStore::ContentStoreImpl& store, size_t poolSize, int minInterval)
: _hStartList(), _store(store), _bQuit(false), _thpool((uint)(poolSize&0xffff)), ThreadRequest(_thpool), _interval(minInterval), _pWebResp(NULL)
{
	_stampStarted = ZQTianShan::now();

	if (poolSize <2)
		_thpool.resize(5);

	if (_interval >0 && _interval < 10*1000)
		_interval = 3600*1000;

	_strIP = pNSSBaseConfig->_videoServer.FeedbackIp;
	_port = pNSSBaseConfig->_videoServer.FeedbackPort;

	char chport[10] = {0};
	sprintf(chport, "%d", pNSSBaseConfig->_videoServer.ContentInterfacePort);

	_defaultA3Url = pNSSBaseConfig->_videoServer.ContentInterfaceIp + ":" + chport + "/" 
		           + pNSSBaseConfig->_videoServer.ContentInterfacePath;

	// enh#17598 - For CacheServer's ContentEdge/ secondary ContentEdge, NSS to enable dual interface to A3 server
	if (!pNSSBaseConfig->_videoServer.ContentInterface2ndIp.empty())
	{
		if (pNSSBaseConfig->_videoServer.ContentInterface2ndPort <=0)
			pNSSBaseConfig->_videoServer.ContentInterface2ndPort = pNSSBaseConfig->_videoServer.ContentInterfacePort;

		sprintf(chport, "%d", pNSSBaseConfig->_videoServer.ContentInterface2ndPort);
		_2ndA3Url = pNSSBaseConfig->_videoServer.ContentInterface2ndIp + ":" + chport + "/" 
			       + pNSSBaseConfig->_videoServer.ContentInterfacePath;
	}
}

NGODStorePortal::~NGODStorePortal()
{
	if (!_bQuit)
		quit();
}

bool NGODStorePortal::init(void)
{
	if(_pWebResp == NULL)
		_pWebResp = new A3Listener(_strIP.c_str(),_port,&(_store._log));
	if(_pWebResp == NULL)
		return false;

	_pWebResp->setA3Handle(this);

	return _pWebResp->start();
}

void NGODStorePortal::quit(void)
{
	_bQuit = true;
	wakeup();
	_hStartList.signal();

	_thpool.stop();

	if(_pWebResp != NULL)
	{
		_pWebResp->close();
		delete _pWebResp;
		_pWebResp = NULL;
	}
}

void NGODStorePortal::wakeup(void)
{
	_hWakeUp.signal();
}

std::string NGODStorePortal::getA3Url()
{
	return _defaultA3Url;
}

std::string NGODStorePortal::getResponseAddr()
{
	char buf[256] = {0};
	sprintf(buf, "%s:%d", _strIP.c_str(), _port);
	return buf;
}

int NGODStorePortal::run(void)
{	
	// the responsibility of this thread is to periodically sync the content of volume with NGOD Streaming server thru A3 messaging
		
	while(!_bQuit)
	{
		// step 1. determine the dirty volumes
		::std::set< ::std::string> dirtyVolumes;
		if (_interval >0)
		{
			ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
			for (NGODStorePortal::VolumeMap::iterator it = _volumeMap.begin(); it != _volumeMap.end(); it++)
			{
				if (it->second.stampLastSync < _stampStarted || ZQTianShan::now() - it->second.stampDirty >= _interval)
				{
					it->second.stampDirty = ZQTianShan::now();
					dirtyVolumes.insert(it->first);
				}
			}

			if (dirtyVolumes.size() >0)
				cslog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "found [%d] dirty volumes of [%d] total volumes"), dirtyVolumes.size(), _volumeMap.size());
		}

		// step 2. start the sync thread
		for (::std::set<std::string>::iterator it = dirtyVolumes.begin(); !_bQuit && it != dirtyVolumes.end(); it++)
		{
			try {
				// TODO: sync with NGOD SS
				(new VolumeSyncCmd(_thpool, *this, *it))->start();
			}
			catch (...) {}
		}

		if (_bQuit)
			break;

		_hWakeUp.wait(_interval >0 ? _interval : TIMEOUT_INF);
	}

	return 0;
}

int NGODStorePortal::OnTransferStatus(const SimpleHttpd::MsgInfo& info, A3Request::MessageCtx& msgData)
{	
	cslog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], content status providerID[%s], assetID[%s], volumeName[%s], state[%s], percentComplete[%s%%]"), 
		info.ip.c_str(), msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str(), msgData.params["volumeName"].c_str(), msgData.params["state"].c_str(), msgData.params["percentComplete"].c_str());

	std::string contentName = msgData.params["assetID"] + "_" + msgData.params["providerID"];
	std::string targetVolName = msgData.params["volumeName"];
	std::string volName;

	ContentInfo oldCI;
	
	{
		ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
		if (_volTargetNameIdx.end() == _volTargetNameIdx.find(targetVolName))
		{
			_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], unknown target volume[%s]"), info.ip.c_str(), targetVolName.c_str());
			return MSG_UNKNOWN_PAID;
		}

		volName = _volTargetNameIdx[targetVolName];
		VolumeMap::iterator itVol = _volumeMap.find(volName);
		if (_volumeMap.end() == itVol)
		{
			_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], couldnot find volume record for target volume[%s]"), info.ip.c_str(), targetVolName.c_str());
			return MSG_UNKNOWN_PAID;
		}

		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], associated volume[%s] for target[%s]"), info.ip.c_str(), volName.c_str(), targetVolName.c_str());
		ContentMap::iterator itCont = itVol->second.contents.find(contentName);
		if (itVol->second.contents.end() == itCont)
		{
			ContentInfo addCI;
			addCI.name = contentName;
			itVol->second.contents.insert(NGODStorePortal::ContentMap::value_type(addCI.name, addCI));
			itCont = itVol->second.contents.find(contentName);

	//		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], content[%s] not found in volume[%s]"), info.ip.c_str(), contentName.c_str(), msgData.params["volumeName"].c_str());
	//		return MSG_UNKNOWN_PAID;
		}

		ContentInfo& ci = itCont->second;
		oldCI = ci; // make a backup

		// update the CI
		ci.contentState = msgData.params["state"];

		if (!msgData.params["contentSize"].empty())
			ci.contentSize = _atoi64(msgData.params["contentSize"].c_str()) *1024; // A3 TransferStatus takes KB here, convert it to Byte 
		if (!msgData.params["supportFileSize"].empty())
			ci.supportFileSize = _atoi64(msgData.params["supportFileSize"].c_str()) *1024; // A3 TransferStatus takes KB here, convert it to Byte 
		if (!msgData.params["bitrate"].empty())
			ci.bitRate = atoi(msgData.params["bitrate"].c_str());

		if (!msgData.params["md5Checksum"].empty())
			ci.md5Checksum = msgData.params["md5Checksum"];
		if (!msgData.params["md5DateTime"].empty())
			ci.stampMD5 = ZQTianShan::ISO8601ToTime(msgData.params["md5DateTime"].c_str());
		

		bool bNeedPopulate = (0 != oldCI.contentState.compare(ci.contentState));
		if (bNeedPopulate)
			_store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], volume[%s] content[%s] state change detected: %s->%s"), info.ip.c_str(),
			msgData.params["volumeName"].c_str(), contentName.c_str(), oldCI.contentState.c_str(), ci.contentState.c_str());
	}
/*
	// processing the special new states
	::TianShanIce::Properties params;
	if (0 == ci.contentState.compare(A3_STATE_TRANSFER) && !bNeedPopulate)
	{
		int64 maxStep = ci.bitRate * (20 * 60 /8);// 20min
		if (maxStep <=0)
			maxStep = 3750000 * (20*60/8); // assume it as 3.75Mbps SD

		if (abs(ci.contentSize - oldCI.contentSize) > maxStep)
			bNeedPopulate = true;
	}
	else if (0 == ci.contentState.compare(A3_STATE_FAILED) || 0 == ci.contentState.compare(A3_STATE_CANCELED))
	{
		char buf[1024] = "\0";
		snprintf(buf, sizeof(buf)-2, "50%03s", msgData.params["reasonCode"].c_str());
		params["sys.LastError"] = buf;
		snprintf(buf, sizeof(buf)-2, "Server notified %s(%03s)", ci.contentState.c_str(), msgData.params["reasonCode"].c_str());
		params["sys.LastErrMsg"] = buf;

		itVol->second.contents.erase(itCont);

		_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, msgData.params["volumeName"] + "\\" + contentName, params, ::Ice::Current());
		return MSG_OK;
	}
	
	if (bNeedPopulate)
		_store.OnFileEvent(::TianShanIce::Storage::fseFileModified, msgData.params["volumeName"] + "\\" + contentName, params, ::Ice::Current());
*/
	::TianShanIce::Properties csProEvent;
	::TianShanIce::Storage::ProvisionEvent	proEvt;
	std::string strTransferState = msgData.params["state"];

	if (strTransferState == A3_STATE_PENDING)
		return MSG_OK; // do nothing

	if (strTransferState == A3_STATE_TRANSFER)
	{
		// FT: some VSS will report TransferStatus reversly by accident, which could mess up NSS
		if (oldCI.contentState == A3_STATE_COMPLETE)
		{
			_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus() content[%s] ignore illegal state change: %s->%s"),
				contentName.c_str(), oldCI.contentState.c_str(), strTransferState.c_str());
			return MSG_OK;
		}

		proEvt = ::TianShanIce::Storage::peProvisionStarted;
		if (oldCI.contentState == strTransferState)
		{
			proEvt = ::TianShanIce::Storage::peProvisionProgress;
			csProEvent["sys.ProgressTotal"] = "100";
			csProEvent["sys.ProgressProcessed"] = msgData.params["percentComplete"];
		}
	}
	else if ( strTransferState == A3_STATE_STREAMABLE)
	{
		// FT: some VSS will report TransferStatus reversly by accident, which could mess up NSS
		if (oldCI.contentState == A3_STATE_COMPLETE)
		{
			_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus() content[%s] ignore illegal state change: %s->%s"),
				contentName.c_str(), oldCI.contentState.c_str(), strTransferState.c_str());
			return MSG_OK;
		}

		proEvt = ::TianShanIce::Storage::peProvisionProgress;
		csProEvent["sys.ProgressTotal"] = "100";
		csProEvent["sys.ProgressProcessed"] = msgData.params["percentComplete"];

		if (oldCI.contentState != strTransferState)
		{
			proEvt = ::TianShanIce::Storage::peProvisionStreamable;
			if (_store._streamableLength >0)
			{
				std::string playTime = msgData.params["playTime"];

				if (playTime.empty() || atoi(playTime.c_str()) <= _store._streamableLength)
				{
					char tmp[64];
					sprintf(tmp, "%d", _store._streamableLength + 1000);
					playTime = tmp;
				}

				csProEvent[METADATA_PlayTime] = playTime;

				_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "OnTransferStatus() %s content[%s] set " METADATA_PlayTime "[%s] per streamableLength[%d]"),
					strTransferState.c_str(), contentName.c_str(), playTime.c_str(), _store._streamableLength);
			}
		}
	}
	else if (strTransferState == A3_STATE_COMPLETE)
	{
		proEvt = ::TianShanIce::Storage::peProvisionStopped;
		char tmp[128];
		sprintf(tmp, "%lld", _atoi64(msgData.params["contentSize"].c_str()) *1024); // A3 ContentInfo takes KB here, convert it to Byte 
		csProEvent[METADATA_FileSize]      = tmp;
		csProEvent["sys.ProgressTotal"]     = tmp;
		csProEvent["sys.ProgressProcessed"] = tmp;

		csProEvent[METADATA_PlayTime] = msgData.params["playTime"];
		sprintf(tmp, "%lld", _atoi64(msgData.params["supportFileSize"].c_str()) *1024); // A3 ContentInfo takes KB here, convert it to Byte 
		csProEvent[METADATA_SupportFileSize] = tmp;
		csProEvent[METADATA_MD5CheckSum] = msgData.params["md5Checksum"];
		csProEvent[METADATA_BitRate] = msgData.params["bitrate"];
	}
	else if (strTransferState == A3_STATE_CANCELED || strTransferState == A3_STATE_FAILED)
	{
		proEvt = ::TianShanIce::Storage::peProvisionStopped;
		std::string reasonCode = msgData.params["reasonCode"];
		if (reasonCode.empty())
			reasonCode = "500";

		csProEvent["sys.LastError"] = reasonCode;

		char buf[1024] = "\0";
		snprintf(buf, sizeof(buf)-2, "Server Notified[%s]: %s", strTransferState.c_str(), reasonCode.c_str());
		csProEvent["sys.LastErrMsg"] = buf;
	}
	else
	{
		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnTransferStatus(), unkown state: %s"), strTransferState.c_str());
		return MSG_OK;
	}

	try
	{
		_store.OnProvisionEvent(proEvt, _store._netId, volName, contentName, csProEvent, ::Ice::Current());
	}
	catch(const ::Ice::Exception& ex)
	{
		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "content[%s] OnProvisionEvent() vol[%s] caught exception[%s] in OnTransferStatus"), contentName.c_str(), volName.c_str(), ex.ice_name().c_str());
	}

	return MSG_OK;
}

int NGODStorePortal::OnContentChecksum(const SimpleHttpd::MsgInfo& info, A3Request::MessageCtx& msgData)
{
	cslog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "OnContentChecksum() from client[%s], content checksum providerID[%s], assetID[%s], volumeName[%s], checksum[%s], md5DateTime[%s], resultCode[%s]"), 
		info.ip.c_str(), msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str(), msgData.params["volumeName"].c_str(), msgData.params["md5Checksum"].c_str(), msgData.params["md5DateTime"].c_str(), msgData.params["resultCode"].c_str());

	std::string contentName = msgData.params["assetID"] + "_" + msgData.params["providerID"];
	std::string targetVolName = msgData.params["volumeName"];
	
	ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
	if (_volTargetNameIdx.end() == _volTargetNameIdx.find(targetVolName))
	{
		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnContentChecksum() from client[%s], unknown target volume[%s]"), info.ip.c_str(), targetVolName.c_str());
		return MSG_UNKNOWN_PAID;
	}

	std::string& volName = _volTargetNameIdx[targetVolName];

	VolumeMap::iterator itVol = _volumeMap.find(volName);
	if (_volumeMap.end() == itVol)
	{
		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnContentChecksum() from client[%s], could not find volume record for target volume[%s]"), info.ip.c_str(), targetVolName.c_str());
		return MSG_UNKNOWN_PAID;
	}

	_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "OnTransferStatus() from client[%s], associated volume[%s] for target[%s]"), info.ip.c_str(), volName.c_str(), targetVolName.c_str());
	ContentMap::iterator itCont = itVol->second.contents.find(contentName);
	if (itVol->second.contents.end() == itCont)
	{
		_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "OnContentChecksum() from client[%s], content[%s] not found in volume[%s]"), info.ip.c_str(), contentName.c_str(), msgData.params["volumeName"].c_str());
		return MSG_UNKNOWN_PAID;
	}

	itCont->second.md5Checksum = msgData.params["md5Checksum"];
	if (!msgData.params["md5DateTime"].empty())
		itCont->second.stampMD5 = ZQTianShan::ISO8601ToTime(msgData.params["md5DateTime"].c_str());

	::TianShanIce::Properties params;
	_store.OnFileEvent(::TianShanIce::Storage::fseFileModified, targetVolName + "\\" + contentName, params, ::Ice::Current());
	
	return MSG_OK;
}

// query A3 for an individual content
bool NGODStorePortal::queryInfoByContent(ContentInfo& ci, const std::string& contentName, const std::string& volumeName, bool byTargetVol)
{
	std::string contName = contentName;
	std::string volName = volumeName;
	size_t pos = contName.find_last_of(LOGIC_FNSEPS FNSEPS);

	if (std::string::npos != pos)
		contName = contName.substr(pos+1);
	if (contName.empty())
		return false;

	std::string a3volume;
	{
		// step 1. validate this sync command
		ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
		if (_bQuit)
			return false;

		if (byTargetVol)
		{
			// convert target Volname to VolName
			TianShanIce::Properties::iterator it = _volTargetNameIdx.find(volName);
			if (_volTargetNameIdx.end() != it)
				volName = it->second;
		}

		NGODStorePortal::VolumeMap::iterator it = _volumeMap.find(volName);
		if (_volumeMap.end() == it)
		{
			_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "queryInfoByContent() unknown volume[%s]"), volName.c_str());
			return false;
		}

		a3volume = it->second.targetName;
		ContentMap::iterator itCont = it->second.contents.find(contName);
		if (it->second.contents.end() != itCont && itCont->second.stampFromServer > (ZQTianShan::now() - CONTENT_ATTR_TTL_MINUTE))
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "queryInfoByContent() found cached info for content[%s] of volume[%s]"), contName.c_str(), a3volume.c_str());
			ci = itCont->second;
			return true;
		}
	}

	// step 1 determin PID/PAID from the contentName
	std::string PID, PAID= contName;
	pos = PAID.find('_');
	if (std::string::npos != pos)
	{
		PID  = PAID.substr(pos+1);
		PAID = PAID.substr(0, pos);
	}

	// step 2 perform the A3::GetContentInfo query
	_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "queryInfoByContent() issuing GetContentInfo for volume[%s] PID[%s] PAID[%s]"), a3volume.c_str(), PID.c_str(), PAID.c_str());
	A3Request::MessageCtx msgCInfo;
	msgCInfo.params["volumeName"] = a3volume;
	msgCInfo.params["providerID"] = PID;
	msgCInfo.params["assetID"]    = PAID;

	std::string a3url = _defaultA3Url;
	int64 stampStart = ZQTianShan::now();
	{
		A3Request A3Req(*this);
		int errorCode = A3Req.request(A3Request::A3_GetContentInfo, msgCInfo, pNSSBaseConfig->_videoServer.ContentInterfaceHttpTimeOut);

		if( errorCode < 200 || errorCode >= 300)
		{
			_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "GetContentInfo for volume[%s] PID[%s] PAID[%s] failed: %s"), a3volume.c_str(), PID.c_str(), PAID.c_str(), A3Req.getStatusMessage().c_str());
			return false;
		}
	} // free A3Req that may be very large and no more useful 

	// step 3 scan the response for the ContentInfo
	for (std::vector< ::TianShanIce::Properties>::iterator it = msgCInfo.table.begin(); !_bQuit && it < msgCInfo.table.end(); it++)
	{
		if (0 != PAID.compare((*it)["assetID"]) || 0 != PID.compare((*it)["providerID"]))
			continue;

		ci.name = (*it)["assetID"] + "_" + (*it)["providerID"];
		ci.contentSize     = _atoi64((*it)["contentSize"].c_str()) *1024; // A3 ContentInfo message takes KB here, convert it to Byte 
		ci.supportFileSize = _atoi64((*it)["supportFileSize"].c_str()) *1024; // A3 ContentInfo message takes KB here, convert it to Byte 
		ci.stampCreated    = ZQTianShan::ISO8601ToTime((*it)["createDate"].c_str());
		ci.md5Checksum     = (*it)["md5Checksum"];
		ci.stampMD5        = ZQTianShan::ISO8601ToTime((*it)["md5DateTime"].c_str());
		ci.contentState    = (*it)["contentState"];

		ci.stampFromServer = ZQTianShan::now();
		
		// update this ContentInfo into the _volumeMap
		do {
			ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
			if (_bQuit)
				break;

			NGODStorePortal::VolumeMap::iterator itVol = _volumeMap.find(volName);
			if (_volumeMap.end() == itVol)
				break;

			MAPSET(ContentMap, itVol->second.contents, contName, ci);

		} while(0);

		_store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "queryInfoByContent() got info of content[%s] of volume[%s] from A3, took %dmsec"), contName.c_str(), a3volume.c_str(), (int)(ci.stampFromServer - stampStart));
		return true;
	}

	// didn't find the matched content info in the response
	_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "GetContentInfo for volume[%s] PID[%s] PAID[%s] failed: A3 no matched content, took %dmsec"), a3volume.c_str(), PID.c_str(), PAID.c_str(), (int)(ci.stampFromServer - stampStart));
	return false;
}

} // namespaces
}
