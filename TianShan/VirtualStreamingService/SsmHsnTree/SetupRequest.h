#ifndef __HSNTree_SetupRequest_H__
#define __HSNTree_SetupRequest_H__

#include "RequestHandler.h"

namespace HSNTree
{
	class FixupSetup : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupSetup> Ptr;
		FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupSetup();
		virtual bool process();

	};

	class HandleSetup : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleSetup> Ptr;
		HandleSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleSetup();
		virtual bool process();

	protected: 
		void addPrivateData(std::string key, TianShanIce::Variant& var);
		void addPrivateData2(std::string key, TianShanIce::Variant& var);
		bool flushPrivateData();
		void ChangeQamMode(int&);
		void destroyWeiwooSession();
		void cleanSessionCtx(SessionContextImplPtr pSessionContext, int errCode=0, const std::string& cleanReason="");

	private: 
		TianShanIce::ValueMap						_pdMap; // buffer for weiwoo session's private data
		TianShanIce::SRM::SessionManagerPrx			_sessMgrPrx;
		bool										_bSetupSuccess;
	
	};

	class SetupResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<SetupResponse> Ptr;
		SetupResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~SetupResponse();
		virtual bool process();
		
	};

} // end namespace TianShanS1

#endif // __HSNTree_SetupRequest_H__

