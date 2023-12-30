

#ifndef _RDS_CPH_IMPL_
#define _RDS_CPH_IMPL_

#include "CPHInc.h"
#include "BaseCPH.h"

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
		class PushSource;
		class BaseTarget;
	}};	

	class FSCPSess : public ZQTianShan::ContentProvision::BaseCPHSession, public ZQTianShan::ContentProvision::BaseGraph
	{
	public:
		FSCPSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseCPHSession(helper, pSess),
			_filesize(0), _bQuit(false) 
		{
			_bCleaned = false;
			_pMainTarget = NULL;
			_bStartEventSent = false;
		}
		
		virtual ~FSCPSess();
		
	public:
		
		virtual bool preLoad(); 
		virtual bool prime(); 
		virtual void terminate(bool cleanupIncompletedOutput=true);
		virtual bool getProgress(::Ice::Long& offset, ::Ice::Long& total);
		
		virtual void OnProgress(LONGLONG& prcvBytes);
		virtual void OnStreamable(bool bStreamable);
		virtual void OnMediaInfoParsed(MediaInfo& mInfo);
	protected:
		// impl of ThreadRequest
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);
		
	protected:
		void cleanup();
		int  _bitrate;
		bool  _bStartEventSent;
		::Ice::Long _filesize, _processed;
		bool _bQuit;
		bool _bCleaned;
		ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
	};
	
	class FSCPHelper : public ZQTianShan::ContentProvision::BaseCPHelper
	{
	public:
		FSCPHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
			:BaseCPHelper(pool, mgr) {}
		virtual ~FSCPHelper() {}
		
		static ZQTianShan::ContentProvision::BaseCPHelper* _theHelper;
		
	public: // impl of ICPHelper
		
		///validate a potential ProvisionSession about to setup
		///@param[in] sess access to the ProvisionSession about to setup
		///@param[out] schema the collection of schema definition
		///@return true if succeeded
		virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource);
		
		virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins);
		virtual uint16 evaluateCost(const char* methodType, const uint32 bandwidthKbps, const uint16 sessions);
		virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost);
		
		virtual ZQTianShan::ContentProvision::ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		{
			if (NULL == methodType || 0 != stricmp(METHODTYPE_FSCOPYVSVSTRM, methodType))
				return NULL;
			
			return new FSCPSess(*this, pSess);
		}
		
	};
	
#endif