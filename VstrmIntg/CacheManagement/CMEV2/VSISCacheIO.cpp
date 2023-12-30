#include "VSISCacheIO.h"
#include "CMEVSIS.h"

#define MAX_IMPORT_REQ_KEEP_TIME	5		// seconds
#define VSIS_NAME(cluid,nodeip)    (cluid + "_" + nodeip)

#define DEFAULT_IMPORT_CONSUME_BW	  2000000	 // 2Mbps for counting, the used BW will be real after next round VSIS importing
#define CME_VSIS_REGISTER_INTERVAL         10    // seconds
#define VSIS_MAX_UPDATE_TIME		      150	 // VSIS report bw usage every 60 seconds		  

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

static uint64 s_recvPackageNo = 0;

namespace CacheManagement {

std::string VSISCODE_TEXT(USHORT code)
{
	std::string text;

	switch (code)
	{

	case CVM_INVALID:
		text = "CVM_INVALID";
		break;

	case CVM_REGISTER:
		text = "CVM_REGISTER";
		break;

	case CVM_DELETE_REQUEST:
		text = "CVM_DELETE_REQUEST";
		break;

	case CVM_STATUS_REQUEST:
		text = "CVM_STATUS_REQUEST";
		break;

	case CVM_LIST_ASSETS_REQUEST:
		text = "CVM_LIST_ASSETS_REQUEST";
		break;

	case CVM_CME_PARAMETERS:
		text = "CVM_CME_PARAMETERS";
		break;

	case CVM_IMPORT_REQUEST:
		text = "CVM_IMPORT_REQUEST";
		break;

	case CVM_SESS_COUNT_REQ:
		text = "CVM_SESS_COUNT_REQ";
		break;

	case CVM_ASSET_STATUS:
		text = "CVM_ASSET_STATUS";
		break;

	case CVM_LIST_ASSETS_RESPONSE:
		text = "CVM_LIST_ASSETS_RESPONSE";
		break;

	case CVM_DISK_USAGE_REPORT:
		text = "CVM_DISK_USAGE_REPORT";
		break;

	case CVM_SMART_PARAMETERS:
		text = "CVM_SMART_PARAMETERS";
		break;

	case CVM_EXPIRE_NOTICE:
		text = "CVM_EXPIRE_NOTICE";
		break;

	case CVM_DEDUCT_NOTICE:
		text = "CVM_DEDUCT_NOTICE";
		break;

	case CVM_SESSION_START_NOTICE:
		text = "CVM_SESSION_START_NOTICE";
		break;

	case CVM_SESSION_END_NOTICE:
		text = "CVM_SESSION_END_NOTICE";
		break;

	case CVM_SESS_COUNT_RSP:
		text = "CVM_SESS_COUNT_RSP";
		break;

	case CVM_IMPORT_FRAGMENT:
		text = "CVM_IMPORT_FRAGMENT";
		break;

	case CVM_DELETE_FRAGMENT:
		text = "CVM_DELETE_FRAGMENT";
		break;

	case CVM_FRAGMENT_STATUS:
		text = "CVM_FRAGMENT_STATUS";
		break;

	case CVM_FRAGSTS_REQUEST:
		text = "CVM_FRAGSTS_REQUEST";
		break;

	case CVM_LIST_FRAGMENTS:
		text = "CVM_LIST_FRAGMENTS";
		break;

	case CVM_VOD_POP_DATA:
		text = "CVM_VOD_POP_DATA";
		break;

	case CVM_LAST_VOD_POP_DATA:
		text = "CVM_LAST_VOD_POP_DATA";
		break;

	case CVM_KNOWN_FRAGMENTS:
		text = "CVM_KNOWN_FRAGMENTS";
		break;

	case CVM_KNOWN_FRAGMENTS_END:
		text = "CVM_KNOWN_FRAGMENTS_END";
		break;

	case CVM_VOD_TLV_DATA:
		text = "CVM_VOD_TLV_DATA";
		break;

	case CVM_VSIS_CDN_STATE:
		text = "CVM_VSIS_CDN_STATE";
		break;

	default:
		text = "(Unknown VSIS Tag)";
		break;
	}

	return text;
}  // szVSIScode()

std::string VSIS_ASSET_STATUS_TEXT(USHORT code)
{
	std::string text;

	switch (code)
	{

	case CACHE_CONTENT_NONLOCAL:
		text = "CACHE_CONTENT_NONLOCAL";
		break;

	case CACHE_CONTENT_COMPLETED:
		text = "CACHE_CONTENT_COMPLETED";
		break;

	case CACHE_CONTENT_STREAMABLE:
		text = "CACHE_CONTENT_STREAMABLE";
		break;

	case CACHE_CONTENT_DELETED:
		text = "CACHE_CONTENT_DELETED";
		break;

	case CACHE_CONTENT_NOT_DELETED:
		text = "CACHE_CONTENT_NOT_DELETED";
		break;

	case CACHE_CONTENT_NOT_CDN:
		text = "CACHE_CONTENT_NOT_CDN";
		break;

	case CACHE_CONTENT_REQ_INVALID:
		text = "CACHE_CONTENT_REQ_INVALID";
		break;

	case CACHE_CONTENT_NO_BANDWIDTH:
		text = "CACHE_CONTENT_NO_BANDWIDTH";
		break;

	case CACHE_CONTENT_PENDING:
		text = "CACHE_CONTENT_PENDING";
		break;

	case CACHE_CONTENT_FRAGMENT_ONLY:
		text = "CACHE_CONTENT_FRAGMENT_ONLY";
		break;

	case CACHE_CONTENT_INSRVERR:
		text = "CACHE_CONTENT_INSRVERR";
		break;
	default:
		text = "(Unknown VSIS CACHE status)";
		break;
	}

	return text;
}  // szVSIScode()

VSISEntity::VSISEntity(VSISCacheIO& vsisIO, std::string name, std::string cluID, std::string ip, uint32 reservedImpBW, uint32 maxReportInterval, const ZQ::common::InetAddress &bind, ZQ::common::tpport_t port)
:_vsisIO(vsisIO), ZQ::common::TCPClient(bind, port)
{
	_name = name;
	_clusterID = cluID;
	_ip = ip;

	_isCMEPremetersSent = false;
	_isCMERegistered = false;
	_isVSISRegistered = false;
	_cmeRegisteredTime = 0;
	_connected = false;
	_status = true;

	_totalOutputBW = 0;					// total output bandwidth; not used

	//set defaultbandwidth and time, because VSIS maynot update them until some C2 traffic happened
	_usedImportBW = 0;
	_totalImportBW = 300 * 1000 * 1000;	// 300Mbps
	_lastUpdate = ZQ::common::now(); 

	_reservedImpBW = reservedImpBW;
	_maxReportInterval = maxReportInterval;
	
	_usedStreamBW = 0;
	_totalStreamBW = 0;

	_addupLength = 0;

	_bwNotChangeTimes = 0;

	_serverAddress.setAddress(_ip.c_str());
	_serverPort = CV_PORT;
	setPeer(_serverAddress, _serverPort);
}

VSISEntity::~VSISEntity()
{
	ZQ::common::MutexGuard gd(_buffLocker);

	REC_VSIS_DATA* vsisdata = NULL;
	while(!_bufferQueue.empty())
	{
		vsisdata = (REC_VSIS_DATA*)_bufferQueue.front();
		_bufferQueue.pop();

		delete vsisdata;
	}
}		

void VSISEntity::setConnStatus(bool connected) 
{ 
	_connected = connected; 

	// reset flags if connection lost
	if(!_connected)
	{
		_isCMERegistered = false;
		_isVSISRegistered = false;
		_cmeRegisteredTime = 0;
	}
}

void VSISEntity::setVSISStatus(bool avaible)
{
	_status = avaible;
	_lastUpdate = ZQ::common::now(); 
}

void VSISEntity::setTotalOutputBW(uint64 bw) 
{ 
	_totalOutputBW = bw; 
	_lastUpdate = ZQ::common::now(); 
}

void VSISEntity::setBandwidth(uint64 usedImportBW, uint64 totalImportBW, uint64 usedStreamBW, uint64 totalStreamBW)		
{
	if(usedImportBW == _usedImportBW)
		_bwNotChangeTimes++;
	else
		_bwNotChangeTimes = 0;

	_usedImportBW = usedImportBW; 
	_totalImportBW = totalImportBW; 
	usedStreamBW = _usedStreamBW;		// VSIS always return 0
	_totalStreamBW = totalStreamBW;		// VSIS always return 0

	_lastUpdate = ZQ::common::now(); 
}

bool VSISEntity::isRecentReported()
{
	uint64 tnow = ZQ::common::now();
	uint64 timediff = tnow > _lastUpdate ? (tnow - _lastUpdate)/1000 : 0;

	return timediff <= _maxReportInterval;
}
static int errorno()
{
#ifdef ZQ_OS_MSWIN
	return WSAGetLastError();
#else
	return errno;
#endif
}
void VSISEntity::OnConnected()
{ 
	std::string conndesc = connDescription(); 
	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISEntity, "%s: OnConnected() conn[%s] VSIS ip=%s port=%d "), _clusterID.c_str(), conndesc.c_str(),  _ip.c_str(), CV_PORT);

	setConnStatus(true);
	setVSISStatus(true);
	_vsisIO.requestRegisterCME(*this);
    
}

void VSISEntity::OnError()
{
	std::string conndesc = connDescription(); 
	setConnStatus(false);
	setVSISStatus(false);
	glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISEntity, "%s: OnError() conn[%s] socket error[%d] occurred, disconnecting... VSIS ip=%s port=%d "),_clusterID.c_str(), conndesc.c_str(), checkSoErr(), _ip.c_str(), CV_PORT);
	disconnect(); 
}
void VSISEntity::OnDataArrived()
{
	struct sockaddr_in fromAddress;
	socklen_t addressSize = sizeof(fromAddress);

	//
	// save the recevied data and notify 
	//
	REC_VSIS_DATA* recvBuff = new REC_VSIS_DATA;

	int bytesRead = recv(_so, (char*)recvBuff->data, MAX_SOCKET_BUFF_SIZE, 0);

	if (bytesRead <= 0)
	{
		int err = errorno();
#ifdef ZQ_OS_MSWIN
		if (WSAEWOULDBLOCK == err || WSAEINPROGRESS == err || WSAEALREADY == err)
#else
		if (EINPROGRESS == err)
#endif // ZQ_OS_MSWIN
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISEntity, "%s: OnDataArrived() conn[%s] recv() temporary fail[%d/%d], errno[%d]"), _clusterID.c_str(),  connDescription(), bytesRead, MAX_SOCKET_BUFF_SIZE, err);
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISEntity, "%s: OnDataArrived() conn[%s] recv() failed[%d/%d], errno[%d]"), _clusterID.c_str(), connDescription(), bytesRead, MAX_SOCKET_BUFF_SIZE, err);
			OnError();
		}
		return;
	}
/*
	{
		char sockdesc[100];
		snprintf(sockdesc, sizeof(sockdesc)-2, CLOGFMT(RTSPClient, "OnDataArrived() conn[%08x]"), TCPSocket::get());

		if (RTSP_VERBOSEFLG_RECV_HEX & _verboseFlags)
			glog.hexDump(Log::L_DEBUG, &_inCommingBuffer[_inCommingByteSeen], bytesRead, sockdesc);
		else
			glog.hexDump(Log::L_INFO, &_inCommingBuffer[_inCommingByteSeen], bytesRead, sockdesc, true);
	}
*/
	recvBuff->length = bytesRead;

	// save the buffer into the queue
	{
		ZQ::common::MutexGuard guard(_buffLocker);
		_bufferQueue.push(recvBuff);
	}
	//
	// only notify if there is no continued data, if there is continued data, wait for next time data coming 
	// The AssetList reponse may very long. Will test it to see this approach would work. If not, need to read the expected length from received buff
	//
	if(bytesRead < _vsisIO._readBuffSize )
	{
		// notify connection thread to process the data
		_vsisIO.onDataReceived(*this);
	}
}
bool VSISEntity::onConnectLost()
{
	std::string conndesc = connDescription(); 
	glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISEntity, "%s: onConnectLost() conn[%s] VSIS ip=%s port=%d "), _clusterID.c_str(), conndesc.c_str(),  _ip.c_str(), CV_PORT);

	setConnStatus(false);

	// trigger VSISIO to re-connect right away
	_vsisIO.onConnectionLost(*this);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

VSISCacheIO::VSISCacheIO(std::string clusterID, CMECacheAlgorithm& cmeAlg)
:_cmeAlg(cmeAlg)
{
	_clusterID = clusterID;

	_threadRunning = false;
	_connTimeout = 5;			// seconds
	_sendTimeout = 5;			// seconds
	_readBuffSize = 64 * 1024;	// bytes

	_paidLength = 20;

	_totalSpace = 2000000000000; // 2T
	_freeSpace = 5 * (_totalSpace / 100);

	_reservedBW = 20000000;	// 20Mbps
	_connScanInterval = 30;

	_usedImportBW = 0;
	_totalImportBW = 0;

}

VSISCacheIO::~VSISCacheIO()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: enter ~VSISCacheIO()"),
			_clusterID.c_str());

	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	while(_vsisEntities.size() > 0)
	{
		itvsis = _vsisEntities.begin();
		vsisEntity = (VSISEntity*) itvsis->second;
		
		_vsisEntities.erase(itvsis);
		delete vsisEntity;
	}

	IOEvent* ioevt = NULL;
	ZQ::common::MutexGuard gd(_eventLock);
	while(!_eventQueue.empty())
	{
		ioevt = _eventQueue.front();
		_eventQueue.pop();

		delete ioevt;
	}

	SIMPLE_CONTENT_INFO* simpleContent = NULL;
	SIMPLE_CONTENTS::iterator itsim;
	while(_tempContents.size() > 0)
	{
		itsim = _tempContents.begin();
		simpleContent = (SIMPLE_CONTENT_INFO*) itsim->second;
		
		_tempContents.erase(itsim);
		delete simpleContent;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: exit ~VSISCacheIO()"),
			_clusterID.c_str());
}

void VSISCacheIO::setLocalIP(std::string& localIP)
{
	_localIP = localIP;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: localIP=%s"),
		_clusterID.c_str(), _localIP.c_str());
}

void VSISCacheIO::setConnParameters(uint32 connTimeout, uint32 sendTimeout, uint32 readBuffSize)
{
	_connTimeout = connTimeout;
	_sendTimeout = sendTimeout;
	_readBuffSize = readBuffSize;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: connTimeout=%d seconds, sendTimeout=%d seconds, readBuffSize=%d bytes"),
		_clusterID.c_str(), _connTimeout, _sendTimeout, _readBuffSize);
}

void VSISCacheIO::setConnScanInterval(uint32 scanInterval)
{
	_connScanInterval = scanInterval;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: connection scanInterval=%d ms"),
		_clusterID.c_str(), _connScanInterval);

}

void VSISCacheIO::setContentParameter(uint32 paidLen)
{
	_paidLength = paidLen;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: default paidLen=%d"),
		_clusterID.c_str(), _paidLength);
}

void VSISCacheIO::setReservedBW(uint32 reserved)
{
	_reservedBW = reserved;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: reserved C2 import bandwidth %d Mbps per node"),
		_clusterID.c_str(), _reservedBW / 1000000);
}

void VSISCacheIO::setMaxRptInterval(uint32 maxInterval)
{
	_maxRptInterval = maxInterval > 120 ? maxInterval : 120;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: VSIS max reported interval is %d seconds"),
		_clusterID.c_str(), _maxRptInterval);
}

void VSISCacheIO::setRecentImportBW()
{
	_usedImportBW = 0;
	_totalImportBW = 0;

	VSISENTITIES::iterator itvsis;
	VSISEntity* vsisEntity = NULL;

	uint64 tnow = ZQ::common::now();
	uint64 interval;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = (VSISEntity*) itvsis->second;
		
		interval = tnow > vsisEntity->getLastUpdateTime() ? (tnow - vsisEntity->getLastUpdateTime())/1000 : 0;
		if(interval > VSIS_MAX_UPDATE_TIME)
			continue;
		
		_usedImportBW  += vsisEntity->_usedImportBW;
		_totalImportBW += (vsisEntity->_totalImportBW - vsisEntity->_reservedImpBW);	// did not count the reserved into total
	}
}

void VSISCacheIO::getRecentImportBW(uint64& usedBW, uint64& totalBW)
{
	usedBW = _usedImportBW;
	totalBW = _totalImportBW;
}

uint32 VSISCacheIO::getRecentImportBWUsage()
{
	uint32 usage = (uint32)( 100.f * (float)((double)_usedImportBW / (double)_totalImportBW) );
	return (usage > 100 ? 100 : usage);
}

uint64 VSISCacheIO::getRecentFreeBW()
{
	// total may less then used, since reserved was deduct from total
	return _totalImportBW > _usedImportBW ? (_totalImportBW - _usedImportBW) : 0;
}

bool VSISCacheIO::initialize(std::string& nodeIPs)
{
	// start the thread
	if (_threadRunning)
		return true;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: enter initialize() with nodeIPs=%s"),
		_clusterID.c_str(), nodeIPs.c_str());

	// node IPs are seperated by (;), each IP reprent node	
	if(nodeIPs.empty())
		return false;

	std::string vsisName;
	std::string leftStr, newIP;
	std::string rightStr = nodeIPs;

	size_t pos = 0;

	do
	{
		pos = rightStr.find_first_of(';');
		if(pos != std::string::npos)
		{
			leftStr = rightStr.substr(0, pos);
			rightStr = rightStr.substr(pos+1, rightStr.size()-pos-1);
		}
		else
		{
			// the last one
			leftStr = rightStr;
		}
		
		// generate a node name 
		vsisName = VSIS_NAME(_clusterID, leftStr);

		// find if there is duplicated ip in this cluster, just ignore it
		// we don't check if there is duplicated ip cross clsuters
		if(_vsisEntities.find(vsisName) != _vsisEntities.end())
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "find duplicated IP %s in cluster %s"),
				leftStr.c_str(), _clusterID.c_str());
			continue;
		}

		// create node object 
		ZQ::common::InetAddress bindaddr;
		bindaddr.setAddress(_localIP.c_str());
		VSISEntity* vsisEntity = new VSISEntity(*this, vsisName, _clusterID, leftStr, _reservedBW, _maxRptInterval, bindaddr, 0);
		_vsisEntities.insert(VSISENTITIES::value_type(vsisName, vsisEntity));

		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: create VSIS entity with name %s"),
			_clusterID.c_str(), vsisName.c_str());

	}while(pos != std::string::npos);

	// calcuate the total bandwidth because VSIS may not update C2 bandwidth until there is C2 traffic.
	setRecentImportBW();

	_threadRunning = true;
	// start the thread
	NativeThread::start();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: exit initialize()"),
		_clusterID.c_str());

	return true;
}

void VSISCacheIO::unInitialize()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: enter unInitialize()"),
		_clusterID.c_str());

	if (!_threadRunning)
		return;
	
	// notify thead to exit
	_threadRunning = false;
	_waitEvent.signal();
	waitHandle(5000);		// wait for 5 seconds at most 

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: exit unInitialize()"),
		_clusterID.c_str());
}

int VSISCacheIO::run( void )
{
	int sleepTime = MAX_IMPORT_REQ_KEEP_TIME*1000+100;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: enter run() with timeout=%d ms"),
			_clusterID.c_str(), sleepTime);

	uint64 lastExecuteTime = ZQ::common::now();
	uint64 curtime;

	//
	// create connection to VSIS as the first thing when the thread start
	//
	connectVSIS();

	// in the thread, to check the connection status
	while(_threadRunning)
	{
		SYS::SingleObject::STATE st = _waitEvent.wait(sleepTime);
		
		switch(st)
		{
		case SYS::SingleObject::TIMEDOUT:

			// check if there's content import waiting for pwe & usercount timeout
			checkUnimportedContent();	// need less interval

			curtime = ZQ::common::now();
			if( (curtime - lastExecuteTime) >= _connScanInterval*1000 )
			{
				// print here
				printImportBW();

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "Scan and recreate CME-VSIS connections if necessary with scan interval [%d] seconds"),
					_connScanInterval );

				// reconnect if there is connection lost
				connectVSIS();
				
				// check if there is un-processed messages(must have) between CME and VSIS
				checkRequiredSentMsg();

				lastExecuteTime = ZQ::common::now();
			}

			break;

		case SYS::SingleObject::SIGNALED :
			while(!_eventQueue.empty() && _threadRunning)
			{
				// read event and process it
				IOEvent* ioevt = NULL;
				
				{
					ZQ::common::MutexGuard evtGuard(_eventLock);

					ioevt = _eventQueue.front();
					_eventQueue.pop();
					
				}// end of lock

				switch(ioevt->evtType)
				{
				case IO_CONN_LOST:
					// connect lost, recreate if necessary
					connectVSIS(*ioevt->pVsisEntity);
					break;

				case IO_DATA_ARRIVE:
					// process the incoming data
					processIncomingData(*ioevt->pVsisEntity);
					break;

				case IO_OP_IMPORT:
					// process the import 
					doImportContent(*ioevt);
					break;

				case IO_OP_DELETE:
					// process the delete 
					doDeleteAsset(*ioevt);
					break;

				case IO_OP_SYNCONE:
					doSyncOneAsset(*ioevt);
					break;
					
				case IO_OP_SYNCALL:
					doSyncAllAsset();
					break;

				default:	// 
					break;
				}
				delete ioevt;

			} // end of while(!_eventQueue.empty() && _threadRunning)

			break;		// break of case SYS::SingleObject::SIGNALED

		default:
			break;
		}
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: exit run()"),
			_clusterID.c_str());

	return 0;
}

void VSISCacheIO::final(void)
{

}

bool VSISCacheIO::VSIS_STAT_2_CONT_STAT(int vsisStat, Content::CONTENT_STATUS& cntStatus)
{
	switch(vsisStat)
	{	
	case CACHE_CONTENT_NOT_CDN:		// content does not exist even in CDN
		cntStatus = Content::CNT_NOT_EXIST;
		break;

	case CACHE_CONTENT_NONLOCAL:	// the content is in CDN
		cntStatus = Content::CNT_UNCACHED;
		break;

	case CACHE_CONTENT_COMPLETED:
		cntStatus = Content::CNT_CACHED;
		break;

	case CACHE_CONTENT_STREAMABLE:
		cntStatus = Content::CNT_CACHING;
		break;

	case CACHE_CONTENT_DELETED:		// content was deleted from storage
		cntStatus = Content::CNT_DELETED;
		break;

	case CACHE_CONTENT_NOT_DELETED: // VSIS failed to delete the content for some reason, need to retry later
		cntStatus = Content::CNT_DELETE_FAILED;
		break;

	case CACHE_CONTENT_NO_BANDWIDTH:// No bandwidth to cache this content right, try later 
		cntStatus = Content::CNT_CACHE_FAIL;
		break;

	case CACHE_CONTENT_INSRVERR:	// Will send query msg to VSIS to check it
	default:
		return false;
	}
	return true;
}

// It is to create the connection to VSIS if the connection has been created.
// The socket connection objects are managed by _postDak & _postHouseEnv
// The connection state is managed by _mcGroup as it know all the Clusters and Nodes
bool VSISCacheIO::connectVSIS()
{
	VSISEntity* vsisEntity = NULL;
	
	// loop for each MediaCluster's node
	VSISENTITIES::iterator itvsis;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = (VSISEntity*) itvsis->second;

		// check and reconnect if necessary
		connectVSIS(*vsisEntity);

	}// loop of nodes
	return true;
}

bool VSISCacheIO::connectVSIS(VSISEntity& vsisEntity)
{
	// check if the connection has been created
	if( vsisEntity.isConnected() )
	{
		// the connction is OK, do nothing
		return true;
	}

	// create socket connection to VSIS, carry the node to the socket object
	VSISEntityPtr vsisPtr = &vsisEntity;
	vsisPtr->__setNoDelete(true);	// avoid to delete, because the Ptr was only used to carry the Node object to socket
									// if not set, the VSISEntity object will be released by the smart Ptr and cause problem
	// do the connection to VSIS
	char szport[20];
	sprintf(szport, "%d", CV_PORT);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: CME will connect to VSIS ip=%s port=%s, local ip=%s"),
		vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), szport, _localIP.c_str());

	if(ZQ::common::Socket::stConnecting == vsisEntity.state())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: CME connect to VSIS  ip=%s port=%s in progress, local ip=%s"),
			vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), szport, _localIP.c_str());
		return true;
	}
	// need to open a connection
	if(!vsisEntity.connect(_connTimeout*1000))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISCacheIO, "%s: CME connect VSIS ip=%s port=%s, local ip=%s, error occured at connect()"),
			vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), szport, _localIP.c_str());
		return false;
	}

	if(ZQ::common::Socket::stConnecting == vsisEntity.state())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: CME connect to VSIS  ip=%s port=%s in progress, local ip=%s"),
			vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), szport, _localIP.c_str());
		return true;
	}
	else if (ZQ::common::Socket::stConnected == vsisEntity.state())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: CME successfully connect to VSIS ip=%s port=%d, %s"),
			vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), CV_PORT, vsisEntity.connDescription());
		vsisEntity.setConnStatus(true);
		requestRegisterCME(vsisEntity);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISCacheIO, "%s: Fail to connect to VSIS ip=%s port=%d"),
			vsisEntity._clusterID.c_str(), vsisEntity._ip.c_str(), CV_PORT);
		return false;
	}

	return true;
}

void VSISCacheIO::checkUnimportedContent()
{
	if(_tempContents.size() == 0)
	{
		return;
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: Total %d contents are waiting for PWE flag or UserCount before really cache it before checking the list"),
		_clusterID.c_str(), _tempContents.size());

	SIMPLE_CONTENT_INFO* simpleContent = NULL;
	SIMPLE_CONTENTS::iterator ittmp;
	uint64 timenow = ZQ::common::now();
	uint64 elapsedSeconds = 0;

	std::vector<std::string> toBeDeleted;

	for(ittmp=_tempContents.begin(); ittmp!=_tempContents.end(); ittmp++)
	{
		simpleContent = (SIMPLE_CONTENT_INFO*) ittmp->second;

		elapsedSeconds = (timenow > simpleContent->timestamp) ? (timenow - simpleContent->timestamp)/1000 : 0;

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s paid=%s is waiting for PWE flag or UserCount with elapsed time %d ms. It's info isPWE=%d, UserCount=%d, reportedVSIS=%d, totalVSIS=%d"),
			_clusterID.c_str(), simpleContent->pid.c_str(), simpleContent->paid.c_str(), elapsedSeconds, 
			simpleContent->isPWE ? 1:0, simpleContent->playCount, simpleContent->reportedVsis, simpleContent->totalVsis);

		if(elapsedSeconds > MAX_IMPORT_REQ_KEEP_TIME)
		{
			doImportContent(simpleContent->pid, simpleContent->paid, simpleContent->isPWE, simpleContent->playCount);

			glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: content pid=%s paid=%s waiting for pwe flag or usercount timeout with %d seconds, cache it anyway"),
				_clusterID.c_str(), simpleContent->pid.c_str(), simpleContent->paid.c_str(), MAX_IMPORT_REQ_KEEP_TIME);
			
			doImportContent(simpleContent->pid, simpleContent->paid, simpleContent->isPWE, simpleContent->playCount);

			toBeDeleted.push_back(ittmp->first);
		}
	}

	// remove them from the temp list
	size_t i=0;
	for(; i<toBeDeleted.size(); i++)
	{
		_tempContents.erase(toBeDeleted[i]);
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: Total %d contents are waiting for PWE flag or UserCount before really cache it after checking the list"),
		_clusterID.c_str(), _tempContents.size());
}

void VSISCacheIO::printImportBW()
{
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;

	uint64 used, total;
	uint32 reserved;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = (VSISEntity*) itvsis->second;
		vsisEntity->getImportBW(used, total, reserved);
		
		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: VSIS %s usedImportBW=%llu Mbps totalImportBW=%llu Mbps reservedImportBW=%d Mbps lastUpdate=%s"), 
			_clusterID.c_str(), vsisEntity->_name.c_str(), used/1000000, total/1000000, reserved/1000000, time64ToUTCwZoneStr(vsisEntity->getLastUpdateTime()).c_str());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: cluster wide recent valid freeImportBW=%llu Mbps usedImportBW=%llu Mbps totalImportBW=%llu Mbps importBWUsage=%d%%"), 
		_clusterID.c_str(), getRecentFreeBW()/1000000, _usedImportBW/1000000, _totalImportBW/1000000, getRecentImportBWUsage());
}

void VSISCacheIO::checkRequiredSentMsg()
{
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;

	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = (VSISEntity*) itvsis->second;

		if(!vsisEntity->isConnected())
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: regular check interactive between VSIS, CME has not connect to %s, do nothing"), 
				_clusterID.c_str(), vsisEntity->_name.c_str());

			continue;
		}
		uint64 timeDiffSeconds = (ZQ::common::now() > vsisEntity->_cmeRegisteredTime) ? (ZQ::common::now() - vsisEntity->_cmeRegisteredTime)/1000 : 0;
		// check if register has been exchanged betewen CME and VSIS
		if(   (!vsisEntity->_isCMERegistered)			// CME has not register to VSIS yet   
			   || 
			  ( vsisEntity->_isCMERegistered && 
			    !vsisEntity->_isVSISRegistered && 
			    timeDiffSeconds > CME_VSIS_REGISTER_INTERVAL
			  ) 
		  )  // have not receive VSIS register msg yet
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: regular check found CME has not registered to %s, going to register"), 
				_clusterID.c_str(), vsisEntity->_name.c_str());

			// register myself to VSIS
			requestRegisterCME(*vsisEntity);
			//break;	// do a thing a time for a node
		}
		// check if CME had sent its parameter to the VSIS
		if(!vsisEntity->_isCMEPremetersSent)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: regular check found CME has not sent its parameters to %s, going to send parameters"), 
				_clusterID.c_str(), vsisEntity->_name.c_str());

			// send CME parameter again
			requestCMEPremeters(*vsisEntity, _cmeAlg.getCushion(), _cmeAlg.getImportTrigger());
		}
	}// loop of nodes
}


int VSISCacheIO::sendBufferToVSIS(VSISEntity& vsisEntity, char* buff, int length)
{
	if(!vsisEntity.isConnected())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISCacheIO, "%s: sendBufferToVSIS() - connection to %s has not been created, add condition code to avoid reaching here"), 
			_clusterID.c_str(), vsisEntity._name.c_str());

		return 0;
	}

	int sentBytes = vsisEntity.send(buff, length);

	// close the connection ??? is there possibility of racing the connection ???
	if(sentBytes <= 0)
	{
		vsisEntity.setConnStatus(false);
	}
	return sentBytes;
}

bool VSISCacheIO::requestRegisterCME(VSISEntity& vsisEntity)
{
	char localhost[128];
	PCV_TLV rmessage  = (PCV_TLV) alloca(sizeof(localhost) + 4 * sizeof(CV_TLV));
	PCV_TLV eamessage = (PCV_TLV) &rmessage->value; // Each CV_TLV message within outer shell
	ULONG version  = CME_VSIS_PROT_VER;

	int   hostlen = 0;		// Length of hostname string
	int nErr = gethostname(localhost, sizeof localhost);
	if (nErr)
	{
		// use node(it's IP) name instead of localhost name
		sprintf(localhost, vsisEntity._name.c_str());
	}

	// Start with rmessage, a constant pointer to our output buffer
	//
	rmessage->tag     = CVM_REGISTER;
	rmessage->length  = sizeof(rmessage->tag) + sizeof(rmessage->length);

	// Set up CME VSIS protocol version, add length to full message TLV,
	// then advance eamessage to next slot.
	//
	eamessage->tag    = CVT_REG_PROTOCOL;
	eamessage->length = sizeof(eamessage->tag) + sizeof(eamessage->length) + sizeof(version);
	memcpy(eamessage->value, &version, sizeof version);
	rmessage->length += eamessage->length;
	advance_message(eamessage); // Step to next TLV

	// Set up CME node name for VSIS, add length to full message CV_TLV
	// then advance eamessage to next slot (good form even though we got nothing more)
	//
	hostlen = (int) strlen(localhost);
	if ( (hostlen > 0) && (hostlen < sizeof(localhost) ) )
	{
		eamessage->tag    = CVT_REG_CMENAME;
		eamessage->length = (USHORT) (sizeof(eamessage->tag) + sizeof(eamessage->length) + hostlen + 1);
		strncpy(eamessage->value, localhost, hostlen+1);
		rmessage->length += eamessage->length;
		advance_message(eamessage); // Step to next TLV
	}					

	// send data to vsis over socket
	int sentBytes = sendBufferToVSIS(vsisEntity, (char *)rmessage, rmessage->length); 

	if (sentBytes > 0)
	{
		vsisEntity._isCMERegistered = true;
		vsisEntity._cmeRegisteredTime = ZQ::common::now();

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_REGISTER message to %s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), rmessage->length, sentBytes);
	}
	else
	{
		vsisEntity._isCMERegistered = false;

		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_REGISTER message to %s failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), rmessage->length, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestUserCount(VSISEntity& vsisEntity, std::string& pid, std::string& paid)
{
#define UsrMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE) // 2 is sizeof(USHORT)

	char   usrmessage[UsrMessageLength];
	PCV_TLV   pm = (PCV_TLV)usrmessage;

	// "Outer" message TLV

	pm->tag    = CVM_SESS_COUNT_REQ;
	pm->length = UsrMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is char[] array

	// "Inner" TLV #1: PID

	pm->tag    = CVT_SCQ_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_SCQ_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	// Message is ready to send. 

	int sentBytes = sendBufferToVSIS(vsisEntity, usrmessage, UsrMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_SESS_COUNT_REQ message to %s with pid=%s paid=%s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), UsrMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_SESS_COUNT_REQ message to %s with for pid=%s paid=%s failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), UsrMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestDeletion(VSISEntity& vsisEntity, std::string& pid, std::string& paid)
{
#define DelMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE) // 2 is sizeof(USHORT)

	char  delmessage[DelMessageLength];
	PCV_TLV  pm=(PCV_TLV)delmessage;

	// "Outer" message TLV

	pm->tag    = CVM_DELETE_REQUEST;
	pm->length = DelMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is char[] array

	// "Inner" TLV #1: PID

	pm->tag    = CVT_DRQ_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_DRQ_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	// Message is ready to send. 

	int sentBytes = sendBufferToVSIS(vsisEntity, delmessage, DelMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_DELETE_REQUEST message to %s with pid=%s paid=%s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), DelMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_DELETE_REQUEST message to %s failed for pid=%s paid=%s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), DelMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestImport(VSISEntity& vsisEntity, std::string& pid, std::string& paid, uint32 pweCount)
{
#define ImpMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE + 2+2+4) // 2 is sizeof(USHORT)

	char  impmessage[ImpMessageLength];
	PCV_TLV  pm=(PCV_TLV)impmessage;

	// "Outer" message TLV

	pm->tag    = CVM_IMPORT_REQUEST;
	pm->length = ImpMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is char[] array

	// "Inner" TLV #1: PID

	pm->tag    = CVT_IMP_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_IMP_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	// "Inner" TLV #3: COUNT

	pm->tag    = CVT_IMP_COUNT;
	pm->length = sizeof(pweCount) + HEADER_BYTES;
	memcpy(pm->value, &pweCount, sizeof(pweCount));
	advance_message(pm);

	// Message is ready to send. 

	int sentBytes = sendBufferToVSIS(vsisEntity, impmessage, ImpMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_IMPORT_REQUEST message to %s with pid=%s paid=%s impcount=%d, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), pweCount, ImpMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_IMPORT_REQUEST message to %s failed for pid=%s paid=%s impcount=%d, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(),  pid.c_str(), paid.c_str(), pweCount, ImpMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestListAll(VSISEntity& vsisEntity)
{
	int listAllMessageLength = HEADER_BYTES;
	CV_TLV message = {CVM_LIST_ASSETS_REQUEST, listAllMessageLength, 0};

	int sentBytes = sendBufferToVSIS(vsisEntity, (char*)&message, listAllMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_LIST_ASSETS_REQUEST message to %s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), listAllMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_LIST_ASSETS_REQUEST message to %s failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(),  listAllMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

// prefer send the request the node which is importing the content
bool VSISCacheIO::requestAssetStatus(VSISEntity& vsisEntity, std::string& pid, std::string& paid)
{
#define StsMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE) // 2 is sizeof(USHORT)

	char  stsmessage[StsMessageLength];
	PCV_TLV   pm = (PCV_TLV)stsmessage;

	// "Outer" message TLV

	pm->tag    = CVM_STATUS_REQUEST;
	pm->length = StsMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is char[] array

	// "Inner" TLV #1: PID

	pm->tag    = CVT_STS_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_STS_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	//
	int sentBytes = sendBufferToVSIS(vsisEntity, (char*)stsmessage, StsMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_STATUS_REQUEST message to %s with pid=%s paid=%s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), StsMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_STATUS_REQUEST message to %s with pid=%s paid=%s failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), StsMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestAssetExpired(VSISEntity& vsisEntity, std::string& pid, std::string& paid)
{
#define ExpMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE) // 2 is sizeof(USHORT)

	char  expmessage[ExpMessageLength];
	PCV_TLV  pm=(PCV_TLV)expmessage;

	// "Outer" message TLV

	pm->tag    = CVM_EXPIRE_NOTICE;
	pm->length = ExpMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is inner TLV

	// "Inner" TLV #1: PID

	pm->tag    = CVT_EXP_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_EXP_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	//
	int sentBytes = sendBufferToVSIS(vsisEntity, expmessage, ExpMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_EXPIRE_NOTICE message to %s with pid=%s paid=%s, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), ExpMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_EXPIRE_NOTICE message to %s with pid=%s paid=%s failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), ExpMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::reqeustDeductAsset(VSISEntity& vsisEntity, std::string& pid, std::string& paid, float fraction)
{
#define DedMessageLength (2+2 + 2+2+PID_SIZE + 2+2+PAID_SIZE +2+2+FRAC_SIZE) // 2 is sizeof(USHORT)

	char  dedmessage[DedMessageLength];
	PCV_TLV  pm=(PCV_TLV)dedmessage;

	// "Outer" message TLV

	pm->tag    = CVM_DEDUCT_NOTICE;
	pm->length = DedMessageLength;
	pm         = (PCV_TLV) pm->value; // pm->value is inner TLV

	// "Inner" TLV #1: PID

	pm->tag    = CVT_DED_PID;
	pm->length = PID_SIZE + HEADER_BYTES;
	strncpy(pm->value, pid.c_str(), PID_SIZE);
	advance_message(pm);

	// "Inner" TLV #2: PAID

	pm->tag    = CVT_DED_PAID;
	pm->length = PAID_SIZE + HEADER_BYTES;
	strncpy(pm->value, paid.c_str(), PAID_SIZE);
	advance_message(pm);

	// "Inner" TLV #3: fraction

	pm->tag    = CVT_DED_FRAC;
	pm->length = FRAC_SIZE + HEADER_BYTES;
	memcpy(pm->value, &fraction, FRAC_SIZE);
	advance_message(pm);

	//
	int sentBytes = sendBufferToVSIS(vsisEntity, dedmessage, DedMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_DEDUCT_NOTICE message to %s with pid=%s paid=%s fraction=%6.4f, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), fraction, DedMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_DEDUCT_NOTICE message to %s with pid=%s paid=%s fraction=%6.4f failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), pid.c_str(), paid.c_str(), fraction, DedMessageLength, sentBytes);
	}

	return (sentBytes > 0);
}

bool VSISCacheIO::requestCMEPremeters(VSISEntity& vsisEntity, uint32 cushion, uint32 playTrigger)
{
#define SplMessageLength (2+2 + 2+2+sizeof(ULONG) + 2+2+sizeof(ULONG)) // Overall header, ULONGs for disk space, trigger

	char  splmessage[SplMessageLength];
	PCV_TLV  pm=(PCV_TLV)splmessage;

	// "Outer" message TLV

	pm->tag    = CVM_CME_PARAMETERS;  // 2
	pm->length = SplMessageLength;    // 12
	pm         = (PCV_TLV) pm->value; // pm->value is inner TLV

	// "Inner" TLV#1: Cushion Value

	pm->tag    = CVT_SPACE_PERCENT;     // 2
	pm->length = sizeof(uint32) + HEADER_BYTES;  // 2+2
	memcpy(pm->value, &cushion, sizeof(cushion));  // 2+2+sizeof(uint32)
	advance_message(pm);

	// "Inner" TLV#2: Import threshold

	pm->tag    = CVT_PLAYS_IMPORT_ALL;     // 2
	pm->length = sizeof(uint32) + HEADER_BYTES;  // 2+2
	memcpy(pm->value, &playTrigger, sizeof(playTrigger));  // 2+2+sizeof(uint32)
	advance_message(pm);

	//
	int sentBytes = sendBufferToVSIS(vsisEntity, splmessage, SplMessageLength);

	if (sentBytes > 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_CME_PARAMETERS message to %s with custion=%d, playTrigger=%d, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), cushion, playTrigger, SplMessageLength, sentBytes);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME sent CVM_CME_PARAMETERS message to %s with custion=%d, playTrigger=%d failed, sent %d:%d bytes"), 
			vsisEntity._name.c_str(), vsisEntity._ip.c_str(), cushion, playTrigger, SplMessageLength, sentBytes);
	}

	return (sentBytes > 0);
} 

void VSISCacheIO::processIncomingData(VSISEntity& vsisEntity)
{
	VSISEntity::REC_VSIS_DATA* vsisdata = NULL;
	VSISEntity::REC_VSIS_DATA* tmpData = NULL;

	char* buff = NULL;
	int bufflen = 0;
	uint32	processedBytes;
	uint32 pos = 0;

	bool ret, relBuff, relVsisData;

	// process the data 
	ZQ::common::MutexGuard buffGuard(vsisEntity._buffLocker);
	while(!vsisEntity._bufferQueue.empty())
	{
		relBuff = false;
		relVsisData = false;

		vsisdata = vsisEntity._bufferQueue.front();
		vsisEntity._bufferQueue.pop();

		// test only
//		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEConnDialog, "%s: processIncomingData() process VSIS data #%d"), 
//			_clusterID.c_str(), vsisdata->No);

//		if(vsisdata->length < _postHouseEnv.getReadBufferSize())
		if(vsisdata->length)
		{	// the received data is less than socket buff size

			if(0 == vsisEntity._addupLength)
			{	// there is no previous data need to assembled
				buff = (char*) vsisdata->data;
				bufflen = vsisdata->length;	

				relVsisData = true; // this vsisdata need to be released
			}
			else
			{	
				vsisEntity._tempVSISDatas.push_back(vsisdata);
				vsisEntity._addupLength += vsisdata->length;
				
				buff = new char[vsisEntity._addupLength];
				bufflen = vsisEntity._addupLength;

				pos = 0;
				for(size_t i=0; i<vsisEntity._tempVSISDatas.size(); i++)
				{
					tmpData = vsisEntity._tempVSISDatas[i];

					memcpy(buff+pos, tmpData->data, tmpData->length);
					pos += tmpData->length;

					delete tmpData;
				}

				vsisEntity._addupLength = 0;
				vsisEntity._tempVSISDatas.clear();

				relBuff = true;		// this assembled buff need to be released
			}
		}
		else
		{	// there is an incomplete data
			vsisEntity._tempVSISDatas.push_back(vsisdata);
			vsisEntity._addupLength += vsisdata->length;

			continue;
		}
		
		// process the buffer
		PCV_TLV pm = NULL;
		ret = true;
		processedBytes = 0;
		while(ret && (processedBytes < bufflen) )
		{
			pm = (PCV_TLV)buff;
			ret = processVSISBuff(vsisEntity, buff, bufflen, processedBytes);
			if(ret)
			{
				if(CVM_SESSION_END_NOTICE != pm->tag) // did not print CVM_SESSION_END_NOTICE log, too many of this kind of msgs and this msg was not used. 
				{
					//  if (bufflen != processedBytes), means there is duty data, we just drop unprocessed bytes without merging to next buffer. 
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME processed buff for tag %s, processed %d:%d bytes"), 
						vsisEntity._name.c_str(), VSISCODE_TEXT(pm->tag).c_str(), bufflen, processedBytes);
				}
				//
				buff += processedBytes;
				bufflen -= processedBytes;
				processedBytes = 0;
			}
		}
		// release memory based on real situation
		if(relVsisData)
		{
			delete vsisdata;
		}
		else if(relBuff)
		{
			delete buff;
		}
	}
}

bool VSISCacheIO::processVSISBuff(VSISEntity& vsisEntity, char* buff, uint32 buffSize, uint32& processedBytes)
{
	bool ret = false;

	PCV_TLV  pm=(PCV_TLV)buff;

	switch(pm->tag)
	{
	case CVM_REGISTER:
		ret= VSISRegister(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_VSIS_CDN_STATE:
		ret= VSISCDNState(vsisEntity, pm, buffSize, processedBytes);	
		break;

	case CVM_ASSET_STATUS:
		ret = VSISAssetStatus(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_LIST_ASSETS_RESPONSE:
		ret = VSISAssetList(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_SESSION_START_NOTICE:
		ret = VSISSessionStart(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_SESSION_END_NOTICE:
		ret = VSISSessionEnd(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_DISK_USAGE_REPORT:
		ret = VSISDiskUsage(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_SESS_COUNT_RSP:
		ret = VSISSessionCount(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_SMART_PARAMETERS:
		ret = VSISSmartParameters(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_VOD_TLV_DATA:
		ret = VSISVodTlvData(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_VOD_POP_DATA:
		ret = VSISVodPopData(vsisEntity, pm, buffSize, processedBytes);
		break;

	case CVM_LAST_VOD_POP_DATA:
		ret = VSISLastVodPopData(vsisEntity, pm, buffSize, processedBytes);
		break;

	default:
		// don't process, but need to set the processedBytes
		processedBytes = pm->length;
		break;
	}	

	return ret;
}

bool VSISCacheIO::VSISRegister(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char tmpbuf[100];
	uint32 protocol = 0;
	uint64 bandwidth = 0;

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after CVM_REGISTER
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{

		case CVT_REG_PROTOCOL:	// Protocol level (not really used, yet)
			memcpy(&protocol, &leftmessage->value, sizeof(uint32));
			break;

		case CVT_REG_VSISCLU:	// VSIS (i.e., Vstrm) cluster name
			memcpy(&tmpbuf, &leftmessage->value, leftmessage->length - HEADER_BYTES);
			vsisEntity._nodeName = tmpbuf;
			break;

		case CVT_REG_NODEBPS:	// Bandwidth capacity of a node in bits/second
			memcpy(&bandwidth, &leftmessage->value, sizeof(uint64));
			break;

		case CVT_REG_FRAGMENT:	// Reminder so when fragment code doesn't work we know why
		case CVT_REG_FRAGCOUNT:
			// glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_REGISTER message from VSIS, don't process inner tag %d"), 
			//	vsisEntity._name.c_str(), tag);
			break;

		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_REGISTER message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}  // switch(tag)

		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while (bytes > 0)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_REGISTER message from VSIS, cluster=%s protocal=%d bandwidth=%llu bps"), 
		vsisEntity._name.c_str(), vsisEntity._nodeName.c_str(), protocol, bandwidth);

	if (0 == bandwidth)
		bandwidth = 3750000 * 1500;

	// VSIS registered
	vsisEntity._isVSISRegistered = true;
	vsisEntity._cmeRegisteredTime = ZQ::common::now();

	// received register, send CME parameter to VSIS
	bool ret = requestCMEPremeters(vsisEntity, _cmeAlg.getCushion(), _cmeAlg.getImportTrigger());
	
	// if not set, need to resend in the thread loop
	vsisEntity._isCMEPremetersSent = ret;

	// update status
	vsisEntity.setVSISStatus(true);			// need assume node status is avaible, the status may be changed in VSISCDNState
	vsisEntity.setTotalOutputBW(bandwidth);
	
	return true;
}  // VSISRegister()

bool VSISCacheIO::VSISCDNState(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	uint32 status = 0;

	char cluid[100]={0};
	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;
	
	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_CDN_CLUID:		// Cluster ID
			strncpy(cluid, leftmessage->value, CLUID_SIZE*sizeof(char));
			break;				// Well, right now we assume sender

		case CVT_CDN_NODEID:	// Node ID
			break;				// Well, right now we assume sender

		case CVT_CDN_STATUS:	// Connectivity boolean
			if (len == 8)
				memcpy((char *)&status, (char *)&leftmessage->value, 4);
			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_VSIS_CDN_STATE message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while (bytes > 0)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_VSIS_CDN_STATE message from VSIS, CluserID=%s, status=%d"), 
		vsisEntity._name.c_str(), cluid, status);
	
	// update status, 1: CDN is up, 0: CDN is down
	vsisEntity.setVSISStatus((1 == status));
	
	return true;
}

bool VSISCacheIO::VSISAssetStatus(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char pid[100]={0};
	char paid[100]={0};
	char cluid[100]={0};
	uint32 status=0;
	bool bPWE = false;
	uint64 bitrate = 0;
	uint64 size = 0;
	uint64 duration = 0; 

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_ASS_STS:
			status = *(uint32 *)(leftmessage->value);
			break;

		case CVT_ASS_PID:
			strncpy(pid, leftmessage->value, PID_SIZE*sizeof(char));
			break;

		case CVT_ASS_PAID:
			strncpy(paid, leftmessage->value, PAID_SIZE*sizeof(char));
			break;

		case CVT_ASS_CLUID:
			strncpy(cluid, leftmessage->value, CLUID_SIZE*sizeof(char));
			break;

		case CVT_ASS_BITRATE:
			bitrate = *(uint64 *)(leftmessage->value);	// Asset bit rate (bits/sec) int64
			break;

		case CVT_ASS_SIZE:
			size   = *(uint64 *)(leftmessage->value);	// Asset size in bytes
			break;

		case CVT_ASS_DURATION:
			duration = *(uint64 *)(leftmessage->value);	// Asset duration (seconds)
			break;

		case CVT_ASS_PWE:
			bPWE   = true;
			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_ASSET_STATUS message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while (bytes > 0)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_ASSET_STATUS message from VSIS, cluster=%s pid=%s paid=%s status=%s size=%llu bytes duration=%llu seconds bitrate=%llu bps isPWE=%d"), 
		vsisEntity._name.c_str(), cluid, pid, paid, VSIS_ASSET_STATUS_TEXT(status).c_str(), size, duration, bitrate, bPWE ? 1 : 0);
	
	// check if this is the response of Querying PWE flag
	std::string spid = std::string(pid);
	std::string spaid = std::string(paid);
	std::string paidpid = PAID_PID(spaid,spid);

	SIMPLE_CONTENTS::iterator ittmp = _tempContents.find(paidpid);
	if(ittmp != _tempContents.end())
	{	// find this content, means this content is waiting for pwe flag
		SIMPLE_CONTENT_INFO* simContent = (SIMPLE_CONTENT_INFO*) ittmp->second;
		
		// do the next thing depends pwe value
		if(!bPWE)
		{	// non-pwe, do the import right away
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s paid=%s got PWE flag=%d, going to import from VSIS"), 
				_clusterID.c_str(), spid.c_str(), spaid.c_str(), 0);

			doImportContent(spid, spaid);

			_tempContents.erase(ittmp);
			delete simContent;
		}
		else
		{	// pwe, need to query usercount again
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s paid=%s got PWE flag=%d, going to query UserCount"), 
				_clusterID.c_str(), spid.c_str(), spaid.c_str(), 1);

			simContent->isPWE = true;
			simContent->timestamp = ZQ::common::now();
			doReqUserCount(*simContent);
		}
	}
	else
	{
		// this is update is not for the query before importing
		Content::CONTENT_STATUS cntStat;
		bool ret = VSIS_STAT_2_CONT_STAT(status, cntStat);
		if(ret)
		{
			_cstorage->onStatusUpdate(spid, spaid, cntStat, (uint32)bitrate, (uint32)duration, size);
		}
		else
		{	// query the content info, put into the queue, which will be processed later. 
			PROPERTIES properties;
			readContent(spid, spaid, properties);
		}
	}
	// update status
	return true;
}

// Process a MULTI_SZ list of asset "names" from a CVT_LAR_ASSETS item in a
// CVM_LIST_ASSETS_RESPONSE message.
//
// Function:
//   Break apart a MULTI_SZ character sequence into individual elements,
//   called "asset names". Each asset name is processed, and we go on to
//   the next until an asset name starts with NUL, indicating there are
//   no more assets.
//
//   Each "asset name" represents an asset PID and PAID combined in a
//   nonstandard format known only to VSIS and this routine. And that
//   format is: 20 bytes PAID, remainder PID. Thus the "asset name"
//
//       ERBU1234567890123456999Priscilla.com
//
//   translates into 20 bytes (exactly) of PAID ERBU1234567890123456
//   and the remainder 999Priscilla.com is the PID.
//
//   A more recent format starts with a + sign, a viewer count, and the
//   PAID/PID as above, i.e.,
//
//       +0047ERBU1234567890123456999Priscilla.com
//
//	 Even more recently the string above is prefixed with "\c" where 'c' 
//	 is the separator between the AssetID and the PID.
//
//   This routine breaks each asset name into PID/PAID components and
//   inserts the asset into the given cluster's CachedAssetTree. It also
//   sets the viewer count if supplied.
bool VSISCacheIO::VSISAssetList(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	bool cluTagged = false;
	char cluid[100] = {0};
	char vct[VIEWER_BYTES+1];
	char pid[100]={0};
	char paid[100]={0};
	int assetcount = 0;
	char pidPaidSeparator = 0;

	uint32 bytes = 0;
	size_t szlen=0;				// String length
	uint8 vc=0;				    // Viewer count for each asset

	std::vector<std::string> pidpaids;

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_LAR_CLUID:
			memcpy(cluid, leftmessage->value, min(len, CLUID_SIZE*sizeof(char)));
			cluTagged = true;
			break;

		case CVT_LAR_ASSETS:
			if (cluTagged)
			{
				int lenofmsz = len - HEADER_BYTES;	// total length of array of +<vc><paid><pid> 
				char* msz = leftmessage->value;			// start point of the array

				int paidLenth, pidLength;
				char* pidPos;
				char* sepPos;
				bool bOK;

				while(msz && *msz && (lenofmsz>0))	// the asset info ended with NULL
				{
					// code change @ 2014-08-22 to support separator between PAID and PID
					// see if VSIS sent us a separator char
					if (*msz == '\\')
					{
						msz++;
						pidPaidSeparator = *msz++;
					}
					else
					{
						pidPaidSeparator = 0;
					}

					if (*msz != '+')
					{
						vc = 0;						// No viewer count present
					}
					else
					{
						memcpy(vct, msz+1, VIEWER_BYTES);
						vct[VIEWER_BYTES] = 0;

						vc      = atoi(vct);
						lenofmsz -= (VIEWER_BYTES+1);
						msz     += (VIEWER_BYTES+1);
					}

					szlen = strlen(msz);		// length of PAIDPID
					if (0 == pidPaidSeparator && szlen <= _paidLength) // Need at least 1 byte for PID in case of fixed PAID length
					{
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE detected PAIDPID %s length is %d which is less than %d"), 
							vsisEntity._name.c_str(), msz, szlen, _paidLength);
					}
					else
					{
						bOK = true;

						// default values, it will be updated if found the separator
						paidLenth = _paidLength;
						pidPos = msz+_paidLength;
						pidLength = szlen-_paidLength;

						if(0 != pidPaidSeparator)
						{
							sepPos = strchr(msz, pidPaidSeparator);	
							if(NULL != sepPos)
							{	// find the separator
								if(sepPos < msz+szlen-1)
								{	
									paidLenth = sepPos - msz;
									pidPos = sepPos+1;	
									pidLength = szlen - paidLenth - 1;
								}
								else
								{	// the separator is the last character, means no pid available
									glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE detected PAIDPID %s does not include pid"), 
										vsisEntity._name.c_str(), msz);

									bOK = false;
								}
							}
							else
							{
								glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE detected PAIDPID %s does not include sepator %c"), 
									vsisEntity._name.c_str(), msz, pidPaidSeparator);

								bOK = false;
							}
						}

						// found the PAID & PID
						if(bOK)
						{
							strncpy(paid, msz, paidLenth); // Trailing null appended by strncpy_s()
							paid[paidLenth] = 0;
							strncpy(pid, pidPos, pidLength);
							pid[pidLength] = 0;

							assetcount++;
							
							glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CVM_LIST_ASSETS_RESPONSE returned No.%04d asset pid=%s paid=%s"), 
								vsisEntity._name.c_str(), assetcount, pid, paid);

							pidpaids.push_back(std::string(pid));
							pidpaids.push_back(std::string(paid));
						}
					}
					bytes = szlen + 1; // bytes of a paidpid string, count the \0

					lenofmsz -= bytes;	// Did this many
					msz      += bytes;	// On to next string
				}
			}
			else
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE message from VSIS, no cluster ID presented"), 
					vsisEntity._name.c_str());
			}
			break;

		case CVT_LAR_STATUS:
			// TBD if we need to note something to indicate we're done here
			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while (bytes > 0)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_LIST_ASSETS_RESPONSE message from VSIS, cluster=%s total %d asset"), 
		vsisEntity._name.c_str(), cluid, assetcount);

	size_t i=0;
	assetcount = 0;
	std::string spid, spaid;
	for(i=0; i<pidpaids.size(); i=i+2)
	{
		assetcount++;
		spid = pidpaids[i];
		spaid = pidpaids[i+1];

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CVM_LIST_ASSETS_RESPONSE No.%04d asset pid=%s paid=%s, query its status"), 
			vsisEntity._name.c_str(), assetcount, spid.c_str(), spaid.c_str());

		requestAssetStatus(vsisEntity, spid, spaid);

		if(rand()/20 == i) // halt for each 10 content
			sleep(1);
	}
 
	return true;
}

bool VSISCacheIO::VSISSmartParameters(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char diskID[DISKID_SIZE]={0};
	uint32 erases = 5000;		// Default, DACAPI supplies from built-in table
	uint32 writeamp = 3;		// Default, not really that reliable but right now not needed
	uint32 onhours = 0;			// No default
	uint32 hdlife = 999990;	    // % of drive life remaining (too big == NOT PRESENT)

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_SMT_CLUID:
			break;

		case CVT_SMT_DISKID:
			memcpy(diskID, leftmessage->value, min(len, DISKID_SIZE*sizeof(char)));
			break;

		case CVT_SMT_ERASES:
			memcpy(&erases, leftmessage->value, min(len, sizeof(uint32))); 
			break;

		case CVT_SMT_ON_HOURS:
			memcpy(&onhours, leftmessage->value, MIN(len, sizeof(uint32))); // ULONG
			break;

		case CVT_SMT_USED_LIFE:
			memcpy(&hdlife, leftmessage->value, MIN(len, sizeof(uint32))); // ULONG
			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_REGISTER message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // (leftBytes > 0 && leftBytes <= (int)msgSize)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_SMART_PARAMETERS message from VSIS"), 
		vsisEntity._name.c_str());

	// TODO: add start parameter process logic

	return true;
}

bool VSISCacheIO::VSISSessionStart(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char cluid[CLUID_SIZE]={0};
	char pid[PID_SIZE]={0};
	char paid[PAID_SIZE]={0};
	uint32 sid     = 0;

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_SSN_CLUID:
			memcpy(cluid, leftmessage->value, min(len, CLUID_SIZE*sizeof(char)));
			cluid[len] = 0;
			break;

		case CVT_SSN_PID:
			memcpy(pid, leftmessage->value, min(len, PID_SIZE*sizeof(char)));
			pid[len] = 0;
			break;

		case CVT_SSN_PAID:
			memcpy(paid, leftmessage->value, min(len, PAID_SIZE*sizeof(char)));
			paid[len] = 0;
			break;

		case CVT_SSN_SID:
			memcpy(&sid, leftmessage->value, min(len, sizeof(uint32))); // ULONG
			break;			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_SESSION_START_NOTICE message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while(leftBytes > 0 && leftBytes <= (int)msgSize)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_SESSION_START_NOTICE message from VSIS, cluster=%s pid=%s paid=%s sid=%d"), 
		vsisEntity._name.c_str(), cluid, pid, paid, sid);

	// old CME do nothing for this tag

	return true;
}

bool VSISCacheIO::VSISSessionEnd(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	// old CME did not implement this. It should because VSIS won't send this message anymore
	
	// but still jump to the end of the message in case this message were sent by VSIS
	processedBytes = message->length;
	return true;
}

bool VSISCacheIO::VSISSessionCount(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char cluid[CLUID_SIZE] = {0}; // No CPU drain from zeroing memory, because
	char pid[PID_SIZE]     = {0}; //  this code is only called for PWE assets,
	char paid[PAID_SIZE]   = {0}; //  which is pretty rare, all things considered
	uint8 playcount        = 0; // Viewers, if any. Default 0 if no value supplied.

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_SCR_CLUID:
			// You know, we already have pclu, so don't need this right now
			break;

		case CVT_SCR_PID:
			memcpy(pid, leftmessage->value, min(len, PID_SIZE*sizeof(char)));
			pid[len] = 0;
			break;

		case CVT_SCR_PAID:
			memcpy(paid, leftmessage->value, min(len, PAID_SIZE*sizeof(char)));
			paid[len] = 0;
			break;

		case CVT_SCR_COUNT:
			playcount = *(uint8 *)(leftmessage->value);
			break;
		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_SESS_COUNT_RSP message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while(leftBytes > 0 && leftBytes <= (int)msgSize)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_SESS_COUNT_RSP message from VSIS, pid=%s paid=%s playcount=%d"), 
		vsisEntity._name.c_str(), pid, paid, playcount);

	// check if this is the response of Querying PWE flag
	std::string spid = std::string(pid);
	std::string spaid = std::string(paid);
	std::string paidpid = PAID_PID(spaid,spid);

	SIMPLE_CONTENTS::iterator ittmp = _tempContents.find(paidpid);
	if(ittmp != _tempContents.end())
	{	// find this content, means this content is waiting for pwe flag
		SIMPLE_CONTENT_INFO* simContent = (SIMPLE_CONTENT_INFO*) ittmp->second;
		
		// sum the total playcount
		simContent->playCount += playcount;
		// do the next thing depends pwe value
		simContent->reportedVsis++;
		// update timestamp
		simContent->timestamp = ZQ::common::now();

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s, paid=%s received usercount=%d from %s, currently total playCount=%d, reportedVSIS=%d, totalVSIS=%d"), 
			_clusterID.c_str(), spid.c_str(), spaid.c_str(), playcount, vsisEntity._name.c_str(), 
			simContent->playCount, simContent->reportedVsis, simContent->totalVsis);

		// check if all vsis reported
		if(simContent->reportedVsis == simContent->totalVsis)
		{	// we got all the VSIS usercount reported
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s, paid=%s all vsis reported UserCount, total UserCount=%d. Going to import it"), 
				_clusterID.c_str(), spid.c_str(), spaid.c_str(), playcount);

			doImportContent(spid, spaid, true, simContent->playCount);

			_tempContents.erase(ittmp);
			delete simContent;
		}
	}

	return true;
}

bool VSISCacheIO::VSISDiskUsage(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	char cluid[CLUID_SIZE]={0};
	uint64 capacity		= 0;		// Leave these signed to allow for the possibility
	uint64 free			= 0;		//  of having negative values with "special" meanings.
	uint64 bpsUsed		= 0;
	uint64 bpsLimit		= 0;
	uint64 streamingLimit	= 0;
	uint64 streamingLoad	= 0;

	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		switch(tag)
		{
		case CVT_DISK_CLUID:
			memcpy(cluid, leftmessage->value, min(len, CLUID_SIZE*sizeof(char)));
			cluid[len] = 0;
			break;

		case CVT_DISK_AVAIL:
			capacity = *(uint64 *)(leftmessage->value);
			break;

		case CVT_DISK_FREE:
			free     = *(uint64 *)(leftmessage->value);
			break;

		case CVT_DISK_BW_USED:
			bpsUsed  = *(uint64 *)(leftmessage->value);
			break;

		case CVT_DISK_BW_LIMIT:
			bpsLimit = *(uint64 *)(leftmessage->value);
			if (bpsLimit <= 0)
				bpsLimit = 1000000000; // 1 Gbit/sec default for "no limit"
			break;

		case CVT_STREAMING_USED:
			streamingLoad	= *(uint64 *)(leftmessage->value);
			break;

		case CVT_STREAMING_LIMIT:
			streamingLimit	= *(uint64 *)(leftmessage->value);
			break;

		default:
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_DISK_USAGE_REPORT message from VSIS, found undefined tag %d"), 
				vsisEntity._name.c_str(), tag);
			// should break the loop, how...
			break;
		}
		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while(leftBytes > 0 && leftBytes <= (int)msgSize)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_DISK_USAGE_REPORT message from VSIS, cluster=%s capacity=%llu MB free=%llu MB totalImportBW=%llu bps usedImportBW=%llu bps streamingLimit=%llu bps streamingLoad=%llu bps"), 
		vsisEntity._name.c_str(), cluid, capacity/1000000, free/1000000, bpsLimit, bpsUsed, streamingLimit, streamingLoad);

	// update bandwidth
	vsisEntity.setBandwidth(bpsUsed, bpsLimit, streamingLoad, streamingLimit);

	// update total cluster bandwidth
	setRecentImportBW();

	// storage usage
	_cstorage->onUsageUpdate(free, capacity);

	return true;
}

bool VSISCacheIO::VSISVodTlvData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	processedBytes = 0;
	PCV_TLV leftmessage = NULL;
	int leftBytes = 0;

	// step to next after "outer" TLV
	leftmessage = (PCV_TLV) ((char *)message + HEADER_BYTES);
	leftBytes = message->length - HEADER_BYTES;		// Bytes left to go
	msgSize -= HEADER_BYTES;						// left message size
	processedBytes = HEADER_BYTES;					// count the "outer" head length

	while (leftBytes > 0 && leftBytes <= (int)msgSize)	// make sure no access vialation
	{
		int tag = leftmessage->tag;	// Get next tag
		int len = leftmessage->length;

		//
		// we do nothing here, just loop the TLV to the end. 
		// Old CME go through the "inner" TLVs to get data, but did not use them. 
		//

		processedBytes += leftmessage->length;	// processed message
		msgSize -= leftmessage->length;			// left un-processed buff lengh 
		leftBytes -= leftmessage->length;		// left msg length of "inner" TLV
		advance_message(leftmessage);			// Advance to next inner CV_TLV if any
	}  // while(leftBytes > 0 && leftBytes <= (int)msgSize)

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "[MESSAGE] %s: CME received CVM_REGISTER message from VSIS"), 
		vsisEntity._name.c_str());

	return true;
}

bool VSISCacheIO::VSISVodPopData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	// old CME did not process this tag, it should becuase VSIS won't send this msg anymore

	// but still jump to the end of the message in case this message were sent by VSIS
	processedBytes = message->length;
	return true;
}

bool VSISCacheIO::VSISLastVodPopData(VSISEntity& vsisEntity, PCV_TLV message, uint32 msgSize, uint32& processedBytes)
{
	// but still jump to the end of the message in case this message were sent by VSIS
	processedBytes = message->length;
	return true;
}

bool VSISCacheIO::isReady()
{
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = itvsis->second;

		if(vsisEntity->isIOReady())
			return true;
	}
	return false;
}

bool VSISCacheIO::isActivity()
{
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = itvsis->second;

		if(vsisEntity->isActivity())
			return true;
	}
	return false;
}

IO_OP_RESULT VSISCacheIO::importContent(std::string& pid, std::string& paid)
{
	IOEvent* ioevt = new IOEvent(IO_OP_IMPORT, pid, paid);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);
	
	_waitEvent.signal();

	return IO_PENDING;
}

IO_OP_RESULT VSISCacheIO::deleteContent(std::string& pid, std::string& paid)
{
	IOEvent* ioevt = new IOEvent(IO_OP_DELETE, pid, paid);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);

	_waitEvent.signal();

	return IO_PENDING;
}

IO_OP_RESULT VSISCacheIO::readContent(std::string& pid, std::string& paid, PROPERTIES& properites)
{
	IOEvent* ioevt = new IOEvent(IO_OP_SYNCONE, pid, paid);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);
	
	_waitEvent.signal();

	return IO_PENDING;
}

IO_OP_RESULT VSISCacheIO::listContents(CONTENTS& contents)
{
	IOEvent* ioevt = new IOEvent(IO_OP_SYNCALL);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);
	
	_waitEvent.signal();

	return IO_PENDING;
}

void VSISCacheIO::onDataReceived(VSISEntity& vsis)
{
	IOEvent* ioevt = new IOEvent(IO_DATA_ARRIVE, vsis);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);

	_waitEvent.signal();
}

void VSISCacheIO::onConnectionLost(VSISEntity& vsis)
{
	IOEvent* ioevt = new IOEvent(IO_CONN_LOST, vsis);

	ZQ::common::MutexGuard evtGuard(_eventLock);
	_eventQueue.push(ioevt);

	_waitEvent.signal();
}


bool VSISCacheIO::doImportContent(IOEvent& evt)
{
	// save this temprary since we need to get the pwe flag & usercount(pwe=true) before really import the content
	SIMPLE_CONTENTS::iterator ittmp = _tempContents.find(PAID_PID(evt.paid,evt.pid));
	if(ittmp == _tempContents.end())
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: content pid=%s paid=%s going to get the PWE flag before really import it. "), 
			_clusterID.c_str(), evt.pid.c_str(), evt.paid.c_str());

		SIMPLE_CONTENT_INFO* tempCnt = new SIMPLE_CONTENT_INFO(evt.pid, evt.paid);

		_tempContents.insert(SIMPLE_CONTENTS::value_type(PAID_PID(evt.paid,evt.pid), tempCnt));

		doSyncOneAsset(evt);
	}
	return true;
}
bool VSISCacheIO::doImportContent(std::string& pid, std::string& paid, bool pwe, uint32 userCount)
{
	bool ret = false;
	size_t vsisCount = _vsisEntities.size();

	VSISEntity* preferVSIS = _cmeAlg.candidateVSISEntity(_vsisEntities);

	if(NULL != preferVSIS)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s is selected to import content pid=%s paid=%s"), 
			_clusterID.c_str(), preferVSIS->_name.c_str(), pid.c_str(), paid.c_str());

		// consume some bandwidth for calcuation purpose
		preferVSIS->consumeImportBW(DEFAULT_IMPORT_CONSUME_BW);

		// send import request to the prefered vsis
		ret = requestImport(*preferVSIS, pid, paid, userCount);
		if(1 == vsisCount)
		{
			return ret;
		}
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(VSISCacheIO, "%s: no VSIS is prefered to import content pid=%s paid=%s"), 
			_clusterID.c_str(), pid.c_str(), paid.c_str());

		// the status back to CNT_CACHE_FAIL
		_cstorage->onStatusUpdate(pid, paid, Content::CNT_CACHE_FAIL, 0, 0, 0);

		return false;
	}

	uint32 leftCount = ret ? (_cstorage->getSubFileCount()-1) : _cstorage->getSubFileCount();

	if(0 == leftCount)
	{
		// no more subfiles need to be cached
		return ret;
	}
	// to see if need to send msg to the other VSIS to support parallel importing subfiles
	// VSIS did not support parallel import sub files in a node, but support it in multiple node if CME send the same request to them
	uint32 count = min(vsisCount, leftCount);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS import content pid=%s paid=%s from %d VSIS because subFiles=%d, vsisCount=%d"), 
		_clusterID.c_str(), pid.c_str(), paid.c_str(), count, _cstorage->getSubFileCount(), vsisCount);

	VSISENTITIES::iterator itvsis;	
	VSISEntity* vsisEntity = NULL;
	uint32 i=0;
	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{

		vsisEntity = (VSISEntity*) itvsis->second;
		
		if(preferVSIS == vsisEntity)
			continue;			// skip this one

		sleep(2000);     // Because VSIS use Find API to determine which sub file need to be imported my itself
		                 // to avoid these requests to different nodes hit each other, it is necessary to sleep awhile. 
		                 // old CME use 2 seconds. 

		// send msg to another VSIS to import the other subfiles
		if( vsisEntity->isConnected() && vsisEntity->isAvailable() 
			&& vsisEntity->isRecentReported() && (vsisEntity->getFreeImportBW() > 0)
			&& requestImport(*vsisEntity, pid, paid, userCount) )
		{
			// consume some bandwidth for calcuation purpose
			vsisEntity->consumeImportBW(DEFAULT_IMPORT_CONSUME_BW);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s import content pid=%s paid=%s"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), pid.c_str(), paid.c_str());

			i++;
		}

		if(count == i)			// reach the max
		{
			break;
		}

	}// loop of nodes

	return true;
}

bool VSISCacheIO::doDeleteAsset(IOEvent& evt)
{
	size_t vsiscount = _vsisEntities.size();
	int selIndex = rand() % (int)vsiscount;

	int i=0; 
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i != selIndex)
			continue;
		
		i++;
		vsisEntity = (VSISEntity*) itvsis->second;
		
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestDeletion(*vsisEntity, evt.pid, evt.paid) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s delete content pid=%s paid=%s"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), evt.pid.c_str(), evt.paid.c_str());

			return true;
		}
	}// loop of nodes
	
	// if above didnot execute, try others
	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i == selIndex)
			continue;

		vsisEntity = (VSISEntity*) itvsis->second;
		
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestDeletion(*vsisEntity, evt.pid, evt.paid) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s delete content pid=%s paid=%s"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), evt.pid.c_str(), evt.paid.c_str());

			return true;
		}
	}// loop of nodes

	return false;
}

bool VSISCacheIO::doSyncOneAsset(IOEvent& evt)
{
	size_t vsiscount = _vsisEntities.size();
	int selIndex = rand() % (int)vsiscount;

	int i=0; 
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i != selIndex)
			continue;
		
		i++;
		vsisEntity = (VSISEntity*) itvsis->second;
			
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestAssetStatus(*vsisEntity, evt.pid, evt.paid) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s query content pid=%s paid=%s status"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), evt.pid.c_str(), evt.paid.c_str());

			return true;
		}
	}// loop of nodes

	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i == selIndex)
			continue;

		vsisEntity = (VSISEntity*) itvsis->second;
		
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestAssetStatus(*vsisEntity, evt.pid, evt.paid) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s query content pid=%s paid=%s status"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), evt.pid.c_str(), evt.paid.c_str());

			return true;
		}
	}// loop of nodes
	
	return false;
}

bool VSISCacheIO::doSyncAllAsset()
{
	size_t vsiscount = _vsisEntities.size();
	int selIndex = rand() % (int)vsiscount;

	int i=0; 
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;
	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i != selIndex)
			continue;
		
		i++;
		vsisEntity = (VSISEntity*) itvsis->second;
				
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestListAll(*vsisEntity) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s list all content"), 
				_clusterID.c_str(), vsisEntity->_name.c_str());

			return true;
		}
	}// loop of nodes
	
	for(i=0, itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		if(i == selIndex)
			continue;

		vsisEntity = (VSISEntity*) itvsis->second;
			
		// send msg to the vsis
		if( vsisEntity->_isCMERegistered &&  requestListAll(*vsisEntity) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s list all content"), 
				_clusterID.c_str(), vsisEntity->_name.c_str());

			return true;
		}
	}// loop of nodes

	return false;
}

bool VSISCacheIO::doReqUserCount(SIMPLE_CONTENT_INFO& simpleCnt)
{
	int count = 0;
	VSISEntity* vsisEntity = NULL;
	VSISENTITIES::iterator itvsis;

	for(itvsis=_vsisEntities.begin(); itvsis!=_vsisEntities.end(); itvsis++)
	{
		vsisEntity = (VSISEntity*) itvsis->second;
		
		// send msg to all vsis
		if(vsisEntity->_isCMERegistered) 
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISCacheIO, "%s: VSIS %s request UserCount for content pid=%s paid=%s"), 
				_clusterID.c_str(), vsisEntity->_name.c_str(), simpleCnt.pid.c_str(), simpleCnt.paid.c_str());

			count++;
			requestUserCount(*vsisEntity, simpleCnt.pid, simpleCnt.paid);
		}
	}// loop of vsis
	
	simpleCnt.totalVsis = count;
	simpleCnt.timestamp = ZQ::common::now();

	return false;
}


}