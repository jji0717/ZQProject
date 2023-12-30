#ifndef __CRM_A3MESSAGECLIENT_A3MESSAGE_HANDLER_H__
#define __CRM_A3MESSAGECLIENT_A3MESSAGE_HANDLER_H__

#include <string>
#include "Log.h"
#include "HttpClient.h"
#include "urlstr.h"
namespace CRM
{
	namespace A3MessageClient
	{
		typedef struct {
			std::string volumeName; //required
			std::string pid;//required
			std::string paid;//required
			std::string captureStart; // PGM or IP Multicast required
			std::string captureEnd;   // PGM or IP Multicast required
			int         transferBitRate;//required
			std::string sourceURL;//required
			std::string sourceIP;//Ftp and file system not use,  PGM or IP Multicast use(Optional)
			std::string sourceURL1;//Optional
			std::string sourceIP1; //Ftp and file system not use,  PGM or IP Multicast use(Optional)
			std::string userName;//Ftp and file system use,  PGM or IP Multicast not use
			std::string password;//Ftp and file system use,  PGM or IP Multicast not use
			std::string responseURL;//required
			std::map<std::string, std::string>props;
			int         reasonCode;
			std::string protocol;
		}ContentInfo; 

		class A3MessageReq
		{
			public:
				A3MessageReq(ZQ::common::Log& log, std::string reqURL);
				~A3MessageReq(void);

				bool init(int iomode, std::string bindIp = "", int nport = 0);

			public:
				bool GetVolumeInfoReq(std::string volumeName);

				bool GetContentInfoReq(std::string volumeName, std::string pid ="", std::string paid ="");

				/////
				bool TransferContentReq(ContentInfo& transInfo);

				bool CancelTransferReq(std::string volumeName, std::string pid, std::string paid, int reasonCode);

				bool DeleteContentReq(std::string volumeName, std::string pid, std::string paid, int reasonCode);

				bool GetContentChecksumReq(std::string volumeName, std::string pid, std::string paid, std::string responseURL);

				/////
				bool ExposeContentReq(std::string volumeName, std::string pid, std::string paid, std::string protocol, int transferBitRate);

				bool GetTransferStatusReq(std::string volumeName, std::string pid, std::string paid);
			protected:
				bool sendRequest(std::string reqID, std::string reqURL, std::string& reqContent);

			protected:
				ZQ::common::Log& _log;
				std::auto_ptr<ZQ::common::HttpClient>	_pHttpClient;
				std::string _reqURL;
		};

	}///end namespace A3MessageClient
} ///end namespace CRM
#endif ///endif __CRM_A3MESSAGECLIENT_A3MESSAGE_HANDLER_H__
