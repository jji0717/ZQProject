
#include <ZQ_common_conf.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"


#if defined ZQ_OS_MSWIN
#define	SESSFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"##y), request->requestHint.c_str() 
#elif defined ZQ_OS_LINUX
#define	SESSFMT(x,y) 	CLOGFMT(x, "  REQUEST[%s]\t"y), request->requestHint.c_str()
#endif	

namespace C2Streamer
{

int32 C2Session::processTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response )
{	
	/// terminate any in progress file content reading procedure and change current session's state to IDLE
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(C2Session,"processTransferTerm() entering"));

	//mDataRunner->stopRunner();
	destroy();
	
	//mSvc.getSessManager().destroySession( mSessionId );
	
	//FIXME: how to terminate content reading procedure ?

	return errorCodeOK;
}

}//namespace C2Streamer
