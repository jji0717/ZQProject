#include "NSSCommitThrd.h"
#include "ngod_rtsp_action.h"

NSSCommitThrd::NSSCommitThrd(::ZQTianShan::NSS::NGODStreamImplPtr NGODStreamObj,
							 const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream):
_NGODStreamObj(NGODStreamObj),
_amdStream(amdStream)
{
}
	
NSSCommitThrd::~NSSCommitThrd()
{

}

bool NSSCommitThrd::initialize(void)
{
	return true;
}

int NSSCommitThrd::terminate(int code)
{
	return 1;
}

int	NSSCommitThrd::run(void)
{
	::IceUtil::RWRecMutex::WLock sync(*_NGODStreamObj);

	DWORD iTime = GetTickCount();
	_NGODStreamObj->_logFile(ZQ::common::Log::L_INFO, CLOGFMT(NSSCommitThrd, "commit_async: session(%s) begin send SETUP"), _NGODStreamObj->ident.name.c_str());
	bool b = ngod_rtsp_action::SetupAction(_NGODStreamObj->ident.name, *_NGODStreamObj->_nssSessionGroupList, _NGODStreamObj->_pool, _NGODStreamObj->_logFile);
	_NGODStreamObj->_logFile(ZQ::common::Log::L_INFO, CLOGFMT(NSSCommitThrd, "commit_async: session(%s) send SETUP over, cost %dms"), _NGODStreamObj->ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
	{
		::std::string strErrMsg = ::std::string("NGODStreamImpl::commit_async() failed to setup session(") + _NGODStreamObj->ident.name + ::std::string(") with video server");
		::ZQ::common::Exception ex(strErrMsg);
		_NGODStreamObj->_logFile(ZQ::common::Log::L_ERROR, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::commit_async() failed to setup session(%s) with video server"), _NGODStreamObj->ident.name.c_str());

		_amdStream->ice_exception(ex);		
		return 0;
	}
	else
	{
		_NGODStreamObj->_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::commit_async() successfully setup sesion(%s) with video server"), _NGODStreamObj->ident.name.c_str());
		RTSPClientSession *pSess = _NGODStreamObj->_env._daemonThrd->findSessionByOnDemandSessionId(_NGODStreamObj->ident.name);

		//add to session map
		if (_NGODStreamObj->_env._daemonThrd->addSession(pSess, _NGODStreamObj->sessGroup) == false)
			_NGODStreamObj->_logFile(ZQ::common::Log::L_WARNING, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::commit_async() join sesion(%s:%s) to session map failed"), _NGODStreamObj->ident.name.c_str(), pSess->m_RTSPR2Header.strSessionID.c_str());

		_NGODStreamObj->sessIdNss = pSess->m_RTSPR2Header.strSessionID;
		//sessGroup = pSess->m_RTSPR2Header.SessionGroup.strToken;

		//renew session key in sessionIdx
		_NGODStreamObj->sessKey = _NGODStreamObj->sessGroup + "#" + _NGODStreamObj->sessIdNss;
		_NGODStreamObj->_env._eNssStream->remove(_NGODStreamObj->ident);
		_NGODStreamObj->_env._eNssStream->add(_NGODStreamObj, _NGODStreamObj->ident);
		SDPResponseContent &pContent = pSess->m_RTSPR2Header.m_SDPResponseContent;
		if (!pContent.strProtocol.empty())
			_NGODStreamObj->controlURl = pContent.strProtocol;
		if (!pContent.strHost.empty())
			_NGODStreamObj->controlURl += "://" + pContent.strHost;
		if (pContent.uPort > 0)
		{
			stringstream ss;
			ss << pContent.uPort;
			_NGODStreamObj->controlURl += ":" + ss.str();
		}
		if (!pContent.strStreamhandle.empty())
			_NGODStreamObj->controlURl += "/" + pContent.strStreamhandle;
	}
	_amdStream->ice_response();
	return 1;
}