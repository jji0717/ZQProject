#ifndef __ZQTianShan_S6HANDLER_H__
#define __ZQTianShan_S6HANDLER_H__

#include "EdgeRM.h"
#include "FileLog.h"
//#include "REInterface.h"
#include "RtspInterface.h"
#include "definition.h"
#include <list>
#include <vector>

//#define TestForNanJing

namespace ZQTianShan {
namespace EdgeRM {
class EdgeRMEnv;
class S6Handler : public ::ZQRtspCommon::IHandler
{
public:
	S6Handler(EdgeRMEnv& env, ::TianShanIce::EdgeResource::EdgeRMPrx &edgeRMPrx);
	~S6Handler();

	virtual bool HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg);
	virtual void onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	
protected:
	ZQ::common::Mutex					_genIdCritSec;
private:
	TianShanIce::EdgeResource::EdgeRMPrx &_edgeRMPrx;
	EdgeRMEnv &_env;
public:
	bool  _b1stSetupMsg;

#ifdef  TestForNanJing
	typedef struct 
	{
		int startPort;
		int startPN;
		int stepPortByPn;
		int maxSession;
		int totalSession;
		int64 frenquence;
	}ChannelInfo;
	typedef std::map<std::string, ChannelInfo>ChToPort;
	ChToPort _chToPort;
	ZQ::common::Mutex					_lockChPort;

	bool getPort(std::string chName, int& port, Ice::Long& pn, int64& frenquence);
#endif

	bool doSetup(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
	bool doTeardown(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
	bool doGetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
	bool doSetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);

	bool procResponse(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);

    void generateSessionID(std::string& sessionID);
};

//typedef ::std::list<::ZQTianShan::EdgeRM::S6Handler *> S6HandlerList;

}//namespace EdgeRM
}//namespace ZQTianShan

#endif //__ZQTianShan_S6HANDLER_H__
