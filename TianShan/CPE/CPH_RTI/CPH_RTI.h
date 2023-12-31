

#ifndef _RTI_CPH_IMPL_
#define _RTI_CPH_IMPL_

#include "CPHInc.h"
#include "BaseCPH.h"
#include "NICSelector.h"
#include "FileIo.h"
#include "CPH_RTICfg.h"

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
		class PushSource;
		class BaseTarget;
	}};	

	extern NetworkIFSelector*  _nNetSelector;  
	class RTISess : public ZQTianShan::ContentProvision::BaseCPHSession, public ZQTianShan::ContentProvision::BaseGraph
	{
	public:
		RTISess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseCPHSession(helper, pSess),BaseGraph(_gCPHCfg.maxAllocSampleCount),
			_filesize(0), _bQuit(false) 
		{
			_bCleaned = false;
			_pMainTarget = NULL;
			_bStartEventSent = false;
			_nSchedulePlayTime = 0;
		}
		
		virtual ~RTISess();
		
	public:
		
		virtual bool preLoad(); 
		virtual bool prime(); 
		virtual void terminate(bool bProvisionSuccess=true);
		virtual bool getProgress(::Ice::Long& offset, ::Ice::Long& total);
		
		virtual void OnProgress(LONGLONG& prcvBytes);
		virtual void OnStreamable(bool bStreamable);
		virtual void OnMediaInfoParsed(ZQTianShan::ContentProvision::MediaInfo& mInfo);
	protected:
		// impl of ThreadRequest
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);
		
	protected:
		void cleanup();
		std::string _strMethod;
		int _nBandwidth;
		int  _bitrate;
		::Ice::Long _filesize, _processed;
		bool _bQuit;
		bool _bCleaned;
		bool _bStartEventSent;
		std::string _strLocalIp;
		std::string _strFileName;
		ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
		static std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;
						
		int	_nSchedulePlayTime;		//in milliseconds
	};

	class MethodCostI : public ZQTianShan::ContentProvision::MethodCost
	{
	public:
		/// evaluate the cost per given session count and total allocated bandwidth
		///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
		///@param[in] sessions to specify the allocated session instances
		///@return a cost in the range of [0, MAX_LOAD_VALUE] at the given load level:
		///		0 - fully available
		///		MAX_LOAD_VALUE - completely unavailable
		virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
		{
			if (sessions >_maxsessionNum)
				return MAX_LOAD_VALUE + 1;

			if(bandwidthKbps > _maxBandwidthKBps)	
				return MAX_LOAD_VALUE + 1;

			int nCost1 = (((float)bandwidthKbps)/_maxBandwidthKBps)*MAX_LOAD_VALUE; 
			int nCost2 = (((float)sessions)/_maxsessionNum)*MAX_LOAD_VALUE; 

			return __max(nCost1, nCost2);
		}

		MethodCostI(unsigned int maxBandwidthKBps, unsigned int maxsessionNum)
		{
			_maxsessionNum = maxsessionNum;
			_maxBandwidthKBps = maxBandwidthKBps;
		}

		virtual std::string getCategory()
		{
			return "CPH_RTI";
		}

	protected:
		unsigned int _maxsessionNum;
		unsigned int _maxBandwidthKBps;
	};

	
	class RTIHelper : public ZQTianShan::ContentProvision::BaseCPHelper
	{
	public:
		RTIHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr);
		virtual ~RTIHelper();
		
		static ZQTianShan::ContentProvision::BaseCPHelper* _theHelper;
		
	public: // impl of ICPHelper
		
		///validate a potential ProvisionSession about to setup
		///@param[in] sess access to the ProvisionSession about to setup
		///@param[out] schema the collection of schema definition
		///@return true if succeeded
		virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource);
		
		virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
		{			
			if (stricmp(METHODTYPE_RTIVSVSTRM, methodType) && stricmp(METHODTYPE_RTIH264VSVSTRM, methodType))
				return false;
			
			bpsAllocated =0;
			bpsMax=1000000000;
			initCost =0;
			return true;
		}
		
		/// query the current load information of a method type
		///@param[in] methodType to specify the method type to query
		///@param[out] allocatedKbps the current allocated bandwidth in Kbps
		///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
		///@param[out] sessions the current running session instances
		///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
		///@return true if the query succeeded
		virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins);
		
		virtual ZQTianShan::ContentProvision::MethodCost* getMethodCost(const std::string& methodType)
		{
			MethodCostList::iterator it = _methodCostList.find(methodType);
			if (it==_methodCostList.end())
				return NULL;

			return it->second;
		}

		virtual ZQTianShan::ContentProvision::ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		{
			if (NULL == methodType || (stricmp(METHODTYPE_RTIVSVSTRM, methodType) && stricmp(METHODTYPE_RTIH264VSVSTRM, methodType)))
				return NULL;
			
			return new RTISess(*this, pSess);
		}

	protected:
		typedef std::map<std::string, MethodCostI*> MethodCostList;
		MethodCostList								_methodCostList;
		
	};
	
#endif