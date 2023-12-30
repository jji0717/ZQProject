#ifndef __TianShanS1_SetupRequest_H__
#define __TianShanS1_SetupRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupSetup : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupSetup> Ptr;
		FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupSetup();
		virtual bool process();

		std::string findServiceGroupByIP(std::string ip);
		std::string calculateGroup(std::string groupExpression, uint64 subnet);

		std::string findServiceGroupByQam(std::string qam);

	};

	class HandleSetup : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleSetup> Ptr;
		HandleSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleSetup();
		virtual bool process();
		virtual TianShanIce::Properties getEventParam() const {return _eventParams;}

	protected: 
		void addPrivateData(std::string key, TianShanIce::Variant& var);
		bool flushPrivateData();
		void ChangeQamMode(int&);
		bool checkTimeStampAndConnection( );
		bool importTransportToAsResource();
		bool prepareResponse();

	private:
		
		void				confirm();
		void				setErrMsg( const char* fmt , ... );

	private: 
		TianShanIce::ValueMap						_pdMap; // buffer for weiwoo session's private data
		TianShanIce::SRM::SessionManagerPrx			_sessMgrPrx;
		TianShanIce::SRM::SessionPrx				mSrmSess;
		bool										mbOpSuccess;
		std::string									mErrMsg;
		int64										mTimestampStart;
		bool										_bQam;
		bool										_bIP;
		bool										_bC2;
		bool										_bNatOpen;
		::Ice::Long									_lBandWidth;
		std::string									_destIP;
		std::string									_destPort;
		std::string									_destMac;

		TianShanIce::Properties                     _eventParams;
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

#endif // __TianShanS1_SetupRequest_H__

