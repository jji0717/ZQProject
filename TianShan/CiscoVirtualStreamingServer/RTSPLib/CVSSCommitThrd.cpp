#include "CVSSCommitThrd.h"
#include "rtsp_action.h"

CVSSCommitThrd::CVSSCommitThrd(::ZQTianShan::CVSS::CiscoVirtualStreamImplPtr CVSStreamObj,
							 const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream):
_CVSStreamObj(CVSStreamObj),
_amdStream(amdStream)
{
}
	
CVSSCommitThrd::~CVSSCommitThrd()
{

}

bool CVSSCommitThrd::initialize(void)
{
	return true;
}

int CVSSCommitThrd::terminate(int code)
{
	return 1;
}

int	CVSSCommitThrd::run(void)
{
	::IceUtil::RWRecMutex::WLock sync(*_CVSStreamObj);

	CVSSRtspSession *_cvssRTSPSession = _CVSStreamObj->_cvssRTSPSession;
	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(SETUP, _CVSStreamObj->_env._rtspCSeqSignal, _cvssRTSPSession, _CVSStreamObj->_pool, _CVSStreamObj->_logFile);
	_CVSStreamObj->_logFile(ZQ::common::Log::L_INFO, CLOGFMT(CVSSCommitThrd, "commit_async: Ident(%s)Socket(%d)Sess(%s)CSeq(%d) send play over, cost %dms"), _CVSStreamObj->ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
	
	if (!b)
	{
		::std::string strErrMsg = ::std::string("CVSStreamImpl::commit_async() failed to setup session(") + _CVSStreamObj->ident.name + ::std::string(") with video server");
		::ZQ::common::Exception ex(strErrMsg);
		_CVSStreamObj->_logFile(ZQ::common::Log::L_ERROR, CLOGFMT(CVSStreamImpl,"CVSStreamImpl::commit_async() failed to setup session(%s) with video server"), _CVSStreamObj->ident.name.c_str());

		_amdStream->ice_exception(ex);		
		return 0;
	}
	else
	{
		_CVSStreamObj->_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSStreamImpl,"commit_async: Ident(%s)Socket(%d)Sess(%s)CSeq(%d) setup with video server"), _CVSStreamObj->ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq);
	}
	_amdStream->ice_response();
	return 1;
}