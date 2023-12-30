#ifndef __C2Streamer_DUMMY_HLS_Header_file_h__
#define __C2Streamer_DUMMY_HLS_Header_file_h__

#include "C2StreamerEnv.h"
#include <CdmiFuseOps.h>
#include <HLSContent.h>

namespace C2Streamer {

class C2Service;

class HLSServer {
public:
	HLSServer( C2StreamerEnv& env , C2Service& svc );
	virtual ~HLSServer();
	bool	init();
	int32	process(HLSRequestParamPtr request, HLSResponseParamPtr response );
	void	uninit();
protected:
	void updateLastError( HLSRequestParamPtr request, HLSResponseParamPtr response , int errorCode, const char* fmt, ... );
private:
	C2StreamerEnv&	mEnv ;
   	C2Service&		mSvc;
	Authen5i		*mAuth;
	CdmiFuseOps		*mCdmiClient;
};

}//namespace C2Streamer

#endif//__C2Streamer_DUMMY_HLS_Header_file_h__

