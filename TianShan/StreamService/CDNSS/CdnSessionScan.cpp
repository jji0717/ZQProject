
#include "CdnSessionScan.h"
#include "CdnEnv.h"
#include "CdnSSConfig.h"

namespace ZQ
{
namespace StreamService
{

class SessionStatusQuery
{
public:
    int32 invoke() {
        request_ = new C2Streamer::SessionStatusRequestParam();
        response_ = new C2Streamer::SessionStatusResponseParam();
        request_->method = C2Streamer::METHOD_SESSION_STATUS;
        request_->includeAggregate = false;
        return C2Streamer::cSessionStatus(request_, response_);
    }
    const SessionAttrS& getSessions() {
        return response_->sessionInfos;
    }
    const std::string& getLastError() const {
        return response_->errorText;
    }
private:
    C2Streamer::SessionStatusRequestParamPtr request_;
    C2Streamer::SessionStatusResponseParamPtr response_;
};
CdnSessionScaner::CdnSessionScaner(  CdnSsEnvironment* environment , SsServiceImpl* serviceInstance )
:env(environment),
ss(serviceInstance)
{

}
CdnSessionScaner::~CdnSessionScaner( )
{

}

void CdnSessionScaner::getSessionData( )
{	
	SessionStatusQuery query;
	if( query.invoke() != 200 )
	{
		SESSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnSessionScaner,"failed to query session status because [%s]"), query.getLastError().c_str());
	}
	else
	{
		mSessionsNew.clear();
		mSessionsNew = query.getSessions();
	}
}
void CdnSessionScaner::stop()
{
    if(isRunning()) {
        mbQuit = true;
        mSem.post();
        waitHandle(100*1000);
    }
}
bool CdnSessionScaner::init()
{
	mbQuit = false;
	mSessionsOld.reserve( 1024 );
	mSessionsNew.reserve( 1024 );
	mSessionsNew.clear();
	mSessionsOld.clear();
	return true;
}

bool lessCompare ( const SessionAttr& elem1, const SessionAttr& elem2 )
{
	return  elem1.transferId < elem2.transferId;
}
int CdnSessionScaner::run()
{
	int32 delayInterval = gCdnSSConfig.sessionScanInterval;
	delayInterval = delayInterval < 100 ? 100 : delayInterval;
	delayInterval = delayInterval > 60000 ? 60000 : delayInterval;
	
	SESSLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnSessionScaner,"*****************************************Start scanner***********************************"));

	while ( !mbQuit )
	{
		getSessionData();
		std::sort(mSessionsNew.begin() , mSessionsNew.end() ,lessCompare );

		SessionAttrS::const_iterator itNew = mSessionsNew.begin();
		SessionAttrS::const_iterator itOld = mSessionsOld.begin();
		while( itNew != mSessionsNew.end() && itOld != mSessionsOld.end() )
		{
			if ( itNew->transferId > itOld->transferId )
			{//old session expired
				onSessionExpired( itOld );
				itOld++;
			}
			else if( itNew->transferId == itOld->transferId )
			{//check state change
				onSessionStateChange( itOld , itNew );
				itNew ++;
				itOld ++;
			}
			else
			{//itNew->transferId < itOld->transferId
				//new session found
				onSessionNew( itNew );
				itNew ++;
			}
		}
		while( itNew != mSessionsNew.end() )
		{//new session
			onSessionNew( itNew );
			itNew ++;

		}
		while ( itOld != mSessionsOld.end() )
		{//expired session
			onSessionExpired( itOld );
			itOld ++;
		}
		mSessionsOld = mSessionsNew;
		//ZQ::common::delay(delayInterval );
		mSem.timedWait(delayInterval);
	}
	return -1;
}
void CdnSessionScaner::onSessionNew( SessionAttrS::const_iterator itNew )
{
	SESSLOG(ZQ::common::Log::L_INFO,EventFMT((env->getNetId().c_str()) ,
												CdnSessionNew,
												New,
												0,
												"found new session [%s] on port[%s] "),
												itNew->transferId.c_str() ,
												itNew->transferPortName.c_str() );
	StreamParams paras;
	TianShanIce::Properties props;
	ss->OnStreamEvent(SsServiceImpl::seNew,itNew->transferId, paras , props );
}
void CdnSessionScaner::onSessionExpired( SessionAttrS::const_iterator itOld )
{
	SESSLOG(ZQ::common::Log::L_INFO,EventFMT((env->getNetId().c_str()) ,
									CdnSessionExpired,
									Expired,
									0,
									"found expired session [%s] on port[%s] "),
									itOld->transferId.c_str() ,
									itOld->transferPortName.c_str() );
	StreamParams paras;
	TianShanIce::Properties props;
	ss->OnStreamEvent(SsServiceImpl::seGone,itOld->transferId, paras , props );
}

void CdnSessionScaner::onSessionStateChange( SessionAttrS::const_iterator itOld , SessionAttrS::const_iterator itNew )
{

}


}
}

