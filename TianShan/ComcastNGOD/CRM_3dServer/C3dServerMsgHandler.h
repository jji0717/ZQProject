#ifndef __CRM_C3DSERVER_MESSAGE_HANDLER_H__
#define __CRM_C3DSERVER_MESSAGE_HANDLER_H__

#include "CRMInterface.h"
#include "XMLPreferenceEx.h"
#include "C3dServerCommon.h"
#include "C3dServerEnv.h"
#include "CURLClient.h"
#include "CPHInc.h"

namespace CRM
{
	namespace C3dServer
	{
		class C3dServerEnv;
		class C3dServerMsgHandler: public CRG::IContentHandler
		{
			typedef std::map<std::string, ZQ::common::StringMap> Node2Props;
		public:
			C3dServerMsgHandler(CRM::C3dServer::C3dServerEnv& env);
			~C3dServerMsgHandler(void);
			/// A3 message handle entry point
			virtual void onRequest(const CRG::IRequest* request, CRG::IResponse* response);
		protected:
			void createContent(const std::string& contentId, const std::string& strMsgBody, CRG::IResponse* response);
			void deleteContent(const std::string& contentId, CRG::IResponse* response);
			void getContentInfo(const std::string& contentId, CRG::IResponse* response);
			void getContentInfo(const std::string& contentId, const std::string& status, CRG::IResponse* response);
			void getContentInfo(const ZQ::common::StringMap& conditionMap, CRG::IResponse* response);

		private:
			inline void setResponseWithBody(CRG::IResponse* response, int statusCode, const char* reasonPhrase, std::string strMsgContent);
			inline void setReponseWithoutBody(CRG::IResponse* response, int statusCode, const char* reasonPhrase);
			/// read XML Doc
			inline bool readXMLDoc(ZQ::common::XMLPreferenceDocumentEx& xmlDoc, const char* buffer, size_t bufLen);
			bool  getChildNode(const std::string& contentId, ZQ::common::XMLPreferenceEx* xmlRoot, const std::string childName, std::string& childText);
			std::string getStatus(const Json::Value& jVals);
			std::string getErrReason(const int errCode);
		public:
			CRM::C3dServer::C3dServerEnv& _env;

		private:
			bool  _isNameExist;
			bool	  _isProviderExist;
			bool	  _isStartTimeExist;

		};
	}///end namespace C3dServer
}///end namespace CRM
#endif /// enddefine __CRM_3DSERVER_A3MESSAGE_HANDLER_H__

