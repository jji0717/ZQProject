#include "GBCSportal.h"
#include "GBCSConfig.h"
#include <set>

#include "GBCSReq.h"
#include "GBCSa4Cmd.h"
#include "GBCSa5Cmd.h"

extern ::ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder *pGBCSBaseConfig;

namespace ZQTianShan {
namespace NGOD_CS {

// -----------------------------
// class PortalListener
// -----------------------------
// a HTTP listener to receive portal feedbacks

GlobalReqType::reqCmd  GlobalReqType::_reqCmdType = GlobalReqType::GB_A4_REQ;

class PortalListener : public SimpleHttpd
{
public:
	PortalListener(const char* localIP, const int& port, ZQ::common::Log* pLog);
	~PortalListener(void);

public:
	int OnHttpMessage(const MsgInfo& info, std::string& respHttpMessageBody);
	void setPortalHandle(GBCSStorePortal* pCtr);

private:
	GBCSStorePortal*		_pCtr;
	ZQ::common::Log*		_pLog;
};


typedef PortalListener A3Listener;
typedef PortalListener NgbA4Listener;
typedef PortalListener NgbA5Listener;

// -----------------------------
// class PortalListener implement
// -----------------------------
#define A3RESPLOG  (*_pLog)
PortalListener::PortalListener(const char* localIP, const int& port, ZQ::common::Log* pLog)
:SimpleHttpd(localIP,port,pLog),_pCtr(NULL),_pLog(pLog)
{
}

PortalListener::~PortalListener(void)
{
}

void PortalListener::setPortalHandle(GBCSStorePortal* pCtr)
{
	_pCtr = pCtr;
}

int PortalListener::OnHttpMessage(const MsgInfo& info, std::string& respHttpMessageBody)
{
	assert(_pCtr != NULL);

	if(info.msgBody.empty()) 
	{
		(*_pLog)(ZQ::common::Log::L_ERROR,"A4Listener::handleMsg() message is NULL,from [%s:%d]",info.ip.c_str(), info.port);
		return MSG_INTERNAL_ERR;
	}

	(*_pLog)(ZQ::common::Log::L_DEBUG,"PortalListener::OnHttpMessage() Have a  message[%s],uri [%s], from [%s:%d]", info.msgBody.c_str(), info.uri.c_str(), info.ip.c_str(), info.port);

	std::string strType;
	const char* pType = strrchr(info.uri.c_str(), '/');
	if(pType)
		strType = ++pType;
	else
		strType = info.uri;	

	return _pCtr->OnTransferStatus(info, respHttpMessageBody);
}

// -----------------------------
// command class VolumeSyncCmd
// -----------------------------
class VolumeSyncCmd : public ZQ::common::ThreadRequest
{
public:
	VolumeSyncCmd(ZQ::common::NativeThreadPool& thpool,
		GBCSStorePortal& csp,
		const std::string& volumeName)
		: ThreadRequest(thpool), _csp(csp), _volName(volumeName)
	{
	}

protected:
	GBCSStorePortal&   _csp;
	std::string _volName, _targetName;

protected:
	virtual bool init(void)
	{
		ZQ::common::MutexGuard g(_csp._lockVolumes);
		GBCSStorePortal::VolumeMap::iterator it = _csp._volumeMap.find(_volName);
		if (_csp._volumeMap.end() == it)
			return false;

		_targetName = it->second.targetName;
		return true;
	}
	virtual void final(int retcode =0, bool bCancelled =false) { delete this; }
	virtual int run(void)
	{
		int64 stampNow = ZQTianShan::now(), stampLastSync =0;

		{
			// step 1. validate this sync command
			ZQ::common::MutexGuard gd(_csp._lockVolumes); // the locker of _volumeMap
			if (_csp._bQuit)
				return -9;

			GBCSStorePortal::VolumeMap::iterator it = _csp._volumeMap.find(_volName);
			if (_csp._volumeMap.end() == it)
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "unknown volume[%s], quit sync"), _volName.c_str());
				return -1;
			}

			stampLastSync = it->second.stampLastSync;

			if (it->second.stampLastTry > it->second.stampLastSync && (stampNow - it->second.stampLastTry) < 10 * 60 *1000)
			{
				_csp._store._log(ZQ::common::Log::L_ERROR, CLOGFMT(VolumeSyncCmd, "volume[%s] sync is recently performed or on-going, quit this sync"), _volName.c_str());
				return 0;
			}

			it->second.stampLastTry = stampNow;
		}

        _csp._store._log(ZQ::common::Log::L_WARNING, CLOGFMT(VolumeSyncCmd, "volume[%s] sync, took [%lld] msec, NGB A4 and A5 not implement"), _volName.c_str(), ZQTianShan::now() - stampNow);

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

			GBCSStorePortal::VolumeInfo& vi = _csp._volumeMap[_volName];

			// step 4.2 stamp this sync procedure
			vi.stampLastSync = now();
		}
		//TODO: make sync one by one, but it may be a lot of cost. as gb a4/a5 have not command like a3 A3_GetVolumeInfo
		if (stampLastSync < _csp._stampStarted) // done if this is the first sync
		{
			_csp._hStartList.signal();
			_csp._store._log(ZQ::common::Log::L_INFO, CLOGFMT(VolumeSyncCmd, "volume[%s] sync completed per this is a fresh after start"), _volName.c_str());
			return 0;
		}

		_csp._store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(VolumeSyncCmd, "volume[%s] complete sync, took [%lld] msec "), _volName.c_str(), ZQTianShan::now() - stampNow);
		return 0;
	}
};

// -----------------------------
// class GBCSStorePortal
// -----------------------------
#define cslog (_store._log)

GBCSStorePortal::GBCSStorePortal(ZQTianShan::ContentStore::ContentStoreImpl& store, size_t poolSize, int minInterval)
: _hStartList(), _store(store), _bQuit(false), _thpool(poolSize), ThreadRequest(_thpool), _interval(minInterval), _pWebResp(NULL)
{
	_stampStarted = ZQTianShan::now();

	if (poolSize <2)
		_thpool.resize(5);

	if (_interval >0 && _interval < 10*1000)
		_interval = 3600*1000;

	_strIP = pGBCSBaseConfig->_videoServer.FeedbackIp;
	_port = pGBCSBaseConfig->_videoServer.FeedbackPort;

	char chport[10] = {0};
	sprintf(chport, "%d", pGBCSBaseConfig->_videoServer.ContentInterfacePort);

	_defaultA3Url = pGBCSBaseConfig->_videoServer.ContentInterfaceIp + ":";
	_defaultA3Url += chport;

	_defaultA3Url += "/" + pGBCSBaseConfig->_videoServer.ContentInterfacePath;

}

GBCSStorePortal::~GBCSStorePortal()
{
	if (!_bQuit)
		quit();
}

bool GBCSStorePortal::init(void)
{
	if(_pWebResp == NULL)
		_pWebResp = new PortalListener(_strIP.c_str(),_port,&(_store._log));
	if(_pWebResp == NULL)
		return false;

	_pWebResp->setPortalHandle(this);

	return _pWebResp->start();
}

void GBCSStorePortal::quit(void)
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

void GBCSStorePortal::wakeup(void)
{
	_hWakeUp.signal();
}

std::string GBCSStorePortal::getA3Url()
{
	return _defaultA3Url;
}

std::string GBCSStorePortal::getResponseAddr()
{
	char buf[256] = {0};
	sprintf(buf, "%s:%d", _strIP.c_str(), _port);
	return buf;
}

int GBCSStorePortal::run(void)
{	
	// the responsibility of this thread is to periodically sync the content of volume with NGOD Streaming server thru A3 messaging
		
	while(!_bQuit)
	{
		// step 1. determine the dirty volumes
		::std::set< ::std::string> dirtyVolumes;
		if (_interval >0)
		{
			ZQ::common::MutexGuard gd(_lockVolumes); // the locker of _volumeMap
			for (GBCSStorePortal::VolumeMap::iterator it = _volumeMap.begin(); it != _volumeMap.end(); it++)
			{
				if (it->second.stampLastSync < _stampStarted || ZQTianShan::now() - it->second.stampDirty >= _interval)
				{
					it->second.stampDirty = ZQTianShan::now();
					dirtyVolumes.insert(it->first);
				}
			}

			if (dirtyVolumes.size() >0)
				cslog(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "found [%d] dirty volumes of [%d] total volumes"), dirtyVolumes.size(), _volumeMap.size());
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

int GBCSStorePortal::OnTransferStatus(const SimpleHttpd::MsgInfo& info, std::string& respHttpMessageBody)
{
	int nRev = MSG_OK;
	using namespace ZQTianShan::ContentStore;

	std::string strType;
	const char* pType = strrchr(info.uri.c_str(), '/');
	if(pType)
		strType = ++pType;
	else
		strType = info.uri;	

    IGBCSCmd* pCmd = NULL;
	std::string respStatus;
	std::string opCode;	
	A4FileStateNotify   a4FileStaNotify;// a4 as default
	A5StreamStateNotify a5StreamStateNotify;

	pCmd = &a4FileStaNotify;
	std::map<std::string, std::string> a4FileStaNotifyParse = pCmd->parseHttpResponse((std::string &)info.msgBody);
	std::map<std::string, std::string>::iterator it = a4FileStaNotifyParse.find("parseHttpResponse");
	if (it != a4FileStaNotifyParse.end())
		respStatus = it->second;

	std::map<std::string, std::string>::iterator itOpcode = a4FileStaNotifyParse.find("OpCode");
	if (itOpcode != a4FileStaNotifyParse.end())
		opCode = itOpcode->second;

	if (respStatus != GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED] && opCode.empty())
	{
		nRev = MSG_UNKNOWN_PAID;
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal,
			"uri[%s], ip[%s], port[%u], command[%s], FeedBack status[%s]"), 
			info.uri.c_str(), info.ip.c_str(), info.port, opCode.c_str(), respStatus.c_str());
	}
	else if(pCmd->getCmdStr() == opCode)//a4 cmd
	{
		GlobalReqType::setReqCmdType(GlobalReqType::GB_A4_REQ);
		respHttpMessageBody = pCmd->makeHttpContent();
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, 
			"uri[%s], ip[%s], port[%u], command[%s], FeedBack status[%s]"), 
			info.uri.c_str(), info.ip.c_str(), info.port, opCode.c_str(), respStatus.c_str());
	}
	else if(a5StreamStateNotify.getCmdStr() == opCode)//a5 cmd
	{
		GlobalReqType::setReqCmdType(GlobalReqType::GB_A5_REQ);
		pCmd = &a5StreamStateNotify;
		std::map<std::string, std::string> a5StreamStateNotifyParse = pCmd->parseHttpResponse((std::string& )info.msgBody);
		std::map<std::string, std::string>::iterator it = a5StreamStateNotifyParse.find("parseHttpResponse");
		if (it != a5StreamStateNotifyParse.end())
			respStatus = it->second;

		std::map<std::string, std::string>::iterator itOpcode = a5StreamStateNotifyParse.find("OpCode");
		if (itOpcode != a5StreamStateNotifyParse.end())
			opCode = itOpcode->second;

		if (respStatus == GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED]) //ok
		{
			respHttpMessageBody = pCmd->makeHttpContent();
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "uri[%s], ip[%s], port[%u], command[%s], FeedBack status[%s]"), 
				info.uri.c_str(), info.ip.c_str(), info.port, opCode.c_str(), respStatus.c_str());
		}else{
			nRev = MSG_UNKNOWN_PAID;
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal,"uri[%s], ip[%s], port[%u], command[%s], FeedBack status[%s]"), 
				info.uri.c_str(), info.ip.c_str(), info.port, opCode.c_str(), respStatus.c_str());
		} 
	}
	else
	{
#pragma message(__MSGLOC__"TODO: support other protocol")
		nRev = MSG_INTERNAL_ERR;
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal,"uri[%s], ip[%s], port[%u], command[%s], FeedBack status[not support]"), 
			info.uri.c_str(), info.ip.c_str(), info.port, opCode.c_str());
	}

	return nRev;
}

} // namespaces
}
