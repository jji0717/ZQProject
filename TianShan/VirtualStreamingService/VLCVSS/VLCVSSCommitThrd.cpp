#include <winsock2.h>
#include "VLCVSSCommitThrd.h"
#include "VLCVSSCfgLoader.h"
#include "tianshandefines.h"

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::VLC::VLCVSSCfg > pConfig;
namespace ZQTianShan{
namespace VSS{
namespace VLC{

static void dummyprint(const char* line,void* pVoid = NULL)
{
	printf("%s\n", line);
}

std::string getVariantValue(::TianShanIce::Variant &val)
{
	std::stringstream ss;
	switch (val.type)
	{
	case ::TianShanIce::vtStrings:
		if (val.strs.empty())
			return VariantDefault;
		else
			return val.strs[0];
	case ::TianShanIce::vtBin:
		if (val.bin.empty())
			return VariantDefault;
		else
		{
			ss << val.bin[0];
			return ss.str();
		}
	case ::TianShanIce::vtFloats:
		if (val.floats.empty())
			return VariantDefault;
		else
		{
			ss << val.floats[0];
			return ss.str();
		}
	case ::TianShanIce::vtInts:
		if (val.ints.empty())
			return VariantDefault;
		else
		{
			ss << val.ints[0];
			return ss.str();
		}
	case ::TianShanIce::vtLongs:
		if (val.lints.empty())
			return VariantDefault;
		else
		{
			ss << val.lints[0];
			return ss.str();
		}
	default:
		return VariantDefault;
	}
}

VLCVSSCommitThrd::VLCVSSCommitThrd(::ZQTianShan::VSS::VLC::VLCStreamImplPtr VLCStreamObj,
								   std::string strPathTicket,
								   const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream)
:_env(VLCStreamObj->_env)
,_VLCStreamObj(VLCStreamObj)
,_amdStream(amdStream)
,_strPathTicket(strPathTicket)
{
}
	
VLCVSSCommitThrd::~VLCVSSCommitThrd()
{

}

bool VLCVSSCommitThrd::initialize(void)
{
	return true;
}

int VLCVSSCommitThrd::terminate(int code)
{
	return 1;
}

int	VLCVSSCommitThrd::run(void)
{
	::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx streamProxy = VLCVSSIdentityToObjEnv(_env, VLCStream, _VLCStreamObj->ident);
	std::string destIp = streamProxy->getDestIp();
	::Ice::Int destPort = streamProxy->getDestPort();

	::IceUtil::RWRecMutex::WLock sync(*_VLCStreamObj);

	DWORD iTime = GetTickCount();
	
	//get playlist idx
	
	std::string strKey = _VLCStreamObj->ident.name;

	//create new playlist
	//_vlcClientSession = new VLCTelnetSession(_env._logFile, strKey, destIp, destPort);
	//_vlcClientSession->_strStreamName = _env._communicator->proxyToString(streamProxy);

	//get value map from pathticket
	try{
		//printf("pathticket string=%s\n", _strPathTicket.c_str());
		::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._communicator->stringToProxy(_strPathTicket));

		::TianShanIce::SRM::ResourceMap resMap = _pathTicketPrx->getResources();
		//::ZQTianShan::dumpResourceMap(resMap, "", dummyprint);
	}
	catch (Ice::Exception& ex)
	{
		_env._logFile(ZQ::common::Log::L_INFO, CLOGFMT(VLCVSSCommitThrd, "get pathticket info caught ice exception[%s]"), ex.ice_name().c_str());
		_amdStream->ice_exception(ex);
		return 0;
	}
	catch (...)
	{	
		::std::string strErrMsg = ::std::string("VLCVSSCommitThrd::commit_async() failed to get info from pathticket, session(") + strKey + ::std::string(")");
		::ZQ::common::Exception ex(strErrMsg);
		_amdStream->ice_exception(ex);
		return 0;
	}

	std::string strRetMsg;
	_VLCStreamObj->_pl._name = _VLCStreamObj->ident.name;
	_VLCStreamObj->_pl._ouputItem._dstIp = _VLCStreamObj->destIp;
	_VLCStreamObj->_pl._ouputItem._dstPort = _VLCStreamObj->destPort;

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCNEW, strKey, _VLCStreamObj->_pl, strNull, strNull, strNull, strRetMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->newPL(_VLCStreamObj->_pl, strRetMsg);
	_env._logFile(ZQ::common::Log::L_INFO, CLOGFMT(VLCVSSCommitThrd, "commit_async: session(%s) send SETUP over, cost %dms"), strKey.c_str(), GetTickCount() - iTime);
	
	if (!b)
	{
		::std::string strErrMsg = ::std::string("VLCVSSCommitThrd::commit_async() failed to setup session(") + _VLCStreamObj->ident.name + ::std::string(") with video server");
		::ZQ::common::Exception ex(strErrMsg);

		_env._logFile(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSSCommitThrd,"VLCVSSCommitThrd::commit_async() failed to setup session(%s) with video server"), strKey.c_str());

		//delete _vlcClientSession;
		_amdStream->ice_exception(ex);
		return 0;
	}
	else
	{
		_env._logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSCommitThrd,"VLCVSSCommitThrd::commit_async() successfully setup sesion(%s) with video server"), strKey.c_str());
	}
	
	_amdStream->ice_response();
	return 1;
}

TianShanIce::Variant VLCVSSCommitThrd::GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap, const TianShanIce::SRM::ResourceType& type, const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(_env._logFile,EXPFMT("VLCVSSCommitThrd",1001, "GetResourceMapData() type %d not found"), type);
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(_env._logFile,EXPFMT("VLCVSSCommitThrd",1001, "GetResourceMapData() value with key=%s not found"), strkey.c_str());
	}
	return it->second;
}

}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan