// QueryLAM.cpp: implementation of the QueryLAM class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NGODEnv.h"
#include "RequestHandler.h"
#include "QueryLAM.h"
#include <log.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define ENVLOG	pEnv->_fileLog
QueryLAM::QueryLAM()
{

}

QueryLAM::~QueryLAM()
{

}
bool QueryLAM::AddBackupLAM(IN const std::string& volumnName ,IN const std::string& lamEndpoint ,NGODEnv* pEnv , bool bWarmUp)
{
	LAMFacadePrx	prx = NULL;
	std::string	endPoint = lamEndpoint;
	if ( endPoint.find(":") != std::string::npos ) 
	{//do not have adapter name
		endPoint = "LAMFacade:";
		endPoint +=lamEndpoint;
		ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(QueryLAM,"adjust lam endpoint from [%s] to [%s]"),
													lamEndpoint.c_str(),endPoint.c_str());
	}
	if ( bWarmUp ) 
	{
		//connect to LAM
		ENVLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(QueryLAM,"connect to LAM:%s"),lamEndpoint.c_str());
		try
		{
			prx =LAMFacadePrx::checkedCast(pEnv->_pCommunicator->stringToProxy(endPoint));
		}
		catch (Ice::Exception& ex) 
		{
			prx = NULL;
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(QueryLAM,"Catch an Ice exception [%s] when connect to [%s]"),
												ex.ice_name().c_str() , endPoint.c_str());
			return false;
		}
		catch (...) 
		{
			prx = NULL;
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(QueryLAM,"Catch unknown exception when connect to [%s]"),endPoint.c_str());
			return false;
		}
	}
	LAMInfo info;
	info.lamEndpoint	= endPoint ; 
	info.lamProxy		= prx;
	
	if (_lamMap.find(volumnName)!=_lamMap.end()) 
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(QueryLAM,"There is already a LAM with volumnName [%s]"),volumnName.c_str());
		return false;
	}
	else
	{
		_lamMap[volumnName] = info;
	}
	return true;
}