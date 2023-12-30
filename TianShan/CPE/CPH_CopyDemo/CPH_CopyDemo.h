#ifndef __CPH_COPYDEMO_H__
#define __CPH_COPYDEMO_H__

#include "BaseCPH.h"
#include "CPHInc.h"
#include "ZQ_common_conf.h"

//#define METHODTYPE_COPYDEMO "CopyDemo"

class CopyDemoSess : public ZQTianShan::ContentProvision::BaseCPHSession
{
public:
	CopyDemoSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);
	virtual ~CopyDemoSess();
	
public:
	
	virtual bool preLoad();
	virtual void terminate(bool cleanupIncompletedOutput=true);
	virtual bool getProgress(Ice::Long& offset, Ice::Long& total);

protected:
	// impl of ThreadRequest
	virtual int run(void);
	virtual void doCleanup();
#ifdef ZQ_OS_LINUX
	bool mountFile(const std::string& strurl, const std::string&);
	void umountFile(const std::string& strTarget);
#endif

private:
	std::string _sourceFilename, _destFilename;

#ifdef ZQ_OS_MSWIN
	HANDLE _hSource, _hDest;
#else
	FILE  *_hSource, *_hDest;
	std::string _strTarget;
	bool _bLocal;
#endif

	::Ice::Long _filesize, _copied;
	int _maxKbps;
	char* _buf;
	bool _bQuit;

};

class CopyDemoHelper : public ZQTianShan::ContentProvision::BaseCPHelper, ZQTianShan::ContentProvision::MethodCost
{
public:
	CopyDemoHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
		:BaseCPHelper(pool, mgr) {}
	virtual ~CopyDemoHelper() {}
	
	static ZQTianShan::ContentProvision::BaseCPHelper* _theHelper;
public: // impl of ICPHelper

    ///validate a potential ProvisionSession about to setup
	///@param[in] sess access to the ProvisionSession about to setup
	///@param[out] schema the collection of schema definition
	///@return true if succeeded
    virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
	{
		return true;
	}
	
    virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
	{

		if (NULL == methodType || 0 != strcmp(METHODTYPE_COPYDEMO, methodType))
			return false;

		bpsAllocated =0;
		bpsMax=1000000000;
		initCost =0;
		return true;
	}
	
	virtual ZQTianShan::ContentProvision::ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
	{
		if (NULL == methodType || 0 != strcmp(METHODTYPE_COPYDEMO, methodType))
			return NULL;

		return new CopyDemoSess(*this, pSess);
	}

	virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
	{
		return true;
	}

	virtual MethodCost* getMethodCost(const std::string& methodType){ return this;}

	/// evaluate the cost per given session count and total allocated bandwidth
	///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
	///@param[in] sessions to specify the allocated session instances
	///@return a cost in the range of [0, MAX_LOAD_VALUE] at the given load level:
	///		0 - fully available
	///		MAX_LOAD_VALUE - completely unavailable
	virtual unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
	{
		return 0;
	}

};

#endif //__CPH_COPYDEMO_H__

