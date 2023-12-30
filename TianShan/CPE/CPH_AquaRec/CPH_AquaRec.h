#ifndef _AquaRec_CPH_IMPL_
#define _AquaRec_CPH_IMPL_

#include "CPHInc.h"
#include "BaseCPHAquaRec.h"
#include "BaseClass.h"
#include "AquaRecVirtualSessI.h"

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
		class PushSource;
		class BaseTarget;
	}};	

	class AquaRecSess : public ZQTianShan::ContentProvision::BaseAquaRecSession, public IceUtil::RecMutex
	{
	public:
		typedef IceUtil::Handle<AquaRecSess> Ptr;

		AquaRecSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseAquaRecSession(helper, pSess),
			_filesize(0), _bQuit(false) 
		{
			_sessState = SSTATE_NONE;
			_processed = 0;
			_bitrate = 0;
		}
		
		virtual ~AquaRecSess();
		
		enum SessionState
		{
			SSTATE_NONE,
			SSTATE_INIT,
			SSTATE_PROCESSING,
			SSTATE_STOP,
			SSTATE_DESTROY
		};
	public:
		
		virtual bool preLoad(); 
		virtual bool prime(); 
		
		void execute();

		virtual void terminate(bool bProvisionSuccess=true);
		virtual bool getProgress(::Ice::Long& offset, ::Ice::Long& total);
		virtual void updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC);

		bool process();		//< with lock
		
		virtual void OnTimer();

	protected:
		// no lock
		bool _process();		
		bool doInit();
		bool doProcessing();
		bool doStop();
		bool doDestroy();


		bool sendStartEvent();
		void sendStreamableEvent();
		void sendProgressEvent();
		void sendStopEvent();

		void createTimer(int nTimeoutInMs);
		void removeTimer();


		bool checkStreamable();

	protected:

		std::string _strMethod;
		int  _bitrate;
		::Ice::Long _filesize, _processed;
		bool _bQuit;

		SessionState	_sessState;

		bool			_bStreamable;

		int				_nBandwidth;

		ZQTianShan::ContentProvision::MediaInfo						_mediaInfo;

		std::auto_ptr<ZQTianShan::ContentProvision::AquaRecVirtualSessI>	_pAquaRecSess;

		std::string							_strLogHint;
	};
	
	class AquaRecHelper : public ZQTianShan::ContentProvision::BaseCPHelper, ZQTianShan::ContentProvision::MethodCost
	{
	public:
		AquaRecHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr);
			
		virtual ~AquaRecHelper();
		
		static ZQTianShan::ContentProvision::BaseCPHelper* _theHelper;
		
	public: // impl of ICPHelper
		
		///validate a potential ProvisionSession about to setup
		///@param[in] sess access to the ProvisionSession about to setup
		///@param[out] schema the collection of schema definition
		///@return true if succeeded
		virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource);
		
		virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost);
	
		
		/// query the current load information of a method type
		///@param[in] methodType to specify the method type to query
		///@param[out] allocatedKbps the current allocated bandwidth in Kbps
		///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
		///@param[out] sessions the current running session instances
		///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
		///@return true if the query succeeded
		virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins);
		

		virtual MethodCost* getMethodCost(const std::string& methodType){ return this;}

		virtual std::string getCategory()
		{
			return "CPH_AquaRec";
		}

		/// evaluate the cost per given session count and total allocated bandwidth
		///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
		///@param[in] sessions to specify the allocated session instances
		///@return a cost in the range of [0, MAX_LOAD_VALUE] at the given load level:
		///		0 - fully available
		///		MAX_LOAD_VALUE - completely unavailable
		virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions);

		virtual ZQTianShan::ContentProvision::ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		{
			if (NULL == methodType || (stricmp(METHODTYPE_AQUAREC, methodType)))
				return NULL;
			
			return new AquaRecSess(*this, pSess);
		}

	};
	
#endif
