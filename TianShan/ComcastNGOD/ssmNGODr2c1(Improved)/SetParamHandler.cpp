#include "./SetParamHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

SetParamHandler::SetParamHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "SET_PARAMETER";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct SET_PARAMETER handler"<<endl;
#endif

}

SetParamHandler::~SetParamHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct SET_PARAMETER handler"<<endl;
#endif

}

RequestProcessResult SetParamHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetParamHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetParamHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}
	
	_connectionID = getRequestHeader("SYS#ConnID");

	getContentBody(ZQ::common::Log::L_INFO);
	
	std::vector<std::string> params;
	ZQ::StringOperation::splitStr(_requestBody, "\r\n", params);
	int params_size = params.size();
	int i;
	for (i = 0; i < params_size; i++)
	{
		std::vector<std::string> values;
		int values_size;
		if (NULL != strstr(params[i].c_str(), "session_groups"))
		{
			ZQ::StringOperation::splitStr(params[i], ": \t", values);
			values_size = values.size();
			for (int cur = 1; cur < values_size; cur++)
			{
				if (false == values[cur].empty())
				{
					ZQ::common::MutexGuard lk(_ssmNGODr2c1._connIDGroupsLock);
					ssmNGODr2c1::ConnIDGroupPair _pair;
					_pair._connectionID = _connectionID;
					_pair._sessionGroup = values[cur];
					int m_connIDGroupsSize = _ssmNGODr2c1._connIDGroups.size();
					bool b_found = false;
					for (int j = 0; j < m_connIDGroupsSize; j++)
					{
						if (_ssmNGODr2c1._connIDGroups[j]._sessionGroup == _pair._sessionGroup)
						{
							b_found = true;
							_ssmNGODr2c1._connIDGroups[j] = _pair;
						}
					}
					if (false == b_found)
					{
						_ssmNGODr2c1._connIDGroups.push_back(_pair);
					}
					HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetParamHandler, "map sessionGroup: [%s] to connectionID: [%s]. vector's size: [%d]"), _pair._sessionGroup.c_str(), _connectionID.c_str(), _ssmNGODr2c1._connIDGroups.size());
				}
			}
		}
		else if (NULL != strstr(params[i].c_str(), "session_list") && true == _ssmNGODr2c1._config._bSetParamHeartBeat)
		{
			ZQ::StringOperation::splitStr(params[i], " \t", values);
			values_size = values.size();
			for (int cur = 1; cur < values_size; cur++)
			{
				std::string cltSessID, onDemandID;
				cltSessID = ZQ::StringOperation::getLeftStr(values[cur], ":", true);
				onDemandID = ZQ::StringOperation::getRightStr(values[cur], ":", true);
				if (true == cltSessID.empty() || true == onDemandID.empty())
					continue;

				Ice::Identity ident;
				ident.name = cltSessID;
				ident.category = SERVANT_TYPE;
				NGODr2c1::ctxData ctxDt;
				Ice::Long ttl = _ssmNGODr2c1._config._timeoutInterval * 1000 + cur * 100;// 每个session多加100ms
				try 
				{
					NGODr2c1 ::ContextPrx pContextPrx = NGODr2c1::ContextPrx::uncheckedCast(_ssmNGODr2c1._pContextAdapter->createProxy(ident));					
					ctxDt = pContextPrx->getState();
					pContextPrx->renew(ttl);
					_ssmNGODr2c1._pSessionWatchDog->watchSession(ident, ttl);
				}
				catch (Ice::Exception& ex)
				{
					HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetParamHandler, "renew() client session caught: [%s]"), ex.ice_name().c_str());
					continue;
				}

				SessionRenewCmd* pRenewCmd = new SessionRenewCmd(_ssmNGODr2c1, ctxDt.ident.name.c_str(), ctxDt.weiwooFullID
					, ZQTianShan::now() + ttl + 60000, ctxDt.expiration);
				pRenewCmd->start();
			}
		}
	}

	responseOK();

	return RequestProcessed;
}

