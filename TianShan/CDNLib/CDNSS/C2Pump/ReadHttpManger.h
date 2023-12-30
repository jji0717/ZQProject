#ifndef  _READHTTPMANGER_H
#define  _READHTTPMANGER_H

#include "RequestHandle.h"
#include "AioFile.h"
#include "HttpFetcher.h"
#include "LocateRequest.h"
#include "GetRequest.h"
#include "LRUMap.h"
#include "ReadClient.h"

using namespace ZQ::StreamService;

namespace C2Streamer{

	enum C2RequestPhase
	{
		phaseInit,
		phaseLocate,
		phaseGet,
		phaseTransferDelete,
		phaseDone
	};
	struct ReadController :virtual public ZQ::common::SharedObject
	{
		public:
			std::string       _locatePath;
			int               _Port;
			std::string       _sessionId;
			uint64            _creatTime;
			int               _count;
		public:
			typedef ZQ::common::Pointer<ReadController> Ptr;
			ReadController(std::string& locatePath,int port,std::string sessionId,uint64 curtime,int count = 0):_locatePath(locatePath),_Port(port),_sessionId(sessionId),_creatTime(curtime),_count(0){}
			~ReadController(){}
	};
	typedef ZQ::common::LRUMap<std::string,ReadController::Ptr> FileClientMap; 

	//////calss LocateClient//////
	class ReadHttpManger;
	class LocateClient :  public IC2RequestCallBack
	{
		public:
			typedef ZQ::common::Pointer<LocateClient> Ptr;
			LocateClient(const std::string filename,RequestParams params,ZQ::common::Log& log,ReadHttpManger* readHttpManger,const std::string& proxyUrl, const std::string& segmenterUrl,BufferUser buf);
			/*LocateClient(const std::string filename,ZQ::common::Log& log,ReadHttpManger* readHttpManger,int64 bufferReqID = 0):
			  _log(log),_readHttpManger(readHttpManger),_clientMap(readHttpManger->clientMap),_timeOut(0)
			  {
			  _proxyUrl     = _readHttpManger->geProxyUrl();
			  _segmenterUrl = _readHttpManger->getSegmenterUrl();
			  }*/
			virtual ~LocateClient();
		public://Implementation  IC2RequestCallBack
			bool retry();
			int  getMaxRetry();
			bool startRequest();
			virtual void onLocate(const LocateResponseData& resp);
			virtual void onData(const char* data, const size_t& size, bool error = false);
			virtual void onRecvComplete();
			virtual void onTransferDelete();
			virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg);
		private:
			C2Streamer::Buffer::ErrorCategory  convertErrorCategoty(C2RequestErrorCategory category);
			std::string getCurrPhaseStr();
		private://for locate
			std::string                         _filename;
			ZQ::common::Log&                    _log;
			static ZQ::common::AtomicInt        _atomicID;
			RequestHandle::Ptr                  _requestHandle;
			RequestParams                       _params;
			C2RequestPhase                      _phase;
			ZQ::common::Mutex                   _mutex;
			int                                 _currRetryCount;
			//for httpfetcher use
			ReadHttpManger*                     _readHttpManger;
			std::string                         _proxyUrl;
			std::string                         _segmenterUrl;
			BufferUser                          _buf;
	};//class LocateClient

	//////class ReadHttpManger//////
	class  ReadHttpManger : public IDataReader,public ReadHttpCallBack,virtual public ZQ::common::SharedObject
	{
		friend class LocateClient;
		public:
		typedef ZQ::common::Pointer<ReadHttpManger> Ptr;
		ReadHttpManger(IReaderCallback* callBack,ZQ::StreamService::ReadClient::Ptr c2client,ZQ::common::Log& log,const std::string& proxyUrl, const std::string& segmenterUrl,const int mapsize = 1000, const int savecount = 10,int timeout = 10*1000,int retrytime = 0, bool replacec2 = false);
		~ReadHttpManger();
		public://Implementation  IDataReader
		virtual bool read( const std::vector<BufferUser>& bufs );
		virtual bool queryIndexInfo(const std::string& filename, AssetAttribute::Ptr attr );
		private:
		HttpFetcher::Ptr addClient(const std::string& fileName,std::string sessionId, int64 offset, std::string resUrl,int retry = 0, int type = 6);
		void             delClient(const std::string& fileName, int64 offset, int type = 6); 
		bool             newArequest(const std::string filename,BufferUser buf);
		public:
		std::string getProxyUrl() {return _proxyUrl;}
		std::string getSegmenterUrl() {return _segmenterUrl;}
		int         getRetryTime(){return _retryTimes;}

		public://Implementation  ReadHttpCallBack
		virtual void onRead(BufferUser buf, bool retry = false);
		virtual void onLatency(std::string& fileName, int64 offset,int64 time);
		virtual void onError( int err );
		virtual void onIndexInfo( AssetAttribute::Ptr attr, bool retry = false);
		private://for httpfetcher
		IReaderCallback*                    _callBack;
		ZQ::common::Log&                    _log;
		ZQ::common::Mutex                   _clientMapLock;
		std::string                         _proxyUrl;
		std::string                         _segmenterUrl;
		FileClientMap                       _clientMap;
		int                                 _saveCount;
		int                                 _timeOut;
		int                                 _retryTimes;
		bool                                _replaceC2;
		int                                 _requestInstID;
		ZQ::StreamService::ReadClient::Ptr  _c2client;
		RequestParams                       _params;
	};//class ReadHttpManger
}//namespace  C2Streamer


#endif //_READHTTPMANGER_H
