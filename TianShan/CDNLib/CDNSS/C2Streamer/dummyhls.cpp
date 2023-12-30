#include <ZQ_common_conf.h>
#include <urlstr.h>
#include "C2StreamerService.h"
#include "dummyhls.h"


namespace C2Streamer {

	
HLSServer::HLSServer(C2StreamerEnv& env , C2Service& svc)
	: mEnv(env),
	mSvc(svc),
	mAuth(0),
	mCdmiClient(0){
}

HLSServer::~HLSServer() {
	uninit();
}

void HLSServer::uninit() {
	if( mAuth ) {
		delete mAuth;
		mAuth = 0;
	}
	if( mCdmiClient ) {
		delete mCdmiClient;
		mCdmiClient = 0;
	}
}

bool HLSServer::init() {
	uninit();
	const C2EnvConfig& conf = mEnv.getConfig();

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HLSServer,"init() initialize HLS Server with: keyfile[%s], rootUrl[%s] homeContainer[%s] logFlag[%x] defaultBitrate[%u]"),
			conf.mKeyFile5I.c_str(), conf.mAquaRootUrl.c_str(),
			conf.mHomeContainer.c_str(), conf.mLogFlag, conf.mDefaultBitrate);

	mAuth = new Authen5i( *mEnv.getLogger() );
	if( !mAuth->loadKeyFile( mEnv.getConfig().mKeyFile5I.c_str() ) ) {
		delete mAuth;
		return false;
	}
	HLSContent::setSubname2Bitrate( conf.mName2Bitrate, conf.mDefaultBitrate);
	std::map<std::string,uint32>::const_iterator it = conf.mName2Bitrate.begin();
	for ( ; it != conf.mName2Bitrate.end(); it ++ )
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HLSServer,"init() name2bitrate : [%s] -> [%u]"),
				it->first.c_str(), it->second);
	}
	mCdmiClient = new CdmiFuseOps( *mEnv.getLogger(), mEnv.getThreadPool(), 
									conf.mAquaRootUrl,
									"",
									conf.mHomeContainer,
									conf.mLogFlag);
	return true;
}

void HLSServer::updateLastError( HLSRequestParamPtr request, HLSResponseParamPtr response , int errorCode, const char* fmt, ... )
{
	char szLocalBuffer[1024];
	szLocalBuffer[sizeof(szLocalBuffer)-1] = 0;
	va_list args;
	va_start( args , fmt );
	int nCount = vsnprintf( szLocalBuffer , sizeof(szLocalBuffer) - 1 ,fmt , args );
	if( nCount < 0 )
		szLocalBuffer[sizeof(szLocalBuffer)-1] = 0;
	else
		szLocalBuffer[nCount] = 0;
	va_end(args);
	response->setLastErr(errorCode,szLocalBuffer);
	MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(HLSServer,"Conetent[%s/%s] %s"), 
													request->contentName.c_str(),
													request->subLevelName.c_str(),
													 szLocalBuffer);
}

int32 HLSServer::process(HLSRequestParamPtr request, HLSResponseParamPtr response ) {
	assert(request!=0);
	assert(response!=0);
	
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(HLSServer,"process() come a request: contentName[%s], subLevelName[%s]"),
			request->contentName.c_str(), request->subLevelName.c_str());

	if( !mAuth->authen( request->url))	{
		if(mAuth->isExpired( request->url )) {
			updateLastError(request,response, errorCodeSessionGone,"failed to auth the url, expired: %s", request->url.c_str());
			return errorCodeSessionGone;
		} else {
			updateLastError(request,response,errorCodeBadCredentials,"failed to auth the url: %s", request->url.c_str());
			return errorCodeBadCredentials;
		}
	}
	HLSContent hls( *mEnv.getLogger(), *mAuth, *mCdmiClient, request->contentName);
	std::string strContent;
	bool bOK = true;
	if(!request->subLevelName.empty()) {
		if(!hls.exportPlaylistM3u8(request->subLevelName, strContent ) ) {
			bOK = false;
		}
	} else  {
		ZQ::common::URLStr urlstr(request->url.c_str());
		const char* sessId = urlstr.getVar("i");
		const char* addr = urlstr.getVar("a");
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(HLSServer,"process() export master M3U8 with: sessId[%s] addr[%s]"),
				sessId,addr);
		if(!hls.exportMasterM3u8(strContent,1, sessId, addr , mEnv.getConfig().mServerHostUrl.c_str() ) ) {
			bOK = false;
		}
	}
	if(!bOK) {
		updateLastError( request, response, errorCodeInternalError,"failed to export m3u8 for [%s/%s]",
				request->contentName.c_str(), request->subLevelName.c_str());
		return errorCodeInternalError;
	}
	response->content = strContent;
	response->errorCode = errorCodeOK;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HLSServer,"process(), processed the request: contentName[%s], subLevelName[%s]"),
			request->contentName.c_str(), request->subLevelName.c_str());
	return errorCodeOK;
}

}//namespace C2Streamer

