// QueryLAM.h: interface for the QueryLAM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUERYLAM_H__3B2024E5_B5A2_40AD_9FC5_1DF61941543F__INCLUDED_)
#define AFX_QUERYLAM_H__3B2024E5_B5A2_40AD_9FC5_1DF61941543F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <map>
#include <tsstreamer.h>
#include "LAMFacade.h"
#include "NGODEnv.h"

using namespace com::izq::am::facade::servicesForIce;

class NGODEnv;
class RequestHandler;
class QueryLAM  
{
public:
	QueryLAM();
	virtual ~QueryLAM();
public:	
	///volumn name must be unique
	bool		AddBackupLAM(IN const std::string& volumnName ,IN const std::string& lamEndpoint ,NGODEnv* pEnv , bool bWarmUp = FALSE);

	/// get aelist and the max bandwidth
	/// NOTE:this function do not clear the parameter aeInfo because the function call be called several times
	bool		GetAeList(IN const std::string& strProviderID,IN const std::string& strAssetID,
								IN int cueIn , IN int cueOut, IN RequestHandler* pHandler,
								OUT AEInfo& aeInfo,OUT long& maxBW);
private:
	typedef struct _LAMInfo
	{
		std::string			lamEndpoint ;	//lam endpoint
		LAMFacadePrx		lamProxy;		//ice proxy of a lam
	}LAMInfo;
	typedef std::map<std::string , LAMInfo>	LAMMAP;
	LAMMAP					_lamMap;
};

#endif // !defined(AFX_QUERYLAM_H__3B2024E5_B5A2_40AD_9FC5_1DF61941543F__INCLUDED_)
