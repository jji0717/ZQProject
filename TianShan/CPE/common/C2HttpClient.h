#pragma once
#include "HttpClient.h"
#include "urlstr.h"
#include <string>
#include <memory>
namespace ZQTianShan
{
	namespace ContentProvision
	{

		typedef bool(*ReadBuf)(void* pCtx, char*pBuf, int64 len);

		typedef struct 
		{
			std::string pid;       /// Asset ProviderID
			std::string paid;      /// Asset ProviderAssetId
			std::string subFile;   /// the transfer file extension name
			int64 beginPos;        /// partial file content begin pos
			int64 endPos;          /// partial file content end pos
			int64 bitRate;         /// bitRate for download content 
			int64 transferDelay;    
		}LocateRequest;

		typedef struct 
		{
			std::string transferId;         /// the transferId from httpserver for download file
			std::string transferHost;     /// the transfer file ip and port
			int transferTimeout;    /// timeout for transfer session
			std::string availableRange;     /// the download file available range	
			bool        OpenForWrite;    
			std::string transferportnum;
		}LocateResponse;

		class C2HttpClient  //:public ZQ::common::HttpClient
		{
		public:
			C2HttpClient(int iomode, ZQ::common::Log* pLog, int timeout = 20, std::string locateRequestIP="", std::string transferIP="", int nport = 0);
			~C2HttpClient();
		protected:
			bool getFileNameFromResponse(std::string& strFileName);

			bool parserLocateReponse(const char* buffer, size_t bufLen, LocateResponse& locateResponse);
			bool locateRequest(const std::string& url, const LocateRequest& locateReqest, std::string& reponseConent);

			void setExclustionList(const std::string exclustionlist);

			void reset();
			bool parserTotalFilesize(const std::string& contentRange, int64& nBeginPos, int64& nEndPos, int64& totalSize);

			//test for csico
			int getPorts();
		public:
			void setIngressCapacity(int64 IngressCapacity); /// ingressCapcity
			void setPIdAndPaid(std::string providerId, std::string providerAssetId);
			void setSubfile(std::string subFile);

			///download the file content from HttpServer
			///@param[in]  url             http server URL. like http://192.168.81.101:5020
			///@param[in]  LocateRequest    locate request info
			///@param[out] LocateResponse   locate response info
			///@return true if succeeded
			bool prepareLocateFile(const std::string& url, const LocateRequest& locateReqest, LocateResponse& locateResponse);

			///download the file from HttpServer by ProvideID and ProvideAssetId
			///@param[int]     transferURL    the transfer URL for get file from httpserver
			///@param[in, out] filePath       saved path and name of file
			///@param[int]     startOffset    the offset of the download file
			///@param[int]     maxLen         the max download length of the download file
			///@param[int]     bUsedLoaclFileName if false, saved file as Original name from http response, else use filepath as saved file name, default value(true).
			///@return received totalbyte , if -1 failed.
			int64 downloadFile(const std::string& transferURL, std::string& filePath, const int64 startOffset, int64 maxLen, bool bUsedLoaclFileName = true);

			///download the Partial file content from HttpServer
			///@param[in]  transferbuf    the callback function to write buffer
			///@param[in]  pCtx           the context
			///@param[int] transferURL    the transfer URL for get file from httpserver
			///@param[int] startOffset    the offset of the download file
			///@param[int] maxLen         the max download length of the download file
			///@return received totalbyte , if -1 failed.
			int64 downloadPartialFile(ReadBuf transferbuf,  void *pCtx, const std::string& transferURL, const int64 startOffset, int64 maxLen);

            ///remove the reservation session by transferId and transferIpPort
			///@param[in]  url             http server URL. like http://192.168.81.101:5020
			///@param[in]  transferId      the transferId  from locate response
			///@param[int] transferIpPort  the transferIpPort from locate response
			///@return true if succeeded
			bool deleteTransferId(const std::string& url, const std::string& transferId, const std::string& transferIpPort);

		    ///return the error message when download file occured
			///@param[in]  nRetCode  error code
			///@param[in]  errMsg    error message 
			///@return  error message
			bool getLastErrorMsg(int& nRetCode, std::string& errMsg);
            
			///parser available range (*** - ***)
			bool parserAavailableRange(std::string& availableRang, int64& nBeginPos, int64& nEndPos);

			///return the get file size
			///@param[in]  url             http server URL. like http://192.168.81.101:5020
			///@param[in]  LocateRequest    locate request info
			///@param[out] LocateResponse   locate response info
			///@return  file size, if -1 failed.
			int64 getFileSize(const std::string& url, const LocateRequest& locateReqest, LocateResponse& locateResponse);

			void setError(bool bErrorOccured, std::string errMsg){_bErrorOccured = bErrorOccured; _strErrMsg =  errMsg;};

			void setTimeout(int timeout);
		private:
			std::auto_ptr<ZQ::common::HttpClient>	_pHttpClient;
			std::auto_ptr<ZQ::common::HttpClient>	_pTransferHttpClient;
			ZQ::common::Log* _log;

			int64        _IngressCapacity; // ingressCapcity
			std::string  _exclustionlist;

			std::string  _strErrMsg;
			int          _errorCode;
			bool         _bErrorOccured;

		protected: 
			int          _timeout;

            //for log , printf pid ,paid and _subFile
			std::string  _providerId;
			std::string  _providerAssetId;
			std::string  _subFile;	
		//add for cisco test
			std::string  _locateRequestIP;
			std::string  _transferIP;
		protected: 
			int          _maxPort;
		public:
			int          _nPortBegin;
			static int   _nCurrentPort;
		};
	}
}
