#include "stdafx.h"
#include <TsStreamerImpl.h>
#include <NativeThread.h>

using namespace ZQ::common;
using namespace SMIL;

DebugMsg dbgMsgLog(Log::L_DEBUG);

namespace TianShanIce {
namespace Streamer {

//////////////////////////////////////////////////////////////////////////
// class IceExporter
IceExporter::IceExporter(Ice::CommunicatorPtr& ic, u_short port): 
	_ic(ic), _icePort(port)
{
	
}

bool 
IceExporter::init()
{
	char endPoint[128];
	sprintf(endPoint, "default -p %u", _icePort);
	try {
		_adapter = _ic->createObjectAdapterWithEndpoints(
			"WMSStreamService", endPoint);
		_adapter->activate();
	} catch(Ice::Exception& e) {
		e.ice_print(std::cout);
		return false;
	}

	return true;
}

Ice::ObjectPrx 
IceExporter::export(Ice::ObjectPtr obj, const std::string& objName)
{
	try {
		Ice::ObjectPrx prx;
		if (objName.length())
			prx = _adapter->add(obj, Ice::stringToIdentity(objName));
		else
			prx = _adapter->addWithUUID(obj);

		return prx;
	} catch(Ice::Exception& e) {
		e.ice_print(std::cout);
		return NULL;
	}
}

Ice::ObjectPtr 
IceExporter::remove(const Ice::Identity& id)
{
	return _adapter->remove(id);
}

//////////////////////////////////////////////////////////////////////////
// Service

class WmsScanThread: public NativeThread {
public:
	WmsScanThread(IWMSServerPtr wmsServer):
	  _wmsServer(wmsServer)
	{
		_quit = false;
	}

	virtual bool init()
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		return true;
	}

	virtual int run()
	{
		
		while(!_quit) {
			try {
				if (WaitForSingleObject(_event, 10000) == WAIT_OBJECT_0) {
					break;
				}

				IWMSPublishingPointsPtr pubPts = _wmsServer->PublishingPoints;
				long count = pubPts->Count;
				for (long i = 0; i < count; i ++) {
					IWMSPublishingPointPtr pubPt = pubPts->Item[i];
					WMS_PUBLISHING_POINT_TYPE type = pubPt->Type;
					if (type != WMS_PUBLISHING_POINT_TYPE_ON_DEMAND && 
						type != WMS_PUBLISHING_POINT_TYPE_BROADCAST) {

						continue;
					}
					
					if (pubPt->Players->Count <= 0) {
						try {
							pubPts->Remove(i);
						} catch(_com_error& err) {
							glog(Log::L_DEBUG, 
								"WmsScanThread::run():\t%s\n", 
								(LPCTSTR )err.Description());
						}
					}
				}
			} catch(_com_error& err) {
				glog(Log::L_DEBUG, 
					"WmsScanThread::run():\t%s\n", 
					(LPCTSTR )err.Description());
			}
		}
		return 0;
	}

	virtual void final()
	{
		CoUninitialize();
	}

	void quit()
	{
		_quit = true;
		SetEvent(_event);
		waitHandle(INFINITE);
	}

protected:
	IWMSServerPtr	_wmsServer;
	HANDLE			_event;
	bool			_quit;
};


// implemented
WMSStreamerServiceImpl::WMSStreamerServiceImpl(IceExporter& exporter, 
											   std::string appName):
	_exporter(exporter), _appName(appName)
{
	_scanThread = NULL;
}

// implemented
WMSStreamerServiceImpl::~WMSStreamerServiceImpl()
{
	if (_scanThread) {
		((WmsScanThread* )_scanThread)->quit();
		delete _scanThread;
	}
}

bool 
WMSStreamerServiceImpl::init()
{
	HRESULT hr;
	hr = _wmsServer.CreateInstance(CLSID_WMSServer);
	if (SUCCEEDED(hr)) {
		_scanThread = new WmsScanThread(_wmsServer);
		_scanThread->start();
		return true;
	}

	return false;
}

// implemented
std::string
WMSStreamerServiceImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
}

// implemented
State
WMSStreamerServiceImpl::getState(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
}

StreamPrx
WMSStreamerServiceImpl::createStream(const Weiwoo::SessionPrx& sess,
						     const Ice::Current& current)
{
	/*
	bool broardcast = sess->privdata["broadcast"]->ints;
	if (broardcast) {

	}
	*/
	throw NotImplemented();
}

Streamer::StreamerDescriptors 
WMSStreamerServiceImpl::listStreamers(const ::Ice::Current&)
{
	return NULL;
}

::std::string 
WMSStreamerServiceImpl::getNetId(const ::Ice::Current&) const
{
	return std::string();
}

// implemented
bool 
WMSStreamerServiceImpl::createTemporaryPlaylist(const PlaylistItems& items, 
												_bstr_t& fileName)
{
	try {
		_bstr_t rootPath = _wmsServer->GetDefaultPath();
		rootPath += "\\";
		rootPath += _appName.c_str();
		TCHAR tmpName[MAX_PATH];
		memset(tmpName, 0, sizeof(tmpName));
		GetTempFileName((LPCTSTR )rootPath, _T("ZQ"), 0, tmpName);
		DeleteFile(tmpName);
		LPTSTR ext = _tcsstr(tmpName, _T(".tmp"));
		lstrcpy(ext, _T(".wsx"));
		
		IXMLDOMDocumentPtr playlist = _wmsServer->CreatePlaylist();
		SmilParser smilParser(playlist);
		smilParser.createPlaylist(tmpName, (LPCTSTR )rootPath, items);

		fileName = tmpName;
		return true;
	} catch (_com_error& err) {
		glog(Log::L_DEBUG, 
			"WMSStreamerServiceImpl::createTemporaryPlaylist():\t"
			"crate playlist failed.\n%s\n", 
			(LPCTSTR )err.Description());
		return false;
	}
}

const TCHAR WMS_IP_ATHORIZATION_REG[] = _T("SOFTWARE\\Microsoft\\Windows Media\\")
										  _T("Server\\RegisteredPlugins\\")
										  _T("Event Notification and Authorization\\")
										  _T("{74A3A581-83F0-11D2-B94F-006008317860}");


const TCHAR TIANSHAN_PLUGIN_REG[] = _T("SOFTWARE\\Microsoft\\Windows Media\\")
										  _T("Server\\RegisteredPlugins\\")
										  _T("Event Notification and Authorization\\")
										  _T("{ABCE7B3C-B8E5-4829-B70B-9556A42F5B34}");

bool 
WMSStreamerServiceImpl::limitPublishingPoint(IWMSPublishingPointPtr pubPt, 
											 PublishingPointType type, 
											 const std::string& destination)
{
	static _bstr_t ipAthorName;
	static _bstr_t tsPluginName;
	try {
		HKEY keyHandle;
		TCHAR pluginName[256];
		LONG r;
		LONG len;
		if (ipAthorName.length() <= 0) {
			
			r = ::RegOpenKey(HKEY_LOCAL_MACHINE, WMS_IP_ATHORIZATION_REG, 
				&keyHandle);
			if (r != ERROR_SUCCESS)
				return false;

			len = sizeof(pluginName);
			r = ::RegQueryValue(keyHandle, NULL, pluginName, &len);
			::RegCloseKey(keyHandle);
			if (r != ERROR_SUCCESS) {				
				return false;
			}
			ipAthorName  = pluginName;
		}

		if (tsPluginName.length() <= 0) {
			r = ::RegOpenKey(HKEY_LOCAL_MACHINE, TIANSHAN_PLUGIN_REG, 
				&keyHandle);
			if (r != ERROR_SUCCESS)
				return false;
			
			len = sizeof(pluginName);
			r = ::RegQueryValue(keyHandle, NULL, pluginName, &len);
			::RegCloseKey(keyHandle);
			if (r != ERROR_SUCCESS) {				
				return false;
			}
			tsPluginName  = pluginName;
		}

		IWMSPublishingPointLimitsPtr pubPtLimits;
		pubPtLimits = pubPt->Limits;
		pubPtLimits->ConnectedPlayers = 1;
		
		IWMSPluginsPtr plugins;
		plugins = pubPt->EventHandlers;

		IWMSPluginPtr plugin;
		plugin = plugins->Item[ipAthorName];
		IDispatchPtr disp;
		IWMSIPAdminPtr ipAdmin;
		disp = plugin->CustomInterface;
		ipAdmin = disp;
		ipAdmin->AccessListOptions = WMS_IP_ACCESS_DISALLOW_BY_DEFAULT;
		IWMSIPListPtr iplist;
		iplist = ipAdmin->AllowIP;
		iplist->Add(destination.c_str(), _T("255.255.255.255"));
		plugin->Enabled = VARIANT_TRUE;

		plugin = plugins->Item[tsPluginName];
		plugin->Enabled = VARIANT_TRUE;

	} catch(_com_error& err) {
		glog(Log::L_DEBUG, 
			"WMSStreamerServiceImpl::limitPublishingPoint():\t%s\n", 
			(LPCTSTR )err.Description());
		return false;
	}
	return true;
}

bool 
getFileName(const char* pathName, std::string& filePart)
{
	const char* strEnd = pathName + strlen(pathName);
	const char* c = strEnd;
	while (c != pathName) {
		if (*c == '\\') {
			filePart = std::string(c + 1, (u_long )strEnd - (u_long ) c);
			return true;
		}
		c --;
	}

	filePart = pathName;
	return true;
}

// implemented
Streamer::PlaylistPrx
WMSStreamerServiceImpl::createPublishingPoint(const ::std::string& pubPtName,
						       const PlaylistItems& items,
						       PublishingPointType type, 
							   const std::string& destination, 
							   std::string& reloc, 
						       const Ice::Current& current)
{
	_bstr_t plPath;
	if (!createTemporaryPlaylist(items, plPath)) {
		glog(Log::L_DEBUG, "WMSStreamerServiceImpl::creatPublishingPoint():\t"
			"createTemporaryPlaylist() failed.\n");
		return NULL;
	}
	
	try {
		IWMSPublishingPointsPtr pubPts = _wmsServer->PublishingPoints;
		IWMSPublishingPointPtr pubPt;
		
		if (type == OnDemandPublishingPoint) {
			pubPt = pubPts->Add(_bstr_t(pubPtName.c_str()), 
				WMS_PUBLISHING_POINT_ON_DEMAND, plPath);
			if (!limitPublishingPoint(pubPt, type, destination)) {
				pubPts->Remove(pubPtName.c_str());
				return NULL;
			}
			PlaylistImpl* playlist = new PlaylistImpl(OnDemandPublishingPoint);
			Ice::ObjectPrx prx = _exporter.export(playlist, std::string());
			playlist->init(prx->ice_getIdentity(), pubPt);
			//getFileName(plPath, reloc);
			reloc = pubPtName;
			return Streamer::PlaylistPrx::checkedCast(prx);

		} else {
			pubPt = pubPts->Add(_bstr_t(pubPtName.c_str()), 
				WMS_PUBLISHING_POINT_BROADCAST, plPath);
			if (!limitPublishingPoint(pubPt, type, destination)) {
				pubPts->Remove(pubPtName.c_str());
				return NULL;
			}
			PlaylistImpl* playlist = new PlaylistImpl(BroadcastPublishingPoint);
			Ice::ObjectPrx prx = _exporter.export(playlist, std::string());
			playlist->init(prx->ice_getIdentity(), pubPt);
			//getFileName(plPath, reloc);
			reloc = pubPtName;
			return Streamer::PlaylistPrx::checkedCast(prx);
		}
		
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "WMSStreamerServiceImpl::creatPublishingPoint():\t"
			"crate publishing point failed.\n%s\n", 
			(LPCTSTR )err.Description());
		return NULL;
	}
}

// implemented
Streamer::PlaylistPrx
WMSStreamerServiceImpl::openPublishingPoint(const ::std::string& pubPtName,
						      const Ice::Current& current)
{
	try {
		IWMSPublishingPointsPtr pubPts = _wmsServer->PublishingPoints;
		_variant_t index(_bstr_t(pubPtName.c_str()));
		IWMSPublishingPointPtr pubPt = pubPts->Item[index];
		if (pubPt->GetType() == WMS_PUBLISHING_POINT_ON_DEMAND) {
			PlaylistImpl* playlist = new PlaylistImpl(OnDemandPublishingPoint);
			Ice::ObjectPrx prx = _exporter.export(playlist, std::string());
			playlist->init(prx->ice_getIdentity(), pubPt);
			return Streamer::PlaylistPrx::checkedCast(prx);
		} else {
			PlaylistImpl* playlist = new PlaylistImpl(BroadcastPublishingPoint);
			Ice::ObjectPrx prx = _exporter.export(playlist, std::string());
			playlist->init(prx->ice_getIdentity(), pubPt);
			return Streamer::PlaylistPrx::checkedCast(prx);
		}

	} catch (_com_error& err) {
		glog(Log::L_DEBUG, "WMSStreamerServiceImpl::openPublishingPoint():\t"
			"crate publishing point failed.\n%s\n", 
			(LPCTSTR )err.Description());
	}

    return 0;
}

// implemented
bool
WMSStreamerServiceImpl::deletePublishingPoint(const ::std::string& pubPtName,
							const Ice::Current& current)
{
	try {
		IWMSPublishingPointsPtr pubPts = _wmsServer->PublishingPoints;
		_variant_t index(pubPtName.c_str());
		pubPts->Remove(index);
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "WMSStreamerServiceImpl::deletePublishingPoint():\t"
			"delete publishing point failed.\n%s\n", 
			(LPCTSTR )err.Description());
		return false;
	}

	return true;
}

// implemented
PublishingPointNames
WMSStreamerServiceImpl::listPublishingPoints(const Ice::Current& current)
{
	PublishingPointNames names;
	try {
		IWMSPublishingPointsPtr pubPts = _wmsServer->PublishingPoints;
		long count = pubPts->length;
		for (long i = 0; i < count; i ++) {
			names.push_back(std::string((LPCTSTR )pubPts->Item[i]->Name));
		}
		return names;
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "WMSStreamerServiceImpl::listPublishingPoints():\t"
			"list publishing point failed.\n%s\n", 
			(LPCTSTR )err.Description());
		return PublishingPointNames();
	}
}

// implemented
bool
WMSStreamerServiceImpl::getAllowClinetsToConnect(const Ice::Current& current)
{
	bool result;
	try {
		result = _wmsServer->AllowClientsToConnect == VARIANT_TRUE;
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, 
			"WMSStreamerServiceImpl::getAllowClinetsToConnect():\t%s\n", 
			(LPCTSTR )err.Description());
		throw BaseException();
		return false;
	}

	return result;
}

// implemented
bool
WMSStreamerServiceImpl::setAllowClinetsToConnect(bool allow,
							   const Ice::Current& current)
{
	try {
		_wmsServer->AllowClientsToConnect = allow;
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, 
			"WMSStreamerServiceImpl::getAllowClinetsToConnect():\t%s\n", 
			(LPCTSTR )err.Description());
		return false;
	}
	
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Playlist

// implemented
PlaylistImpl::PlaylistImpl(PublishingPointType pubPtType /* = OnDemandPublishingPoint */)
{
	_pubPtType = pubPtType;
}

// implemented
PlaylistImpl::~PlaylistImpl()
{

}

// implemented
bool 
PlaylistImpl::init(const Ice::Identity& objId, IWMSPublishingPointPtr pubPt)
{
	id = objId;
	try {
		_pubPt = pubPt;
		if (_pubPtType == OnDemandPublishingPoint) {
			_onDemondPubPt = pubPt;
			_broadPubPt = NULL;
		} else {
			_onDemondPubPt = NULL;
			_broadPubPt = pubPt;
			_wmsPlaylist = _broadPubPt->GetSharedPlaylist();
		}

	} catch (_com_error& err) {
		glog(Log::L_DEBUG, "PlaylistImpl::init():\t%s\n", 
			(LPCTSTR )err.Description());
		_pubPt = NULL;
		_onDemondPubPt = NULL;
		_broadPubPt = NULL;
		_wmsPlaylist = NULL;
		return false;		
	}

	return true;
}

// implemented
void 
PlaylistImpl::setSession(Weiwoo::SessionPrx session)
{
	_session = session;
}

// implemented
void 
PlaylistImpl::setPlaylistId(const std::string& playlistId)
{
	_playlistId = playlistId;
}

// implemented
void
PlaylistImpl::allotAccreditPathTicket(const AccreditedPath::PathTicketPrx& ticket,
							const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
}

// implemented
void
PlaylistImpl::destroy(const Ice::Current& current)
{
	_pubPt = NULL;
	_onDemondPubPt = NULL;
	_broadPubPt = NULL;
	_wmsPlaylist = NULL;
}

// implemented
::std::string
PlaylistImpl::lastError(const Ice::Current& current) const
{
    return _lastError;
}

// implemented
::Ice::Identity
PlaylistImpl::getIdent(const Ice::Current& current) const
{
    return id; // id is member of class Stream;
}

bool
PlaylistImpl::play(const Ice::Current& current)
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			_broadPubPt->Start();
			return true;
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::play():\t%s\n", 
				(LPCTSTR )err.Description());
			return false;
		}
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
	}

    return false;
}

// implemented
bool
PlaylistImpl::setSpeed(::Ice::Float newSpeed,
					 const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
	return false;
}

// implemented
bool
PlaylistImpl::pause(const Ice::Current& current)
{
    throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
	return false;
}

// implemented
bool
PlaylistImpl::resume(const Ice::Current& current)
{
    throw NotImplemented();
	return false;
}

StreamState
PlaylistImpl::getCurrentState(const Ice::Current& current) const
{
    return stsSetup;
}

// implemented
Weiwoo::SessionPrx
PlaylistImpl::getSession(const Ice::Current& current)
{
    return _session;
}

// implemented
::std::string
PlaylistImpl::getId(const Ice::Current& current) const
{
    return _playlistId;
}

// implemented
::Ice::Int
PlaylistImpl::insert(::Ice::Int userCtrlNum,
					 const PlaylistItemSetupInfo& newItemInfo,
					 ::Ice::Int whereUserCtrlNum,
					 const Ice::Current& current)
{
	if (_pubPtType == OnDemandPublishingPoint) {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return 0;
	}

	try {
		SmilParser smilParser(_wmsPlaylist);
		smilParser.insertBefore(whereUserCtrlNum, userCtrlNum, 
			_bstr_t(newItemInfo.contentName.c_str()));
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "PlaylistImpl::insert():\t%s\n", 
			(LPCTSTR )err.Description());
		throw InvalidParameter();
		return -1;
	}
	
    return userCtrlNum;
}

// implemented
::Ice::Int
PlaylistImpl::pushBack(::Ice::Int userCtrlNum,
					   const PlaylistItemSetupInfo& newItemInfo,
					   const Ice::Current& current)
{
	if (_pubPtType == OnDemandPublishingPoint) {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return 0;
	}
	
	try {
		SmilParser smilParser(_wmsPlaylist);
		smilParser.apppendMedia(userCtrlNum, 
			_bstr_t(newItemInfo.contentName.c_str()));
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "PlaylistImpl::pushBack():\t%s\n", 
			(LPCTSTR )err.Description());
		throw InvalidParameter();
		return -1;
	}

	return userCtrlNum;
}

// implemented
::Ice::Int
PlaylistImpl::size(const Ice::Current& current) const
{
	try {
		if (_pubPtType == OnDemandPublishingPoint) {
			IXMLDOMDocumentPtr doc;
			HRESULT hr = doc.CreateInstance(MSXML::CLSID_DOMDocument);
			if (SUCCEEDED(hr)) {
				doc->load(_onDemondPubPt->Path);
				SmilParser smilParser(doc);
				return smilParser.getElementCount();
			} else
				return -1;
		} else {
			SmilParser smilParser(_wmsPlaylist);
			return smilParser.getElementCount();
		}
	} catch(_com_error& err) {
		glog(Log::L_DEBUG, "PlaylistImpl::size():\t%s\n", 
			(LPCTSTR )err.Description());
		return -1;
	}

}

// implemented
::Ice::Int
PlaylistImpl::left(const Ice::Current& current) const
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			SmilParser smilParser(_wmsPlaylist);
			return smilParser.getNextElementsCount(
				_wmsPlaylist->CurrentPlaylistEntry);
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::left():\t%s\n", 
				(LPCTSTR )err.Description());
			return -1;
		}
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return -1;
	}
}

// implemented
bool
PlaylistImpl::empty(const Ice::Current& current)
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			SmilParser smilParser(_wmsPlaylist);
			return smilParser.empty();
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::empty():\t%s\n", 
				(LPCTSTR )err.Description());
			return true;
		}
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return true;
	}
}

// implemented
::Ice::Int
PlaylistImpl::current(const Ice::Current& current) const
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			SmilParser smilParser(_wmsPlaylist);
			return smilParser.getElementId(
				_wmsPlaylist->CurrentPlaylistEntry);
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::current():\t%s\n", 
				(LPCTSTR )err.Description());
			return -1;
		}
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return -1;
	}
}

// implemented
void
PlaylistImpl::erase(::Ice::Int whereUserCtrlNum,
					const Ice::Current& current) const
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			SmilParser smilParser(_wmsPlaylist);
			smilParser.erase(whereUserCtrlNum);
			return;
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::erase():\t%s\n", 
				(LPCTSTR )err.Description());
			return;
		}
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return;
	}
}

// implemented
::Ice::Int
PlaylistImpl::flushExpired(const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));
    return 0;
}

// implemented
bool
PlaylistImpl::clearPending(bool includeInitedNext,
					       const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));

    return false;
}

// implemented
bool
PlaylistImpl::isCompleted(const Ice::Current& current)
{
	if (_pubPtType == BroadcastPublishingPoint) {
		return _broadPubPt->BroadcastStatus == 
			WMS_BROADCAST_PUBLISHING_POINT_STOPPED;
	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return false;
	}
}

// implemented
::Ice::Int
PlaylistImpl::findItem(::Ice::Int userCtrlNum,
					   ::Ice::Int from,
					   const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));

    return 0;
}

// implemented
bool
PlaylistImpl::distance(::Ice::Int to,
					   ::Ice::Int from,
					   ::Ice::Int& dist,
					   const Ice::Current& current)
{
	throw TianShanIce::NotImplemented(
			_T("WMSStreamService don't support this operation"));

    return false;
}

// implemented
bool
PlaylistImpl::skipToItem(::Ice::Int where,
					     bool bPlay,
					     const Ice::Current& current)
{
	if (_pubPtType == BroadcastPublishingPoint) {
		try {
			SmilParser smilParser(_wmsPlaylist);
			IXMLDOMElementPtr elem = smilParser.getElementById(where);
			_wmsPlaylist->CueStream(elem);
			long playerCount = _pubPt->Players->Count;
			for (long i = 0; i < playerCount; i ++) {
				IWMSPlayerPtr player = _pubPt->Players->Item[_variant_t(i)];
				IWMSPlaylistPtr reqPl = player->RequestedPlaylist;
				reqPl->CueStream(elem);
				reqPl->FireEvent(_T("EmergencyNotice"));
			}
			
			return true;
		} catch(_com_error& err) {
			glog(Log::L_DEBUG, "PlaylistImpl::skipToItem():\t%s\n", 
				(LPCTSTR )err.Description());
			return false;
		}

	} else {
		throw TianShanIce::NotImplemented(
			_T("On demand publishing point don't support this operation"));
		return false;
	}
}

} // namespace Streamer {
} // namespace TianShanIce {
