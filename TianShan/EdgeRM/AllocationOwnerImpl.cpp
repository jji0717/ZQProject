#include "EdgeRMEnv.h"
#include "AllocationOwnerImpl.h"
#include "RtspInterfaceImpl.h"
#include "TimeUtil.h"
#include "EdgeRMCfgLoader.h"
extern 	ZQ::common::Config::Loader<ZQTianShan::EdgeRM::EdgeRMCfgLoader > pConfig;

#define  ERM_SERVER  "ERM_NGOD2/1.6"
#define  ERM_REQUIRE "com.comcast.ngod.s6"
static char* Month[12] = {"Jar","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Dec","Nov","Dec"};

namespace ZQTianShan{
namespace EdgeRM{

::Ice::Int AllocationOwnerImpl::OnAllocationExpiring(const ::std::string& ownerContextKey, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::Ice::Current& c)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AllocationOwnerImpl,"[%s]OnAllocationExpiring alloction"), ownerContextKey.c_str());

	std::string sessionGroup, onDemandSession;
	try
	{
		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = TianShanIce::EdgeResource::AllocationExPrx::uncheckedCast(alloc);
		sessionGroup = allocExPrx->getSessionGroup();
		onDemandSession = allocExPrx->getOnDemandSessionId();
	}
	catch (Ice::ObjectNotExistException& ex)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AllocationOwnerImpl,"sessionId[%s]OnAllocationExpiring alloction not exists, renew 0 msec"),ownerContextKey.c_str());
		return 0;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(AllocationOwnerImpl,"sessionId[%s]OnAllocationExpiring alloction caught exception (%s)"), ownerContextKey.c_str(),ex.ice_name().c_str()); 
		return _time2Expiring; 
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(AllocationOwnerImpl,"sessionId[%s]OnAllocationExpiring alloction caught unknown exception (%d)"), ownerContextKey.c_str(), SYS::getLastErr()); 
		return _time2Expiring; 
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AllocationOwnerImpl,"sessionId[%s]OnAllocationExpiring send announce message to s6client"), ownerContextKey.c_str());

	S6Connection::Ptr s6ConnectionPtr = S6Connection::OpenS6Connection(sessionGroup, _env, false);

	if(!s6ConnectionPtr)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("AllocationOwnerImpl", 1002,"sessionId[%s]failed to get connection info", ownerContextKey.c_str());
	}

	//先找到这个Session的connectID, 如果没找到, 继续找ServiceGroups中的ConnectID
	 ZQ::DataPostHouse::IDataCommunicatorPtr connectionId = s6ConnectionPtr->findConnectionId(ownerContextKey);

	 if(connectionId == NULL)
	 {
		 connectionId =  s6ConnectionPtr ->getSGConnectionId();
	 }

	if(connectionId != NULL)
	{
		char strnotice[256]="";
		time_t now;
		now = time(NULL);
		struct tm* ptm = gmtime(&now);
		if(NULL == ptm)  // t is not a valid value
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("AllocationOwnerImpl", 1003,"sessionId[%s] failed to generate time", ownerContextKey.c_str());
		}
	    struct tm& atm = *ptm;
		snprintf(strnotice, sizeof(strnotice) -1, "5700 \"Session In Progress\" event-date=%4d%02d%02dT%02d%02d%02d.%03dZ npt=\0",
			atm.tm_year + 1900, atm.tm_mon + 1, atm.tm_mday,atm.tm_hour, 
			atm.tm_min,atm.tm_sec, 0);

		char strDate[256]="";
		snprintf(strDate, sizeof(strDate)-1, "%02d %s %04d %02d:%02d:%02d.%03d GMT",
			atm.tm_mday, Month[atm.tm_mon], atm.tm_year + 1900, atm.tm_hour, 
			atm.tm_min, atm.tm_sec, 0);

        char announce[256]="";
		snprintf(announce, sizeof(announce) -1, "ANNOUNCE rtsp://%s:%s/ RTSP/1.0",pConfig.ipv4.c_str(),pConfig.tcpPort.c_str());

		char strSeq[65] = "";
		Ice::Long lseq;
		{
			ZQ::common::MutexGuard gd(_seqMutex);
			_sequence++;
			_sequence = _sequence & 0xFFFFFFFF;
			lseq = _sequence;
		}
		sprintf(strSeq,"%lld", lseq);

		ZQRtspCommon::RtspSendMsg* psendmsg = new ZQRtspCommon::RtspSendMsg(connectionId, _env._RtspEngineLog);

		if(!psendmsg)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("AllocationOwnerImpl", 1001,"sessionId[%s] failed to create rtspsend message object", ownerContextKey.c_str());
		}

		psendmsg->setStartline(announce);
		psendmsg->setHeader(NGOD_HEADER_REQUIRE, ERM_REQUIRE);
		psendmsg->setHeader(NGOD_HEADER_SEQ, strSeq);  
		psendmsg->setHeader(NGOD_HEADER_NOTICE, strnotice);
		psendmsg->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandSession.c_str());
		psendmsg->setHeader(NGOD_HEADER_SERVER, ERM_SERVER);
		psendmsg->setHeader(NGOD_HEADER_SESSION, ownerContextKey.c_str());	   
		psendmsg->setHeader(NGOD_HEADER_DATE, strDate);  
		int32 nret = psendmsg->post();
		psendmsg->release();

		if(nret <= 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("AllocationOwnerImpl", 1000,"sessionId[%s] s6 client not exists", ownerContextKey.c_str());
     	}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(AllocationOwnerImpl,"sessionId[%s]]OnAllocationExpiring OnDemandSessionID(%s),ServiceGroup(%s),ConnectId(0x%08x)"),
			 ownerContextKey.c_str(), onDemandSession.c_str(), sessionGroup.c_str(), connectionId.get());
	}
	else
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("AllocationOwnerImpl", 1000,"sessionId[%s] s6 client not exists",  ownerContextKey.c_str());
	}

	return _time2Expiring; 
}

}//namespace EdgeRM
}//namespace ZQTianShan
