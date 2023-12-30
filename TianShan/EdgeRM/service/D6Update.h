#ifndef __D6_UPDATE_HEADER_FILE_H__
#define __D6_UPDATE_HEADER_FILE_H__

#include "TianShanDefines.h"
#include "VrepStates.h"
#include "VrepListener.h"
#include "EdgeRMImpl.h"
#include "EdgeRMCfgLoader.h"

extern 	ZQ::common::Config::Loader<ZQTianShan::EdgeRM::EdgeRMCfgLoader > pConfig;

namespace ZQTianShan {
	namespace EdgeRM {

class D6Update : public ZQ::Vrep::StateMachine::Monitor
{
public:
	D6Update(EdgeRMEnv& env, ::TianShanIce::EdgeResource::EdgeRMPrx& edgeRMPrx);
	~D6Update(void);

public: //Monitor
	void onStateChanged(ZQ::Vrep::StateDescriptor from, ZQ::Vrep::StateDescriptor to);
	 void onEvent(ZQ::Vrep::Event e);
	 void onOpenMessage(const ZQ::Vrep::OpenMessage& msg);
	 void onUpdateMessage(const ZQ::Vrep::UpdateMessage& msg);
	 void onNotificationMessage(const ZQ::Vrep::NotificationMessage& msg);
protected:
	 EdgeRMEnv& _env;
	 TianShanIce::EdgeResource::EdgeRMPrx& _edgeRMPrx;
	 ZQ::Vrep::EdgeInputs                  _edgeInputs;
	 std::string         _edgeDeviceName;
	 std::string         _edgeDeviceZone;
	 std::string         _edgeDeviceVendor;
	 IdentCollection     _identChannels;
protected:
	bool addDevice(int RFPortId, int chId, const ZQ::Vrep::UpdateMessage& msg);
	bool addChannelToDevice(TianShanIce::EdgeResource::EdgeDeviceExPrx& edgeDevicePrx, int RFPortId, int chId, const ZQ::Vrep::UpdateMessage& msg);
	bool WithdrawnRoutes(TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx, int RFPortId, int chId, ZQ::Vrep::Routes& routes);
	bool UpdateChannel(TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx, const ZQ::Vrep::UpdateMessage& msg);

};
class D6Factory: public ZQ::Vrep::MonitorFactory
{
public:
	D6Factory(ZQTianShan::EdgeRM::EdgeRMEnv& env, ::TianShanIce::EdgeResource::EdgeRMPrx& edgeRMPrx)
		: _env(env), _edgeRMPrx(edgeRMPrx){};
	virtual ~D6Factory(void){};
protected:
	EdgeRMEnv& _env;
	TianShanIce::EdgeResource::EdgeRMPrx& _edgeRMPrx;
public:
    ZQ::Vrep::StateMachine::Monitor* create() 
	{
		ZQ::Vrep::StateMachine::Monitor*  pMonitor = new D6Update(_env, _edgeRMPrx);
		return pMonitor;
	}
	virtual void destroy(ZQ::Vrep::StateMachine::Monitor* m)
	{
		try
		{
			if(m)
				delete m;
		}	
		catch (...){}
	}
};	
class QAMWithDrawTimeOut: public ZQ::Vrep::TimeoutObject
{
public:
	QAMWithDrawTimeOut(::TianShanIce::EdgeResource::EdgeChannelExPrx& chExPrx, std::string& chName):
	  _chExPrx(chExPrx), _chName(chName){};
	  ~QAMWithDrawTimeOut(void){};
public:
	void onTimer()
	{
		try
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(QAMWithDrawTimeOut, "channel[%s] OnTimer, disable channel set state whth OutOfService"), _chName.c_str());

			TianShanIce::Properties attrs;
			attrs.clear();
			char temp[32]="";
			itoa(TianShanIce::stOutOfService, temp, 10);
			attrs[SYS_PROP(DeviceState)] = temp;
			_chExPrx->updateAttributes(attrs);
		}
		catch (TianShanIce::ServerError&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(QAMWithDrawTimeOut, "[%s]disable channel caught TianShanIce ServerError errorcode[%d], errmsg[%s]"),
				_chName.c_str(), ex.errorCode, ex.message.c_str());
		}
		catch(Ice::Exception&ex)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(QAMWithDrawTimeOut, "[%s]disable channel caught Ice Exception[%s]"),
				_chName.c_str(), ex.ice_name().c_str());
		}
	};
protected:
	TianShanIce::EdgeResource::EdgeChannelExPrx _chExPrx;
	std::string _chName;
};
class QAMTimerObjects;
typedef boost::shared_ptr<ZQ::Vrep::Timer> TimerPtr;
typedef std::map<std::string, TimerPtr> QAMTimers;

class QAMTimerObjects
{
public: 
	QAMTimerObjects(ZQTianShan::EdgeRM::EdgeRMEnv& env,ZQ::common::NativeThreadPool& thpool, int timeoutInterval);
	~QAMTimerObjects(void);
public:
	bool addTimer(std::string& chName, TianShanIce::EdgeResource::EdgeChannelExPrx& chPrx);
	bool removeTimer(std::string& chName);
	void removeAll();
protected:
	EdgeRMEnv& _env;
    int _timeOutInterval; ///ms
	QAMTimers qamTimers;
	ZQ::Vrep::Watchdog* _pwatchDog;
	ZQ::common::Mutex   _lkqamtimer;
};
}} // namespace
#endif //__D6_UPDATE_HEADER_FILE_H__
