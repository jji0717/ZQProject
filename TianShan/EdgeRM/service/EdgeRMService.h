#ifndef __ZQTianShan_EdgeRMService_H__
#define __ZQTianShan_EdgeRMService_H__

#include "ConfigHelper.h"

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include <ZQDaemon.h>
#endif

#include "EdgeRMEnv.h"
#include "EdgeRMImpl.h"
#include "IceLog.h"
#include "RtspEngine.h"
#include "S6Handler.h"

namespace ZQTianShan {
namespace EdgeRM{

class EdgeRMSvc : public ZQ::common::BaseZQServiceApplication 
{
public:

	EdgeRMSvc ();
	virtual ~EdgeRMSvc ();

	//public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	virtual void doEnumSnmpExports();
	virtual bool isHealth(void);

	uint32 snmp_dummyGet() { return 0; }

	void snmp_refreshDevices(const uint32&);
	void snmp_refreshErmPorts(const uint32&);
	void snmp_refreshErmChannels(const uint32&);
private:
	typedef  std::list< TianShanIce::EdgeResource::EdgeChannelInfos>      EdgeChannelInfosList;
	bool getEdgeChannelInfosList(EdgeChannelInfosList& channelInfosList, ZQ::common::Log& reporter);//all EdgeChannelInfos store in list

public:
	EdgeRMEnv						*_edgeRMEnv;

#ifdef ZQ_OS_MSWIN
	std::string m_strProgramRootPath;
#endif

	std::string _strLogFolder;

	Ice::CommunicatorPtr			_ic;

	TianShanIce::common::IceLogIPtr	_iceLog;

	ZQ::common::FileLog			_iceFileLog;
	ZQ::common::FileLog			_eventFileLog;

	ZQ::common::FileLog          _RtspEngineLog;

	ZQADAPTER_DECLTYPE				_adapter;
	ZQ::common::NativeThreadPool	_RequestThreadPool;
};

}//namespace EdgeRM
}//namespace ZQTianShan

#endif //__ZQTianShan_EdgeRMService_H__
