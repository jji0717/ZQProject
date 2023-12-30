// hODServ.h: interface for the ChODServ class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HODSERV_H__026153AF_4A8A_42CF_932E_FD9BD4C60AA0__INCLUDED_)
#define AFX_HODSERV_H__026153AF_4A8A_42CF_932E_FD9BD4C60AA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable : 4251 4275) 

#include "BaseZQServiceApplication.h"
#include <string>
#include "ChODSvcEnv.h"
#include "ChannelOnDemandAppImpl.h"
#include "ChannelPublisherImpl.h"
#include "EventChannel.h"
#include "IceLog.h"
#include "CODConfig.h"


class ChODServ   : public ZQ::common::BaseZQServiceApplication
{
public:
	ChODServ();
	virtual ~ChODServ();

public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	
	bool isHealth(void);
	bool loadConfig();

protected:
	
private:
	Ice::CommunicatorPtr							_communicator;
	ZQChannelOnDemand::ChODSvcEnv*					_pChOdSvcEnv;	
	::TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	ZQ::common::FileLog*							_iceFileLog;
};

#endif // !defined(AFX_HODSERV_H__026153AF_4A8A_42CF_932E_FD9BD4C60AA0__INCLUDED_)
