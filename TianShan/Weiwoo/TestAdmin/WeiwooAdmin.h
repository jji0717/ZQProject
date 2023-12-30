// WeiwooAdmin.h: interface for the WeiwooAdmin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEIWOOADMIN_H__BC68C4C7_150C_4BB4_AB6E_B1292FBA88ED__INCLUDED_)
#define AFX_WEIWOOADMIN_H__BC68C4C7_150C_4BB4_AB6E_B1292FBA88ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CmdParser.h"

#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/IceUtil.h>

#include <tssrm.h>
#include <TsStreamer.h>

#include <EventChannel.h>

#include <map>
#include <string>
#include <Locks.h>

#include "InitInfo.h"

using namespace TianShanIce::SRM;

enum
{
	OPERATION_FREECPU				= 2,
	OPERATION_SUCCESS				= 1,
	OPERATION_FAIL					= -1,
	OPERATION_INVALIDPARAMETER		= -2,
	TIANSHAN_SERVERERROR			= -3,
	TIANSHAN_INVALIDPARAMETER		= -4,
	TIANSHAN_INVALIDSTATE			= -5,
	TIANSHAN_GENERALERROR			= -6,
	ICEERROR_OPERATIONTIMEOUT		= -7,
	ICEERROR_CONNECTIONTIMEOUT		= -8,
	ICEERROR_GENERALERROR			= -9
};

class WeiwooAdmin : public CCmdParser  
{
public:
	WeiwooAdmin( ::Ice::CommunicatorPtr& ic , SessionManagerPrx& sessManagerPrx);
	WeiwooAdmin( );
	~WeiwooAdmin();

public:
	FUNCDEF(Help)
	FUNCDEF(Connect)
	FUNCDEF(DisConnect)	
	FUNCDEF(createSession)	
	FUNCDEF(StartSession)
	FUNCDEF(Destroy)
	FUNCDEF(ParseIni)
	FUNCDEF(ShowTimeCount)	
	FUNCDEF(AddResource)
	FUNCDEF(AddRes)
	FUNCDEF(AddPriveteData)
	FUNCDEF(AddPD)
	FUNCDEF(renew)
	FUNCDEF(AddDVBCResource)
	FUNCDEF(AddIPResource)
	FUNCDEF(AddNGODResource)


private:
	::TianShanIce::SRM::SessionPrx	GetSession(const std::string& sessID);
	std::string						ComposeUri(InitInfo& ini);
private:
	bool							ConvertStringToVariant(const std::string& varType , 
															const std::string& strValue1,
															const std::string& strValue2,
															TianShanIce::Variant& varOut);

	bool							ConvertStringToValuemap( const std::string& varKey ,
															const TianShanIce::Variant& var,
															TianShanIce::ValueMap& vMap);

	bool							ConvertStringToResourceType(const std::string& strType , 
																TianShanIce::SRM::ResourceType& resType);
protected:
	
	SessionManagerPrx		m_sessManager;	
	Ice::CommunicatorPtr	m_Ic;

	::TianShanIce::SRM::SessionPrx m_weiwooSess;

	bool					m_bShowTimeCount;

	bool					m_bDestroyCommunicator;
	
protected:	
	DECLEAR_CMDROUTE();
};

#endif // !defined(AFX_WEIWOOADMIN_H__BC68C4C7_150C_4BB4_AB6E_B1292FBA88ED__INCLUDED_)
