#ifndef __CRM_A3SERVER_A3MESSAGE_HANDLER_H__
#define __CRM_A3SERVER_A3MESSAGE_HANDLER_H__

#include "CRMInterface.h"
#include "TsStorage.h"
#include "TsCache.h"
#include "XMLPreferenceEx.h"
#include "A3MsgCommon.h"
#include "A3MsgEnv.h"
namespace CRM
{
	namespace A3Message
	{
		class A3MsgEnv;
		class A3MessageHandler: public CRG::IContentHandler
		{
		public:
			A3MessageHandler(CRM::A3Message::A3MsgEnv& env);
			~A3MessageHandler(void);
			/// A3 message handle entry point
			virtual void onRequest(const CRG::IRequest* request, CRG::IResponse* response);

		private:
			/// get volume info from APM client
			void GetVolumeInfo(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);// ºöÂÔ

			/// get Content info from APM client
			void GetContentInfo(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response); //ºöÂÔListAllContent

			/// transfer content form APM client, asynchronous method
			void TransferContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);

			/// cancel content from APM client
			void CancelTransfer(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);

			/// delete content from APM client
			void DeleteContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);//

			/// get content checksum form APM client, asynchronous method
			void GetContentChecksum(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);

			/// expose content from APM client
			void ExposeContent(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);//²»×ö

			/// get transfer status from APM client
			void GetTransferStatus(const std::string reqID, const CRG::IRequest* request, CRG::IResponse* response);

		private:
			bool getVolumeNameAndId(const std::string& strFullVol, std::string& strNetId, std::string& strVolume);

			/// read XML Doc
			inline bool readXMLDoc(const std::string reqID,ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen);

			/// parse XML message content
			bool parseMsgContent(const std::string reqID, const char* buffer, size_t bufLen, StringMap& xmlElement);

			/// parse XML message content
			bool parseMsgContentEx(const std::string reqID, const char* buffer, size_t bufLen, StringMap& xmlElement, StringMap& metaDatas);

			/// set http OK response with content body
			inline void setResponseWithBody(const std::string reqID,const CRG::IRequest* request, CRG::IResponse* response, 
				int statusCode, const char* reasonPhrase, std::string strMsgContent);

			/// set http response without content body
			inline void setReponseWithoutBody(const std::string reqID,const CRG::IRequest* request, CRG::IResponse* response, 
				int statusCode, const char* reasonPhrase);

			bool getContentName(std::string& strNetId, std::string&  strVolumeName,
				 std::string& strProviderID, std::string& strAssetId,
				 std::string& strContentName);
		public:
			CRM::A3Message::A3MsgEnv& _env;

		};
	}///end namespace A3Message
}///end namespace CRM
#endif /// enddefine __CRM_A3SERVER_A3MESSAGE_HANDLER_H__

