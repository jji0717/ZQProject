#ifndef _STREAM_SMITH_SERVICE_H__
#define	_STREAM_SMITH_SERVICE_H__

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include <ZQDaemon.h>
#endif
#include "FileLog.h"
#include "FingerPrint.h"
#include "json/json.h"

#define SERVICE_CFG_FILE		"StreamNow."

class StreamSmithService : public ZQ::common::BaseZQServiceApplication  
{
public:
	StreamSmithService();
	virtual ~StreamSmithService();
	
public:
	HRESULT OnInit();
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnUnInit();	
	virtual bool isHealth(void);
	virtual void doEnumSnmpExports();

    // virtual void OnSnmpSet(const char *varName);
    
    void readLicenseFile(const std::string& fileName);

	uint32 dummyGet() { return 1; }
	void   resetMeasure(const uint32&);

private:
	ZQ::common::Log*					m_pLog;
	ZQ::common::Log*					m_pPluginLog;
	unsigned short						m_usrtsPort;
	unsigned short						m_uslscPort;	

public:
	uint32 getSessionCount();
	uint32 getPendingSize();
	uint32 getBusyThreads();
	uint32 getThreads();
};

#endif
