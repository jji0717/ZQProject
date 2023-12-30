#include "./SetParamHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

SetParamHandler::SetParamHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
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
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetParamHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol))
		{
			return RequestError;
		}
	}
	
	_connectionID = getRequestHeader("SYS#ConnID");

	//add by lxm for C1 SET_PARAMETER
	::std::string strRequir = getRequestHeader(NGOD_HEADER_REQUIRE);
	if (strRequir.find("c1") != ::std::string::npos || strRequir.find("C1") != ::std::string::npos )
	{
		getContentBody(ZQ::common::Log::L_INFO);

		std::vector<std::string> params;
		ZQ::StringOperation::splitStr(_requestBody, "\r\n", params);
		int params_size = params.size();
		int i;
		for (i = 0; i < params_size; i++)
		{
			std::vector<std::string> values;
			int values_size;
			ZQ::StringOperation::splitStr(params[i], ": \t", values);
			values_size = values.size();
			if (values_size > 0 && values[0] == "session_list")
			{
				HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetParamHandler, "enter C1 session list sync"));
				for (int cur = 1; cur < values_size; cur = cur + 2)
				{
					if (false == values[cur].empty())
					{
						_session = values[cur];
						updateContextProp(C1CONNID, _connectionID);
					}
				}
			}
			else if (values_size > 0 && values[0] == "position")
			{
				//TODO: NGOD ECR003 mentioned to support a SET_PARAMETER(position=xxx) to reset a session
				#pragma message ( __MSGLOC__ "NGOD ECR003 mentioned to support a SET_PARAMETER(position=xxx) to reset a session")
				//HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetParamHandler, "enter C1 session reset");
				//if (false == values[1].empty())
				//{
				//}
			}
		}
	}
	else
	{
		getContentBody(ZQ::common::Log::L_INFO);
		
		std::vector<std::string> params;
		ZQ::StringOperation::splitStr(_requestBody, "\r\n", params);
		int params_size = params.size();
		int i;
		for (i = 0; i < params_size; i++)
		{
			std::vector<std::string> values;
			int values_size;
			ZQ::StringOperation::splitStr(params[i], ": \t", values);
			values_size = values.size();
			if (values_size > 0 && values[0] == "session_groups")
			{
				for (int cur = 1; cur < values_size; cur++)
				{
					if (false == values[cur].empty())
					{
						ZQ::common::MutexGuard lk(_ssmNGODr2c1._connIDGroupsLock);
						NGODEnv::ConnIDGroupPair _pair;
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
						HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetParamHandler, "map sessionGroup: [%s] to connectionID: [%s]. vector's size: [%d]"), _pair._sessionGroup.c_str(), _connectionID.c_str(), m_connIDGroupsSize);
					}
				}
			}
		}
	}

	responseOK();

	return RequestProcessed;
}

