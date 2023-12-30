#include <winsock2.h>
#include "TMVSSCommitThrd.h"
#include "TMVSSCfgLoader.h"
#include "tianshandefines.h"

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::TM::TMVSSCfg > pConfig;
namespace ZQTianShan{
namespace VSS{
namespace TM{

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

TMVSSCommitThrd::TMVSSCommitThrd(std::string cbNotification,
								 std::string ctxNotification,
								 ::ZQTianShan::VSS::TM::TMVStreamImplPtr TMVStreamObj,
								 std::string strPathTicket,
								 const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream)
:_env(TMVStreamObj->_env)
,_cbNotification(cbNotification)
,_ctxNotification(ctxNotification)
,_TMVStreamObj(TMVStreamObj)
,_amdStream(amdStream)
,_strPathTicket(strPathTicket)
{
}
	
TMVSSCommitThrd::~TMVSSCommitThrd()
{

}

bool TMVSSCommitThrd::initialize(void)
{
	return true;
}

int TMVSSCommitThrd::terminate(int code)
{
	return 1;
}

int	TMVSSCommitThrd::run(void)
{
	::IceUtil::RWRecMutex::WLock sync(*_TMVStreamObj);

	DWORD iTime = GetTickCount();

	_soapClientSession = new TMVSoapClientSession(_env._logFile, _env._strServerPath.c_str(), _env._uServerPort);
	//_soapClientSession = new TMVSSProxy();
	//memcpy(_soapClientSession->endpoint, _env._urlStr.getPath(), strlen(_env._urlStr.getPath()));

	::TianShanIce::Streamer::StreamPrx streamProxy = TMVSSIdentityToObjEnv(_env, TMVStream, _TMVStreamObj->ident);
	_soapClientSession->strStreamName = _env._communicator->proxyToString(streamProxy);
	_soapClientSession->strOndemandSessionId = _TMVStreamObj->ident.name;

	ZQ2__setupInfo setupInfo;
	setupInfo.resource = new ZQ2__map();
	setupInfo.params = new ZQ2__map();

	//get value map from pathticket
	try{
		printf("pathticket string=%s\n", _strPathTicket.c_str());
		//::Ice::Identity tmpident = _env._communicator->stringToIdentity(_strPathTicket);
		::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._communicator->stringToProxy(_strPathTicket));

		::TianShanIce::ValueMap privateData = _pathTicketPrx->getPrivateData();
		::ZQTianShan::dumpValueMap(privateData, "", dummyprint);
		for (::TianShanIce::ValueMap::iterator iter = privateData.begin(); iter != privateData.end(); iter++)
		{
			ZQ2__pair *pair = new ZQ2__pair();
			pair->key = iter->first;
			pair->value = getVariantValue(iter->second);
			setupInfo.params->ptr.push_back(pair);
			//setupInfo.params[iter->first] = getVariantValue(iter->second);
		}
		
		::TianShanIce::SRM::ResourceMap resMap = _pathTicketPrx->getResources();
		::ZQTianShan::dumpResourceMap(resMap, "", dummyprint);
		for (::TianShanIce::SRM::ResourceMap::iterator iter = resMap.begin(); iter != resMap.end(); iter++)
		{
			for (::TianShanIce::ValueMap::iterator viter = privateData.begin(); viter != privateData.end(); viter++)
			{
				ZQ2__pair *pair = new ZQ2__pair();
				pair->key = viter->first;
				pair->value = getVariantValue(viter->second);
				setupInfo.resource->ptr.push_back(pair);
			}
		}
	}
	catch (Ice::Exception& ex)
	{
		_env._logFile(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSCommitThrd, "get pathticket info caught ice exception[%s]"), ex.ice_name().c_str());
	
		//printf("catch exception when get information from pathticket\n");
		printf("get information from config for test\n");
		for (ZQTianShan::VSS::TM::PrivateData::params::iterator iter = pConfig._privateData._params.begin(); iter != pConfig._privateData._params.end(); iter++)
		{
			ZQ2__pair *pair = new ZQ2__pair();
			pair->key = (*iter).key;
			pair->value = (*iter).value;
			setupInfo.params->ptr.push_back(pair);
			//setupInfo.params[iter->first] = getVariantValue(iter->second);
		}

		for (ZQTianShan::VSS::TM::ResourceMap::params::iterator iter = pConfig._resourceMap._params.begin(); iter != pConfig._resourceMap._params.end(); iter++)
		{
			ZQ2__pair *pair = new ZQ2__pair();
			pair->key = (*iter).key;
			pair->value = (*iter).value;
			setupInfo.resource->ptr.push_back(pair);
		}
	}
	catch (...)
	{	
		::std::string strErrMsg = ::std::string("TMVSStreamImpl::commit_async() failed to get info from pathticket, session(") + _TMVStreamObj->ident.name + ::std::string(")");
		::ZQ::common::Exception ex(strErrMsg);
		_amdStream->ice_exception(ex);
		return 0;
	}

	setupInfo.cbNotification = _cbNotification;
	setupInfo.ctxNotification = _ctxNotification;

	ZQ2__setupResponse setupInfoRes;

	bool b = _soapClientSession->soapSetup(&setupInfo, setupInfoRes);
	_env._logFile(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSCommitThrd, "commit_async: session(%s) send SETUP over, cost %dms"), _TMVStreamObj->ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
	{
		::std::string strErrMsg = ::std::string("TMVSStreamImpl::commit_async() failed to setup session(") + _TMVStreamObj->ident.name + ::std::string(") with video server");
		::ZQ::common::Exception ex(strErrMsg);
		_env._logFile(ZQ::common::Log::L_ERROR, CLOGFMT(TMVSSCommitThrd,"TMVSStreamImpl::commit_async() failed to setup session(%s) with video server"), _TMVStreamObj->ident.name.c_str());

		delete _soapClientSession;
		_amdStream->ice_exception(ex);
		return 0;
	}
	else
	{
		_env._logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSCommitThrd,"TMVSStreamImpl::commit_async() successfully setup sesion(%s) with video server"), _TMVStreamObj->ident.name.c_str());

		_TMVStreamObj->controlURl = setupInfoRes.controlURL;
		_TMVStreamObj->props["a-control"] = setupInfoRes.controlURL;
		//add to session map
		if (_env._pTMVSSSoapServer->_soapClientMap.addSoapClient(setupInfoRes.sessionId, _soapClientSession) == false)
		{
			::std::string strErrMsg = ::std::string("TMVSStreamImpl::commit_async() failed to add session(") + _TMVStreamObj->ident.name + ::std::string(") to map");
			::ZQ::common::Exception ex(strErrMsg);
			_env._logFile(ZQ::common::Log::L_ERROR, CLOGFMT(TMVSSCommitThrd,"TMVSStreamImpl::commit_async() join sesion(%s:%s) to session map failed"), _TMVStreamObj->ident.name.c_str(), setupInfoRes.sessionId.c_str());
			delete _soapClientSession;
			_amdStream->ice_exception(ex);
			return 0;
		}
		else
		{
			_TMVStreamObj->sessId = setupInfoRes.sessionId;
			_TMVStreamObj->_soapClientSession = _soapClientSession;
		}
	}
	
	_amdStream->ice_response();
	return 1;
}

TianShanIce::Variant TMVSSCommitThrd::GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap, const TianShanIce::SRM::ResourceType& type, const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(_env._logFile,EXPFMT("TMVSSCommit",1001, "GetResourceMapData() type %d not found"), type);
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(_env._logFile,EXPFMT("TMVSSCommit",1001, "GetResourceMapData() value with key=%s not found"), strkey.c_str());
	}
	return it->second;
}

}//namespace TM
}//namespace NSS
}//namespace ZQTianShan