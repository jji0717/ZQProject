#include <FileSystemOp.h>
#include <ContentImpl.h>
#include <TimeUtil.h>
#include <ContentProvisionWrapper.h>
#include "EdgeConfig.h"
#include <ContentSysMD.h>
#include <CacheStoreImpl.h>
#include <ContentUser.h>
#include <CPHInc.h>
#include <NativeThread.h>
#include <UDPSocket.h>
#include <Guid.h>
#include <TianShanIceHelper.h>

#include <pthread.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <IndexFileParser.h>

#include <C2HttpClient.h>
#include <ParseIndexFile.h>
#include <CDNDefines.h>
#include <urlstr.h>
//#define DEFAULT_WATCH_MASK (IN_CREATE|IN_MODIFY|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM)
#define DEFAULT_WATCH_MASK (IN_CREATE|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM|IN_CLOSE_WRITE)
#define BUFFER_SIZE 1024

using namespace ZQTianShan::ContentProvision;
namespace ZQTianShan {
namespace ContentStore {

#define storelog (store._log)

void CacheStoreImpl::initializePortal(CacheStoreImpl& store)
{
#pragma message ( __TODO__ "impl here")
}

void CacheStoreImpl::uninitializePortal(CacheStoreImpl& store)
{
#pragma message ( __TODO__ "impl here")
}

std::string CacheStoreImpl::portalSubfileToFileExtname(const CacheStoreImpl& store, const std::string& subType)
{

	size_t pos = subType.find_first_not_of(". \t");
	if (pos >=0)
		return subType.substr(pos);
		
	return "";
}

int CacheStoreImpl::portalContentNameToken(const CacheStoreImpl& store, const std::string& contentName, TianShanIce::Properties& contentNameField)
{
	size_t pos = contentName.find_last_of("\\/");
	std::string paid =  (std::string::npos == pos) ? contentName : contentName.substr(pos+1);
	std::string pid;

	ZQ::common::Guid guid(paid.c_str());

	if (guid.isNil() && paid.length() > store._paidLength)
	{
		pid  = paid.substr(store._paidLength);
		paid = paid.substr(0, store._paidLength);

		pos = pid.find_first_not_of(".-_ \t@");
		if (std::string::npos == pos)
			pid = "";
		else if (0 != pos)
			pid = pid.substr(pos);
	}

	MAPSET(TianShanIce::Properties, contentNameField, METADATA_ProviderAssetId, paid);
	MAPSET(TianShanIce::Properties, contentNameField, METADATA_ProviderId, pid);

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "portalContentNameToken()paid[%s] pid[%s]"), paid.c_str(), pid.c_str());

	return contentNameField.size();
}


// A common function called by CacheStoreImpl::portalBookTransferSession() or CacheStoreImpl::portalLocateContent()
// up to the portal's local context about the C2 client, such as the c2client instance and transferInfo struct
static int LocateSourceSessionEx(const CacheStoreImpl& store, const std::string& pid, const std::string& paid, const std::string& subFile, const std::string& extSessionInterface, long transferBitrate, ZQTianShan::ContentProvision::LocateResponse& locateResponse, bool delTransferId = false)
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "[%s%s]LocateSourceSessionEx()"), paid.c_str(), pid.c_str());
	
	int64 stampStart = ZQ::common::now();

	ZQ::common::URLStr url(extSessionInterface.c_str());
	std::string proto = url.getProtocol();
	if (0 == proto.compare("c2http")) // convert the c2http:// to http:// to pass into HttpClient
		url.setProtocol("http");
	else if (0 != proto.compare("http"))
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s%s]LocateSourceSessionEx() unsupported import protocol[%s]"), paid.c_str(), pid.c_str(), proto.c_str());
		return 400;
	}

//	url.setPath("vodadi.cgi");
/*	std::string  strServer= url.getHost();
	int nPort = url.getPort();

	char strTempPort[64] = "";
	itoa(nPort, strTempPort, 10);
	std::string  strPort= strTempPort;*/

	ZQTianShan::ContentProvision::LocateRequest locateRequest;

	locateRequest.pid  = pid;
	locateRequest.paid = paid;
	locateRequest.subFile = subFile;
	locateRequest.beginPos = -1;
	locateRequest.endPos = -1;
	locateRequest.bitRate = transferBitrate;
	locateRequest.transferDelay = 0;

//	std::string locateUrl = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";
	std::string locateUrl = url.generate();

	/// need configration following parameter
	std::string locateBindIp = "";
	std::string transferFileBindIp = "";

	ZQTianShan::ContentProvision::C2HttpClient* pC2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, &storelog, 20, locateBindIp, transferFileBindIp);
    std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	pHttpDownloader;
	pHttpDownloader.reset(pC2HttpClient);
	pHttpDownloader->setIngressCapacity(store._totalProvisionKbps * 1000);
	pHttpDownloader->setPIdAndPaid(pid, paid);
	pHttpDownloader->setSubfile(subFile);

	if (!pHttpDownloader->prepareLocateFile(locateUrl, locateRequest, locateResponse))
	{
		int nRetCode;
		std::string errMsg;
		pHttpDownloader->getLastErrorMsg(nRetCode, errMsg);
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s%s] LocateSourceSessionEx() locate index file with error:(%d) %s"), paid.c_str(), pid.c_str(),nRetCode, errMsg.c_str());
		return nRetCode;
	}
	if(delTransferId)
	{
		//delete transfer url
		if (0 == proto.compare("c2http")) // convert the c2http:// to http:// to pass into HttpClient
			url.setProtocol("http");

		std::string deleteURL(url.generate());

		pHttpDownloader->deleteTransferId(deleteURL, locateResponse.transferId, locateResponse.transferHost);
	}
	return 0;
}

int CacheStoreImpl::portalBookTransferSession(const CacheStoreImpl& store, std::string& sessionURL, const TianShanIce::Properties& contentNameToken, const std::string& subFile, const std::string& extSessionInterface, long transferBitrate, ::TianShanIce::Properties& params)
{
	int64 stampStart = ZQ::common::now();

	/// get ProvideId ProvideAssetId
	std::string strProvideId, strProvideAssetId;
	TianShanIce::Properties::const_iterator itorNameField;

	itorNameField = contentNameToken.find(METADATA_ProviderId);
	if(itorNameField ==contentNameToken.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() failed to get parameter %s"), METADATA_ProviderId);
		return 400;
	}
	strProvideId = itorNameField->second;

	itorNameField = contentNameToken.find(METADATA_ProviderAssetId);
	if (itorNameField == contentNameToken.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "LocateSourceSessionEx() failed to get parameter %s"),METADATA_ProviderAssetId );
		return 400;
	}

	strProvideAssetId = itorNameField->second;

    ZQTianShan::ContentProvision::LocateResponse locateResponse;
    int nRetCode = LocateSourceSessionEx(store, strProvideId, strProvideAssetId, subFile, extSessionInterface, transferBitrate, locateResponse);
	if(nRetCode > 0)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s%s]failed to BookTransferSession(errorcode:%d)"), strProvideAssetId.c_str(), strProvideId.c_str(), nRetCode);
		return nRetCode;
	}

	char urlPort[65] = "";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", store._defaultTransferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;

	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
	sessionURL = buf;

	//cdn.transferId, cdn.transferPort, cdn.transferTimeout, cdn.availableRange, cdn.openForWrite
	char strTimeout[33];
	memset(strTimeout, 0, sizeof(strTimeout));
	itoa(locateResponse.transferTimeout, strTimeout,10);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERID, locateResponse.transferId);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERPORT, locateResponse.transferHost);
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERTIMEOUT, std::string(strTimeout));
	MAPSET(TianShanIce::Properties, params, CDN_AVAILRANGE, locateResponse.availableRange);
	MAPSET(TianShanIce::Properties, params, CDN_OPENFORWRITE, locateResponse.OpenForWrite == true ? "yes" : "no");
	MAPSET(TianShanIce::Properties, params, CDN_TRANSFERPORTNUM, locateResponse.transferportnum);


	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "[%s%s]Book TransferSession for subFile[%s] sessionURL[%s] took %dms"), strProvideAssetId.c_str(), strProvideId.c_str(), subFile.c_str(),sessionURL.c_str(), (int)(ZQ::common::now() -  stampStart));

	return 0;
}


int CacheStoreImpl::portalLocateContent(const CacheStoreImpl& store, const std::string& extSessionInterface, CacheTaskImpl& cacheTask)
{
	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "portalLocateContent() content[%s]"), cacheTask.ident.name.c_str());

	int64 stampStart = ZQ::common::now();

	//init tempDir for save index file
	std::string tempDir = store._cacheDir ; ///temp directory to save index file

#ifdef ZQ_OS_MSWIN
	if(tempDir.empty())
		tempDir = "c:\\temp\\"; ///temp directory to save index file
	if(tempDir.size() > 0 && tempDir[tempDir.size() -1] != '\\')
		tempDir += "\\";
#else
	if(tempDir.empty())
		tempDir = "/tmp/"; ///temp directory to save index file
	if(tempDir.size() > 0 && tempDir[tempDir.size() -1] != '/')
		tempDir += "/";
#endif

	/// get ProvideId ProvideAssetId
	std::string strProvideId, strProvideAssetId;
	TianShanIce::Properties::const_iterator itorNameField;

	itorNameField = cacheTask.nameFields.find(METADATA_ProviderId);
	if(itorNameField == cacheTask.nameFields.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to get parameter 'ProvideId'"), cacheTask.ident.name.c_str());
		return 400;
	}
	strProvideId = itorNameField->second;

	itorNameField = cacheTask.nameFields.find(METADATA_ProviderAssetId);
	if (itorNameField == cacheTask.nameFields.end() && itorNameField->second.empty())
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to get parameter 'ProvideAssetId'"), cacheTask.ident.name.c_str());
		return 400;
	}

	strProvideAssetId = itorNameField->second;

   ///book Book TransferSession
	ZQTianShan::ContentProvision::LocateResponse locateResponse;

	int nRetCode = LocateSourceSessionEx(store, strProvideId, strProvideAssetId, "index", extSessionInterface, cacheTask.bwMax, locateResponse);
	if(nRetCode > 0)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to BookTransferSession(errorcode:%d)"), cacheTask.ident.name.c_str(), nRetCode);
		return nRetCode;
	}

	cacheTask.isSrcPWE = locateResponse.OpenForWrite;
	cacheTask.bwCommitted =  cacheTask.bwMax;

	//delete transfer url
	ZQ::common::URLStr url(extSessionInterface.c_str());
	std::string proto = url.getProtocol();
	if (0 == proto.compare("c2http")) // convert the c2http:// to http:// to pass into HttpClient
		url.setProtocol("http");

	std::string deleteURL(url.generate());

	///organize transfer URL

	char urlPort[65] = "";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", store._defaultTransferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;

	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
	std::string transferURL = buf;

	/// need configration following parameter
	std::string locateBindIp= "";
	std::string transferFileBindIp = "";

	ZQTianShan::ContentProvision::C2HttpClient* pC2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, &storelog, 20, locateBindIp, transferFileBindIp);
	std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	pHttpDownloader;
	pHttpDownloader.reset(pC2HttpClient);
	pHttpDownloader->setIngressCapacity(store._totalProvisionKbps * 1000);
	pHttpDownloader->setPIdAndPaid(strProvideId, strProvideAssetId);
	pHttpDownloader->setSubfile("index");

	std::string indexFilename = tempDir + strProvideAssetId + strProvideId +".index";

	int64 maxLen = 8192;
/*	if(locateResponse.OpenForWrite)
	{
		int64 nBeginPos = 0;
		int64 nEndPos = 0;

		if(pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
		{
			maxLen = nEndPos;
		}
	}*/

	int64 nBeginPos = 0;
	int64 nEndPos = 0;
	if(pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
	{
		maxLen = nEndPos;
	}

	if(maxLen > 8192)
		maxLen = 8192;

	int64 byteRecved = pHttpDownloader->downloadFile(transferURL, indexFilename, 0, maxLen, false);

	if(byteRecved < 0)
	{
		int nRetCode;
		std::string errMsg;
		pHttpDownloader->getLastErrorMsg(nRetCode, errMsg);
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s]download index file with error: (%d)%s"), cacheTask.ident.name.c_str(), nRetCode, errMsg.c_str());
		
		pHttpDownloader->deleteTransferId(deleteURL, locateResponse.transferId, locateResponse.transferHost);

		return nRetCode;
	}

	pHttpDownloader->deleteTransferId(deleteURL, locateResponse.transferId, locateResponse.transferHost);

	//get IndexFile extension
	int npos = indexFilename.rfind('.');
	if(npos <  0)
	{
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s]failed to get index file %s extension"), cacheTask.ident.name.c_str(), indexFilename.c_str());
		return 400;
	}

	std::string strIndexExt = indexFilename.substr(npos + 1);

	///parse IndexFile
	std::vector<SubFileInfo> subFiles;
	ZQTianShan::ContentProvision::MediaInfo				 mediaInfo;
	if(!ParseIndexFileInfo::getIdxSubFileInfo(indexFilename.c_str(), strIndexExt, subFiles, mediaInfo))
	{
		//failed to parse the index file
		storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to parse index file[%s]"), cacheTask.ident.name.c_str(), indexFilename.c_str());
		return 400;
	}

/*	cacheTask.fileSizeMB = 0;
	for(std::vector<SubFileInfo>::iterator itor = subFiles.begin(); itor != subFiles.end(); itor++)
	{
		if(itor->finalOffset > 0)
			cacheTask.fileSizeMB += itor->finalOffset  -  itor->firstOffset + 1;
	}

	cacheTask.fileSizeMB +=1024*1024-1;
	cacheTask.fileSizeMB >>=20;
*/

	if(!subFiles.empty())
	{
		cacheTask.startOffset = subFiles[0].firstOffset;
		cacheTask.endOffset = subFiles[0].finalOffset;
	}

	if(locateResponse.OpenForWrite)
	{
		//for PWE  get MainFile startOffset and endOffset;
		cacheTask.bwCommitted = mediaInfo.bitrate * 1.1; //bps;

		ZQTianShan::ContentProvision::LocateResponse locateResponse;
		int nRetCode = LocateSourceSessionEx(store, strProvideId, strProvideAssetId, "forward/1", extSessionInterface, cacheTask.bwMax, locateResponse, true);
		if(nRetCode > 0)
		{
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to locate mainfile with PWE mode for get mainfile available Range (errorcode:%d)"), cacheTask.ident.name.c_str(), nRetCode);
			return nRetCode;
		}
		int nPos = locateResponse.availableRange.find("-");
		if(nPos < 0)
		{
			storelog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStoreImpl, "[%s] failed to parser mainfile available Range [%s]"), cacheTask.ident.name.c_str(), locateResponse.availableRange.c_str());
			return 400; 
		}

		sscanf((locateResponse.availableRange.substr(0, nPos)).c_str(), FMT64, &cacheTask.startOffset);
		sscanf((locateResponse.availableRange.substr(nPos + 1)).c_str(), FMT64, &cacheTask.endOffset);
	}

	remove(indexFilename.c_str());

	storelog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStoreImpl, "[%s]completed portal Locate Content took %dms"), cacheTask.ident.name.c_str(), (int)(ZQ::common::now() - stampStart));

#pragma message ( __TODO__ "the test portal simply set cacheTask.urlSrcStream = extSessionInterface, should be corrected")
	cacheTask.urlSrcStream = extSessionInterface ;//+ cacheTask.ident.name;
	return 0;
}

ZQ::common::Mutex _cachedEndpointLocker;
std::map<std::string, std::string> _cachedEndpointMap;
void updateCachedEndpoint( const std::string& netid, const std::string& endpoint)
{
	if(netid.empty() ||endpoint.empty() )
		return;
	ZQ::common::MutexGuard gd(_cachedEndpointLocker);
	std::map<std::string, std::string>::iterator it = _cachedEndpointMap.find(netid);
	if( it != _cachedEndpointMap.end() && !it->second.empty())
		return;
	_cachedEndpointMap[netid] = endpoint;
}
std::string getCachedEndpoint( const std::string& netid)
{
	if(netid.empty())
		return"";
	ZQ::common::MutexGuard gd(_cachedEndpointLocker);
	std::map<std::string, std::string>::iterator it = _cachedEndpointMap.find(netid);
	if( it != _cachedEndpointMap.end() )
		return it->second;
	return "";
}

#define EVENT_ATTR_SIGNATURE_FILEEVENT	"FileEventFwd$"
#define EVENT_ATTR_SIGNATURE_NEMAPPING	"NeTiDenDpoINtmAPpInG$"

struct NeMappingMessage
{
	std::string netid;
	std::string endpoint;
	std::string messagegroup;
		
	bool toBuffer( char* buf, size_t& bufSize) const
	{
		ZQTianShan::Util::BufferMarshal m( buf, bufSize );	
		m << std::string(EVENT_ATTR_SIGNATURE_NEMAPPING) << netid << endpoint<<messagegroup;
		return true;
	}
	bool fromBuffer( const char* buf, size_t dataSize, const std::string& ignoreSender )
	{
		ZQTianShan::Util::BufferMarshal m((char*)buf,dataSize);
		std::string signature;
		
		m >> signature >> netid >> endpoint>>messagegroup;
		
		if( signature != std::string(EVENT_ATTR_SIGNATURE_NEMAPPING) )
			return false;
			
		if( messagegroup != gEdgeFEConfig.csStrReplicaGroupId )
			return false;

		return true;
	}
	void updateCache()
	{
		updateCachedEndpoint( netid, endpoint );
	}
};

enum FileEventType
{
	EVENT_TYPE_NULL,
	EVENT_TYPE_ADD,
	EVENT_TYPE_RENAME,
	EVENT_TYPE_MODIFY,
	EVENT_TYPE_REMOVE,
	EVENT_TYPE_CLOSEWRITE
};

struct FileEventMessage
{
	FileEventMessage()
	{
		type = EVENT_TYPE_NULL;
	}
	std::string senderEndpoint;//this is the endpoint of the ContentStore which send out the event, and this endpoint will be report to CDNCS
	std::string messageSender;
	std::string messageGroup;
	std::string filename1;
	std::string filename2;	
	FileEventType type;	
	
	size_t fileCount() const;
	bool toBuffer( char* buf, size_t& bufSize) const;
	bool fromBuffer( const char* buf, size_t dataSize, const std::string& ignoreSender );
};
size_t FileEventMessage::fileCount() const
{
	switch(type)
	{
	case EVENT_TYPE_RENAME:
		return 2;
	case EVENT_TYPE_NULL:
	case EVENT_TYPE_ADD:	
	case EVENT_TYPE_MODIFY:
	case EVENT_TYPE_REMOVE:
	case EVENT_TYPE_CLOSEWRITE:
	default:
		return 1;
	}	
}

bool FileEventMessage::toBuffer(char* buf, size_t& bufSize) const
{
	ZQTianShan::Util::BufferMarshal m( buf, bufSize );	
	m << std::string(EVENT_ATTR_SIGNATURE_FILEEVENT) << messageSender << messageGroup<< senderEndpoint << (int)type << filename1 << filename2;
	return true;
}
bool FileEventMessage::fromBuffer( const char *buf, size_t dataSize, const std::string& ignoreSender )
{
	ZQTianShan::Util::BufferMarshal m((char*)buf,dataSize);
	std::string signature;
	int eventType = 0;
	m >> signature >> messageSender >> messageGroup>> senderEndpoint >> eventType >> filename1 >> filename2;
	
	if( signature != std::string(EVENT_ATTR_SIGNATURE_FILEEVENT) )
		return false;
	if( messageGroup != gEdgeFEConfig.csStrReplicaGroupId )
		return false;

	type = FileEventType(eventType);
	
	if( !ignoreSender.empty() && ( ignoreSender == messageSender ) )
		return false;
	
	return true;
}

class McastEventReceiver : public ZQ::common::NativeThread
{
public:
	McastEventReceiver(ContentStoreImpl& store);
	virtual ~McastEventReceiver();

	void onEvent( FileEventMessage& event, bool bupTolib = true );
	
	void onNeMapping( );

	bool start( const std::string& localIp, const std::string& channelIp, int channelPort );

	void stop( );	
	
protected:

	int run();

	void dispatchEvent( const FileEventMessage& event, bool bFs = true );	

private:
	ContentStoreImpl& _store;
	ZQ::common::UDPReceive* mReceiver;
	//int mReceiver;
	ZQ::common::UDPMulticast* mMulticaster;
	bool mbQuit;
	std::string	mLocalIp;
		ZQ::common::tpport_t _localport;	
};

bool tmpfileExist( const std::string& filename )
{
	size_t len = filename.length();
	if( len > 0 && filename.at(len-1)=='~' )
	{
		return true;
	}
	return false;	
}
bool hasTmpfile( const std::string& filename )
{
	size_t len = filename.length();
	if( len > 0 )
	{
		FS::FileAttributes f(filename+"~");
		return f.exists();		
	}
	return false;	
}

McastEventReceiver::McastEventReceiver(ContentStoreImpl& store)
:_store(store),
mReceiver(NULL),
mMulticaster(NULL),
mbQuit(true), _localport(0)
{
}

McastEventReceiver::~McastEventReceiver()
{
	if(mReceiver)
	{
		delete mReceiver;
		mReceiver = NULL;
		//close(mReceiver);
		//mReceiver = -1;
	}
	if( mMulticaster )
	{
		delete mMulticaster;
		mMulticaster = NULL;
	}
}

bool McastEventReceiver::start( const std::string& localIp, const std::string& channelIp, int channelPort )
{
	if( channelIp.empty() || localIp.empty() )
	{
		_store._log(ZQ::common::Log::L_WARNING,CLOGFMT(McastEventReceiver,"invalid channelip or localIp passed in"));
		return false;
	}
	ZQ::common::InetMcastAddress groupAddress( channelIp.c_str() );
	ZQ::common::InetAddress localAddress( localIp.c_str() );
	ZQ::common::InetAddress localAddress1( "0.0.0.0");


	mReceiver = new ZQ::common::UDPReceive( localAddress1, channelPort );
	mReceiver->setMulticast(true);
	
	ZQ::common::Socket::Error err = mReceiver->join( groupAddress );
	mReceiver->setCompletion(true); // make the socket block-able
	
	if (err != ZQ::common::Socket::errSuccess)	
	{
		_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(MCastEventReceiver,"failed to create multicast listener socket"));
		return false;
	}
// 	struct sockaddr_in listenaddr;
// 	struct ip_mreq imreq;
// 	memset(&listenaddr,0,sizeof(listenaddr));
// 	memset(&imreq,0,sizeof(imreq));
// 	mReceiver = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
// 	if( mReceiver < 0 )
	
// 	listenaddr.sin_family = PF_INET;
// 	listenaddr.sin_port = htons(channelPort);
// 	listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	if(::bind(mReceiver,(struct sockaddr*)&listenaddr,sizeof(listenaddr))<0)
// 	{
// 		_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(MCastEventReceiver,"failed to bind multicast listener on port[%d]"),channelPort);
// 		return false;
// 	}
// 	imreq.imr_multiaddr.s_addr = inet_addr(channelIp.c_str());
// 	imreq.imr_interface.s_addr = INADDR_ANY;
// 	if( setsockopt(mReceiver, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void*)&imreq, sizeof(imreq))<0 )
// 	{
// 		_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(MCastEventReceiver,"failed to join multicast group[%s]"),channelIp.c_str());
// 		return false;
// 	}

			
	//{
	//	_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(McastEventReceiver, "initialze receiver failed: group:[%s]:%d local[%s]"), channelIp.c_str(), channelPort, localIp.c_str() );
	//	return false;
	//}
	mMulticaster = new ZQ::common::UDPMulticast( localAddress, 0 );
	mMulticaster->setGroup( groupAddress, channelPort);
	mMulticaster->setTimeToLive(100);
	if( !mMulticaster->isActive() )
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(McastEventReceiver, "initialze sender failed: group:[%s]:%d local[%s]"), channelIp.c_str(), channelPort, localIp.c_str() );
		return false;
	}
	
	// mLocalIp = localIp;
	mLocalIp = mMulticaster->getLocal(&_localport).getHostAddress(); // read the actural IP from socket

	mbQuit = false;
	return ZQ::common::NativeThread::start();
}

void McastEventReceiver::stop()
{
	if( mbQuit)
		return ;

	mbQuit = true;
	FileEventMessage msg;
	msg.type = EVENT_TYPE_NULL;
	char szBuf[1024];
	size_t sz =sizeof(szBuf);
	msg.toBuffer(szBuf,sz);
	mMulticaster->send(szBuf,sz);
	waitHandle(10000);  // hongquan: will this lead to slow service stop?
}

int McastEventReceiver::run()
{
	char szBuf[1024];
	//struct sockaddr_in from;
	
	while(!mbQuit)
	{
		try {
			ZQ::common::InetHostAddress from;
			int sport = 0;
					
			size_t bufSize = sizeof(szBuf);
	
			int len = mReceiver->receiveFrom( szBuf, bufSize, from, sport);
			//int len = recvfrom(mReceiver,szBuf,bufSize,0,(struct sockaddr*)&from,&socklen);
			if ( mbQuit || len <=0)
				break;

			//TODO: ignore that sent by self: if (sport == _localport && from == mLocalIp)	continue; -andy

			FileEventMessage msg;
			if ( msg.fromBuffer(szBuf, len, gEdgeFEConfig.netId)) // jira SCS-36, netId could be a string name that was not translated to numeric IP
				dispatchEvent( msg, false );
			else
			{
				NeMappingMessage mappingMsg;
				if( mappingMsg.fromBuffer(szBuf, len, gEdgeFEConfig.netId ) )
				{
					mappingMsg.updateCache();
				}
			}
		}
		catch(...) {}
	}
	return 0;
}

void McastEventReceiver::onNeMapping(  )
{
	NeMappingMessage msg;
	msg.netid = gEdgeFEConfig.netId;
	msg.endpoint = gEdgeFEConfig.serviceEndpoint;
	msg.messagegroup   = gEdgeFEConfig.csStrReplicaGroupId;
	if( !mMulticaster )
		return;

	char szBuf[1024];	
	size_t  size = sizeof(szBuf)-1;
	
	if( msg.toBuffer( szBuf, size ) )
	{
		int ret =  mMulticaster->send(szBuf,size);
		if( ret != (int)size )
		{
			_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(EventGenerator,"onNeMapping() failed to multicast event"));
		}
		else
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator,"onNeMapping() forwarded netid[%s] endpoint[%s] messagegroup[%s]"), 
				msg.netid.c_str(), msg.endpoint.c_str(), msg.messagegroup.c_str());
	}
	else
	{
		_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(EventGenerator,"onNeMapping() failed to convert event to raw message"));
	}
}

void McastEventReceiver::onEvent( FileEventMessage& event, bool bUpToLib )
{
	event.messageSender  = gEdgeFEConfig.netId;
	event.senderEndpoint = gEdgeFEConfig.serviceEndpoint;
	event.messageGroup   = gEdgeFEConfig.csStrReplicaGroupId;

	if( bUpToLib )
	{
		dispatchEvent(event);
	}
	
	if( !mMulticaster )
		return;
	
	char szBuf[1024];	
	size_t  size = sizeof(szBuf)-1;
	
	if( event.toBuffer( szBuf, size ) )
	{
		int ret =  mMulticaster->send(szBuf,size);
		if( ret != (int)size )
		{
			_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(EventGenerator,"onEvent() failed to multicast event"));
		}
		else
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator,"onEvent() forwarded event[%d] src[%s@%s] file[%s]"), event.type, event.messageSender.c_str(), event.messageGroup.c_str(), event.filename1.c_str());
	}
	else
	{
		_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(EventGenerator,"failed to convert event to raw message"));
	}
}
void McastEventReceiver::dispatchEvent( const FileEventMessage& msg, bool bFs )
{
	//const char* pTag = bFs ? "FsEvent" :"MulticastEvent";
	std::string tag = bFs ? std::string("FsEvent") : std::string("MultiCast:")+msg.messageSender;

	TianShanIce::Properties props;
	if( !msg.senderEndpoint.empty() )
	{
		//ZQTianShan::Util::updatePropertyData(props, METADATA_MasterReplicaEndpoint, msg.senderEndpoint );
	}
	if( !msg.messageSender.empty() )
	{
		ZQTianShan::Util::updatePropertyData(props, METADATA_MasterReplicaNetId, msg.messageSender );
	}
	
	updateCachedEndpoint( msg.messageSender, msg.senderEndpoint );
	
	switch(msg.type)
	{
	case EVENT_TYPE_ADD:
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator, "[%s] file added: (%s)"), tag.c_str(), msg.filename1.c_str());
			_store.OnFileEvent(TianShanIce::Storage::fseFileCreated, msg.filename1, props, Ice::Current());
		}
		break;
	case EVENT_TYPE_RENAME:
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator, "[%s] file renamed: [old: (%s) new: (%s)]"), tag.c_str(), msg.filename1.c_str(), msg.filename2.c_str());

			props.insert(TianShanIce::Properties::value_type("newFilename", msg.filename2));
			_store.OnFileEvent(::TianShanIce::Storage::fseFileRenamed, msg.filename1, props, Ice::Current());
		}
		break;
	case EVENT_TYPE_MODIFY:
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator, "[%s] file modified: (%s)"),tag.c_str(), msg.filename1.c_str());
			_store.OnFileEvent(TianShanIce::Storage::fseFileModified, msg.filename1, props, Ice::Current()); 
		}
		break;
	case  EVENT_TYPE_REMOVE:
		{			
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator, "[%s] file removed: (%s)"),tag.c_str(), msg.filename1.c_str());
			_store.OnFileEvent(TianShanIce::Storage::fseFileDeleted, msg.filename1, props, Ice::Current());
		}
		break;
	case EVENT_TYPE_CLOSEWRITE:
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGenerator, "[%s] file closed write: (%s)"),tag.c_str(), msg.filename1.c_str());
			_store.OnFileEvent(TianShanIce::Storage::fseFileModified, msg.filename1, props, Ice::Current());
		}
		break;
	default:
			break;
	}
}
class DeamonThread: public ZQ::common::NativeThread
{
public:
	DeamonThread(McastEventReceiver& mulcaster)
	:mbQuit(false),
	mMulCast(mulcaster)	
	{
	}
	virtual ~DeamonThread()
	{
	}
	void stop()
	{
		if(mbQuit)
			return;
		mbQuit = true;
		mSem.post();
		waitHandle(100*1000);		
	}
protected:
	int run()
	{
		int32 interval = gEdgeFEConfig.csNeMappingSendInterval;
		if(interval < 100) interval = 500;
		if(interval > 3600*1000) interval = 3600* 1000;			
		mbQuit = false;
		while(!mbQuit)
		{			
			mMulCast.onNeMapping();
			mSem.timedWait(interval);			
		}
		return 0;
	}
private:
	bool	mbQuit;
	ZQ::common::Semaphore mSem;	
	McastEventReceiver& mMulCast;
};
class FilesystemSink {

    typedef std::vector<std::string> DIRS;

public:
	
	FilesystemSink(ContentStoreImpl& store);
	~FilesystemSink();

public:

	void getMonitorDirs(DIRS& dirs) const {
		ZQ::common::MutexGuard gd(_lock);
        std::map<int, std::string>::const_iterator iter = _watchGroup.begin();
        for(; iter != _watchGroup.end(); ++iter) {
            dirs.push_back(iter->second); 
        }
	}

	void setWatchMask(unsigned mask) {
		_mask = mask;
	}

    void addMonitorDir(const std::string& dir) {
        std::string local = dir;
        if(local.at(local.length()-1) != '/') {
            local += '/';
        }     

        int wd = inotify_add_watch(_dirHandle, local.c_str(), _mask);

        if(wd < 0) {
            ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
                _store._log,
                EXPFMT(FileSystemSink, csexpInternalError, 
                "failed to add watch for (%s): [%s]"), 
                dir.c_str(),
                strerror(errno)
            );
        }
		_store._log(ZQ::common::Log::L_INFO, CLOGFMT(FileSystemSink, "addMonitorDir() dir[%s] add to watch"), local.c_str());
		ZQ::common::MutexGuard gd(_lock);
        _watchGroup[wd] = local;		
    }
	
	bool filter(const std::string& name) const {
		if(name == "test") {
			return true;
		}
		return false;
	}

	void rmMonitorDir(const std::string& dir);

	void startWatch();
	void stopWatch();

protected:
	
public:

	void onFileAdded(const std::string& f) 
	{
		if( tmpfileExist(f) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file[%s], skip the event [onFileAdded]"),
						f.c_str() );
			return;
		}
		FileEventMessage msg;
		msg.type = EVENT_TYPE_ADD;
		msg.filename1 = f;
		_eventReceiver.onEvent(msg);		
	}
	void onFileRenamed(const std::string& oldF, const std::string& newF) 
	{
		if( tmpfileExist(oldF) || tmpfileExist(newF) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file[%s][%s], skip the event [onFileRenamed]"),
						oldF.c_str(), newF.c_str() );
			return;
		}
		FileEventMessage msg;
		msg.type = EVENT_TYPE_RENAME;
		msg.filename1 = oldF;
		msg.filename2 = newF;
		_eventReceiver.onEvent(msg);
	}

    void onFileModified(const std::string& f, bool upToLib = true ) 
	{
		if( tmpfileExist(f) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file[%s], skip the event [onFileModified]"),
						f.c_str() );
			return;
		}
		FileEventMessage msg;
		msg.type = EVENT_TYPE_MODIFY;
		msg.filename1 = f;
		_eventReceiver.onEvent( msg, upToLib );
    }

	void onFileRemoved(const std::string& f) 
	{
		if( tmpfileExist(f) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file[%s], skip the event [onFileRemoved]"),
						f.c_str() );
			return;
		}
		if( hasTmpfile(f) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file for [%s] exist, skip the event [onFileRemoved]"),
						f.c_str() );
			return;
		}				
				
		FileEventMessage msg;
		msg.type = EVENT_TYPE_REMOVE;
		msg.filename1 = f;
		_eventReceiver.onEvent(msg);
	}
    
    void onFileCloseWrite(const std::string& f)
	{
		if( tmpfileExist(f) )
		{
			_store._log(ZQ::common::Log::L_INFO,CLOGFMT(FilesystemSink,"temp file[%s], skip the event [onFileCloseWrite]"),
						f.c_str() );
			return;
		}
		FileEventMessage msg;
		msg.type = EVENT_TYPE_CLOSEWRITE;
		msg.filename1 = f;
		_eventReceiver.onEvent(msg);
    }

private:

	unsigned _mask;
	int _dirHandle;
	bool _subTree;	
	bool _stopped;
	char* _eventBuffer;
	McastEventReceiver _eventReceiver;
	DeamonThread _daemon;

private:

	static void* watch(void*);
	static void processChanges(FilesystemSink*, ssize_t);

	std::map<int, std::string> _watchGroup;    
	ZQ::common::Mutex _lock;
    pthread_t _id;

public:

	ContentStoreImpl& _store;

}; 

typedef struct __tagCtx {
    FilesystemSink* sink;
    ContentProvisionWrapper* prov;
    ZQ::IdxParser::IdxParserEnv idxParseEnv;
} Context;

#define MLOG (*portalLogger)

#ifdef ZQ_OS_MSWIN
#  define	PORTALFMT(x,y) 	CLOGFMT(x, "cspNative "##y)
#elif defined ZQ_OS_LINUX
#  define	PORTALFMT(x,y) 	CLOGFMT(x, "cspNative " y)
#else
#  error "NOT SUPPORTED OS"
#endif

ZQ::common::Log* portalLogger = NULL;
	
void ContentStoreImpl::initializePortal(ContentStoreImpl& store) {
	if (store._ctxPortal) {
		return;
    }
    
    portalLogger = &store._log;
    Context* ctx = new Context();
    ctx->sink = new FilesystemSink(store);
    ctx->prov = new ContentProvisionWrapper(store._log);
    ctx->idxParseEnv.AttchLogger(&store._log);

    assert(ctx->sink != 0); 
    assert(ctx->prov != 0);

	store._ctxPortal = (void*)ctx;
	if (!store._ctxPortal) {
		return;
    }

    Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);

    using namespace TianShanIce::Storage;
	ContentStoreExPrx csPrx = ContentStoreExPrx::uncheckedCast(store._adapter->createProxy(csIdent));		
	
	if (!ctx->prov->init(store._adapter->getCommunicator(), 
        csPrx, gEdgeFEConfig.cpcEndpoint, gEdgeFEConfig.cpcRegisterInterval)) {		
 		store._log.flush();
        ZQTianShan::_IceThrow<TianShanIce::ServerError>(
                store._log,
                EXPFMT(initializePortal, csexpInternalError, "failed to init ContentProvisionWrapper"));

		return;
	}	
	//set auto sync
	if(gEdgeFEConfig.isAutoSync == 1)
		store._autoFileSystemSync = true;

	ctx->prov->setTrickSpeeds(gEdgeFEConfig.trickSpeedCollection);
    ctx->sink->startWatch();
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store) {
	if (store._ctxPortal) {
		FilesystemSink* sink = static_cast<Context*>(store._ctxPortal)->sink;
        ContentProvisionWrapper* prov = static_cast<Context*>(store._ctxPortal)->prov;

        try{
            sink->stopWatch();
            delete sink;

            prov->unInit();
//            delete prov;
        }
        catch(...) {
            throw;
        }
	}

	store._ctxPortal = 0;
}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname) {
    return pathname;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* volRootPath) {
	freeMB = totalMB = 0;

	if (!volRootPath || strlen(volRootPath) <= 0)
		return;

	if (!store._ctxPortal)
        return;

	uint64 free=0, avail=0, total=0;
	if(FS::getDiskSpace(volRootPath, avail, total, free))
	{
		// On Linux, free means total size of free blocks in the file system, while avail means
		// the total size of free blocks available to a non-privileged process.
		// we should take whichever is smaller as free space here
		free = MIN(avail, free);
		
		totalMB = total/1024L/1024L;
		freeMB = free/1024L/1024L;
	}
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType) {
	return true;
}

std::string showCheckResidentialFlag(uint64 flag) {
    char    szTemp[1024] = {0};
    char    *p = szTemp;
    size_t  lenTemp = sizeof(szTemp)-1; 

    if( flag & ( 1 << TianShanIce::Storage::frfResidential) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfResidential" );
        p += iRet;
        lenTemp -= iRet ;
    }
    if( flag & ( 1 << TianShanIce::Storage::frfReading ) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfReading" );
        p += iRet;
        lenTemp -= iRet ;
    }
    if( flag & ( 1 << TianShanIce::Storage::frfWriting ) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfWriting" );
        p += iRet;
        lenTemp -= iRet ;
    }

    if( flag & ( 1 << TianShanIce::Storage::frfAbsence ) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfAbsence" );
        p += iRet;
        lenTemp -= iRet ;
    }

    if( flag & ( 1 << TianShanIce::Storage::frfCorrupt ) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfCorrupt" );
        p += iRet;
        lenTemp -= iRet ;
    }

    if( flag & ( 1 << TianShanIce::Storage::frfDirectory ) )
    {
        int iRet = snprintf(p,lenTemp, "%s ", "frfDirectory" );
        p += iRet;
        lenTemp -= iRet ;
    }
    return szTemp;

}

bool getMetaData(const TianShanIce::Properties& metaData, const std::string& key, OUT std::string& value) {
    TianShanIce::Properties::const_iterator itMetaData = metaData.find(key);
    if( itMetaData != metaData.end()) {
        value = itMetaData->second;
        return true;
    }
    return false;
}
void setMetaData( ZQTianShan::ContentStore::ContentImpl& content, const std::string& mainFileName, const std::string& key, const std::string value )
{
	content.metaData[key] = value;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		mainFileName.c_str(), key.c_str(), value.c_str() );	
}
void setMetaData( ZQTianShan::ContentStore::ContentImpl& content, const std::string& mainFileName, const std::string& key, const int32 value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf, sizeof(szBuf)-1, "%d", value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		mainFileName.c_str(),	key.c_str(), szBuf);
}
void setMetaData( ZQTianShan::ContentStore::ContentImpl& content, const std::string& mainFileName, const std::string& key, const uint32 value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf, sizeof(szBuf)-1, "%u", value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		mainFileName.c_str(),	key.c_str(), szBuf);
}
void setMetaData( ZQTianShan::ContentStore::ContentImpl& content, const std::string& mainFileName, const std::string& key, const int64 value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf, sizeof(szBuf)-1, FMT64, value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		mainFileName.c_str(),	key.c_str(), szBuf);
}
void setMetaData( ZQTianShan::ContentStore::ContentImpl& content, const std::string& mainFileName, const std::string& key, const uint64 value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf, sizeof(szBuf)-1, FMT64, value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		mainFileName.c_str(),	key.c_str(), szBuf);
}

bool getFileSet(const TianShanIce::Properties& metaData, const std::string& mainFilePathname, 
				std::vector<std::string>& fileset, void* pCtx, int64* mainfilesize = 0) 
{
	char buf[512];
	int iSubFileIndex = 0;
	std::string value;
	while(true)
	{
		sprintf( buf,"%s%d",METADATA_SUBFILENAME, iSubFileIndex );
		if( getMetaData( metaData, buf, value ) )
		{
			fileset.push_back( value );
		}
		else
		{
			break;
		}
		iSubFileIndex++;
	}
	strcpy(buf,"sys.FileSize");
	if(mainfilesize && getMetaData(metaData,buf,value))
	{
		*mainfilesize = value.empty() ? 0 : atoll(value.c_str());
	}

    if(fileset.empty()) {
	ZQ::IdxParser::IndexFileParser idxParser( ((Context*)pCtx)->idxParseEnv );
	ZQ::IdxParser::IndexData idxData;
	if( idxParser.ParserIndexFileFromCommonFS(mainFilePathname, idxData))
	{
		if( mainfilesize)
			*mainfilesize = idxData.getMainFileSize();
		{
			int iSubFileCount = idxData.getSubFileCount();
			for(int i =0 ;  i< iSubFileCount ;i ++ )
			{
				const std::string& strSubfileExtension =  idxData.getSubFileName( i ) ;
				if( !strSubfileExtension.empty() )
				{
					std::string	subFileName = mainFilePathname + strSubfileExtension;
					fileset.push_back( subFileName );				
				}
				else
				{
					fileset.push_back( mainFilePathname );
				}
			}	
		}
	}
	else
	{
		fileset.clear();
	}
    }

    return (fileset.size()>0);
}
bool getFileSet(const ContentImpl& content, const std::string& mainFilePathname, std::vector<std::string>& fileset, void* pCtx ) 
{
	return getFileSet(content.metaData,mainFilePathname,fileset,pCtx);
}

bool getFileSetFromFS( const std::string& mainFilePathname, std::vector<std::string>& fileSet, void* pCtx, int64* mainfilesize = 0)
{
	ZQ::IdxParser::IndexFileParser idxParser( ((Context*)pCtx)->idxParseEnv);
	ZQ::IdxParser::IndexData idxData;
	if( idxParser.ParserIndexFileFromCommonFS( mainFilePathname, idxData  ) )
	{
		if(mainfilesize)
			*mainfilesize = idxData.getMainFileSize();
		int iSubFileCount = idxData.getSubFileCount();
		for(int i = 0 ;  i< iSubFileCount ;i ++ )
		{
			const std::string& strSubfileExtension =  idxData.getSubFileName( i ) ;
			if( !strSubfileExtension.empty() )
			{
				std::string	subFileName = mainFilePathname + strSubfileExtension;
				fileSet.push_back( subFileName );				
			}
			else
			{
				fileSet.push_back( mainFilePathname );
			}
		}
	}
	else
	{
		fileSet.clear();
	}
	return fileSet.size() > 0;
}

bool getFileSet(const ContentImpl* content, const std::string& mainFilePathname, std::vector<std::string>& fileset,
				void* pCtx, const ContentStoreImpl& store, int64 *mainfilesize = 0) 
{
	if(content)
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStoreImpl,"get file set from content's metadata for [%s]"),content->ident.name.c_str() );
    	return getFileSet(content->metaData,mainFilePathname,fileset,pCtx, mainfilesize);
	}
    else
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStoreImpl,"get file set from FS for [%s]"),mainFilePathname.c_str() );
		return getFileSetFromFS( mainFilePathname, fileset, pCtx,mainfilesize );
	}
}

bool fileSetExist( const std::vector<std::string>& fileset, const std::string& mainFilePathname )
{
	bool res = false;
	if(!fileset.empty()) 
	{
		std::vector<std::string>::const_iterator iter = fileset.begin();
		for(; iter != fileset.end(); ++iter) 
		{
			FS::FileAttributes attr(*iter);
			if(attr.exists()) 
			{
				res = true;
				break;
			}
		}
	}
	else 
	{
		FS::FileAttributes attr(mainFilePathname);
		if(attr.exists()) 
		{
			res = true;
		}
	}
	return res;
}

bool fileSetAbsence( const std::vector<std::string>& fileset )
{
	bool absence = fileset.empty();

	std::vector<std::string>::const_iterator iter = fileset.begin();
	for(; iter != fileset.end(); ++iter)
	{
		FS::FileAttributes attr(*iter);
		if(!attr.exists()) 
		{
			absence = true;
			break;
		}
	}
	return absence;
}

bool checkFileSetStatusFromMasterReplica( const std::string& contentName, const std::string& endpoint, ContentStoreImpl& store )
{
	//connect to master contentstore
	std::string masterEndpoint;
	if( endpoint.find(":") == std::string::npos	)
	{
		masterEndpoint = std::string("ContentStore:") + endpoint;
	}
	else
	{
		masterEndpoint = endpoint;
	}
	TianShanIce::Storage::ContentStorePrx masterContentStore = TianShanIce::Storage::ContentStorePrx::uncheckedCast(
													store._adapter->getCommunicator()->stringToProxy( masterEndpoint) ) ;
	//open content
	TianShanIce::Storage::ContentPrx content = masterContentStore->openContentByFullname( contentName );
	TianShanIce::Storage::ContentState state = content->getState();
	store._log(ZQ::common::Log::L_INFO,CLOGFMT(checkMasterReplicaStatus,"got content[%s] from [%s] for state[%d]"), contentName.c_str(), endpoint.c_str(), state );
	return ( ( state >= TianShanIce::Storage::csNotProvisioned ) && (state <= TianShanIce::Storage::csProvisioningStreamable) );
}
void checkFileSetStatusFromFS( const std::vector<std::string>& fileset, uint64& flag, ContentStoreImpl& store, const ContentImpl::Ptr content )
{
	std::vector<std::string>::const_iterator iter = fileset.begin();
	for (; iter != fileset.end(); ++iter) 
	{
		store._log(ZQ::common::Log::L_DEBUG, LOGFMT("checking file %s"), iter->c_str());
		struct stat query;
		if(stat(iter->c_str(), &query)) 
		{
			store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to stat file %s"), iter->c_str());
			break;
		}
		DIR* procdir = opendir("/proc");
		if(!procdir)
		{
			store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to open /proc"));
			break;
		}

		struct dirent* proc = 0;
		pid_t mypid = getpid();
		while((proc = readdir(procdir)) != 0) 
		{
			if(proc->d_name[0] < '0' || proc->d_name[0] > '9') 
			{
				/* not a process entry */
				continue;
			}
			pid_t curr_pid = atoi(proc->d_name);
			if(curr_pid == mypid) 
			{
				/* skip myself */
				continue;
			}

			char path[255]; 
			snprintf(path, 255, "/proc/%d/fd", curr_pid);
			DIR* tmpdir = opendir(path);
			if(!tmpdir)
			{
				continue;
			}

			char filepath[255];
			struct dirent* tmpent = 0;
			while((tmpent = readdir(tmpdir))!= 0) 
			{
				snprintf(filepath, 255, "/proc/%d/fd/%s", curr_pid, tmpent->d_name);
				struct stat st;
				if(!stat(filepath, &st) && (st.st_dev == query.st_dev) && (st.st_ino == query.st_ino))
				{
					/* found the file */
					struct stat st2;
					lstat(filepath, &st2);

					if(st2.st_mode & S_IRUSR) 
					{
						store._log(ZQ::common::Log::L_DEBUG, LOGFMT("file %s opened for reading by %d"), iter->c_str(), curr_pid);
						flag |= RSDFLAG(frfReading);
					}
					if(st2.st_mode & S_IWUSR) 
					{
						store._log(ZQ::common::Log::L_DEBUG, LOGFMT("file %s opened for writting by %d"), iter->c_str(), curr_pid);
						flag |= RSDFLAG(frfWriting);						
					}
					break;
				}
			} /* end reading /proc/PID/fd */
			closedir(tmpdir);
		} /* end reading /prod/PID */
		closedir(procdir);
	} /* end fileset */
}
void checkFileSetStatus( const std::string& mainFilePathName, const std::vector<std::string>& fileset, uint64& flag, ContentStoreImpl& store, const ContentImpl::Ptr content, FilesystemSink* sink )
{
	if( content)
	{	
		//check if we should send out file modified event through multicast
		std::string masterReplicaEndpoint;
		//ZQTianShan::Util::getPropertyDataWithDefault( content->metaData, METADATA_MasterReplicaEndpoint, "", masterReplicaEndpoint );
		std::string masterReplicaNetId;
		ZQTianShan::Util::getPropertyDataWithDefault( content->metaData, METADATA_MasterReplicaNetId, "", masterReplicaNetId );
		
		if (!masterReplicaNetId.empty() &&  masterReplicaEndpoint.empty() )
		{
			masterReplicaEndpoint = getCachedEndpoint(masterReplicaNetId);
			store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStorePortal,"content[%s] do not have masterEndpoint but netid[%s], using cached endpoint[%s]"),
				content->ident.name.c_str(), masterReplicaNetId.c_str(), masterReplicaEndpoint.c_str() );	
		}
		
		bool bOldBeWritten = (content->state >= TianShanIce::Storage::csProvisioning && content->state <= TianShanIce::Storage::csProvisioningStreamable);
		bool bNewBeWritten = bOldBeWritten;
		bool bContentDirty = content->bDirtyAttrs; 
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStorePortal,"masterNetId[%s] masterReplicaEndpoint[%s], content[%s] is %s dirty"),
				masterReplicaNetId.c_str(), masterReplicaEndpoint.c_str(), content->ident.name.c_str(), bContentDirty ? "" :"NOT");
		
		if( masterReplicaNetId == gEdgeFEConfig.netId || masterReplicaNetId.empty() || masterReplicaEndpoint.empty() )
		{
			checkFileSetStatusFromFS( fileset, flag, store, content );
			if( flag & RSDFLAG(frfWriting))
			{
				sink->onFileModified( mainFilePathName, false );
				store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStoreImpl,"file[%s] is being written, send out fileModified event"), mainFilePathName.c_str() );
			}
		}
		else
		{
			Ice::Long cur = ZQTianShan::now();
			Ice::Long delta = cur - content->stampLastUpdated;
			try
			{
				if( bContentDirty ||  delta > 10 * 1000  || content->state <= TianShanIce::Storage::csNotProvisioned )
				{
					store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentStoreImpl,"trying to query status from [%s] for content[%s]"),
							masterReplicaEndpoint.c_str(), content->ident.name.c_str() );
					bNewBeWritten = checkFileSetStatusFromMasterReplica( content->ident.name, masterReplicaEndpoint, store );
				}
				else
				{
					store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentStorePortal,"content[%s], delta[%lld] < 10000, use last state"),
						 	content->ident.name.c_str(), delta);
				}
				store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentStorePortal,"content %s is %s being written"), content->ident.name.c_str(), bNewBeWritten ?"":"NOT" );
			}
			catch( const TianShanIce::BaseException& ex)
			{
				store._log(ZQ::common::Log::L_WARNING,CLOGFMT(ContentStorePortal,"failed to query content[%s] for state: %s"),
					content->ident.name.c_str(), ex.message.c_str());			
			}
			catch( const Ice::Exception& ex)
			{//do nothing
				store._log(ZQ::common::Log::L_WARNING,CLOGFMT(ContentStorePortal,"failed to query content[%s] for state: %s"),
					content->ident.name.c_str(), ex.ice_name().c_str());
			}

			if( bNewBeWritten )
			{
				flag |= RSDFLAG(frfWriting);
			}
		}
	}
	else
	{
		checkFileSetStatusFromFS( fileset, flag, store, content );
	}
}
uint64 ContentStoreImpl::checkResidentialStatus( ContentStoreImpl& store, uint64 flagsToTest, 
												const ContentImpl::Ptr content, const std::string& fullName,const std::string& mainFilePathname) 
{

    store._log(ZQ::common::Log::L_INFO, CLOGFMT(ContentStorePortal, "MainFile[%s] checkResidential with flag[%s]:content instance[%s]"),
            mainFilePathname.c_str(), showCheckResidentialFlag(flagsToTest).c_str(),	content?"avail":"NA");

    std::vector<std::string> fileset;
	int64 mainfilesize = 0;
    if(!getFileSet(content.get(), mainFilePathname, fileset,store._ctxPortal, store, &mainfilesize)) 
	{
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, "MainFile[%s] can't get file set"), mainFilePathname.c_str( ) );
    }
    uint64 ret = 0;

	if ((flagsToTest & RSDFLAG(frfResidential))) 
	{
		bool res = fileSetExist( fileset, mainFilePathname );

        if(res) 
		{
			ret |= RSDFLAG(frfResidential);
        }
        else 
		{
            store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to check residential for file %s"), mainFilePathname.c_str());
        }
	}

    if(flagsToTest & RSDFLAG(frfAbsence)) 
	{
		bool absence = fileSetAbsence(fileset);
		if(absence) 
		{
			ret |= RSDFLAG(frfAbsence);
		}
		if( mainfilesize < gEdgeFEConfig.csMinimalMainfileSize )
		{
			ret |= RSDFLAG(frfAbsence);
			store._log(ZQ::common::Log::L_INFO,CLOGFMT(ContentStorePortal,"MainFile[%s] check Residential, mark as frfAbsence due to mainfilesize[%lld] < configsize[%lld]"),
				mainFilePathname.c_str(), mainfilesize, gEdgeFEConfig.csMinimalMainfileSize );
		}
    }

	if(flagsToTest & RSDFLAG(frfReading) || flagsToTest & RSDFLAG(frfWriting)) 
	{
		FilesystemSink* sink = (( Context* )store._ctxPortal)->sink;
       checkFileSetStatus( mainFilePathname, fileset, ret, store, content, sink );
    } 

    store._log(ZQ::common::Log::L_INFO, CLOGFMT(ContentStorePortal,"MainFile[%s] check Residential return [%s]"),
            mainFilePathname.c_str(), showCheckResidentialFlag(ret).c_str());

	return (ret & flagsToTest);
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath) 
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt)
{
	ContentStoreImpl::FileInfos infos;

	std::string searchFor = fixupPathname(store, rootPath ? rootPath : "");
    std::vector<std::string> files = FS::searchFiles(searchFor, "*");

	ContentStoreImpl::FileInfo info;

    std::vector<std::string>::const_iterator iter = files.begin();
    for(; iter != files.end(); ++iter) {
        FS::FileAttributes attr(*iter);
        if(attr.isDirectory()) {
            continue;
        }

        std::string name = memberFileNameToContentName(store, attr.name());
		if (0 == info.filename.compare(name))
			continue; // skip the duplicated contents

		info.filename = name;
        char buf[80];
        if(ZQ::common::TimeUtil::Time2Iso(attr.mtime(), buf, 80)) {
            info.stampLastWrite = buf;
        }

        infos.push_back(info);

    }

	return infos;
}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename) {
    std::string name = memberFilename;

    std::string::size_type pos = name.find_last_of("\\/");
    if(pos != std::string::npos) {
        name = name.substr(pos+1);        
    }

    pos = name.find_last_of('.');
    if(pos == std::string::npos) {
        return name;
    }    
    
    std::string ext1 = name.substr(pos+1);
    std::transform(ext1.begin(), ext1.end(), ext1.begin(), (int (*)(int))(toupper));
    if(ext1.find("0X") == 0 || ext1 == "INDEX") {
        return name.substr(0, pos);
    }

    std::string ext = name.substr(pos+1, 2);
    std::transform(ext.begin(), ext.end(), ext.begin(), (int (*)(int))(toupper));
    
    if(ext == "VV" || ext == "FF" || ext == "FR") {
        return name.substr(0, pos);
    }
    else {
        return name;
    }
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume,const std::string& volname) {
    FS::FileAttributes attr(pathOfVolume);

    /* the path exists but not a directory */
    if(attr.exists() && !attr.isDirectory()) {
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, 
                    "createPathOfVolume() foldername[%s] conflict with existing file"), pathOfVolume.c_str());
        return false;
    }

    bool ret = true;

    /* create if not exists */
    if(!attr.exists()) {
        ret = FS::createDirectory(pathOfVolume, true);
    }
        
    if(ret) {
        // start monitoring changes of this directory
        FilesystemSink* sink = static_cast<Context*>(store._ctxPortal)->sink;
        sink->addMonitorDir(pathOfVolume);
    }

	return ret;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume) {
    FS::FileAttributes attr(pathOfVolume);

    if(!attr.exists()) {
        return true;
    }
    if(attr.exists() && !attr.isDirectory()) {
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, 
                    "deletePathOfVolume() foldername[%s] conflict with existing file"), pathOfVolume.c_str());
		return false;
	}

    FilesystemSink* sink = static_cast<Context*>(store._ctxPortal)->sink;
    sink->rmMonitorDir(pathOfVolume);
    
	return FS::remove(pathOfVolume);
}

bool ContentStoreImpl::deleteFileByContent(
            ContentStoreImpl& store, 
            const ContentImpl& content, 
            const std::string& mainFilePathname) {

	std::vector<std::string>	fileset;
	getFileSet( content, mainFilePathname, fileset, store._ctxPortal);
	fileset.push_back(mainFilePathname);
	std::vector<std::string>::const_iterator it = fileset.begin() ;
	for( ; it != fileset.end() ; it ++ )
	{
		FS::remove(*it);
	}
	return (0 == checkResidentialStatus(store, RSDFLAG(frfResidential), const_cast<ContentImpl *>(&content), "", mainFilePathname));
}

static bool populateAttrsFromFileInter(Context* pCtx, ContentStoreImpl& store, ContentImpl& content, const std::string& mainFilePathname) {
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStorePortal, 
                "populateAttrsFromFile() content[%s] popluate attributes from main file [%s]"), 
                content.ident.name.c_str(), mainFilePathname.c_str());

	Ice::Current c;
	ZQ::IdxParser::IndexFileParser idxParser(pCtx->idxParseEnv);
	ZQ::IdxParser::IndexData	idxData;
	if(!idxParser.ParserIndexFileFromCommonFS(mainFilePathname,idxData)) {
		MLOG(ZQ::common::Log::L_WARNING,
			PORTALFMT(populateAttrsFromFile,"MainFile[%s] failed to populate attribute "),
			mainFilePathname.c_str());
		return false;
	}
	if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVX ) {
		setMetaData( content, mainFilePathname, METADATA_SubType, TianShanIce::Storage::subctVVX );
	}
	else if( idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VV2 ) {
		setMetaData( content, mainFilePathname, METADATA_SubType, TianShanIce::Storage::subctVV2 );
	}
	else if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVC) {
		setMetaData( content, mainFilePathname, METADATA_SubType, "VVC" );
	}
	else {
		MLOG(ZQ::common::Log::L_ERROR,
			PORTALFMT(populateAttrsFromFile,"MainFile[%s] unknown index format"),
			mainFilePathname.c_str() );
		return false;
	}
	ZQ::IdxParser::IndexData::SubFileInformation subinfo;
	if(	idxData.getSubFileInfo(0,subinfo) )
	{
		setMetaData( content, mainFilePathname, METADATA_EXTNAME_MAIN, subinfo.fileExtension );
	}

	setMetaData( content, mainFilePathname, METADATA_BitRate, idxData.getMuxBitrate() );
	setMetaData( content, mainFilePathname, METADATA_FILENAME_MAIN, idxData.getMainFilePathName() );
	setMetaData( content, mainFilePathname, METADATA_PlayTime, idxData.getPlayTime() );
	setMetaData( content, mainFilePathname, METADATA_FileSize, idxData.getMainFileSize() );
	setMetaData( content, mainFilePathname, METADATA_IDX_GENERIC_INFO, idxData.baseInfoToXML() );
	setMetaData( content, mainFilePathname, METADATA_IDX_SUBFULE_INFO, idxData.memberFileToXML() );

	int32 iSubFileCount = idxData.getSubFileCount();
	int iSubFileIndex = 0;
	char buf[1024];
	std::string strAllExts;
	for(int i =0 ;  i< iSubFileCount ;i ++ ) {
		const std::string& strExtension =  idxData.getSubFileName( i ) ;
		if( !strExtension.empty() ) {
			std::string	subFileName = mainFilePathname + strExtension;				
			sprintf(buf, "%s%d", METADATA_SUBFILENAME, iSubFileIndex++ );
			setMetaData( content, mainFilePathname, buf, subFileName );
			
			//record subfile data start-byte end-byte
			ZQ::IdxParser::IndexData::SubFileInformation sfinfo;
			idxData.getSubFileInfo(i,sfinfo);
			
			sprintf(buf,"%s%s.%s", "Sys.SubFile.",strExtension.c_str(),"start");
			setMetaData( content, mainFilePathname, buf, sfinfo.startingByte );
			
			sprintf(buf,"%s%s.%s", "Sys.SubFile.",strExtension.c_str(),"end");
			setMetaData( content, mainFilePathname, buf, sfinfo.endingByte );

			strAllExts = strAllExts + strExtension;
			strAllExts = strAllExts + ";";
		}
		else {				
			sprintf(buf, "%s%d", METADATA_SUBFILENAME, iSubFileIndex++ );
			setMetaData( content, mainFilePathname, buf, mainFilePathname );
			strAllExts = strAllExts + ";";
		}
	}
	setMetaData(content, mainFilePathname, std::string("sys.allextentionnames"), strAllExts);
	const std::string& tmpIndexFileName = idxData.getIndexFilePathName();
	sprintf(buf, "%s%d", METADATA_SUBFILENAME, iSubFileIndex++ );
	setMetaData( content, mainFilePathname, buf, tmpIndexFileName );
	setMetaData( content, mainFilePathname, METADATA_FILENAME_INDEX, tmpIndexFileName);
	MLOG( ZQ::common::Log::L_INFO, 
		PORTALFMT(populateAttrsFromFile,"end populate properties for Content[%s]"),
		mainFilePathname.c_str());
	return true;


}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname) {
	Ice::Current c;
	Context* pCtx = (Context*)store._ctxPortal;
	bool canRetry = content.getState(c) >= TianShanIce::Storage::csProvisioning;
	for( size_t i = 0; i < 2 ; i ++ ) {
		bool bOK = populateAttrsFromFileInter(pCtx,store,content,mainFilePathname);
		if(bOK)
			return bOK;
		if(!canRetry)
			return bOK;
		storelog(ZQ::common::Log::L_INFO, PORTALFMT(populateAttrsFromFile,"retry populate[%s]"), mainFilePathname.c_str());
	}
	return false;
}

bool ContentStoreImpl::completeRenaming(
            ContentStoreImpl& store, 
            const std::string& mainFilePathname, 
            const std::string& newPathname) {
	return (0 != checkResidentialStatus(store, RSDFLAG(frfResidential), NULL, "", newPathname));
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const std::string& contentName,
            const std::string& sourceUrl, 
            const std::string& sourceType, 
            const std::string& startTimeUTC, 
            const std::string& stopTimeUTC, 
            const int maxTransferBitrate) 
//			throw (TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException,
//                   TianShanIce::ServerError, TianShanIce::InvalidStateOfArt)
{

    /* provision session exists */
	std::string strProvisionSess = content.provisionPrxStr;
	if(!strProvisionSess.empty()) {
		TianShanIce::ContentProvision::ProvisionSessionPrx session;

		try {
 			session = TianShanIce::ContentProvision::ProvisionSessionPrx::checkedCast(
 				store._adapter->getCommunicator()->stringToProxy(strProvisionSess));
		}
		catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_WARNING, 
				LOGFMT("[%s] Open provision session[%s] for updateScheduledTime() caught exception[%s]"),
				contentName.c_str(), strProvisionSess.c_str(), ex.ice_name().c_str());
		}

		try {
			std::string start, stop;
			session->getScheduledTime(start, stop);

			//
			// need to change the time to tianshan time to compare, because IM use localtime+timezero sometime, but we always use utc
			//
			if(start != startTimeUTC || stop != stopTimeUTC) {
				store._log(ZQ::common::Log::L_INFO, 
					LOGFMT("[%s] update schedule time: [start (%s --> %s) stop (%s --> %s"),
					contentName.c_str(), start.c_str(), startTimeUTC.c_str(), stop.c_str(), stopTimeUTC.c_str());

				session->updateScheduledTime(startTimeUTC, stopTimeUTC);
			}
		} 
		catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to update schedule time [start: (%s) stop: (%s)] for (%s): (%s)"),
				startTimeUTC.c_str(), stopTimeUTC.c_str(), contentName.c_str(), ex.ice_name().c_str());

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(
				store._log,
				EXPFMT(NativeService, csexpInternalError, "failed to updateScheduledTime() with start[%s] stop[%s]"), 
				startTimeUTC.c_str(),
				stopTimeUTC.c_str()
            );
		}

		return session;
	}

	int transferBitrate = maxTransferBitrate;
	if (!transferBitrate) {
		transferBitrate = gEdgeFEConfig.defaultProvisionBW;
	}

	::TianShanIce::Properties sessMdata;
	{
		::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
		::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_IndexType);
		if (it!=metaDatas.end())
		{
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, it->second.c_str()));
		}
		else
		{
			if (stricmp(gEdgeFEConfig.cpcDefaultIndexType.c_str(),"VVC") == 0)
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVC"));
			else if(stricmp(gEdgeFEConfig.cpcDefaultIndexType.c_str(),"VVX") == 0)
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVX"));
			else if(stricmp(gEdgeFEConfig.cpcDefaultIndexType.c_str(),"VV2") == 0)
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VV2"));

		}
		it = metaDatas.find(METADATA_ProviderId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERID, it->second));


		it = metaDatas.find(METADATA_ProviderAssetId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERASSETID, it->second));

		it = metaDatas.find(METADATA_AugmentationPIDs);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_AUGMENTATIONPIDS, it->second));

		it = metaDatas.find(METADATA_Preencryption);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PREENCRYPTION, it->second));

		it = metaDatas.find(METADATA_WishedTrickSpeeds);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_WISHEDTRICKSPEEDS, it->second));
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

    TianShanIce::Storage::ContentPrx contentPrx = 
        TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

	TianShanIce::ContentProvision::ProvisionContentKey contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;
	
	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = 0;
    try {
        pPrx = cpWrapper->activeProvision(
                contentPrx,
                contentKey,
                strFilePathName,	
                sourceUrl,
                sourceType, 
                startTimeUTC,
                stopTimeUTC, 
                transferBitrate,
                sessMdata,false);
    }
    catch(const TianShanIce::InvalidParameter& ex) {
        ex.ice_throw();
    }
    catch(const TianShanIce::InvalidStateOfArt& ex) {
        ex.ice_throw();
    }
    catch(const Ice::Exception& ex) {
        ZQTianShan::_IceThrow<TianShanIce::ServerError>(
            store._log,
            EXPFMT(NativeService, csexpInternalError, "failed to submit provision [%s]"), 
            ex.ice_name().c_str()
        );
    }
    
	return pPrx;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(
            ContentStoreImpl& store, 
            const ContentImpl& content, 
            const std::string& contentName,
            std::string& pushUrl, 
            const std::string& sourceType, 
            const std::string& startTimeUTC, 
            const std::string& stopTimeUTC, 
            const int maxTransferBitrate)
//			throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt)
{

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	TianShanIce::Storage::ContentPrx contentPrx = 
            TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->passiveProvision(
		contentPrx,
		contentKey,
		strFilePathName,					  
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		maxTransferBitrate,
		pushUrl);

	return pPrx;
}

std::string ContentStoreImpl::getExportURL(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const TianShanIce::ContentProvision::ProvisionContentKey& contentkey,
            const std::string& transferProtocol, 
            Ice::Int transferBitrate, 
            Ice::Int& ttl, 
            TianShanIce::Properties& params) {

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;

	int transBitrate = transferBitrate;
	int nTTL = 0;
	int permittedBitrate;

	/* invalidate the protocol. */
	if(transferProtocol != TianShanIce::Storage::potoFTP){
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
			store._log,
			EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol (%s) not supported"), transferProtocol.c_str()
			);
	}

	std::string strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentkey, transBitrate, nTTL, permittedBitrate);

	ttl = nTTL;

	{
		std::ostringstream oss;
		oss << ttl;		
		params[expTTL] = oss.str();

		oss.str("");
		oss << permittedBitrate;
		params[expTransferBitrate] = oss.str();		
	}

	time_t now = time(0);
	char window[255];

	ZQ::common::TimeUtil::Time2Iso(now, window, 255);
	std::string stStart = window;
	params[expTimeWindowStart] = stStart;

	ZQ::common::TimeUtil::Time2Iso(now+ttl, window, 255);
	std::string stEnd = window;
	params[expTimeWindowEnd] = stEnd;


	store._log(ZQ::common::Log::L_DEBUG, 
		LOGFMT("(%s) getExportURL [URL: (%s) ttl: (%d) timeWindowStart: (%s) timeWindowEnd: (%s) bitrate: (%d)e"), 
		contentkey.content.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;

/*
    std::ostringstream oss;
    oss << "cifs://192.168.81.107" << content.identVolume.name << LOGIC_FNSEPC << mainFileName;

    std::replace(oss.str().begin(), oss.str().end(), FNSEPC, LOGIC_FNSEPC);

	return oss.str();
*/
}

void ContentStoreImpl::cancelProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const std::string& provisionTaskPrx) 
//            throw (::TianShanIce::ServerError, TianShanIce::InvalidStateOfArt)
{ 

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;
	
	std::string contentName = content._name();
	cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

void ContentStoreImpl::notifyReplicasChanged(
            ContentStoreImpl& store, 
            const TianShanIce::Replicas& replicasOld, 
            const TianShanIce::Replicas& replicasNew) {
}


FilesystemSink::FilesystemSink(ContentStoreImpl& store)
:_mask(DEFAULT_WATCH_MASK), 
_dirHandle(-1),
_stopped(false),
_eventBuffer(0),
_eventReceiver(store),
_daemon(_eventReceiver),
_store(store)
{

	_eventBuffer = new char[BUFFER_SIZE];	
	memset(_eventBuffer, '\0', BUFFER_SIZE);

	_dirHandle = inotify_init();

	if(_dirHandle < 0) {
        ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
            store._log,
            EXPFMT(FileSystemSink, csexpInternalError, 
            "failed to init inotify [%s]"), 
            strerror(errno)
        );
	}
}

FilesystemSink::~FilesystemSink() {
}

void FilesystemSink::startWatch() {

	const std::string& localIp = gEdgeFEConfig.csEventMulticastLocalIp;
	const std::string& groupIp = gEdgeFEConfig.csEventMulticastGroupIp;
	int groupPort = gEdgeFEConfig.csEventMulticastGroupPort;

	if( gEdgeFEConfig.csEnableEventMulticast >= 1)
	{
		_eventReceiver.start( localIp, groupIp, groupPort );
		_daemon.start();
	}

	if(pthread_create(&_id, NULL, watch, (void*)this)) {
        ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
            _store._log,
            EXPFMT(FileSystemSink, csexpInternalError, 
            "failed to start watch thread [%s]"), 
            strerror(errno)
        );
	}
}

void FilesystemSink::stopWatch() {
	
	if( gEdgeFEConfig.csEnableEventMulticast >= 1)
	{
		_daemon.stop();
		_eventReceiver.stop();
	}


	_store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(FileSystemMonitor,"tring to stop monitor thread"));
    void* res = 0;
	
	_stopped = true;
	
    //pthread_cancel(_id);
    pthread_join(_id, &res);

	_store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(FilesystemSink,"monitor thread stopped"));

	{
		ZQ::common::MutexGuard gd(_lock);	
		std::map<int, std::string>::iterator iter = _watchGroup.begin();
		for(; iter != _watchGroup.end(); ++iter) {
			inotify_rm_watch(_dirHandle, iter->first);
		}
	}

	if(_eventBuffer) {
		delete _eventBuffer;
		_eventBuffer = 0;
	}
}

void* FilesystemSink::watch(void* params) {
   // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    FilesystemSink* This = (FilesystemSink*)params;
	This->_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FileSystemSink, "watch thread started"));

    try 
	{
		fd_set fds;
		struct timeval tv;		
        while( !This->_stopped ) 
		{
			tv.tv_sec = 0;
			tv.tv_usec = 500 * 1000;//500ms
			FD_ZERO(&fds);
			FD_SET( This->_dirHandle, &fds );
			int r = select( This->_dirHandle + 1, &fds, NULL, NULL, &tv ); 
			if( r == 0 )
				continue;
			if( r < 0 )
			{
				This->_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(FilesystemSink,"got unexpect error while reading"));
				continue;
			}
			ssize_t len = read(This->_dirHandle, This->_eventBuffer, BUFFER_SIZE);
			if( len > 0 )
	            processChanges(This, len);	
       } 
    }
    catch(...) {
		This->_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(FilesystemSink,"unexpect error, FilesystemSink quiting"));
    }
	This->_store._log(ZQ::common::Log::L_ERROR,CLOGFMT(FilesystemSink," FilesystemSink quited"));
    return (0);
}

void FilesystemSink::processChanges(FilesystemSink* This, ssize_t len) {

	char* offset = This->_eventBuffer;
	struct inotify_event* event = (inotify_event*)This->_eventBuffer;

    static std::string oldF;

	while(offset - This->_eventBuffer < len) {
		
        std::string name;
		{
		ZQ::common::MutexGuard gd(This->_lock);
        std::map<int, std::string>::iterator iter = This->_watchGroup.find(event->wd);
        if(iter != This->_watchGroup.end()) {

            if(event->mask & IN_ISDIR) {

                name = iter->second + event->name + "/";

                if(event->mask & IN_CREATE) {
                    
//                    std::cout << "add folder: " << name << std::endl;

//                    int wd = inotify_add_watch(This->_dirHandle, name.c_str(), This->_mask);

//                    This->_watchGroup[wd] = name; 
                }

                if(event->mask & IN_DELETE) {
//                   std::cout << "folder delete: (" << name << ")" << std::endl;
                }

                return;
			}
            else {
                name = iter->second + event->name;
            }
		}
        else {
            break;
        }
		}//lock block
		if(event->mask & IN_CREATE) {
			This->onFileAdded(name);
		}

        if(event->mask & IN_MODIFY) {
			This->onFileModified(name);
        }

		if(event->mask & IN_DELETE) {
			This->onFileRemoved(name);
		}
        
        if(event->mask & IN_CLOSE_WRITE) {
            This->onFileCloseWrite(name);
        }

		if(event->mask & IN_MOVED_TO || event->mask & IN_MOVED_FROM) {

			if(event->mask & IN_MOVED_FROM) {
                oldF = name;
			}
			else {
                This->onFileRenamed(oldF, name);
                oldF.clear();
			}

		}

		size_t tmp = sizeof(inotify_event) + event->len;
		event = (inotify_event*)(offset + tmp);

		offset += tmp;
	}
}

void FilesystemSink::rmMonitorDir(const std::string& dir)
{
	if(dir.empty())
		return;	

	std::string local = dir;
    if(local.at(local.length()-1) != '/') 
		local += '/';
	
	_store._log(ZQ::common::Log::L_INFO, CLOGFMT(FileSystemSink, "rmMonitorDir() dir[%s] remove frome watch list"), local.c_str());
	int md = 0;
	std::map<int, std::string>::iterator it;
	ZQ::common::MutexGuard gd(_lock);
	for(it = _watchGroup.begin(); it != _watchGroup.end(); it++)
	{
		if(local.compare(it->second) == 0)
		{
			md = it->first;
			_watchGroup.erase(it);
		}
	}
	if(md != 0)
		inotify_rm_watch(_dirHandle, md);
}

}} 

// vim: ts=4 sw=4 bg=dark nu
