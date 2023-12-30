

#ifndef _CSI_CPH_IMPL_
#define _CSI_CPH_IMPL_

#include "CPHInc.h"
#include "BaseCPH.h"
#include "NICSelector.h"
#include "FileIo.h"
#include "CPH_CSICfg.h"

#define MAX_AUGMENTATION_PIDS			    4

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
		class PushSource;
		class BaseTarget;
	}};	

    class SelectPort;

	extern NetworkIFSelector*  _nNetSelector;  
	class CSISess : public ZQTianShan::ContentProvision::BaseCPHSession, public ZQTianShan::ContentProvision::BaseGraph
	{
	public:
		CSISess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
			: BaseCPHSession(helper, pSess),_enableNoTrickSpeed(false),
			_filesize(0), _bQuit(false), BaseGraph(_gCPHCfg.maxAllocSampleCount)
		{
			_bCleaned = false;
			_pMainTarget = NULL;
			_bStartEventSent = false;
			_pRTFProc = NULL;
		}
		
		virtual ~CSISess();
		
	public:
		virtual bool Start(); 
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

		bool  setSpeedInfo(bool bH264Type);
		bool  checkFileFormat(const std::string& protocol, const std::string& sourceUrl, bool& bH264);
		
	protected:
		void cleanup();
	    bool  _bPushTrigger;
		int  _bitrate;
		bool _bStartEventSent;

		std::string _strMethod;
		int _nBandwidth;
		::Ice::Long _filesize, _processed;
		bool _bQuit;
		bool _bCleaned;
		ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
		ZQTianShan::ContentProvision::BaseProcess*		_pRTFProc;		//the rtf process

		ZQTianShan::ContentProvision::BaseSource*       _pSource;
		bool										   _bAudioFlag;
		std::string _protocol;
		std::string _sourceURL;

		std::string _strLocalIp;
		std::string _strFileName;

		std::list<float>		_trickspeed;
		uint16					_augmentationPids[ MAX_AUGMENTATION_PIDS ];
		int						_augmentationPidCount ;
		int						_nMaxBandwidth;
	    std::string             _sourceType;

		bool					_enableNoTrickSpeed;



	public:
		static std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;
		static SelectPort*     _pSelectPort;

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
			return "CPH_CSI";
		}
	protected:
		unsigned int _maxsessionNum;
		unsigned int _maxBandwidthKBps;
	};

	class CSIHelper : public ZQTianShan::ContentProvision::BaseCPHelper
	{
	public:
		CSIHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr);

		virtual ~CSIHelper();
		
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
		
		virtual ZQTianShan::ContentProvision::MethodCost* getMethodCost(const std::string& methodType)
		{
			MethodCostList::iterator it = _methodCostList.find(methodType);
			if (it==_methodCostList.end())
				return NULL;

			return it->second;
		}

		virtual ZQTianShan::ContentProvision::ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		{
			if (NULL == methodType || (stricmp(METHODTYPE_CSI, methodType)))
				return NULL;
			
			return new CSISess(*this, pSess);
		}
		
	protected:
		typedef std::map<std::string, MethodCostI*> MethodCostList;
		MethodCostList								_methodCostList;
	};
	
#endif