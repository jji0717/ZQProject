
#include <ZQ_common_conf.h>
#include "TianShanIceHelper.h"
#include "NgodEnv.h"
#include "NgodSession.h"
#include "ClientRequest.h"
#include "SelectionCommand.h"
#include "NgodConfig.h"
#include "NgodHelper.h"

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, request->sessionId.c_str() ,  request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, request->sessionId.c_str() ,  request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str() , pthread_self(),#x	
#endif	

namespace NGOD
{

int NgodSessionI::processTeardown( const NgodRequestTeardownPtr& request )
{
	ServerResponseTeardownPtr response = ServerResponseTeardownPtr::dynamicCast( request->getResponse() );
	assert( response != NULL );
	updateConnectionAndVerCode( request );


	const TeardownParam& para = request->getTeardownParam();
	TeardownResponseParameter info;
	info.remoteIp		= para.clientIp;
	info.localIp		= para.serverIp;
	info.resultCode		= para.reason;

	{
		WLock sync(*this);
		updateSessState(TianShanIce::stOutOfService);
	}

	StreamSessionInfo ssInfo;
	int ret =getStreamSessionInfo( request , ssInfo );
	if( ret != errorcodeOK)
	{
		recordStreamDestroyStatusAndUpdateTimer( true );
		return ret;
	}
	info.npt			= ssInfo.npt;
	info.assetIndex		= ssInfo.assetIndex;	
	info.playTime		= ssInfo.playTime;

	//adjust npt to asset based value according to NGOD 488
	int64 assetBasedNpt = adjustNptToAssetBased( ssInfo.assetIndex , ssInfo.assetNpt );
	char szAssetBasedNptBuffer[128];
	//FIXME
	if( ngodConfig.messageFmt.rtspNptUsage >= 1)
	{
		sprintf( szAssetBasedNptBuffer , "%lld.%03lld", assetBasedNpt/1000, assetBasedNpt%1000 );
	}
	else
	{
		if( ngodConfig.protocolVersioning.enableVersioning >= 1 && mR2VerControlCode ==  NgodVerCode_R2_DecNpt )
		{//hex
			sprintf( szAssetBasedNptBuffer , "%lld.%03lld", assetBasedNpt/1000, assetBasedNpt%1000 );
		}
		else
		{
			sprintf( szAssetBasedNptBuffer , "%llx" , assetBasedNpt );
			
		}
	}
	info.assetBasedNpt = szAssetBasedNptBuffer;
	 

	std::string strNpt;
	if( ssInfo.assetNpt < 0)
		strNpt	= "EOS";
	else
		strNpt = convertIntToNptString( ssInfo.assetNpt );

	std::ostringstream strStopIdx;
	strStopIdx << ssInfo.assetIndex;

	recordEvent_StreamEnd( "USER" , strNpt , strStopIdx.str() );

	{
		RLock sync(*this);

		{//fill response information
			info.componentName	= mEnv.moduleName();
			info.odsessid		= mOndemandId;
			info.sessGroupName	= mGroupId;
			//info.setupDate		= ;
			ZQTianShan::Util::getPropertyDataWithDefault( mProps , SYSKEY(SetupTime),"",info.setupDate);
			info.teardownDate	= getISOTimeString();
			info.sopname		= mSopName;	
		}
		response->setSessionHistoryData( info , mItemInfos , mSessionEvents );
	}

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI,"trying to destroy current session"));
	ret = destroy(request);
	if( ret == errorcodeOK )
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI,"session destroyed"));
		if(ngodConfig.publishLogs.enabled)
		{
			ELOG(ZQ::common::Log::L_INFO, EVENTLOGFMT(NgodSessionI,"state(%s)"), errorCodeTransformer(errorcodeOK) );
		}
	}
	return ret;
}

}

