

#ifndef _NPVR_CPH_IMPL_
#define _NPVR_CPH_IMPL_

#include "CPHInc.h"
#include "BaseCPHnPVR.h"
#include "BaseClass.h"
#include "VirtualSessI.h"
#include "NICSelector.h"

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
		class PushSource;
		class BaseTarget;
	}};	

	extern NetworkIFSelector*  _nNetSelector;   
	class NPVRSess : public ZQTianShan::ContentProvision::BaseNPVRSession, public IceUtil::RecMutex
	{
	public:
		typedef IceUtil::Handle<NPVRSess> Ptr;

		NPVRSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseNPVRSession(helper, pSess),
			_filesize(0), _bQuit(false) 
		{
			_sessState = SSTATE_NONE;
			_bStreamable = false;
			_processed = 0;
			_bitrate = 0;
		}
		
		virtual ~NPVRSess();
		
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

		bool checkStreamable();

		/// set the NPVR content property 
		///@return	if content proxy is empty, return true; if not empty and set property successful, then return true; else return false
		bool setContentProperty();

		bool setContentPropertyLeadCopy(const std::string& strLeadCopy);


		int getTimeoutMilisecond();

		void createTimer(int nTimeoutInMs);
		void removeTimer();
	protected:

		std::string _strMethod;
		int _nBandwidth;
		int  _bitrate;
		::Ice::Long _filesize, _processed;
		bool _bQuit;

		SessionState	_sessState;

		bool			_bStreamable;

		ZQTianShan::ContentProvision::MediaInfo						_mediaInfo;

		std::auto_ptr<ZQTianShan::ContentProvision::VirtualSessI>	_pNPVRSess;

		std::string							_strLogHint;
	};
	
	class NPVRHelper : public ZQTianShan::ContentProvision::BaseCPHelper, ZQTianShan::ContentProvision::MethodCost
	{
	public:
		NPVRHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr);
			
		virtual ~NPVRHelper();
		
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
			return "CPH_NPVR";
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
			if (NULL == methodType || (stricmp(METHODTYPE_NPVRVSVSTRM, methodType)))
				return NULL;
			
			return new NPVRSess(*this, pSess);
		}

	protected:
		bool validateContentNPVRProperty(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess);	


	};
	
#endif