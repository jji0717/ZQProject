// MCastFwDServ.h: interface for the MCastFwDServ class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCASTFWDSERV_H__64EAEE5D_2DEF_4932_B99A_9F9EFB3A1B0B__INCLUDED_)
#define AFX_MCASTFWDSERV_H__64EAEE5D_2DEF_4932_B99A_9F9EFB3A1B0B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSchangeServiceApplication.h"
#include "..\Mcastfwd.h"
#include "XMLPreference.h"


#define	MCASTFWD_MANPKG_TUNNELS		_T("Tunnels")
#define MCASTFWD_MANPKG_LISTENERS	_T("Conversations")
#define MCASTFWD_MANPKG_RESENDERS	_T("Resenders")
#define MCASTFWD_MANPKG_DENYLIST	_T("DenyList")

#define DEFAULT_REMCAST_TTL				1
#define DEFAULT_EXT_CONFIG_FILE_NAME	_T("McastFwd.xml")

// extern DWORD gdwMaxUDPBufferSize;
extern ZQ::common::BaseSchangeServiceApplication * Application;
// static member 
typedef struct tag_DIS_TUNNEL{
	int ID;
	std::string PeerID;
	std::string PeerIP;
	std::string EstablishTime;
	std::string Owner;
	std::string status;
}DIS_TUNNEL;

typedef struct tag_DIS_CONVERSATION{
	int ID;
	std::string McastGroup;
	int McastPort;
	std::string BindIP;
	std::string PrivateTunnel;
}DIS_CONVERSATION;

typedef struct tag_DIS_REMCASTER{
	std::string BindIP;
	int bindPort;
	std::string Owner;
}DIS_REMCASTER;

typedef struct tag_DIS_DENY{
	std::string DenyIP;
	std::string Mask;
	int			Port;
	std::string Owner;
}DIS_DENY;

class MCastFwdServ  : public ZQ::common::BaseSchangeServiceApplication, public McastFwd//TODO:, public McastFwd
{
public:

	
	//create by compiler
	MCastFwdServ();
	
protected:
	HRESULT OnInit(void);
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	bool isHealth(void);
	HRESULT OnUnInit(void);
private:

	//Man Utility Management 

	static UINT MgmtTunnels(WCHAR *, WCHAR **, DWORD *);
	static UINT MgmtConversation(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);
	static UINT MgmtDenylist(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);
	static UINT MgmtResenders(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);

private:	
	//static McastFwd* m_MCastFwd;
	static ZQ::common::IPreference* m_IPreference;
	
		
	//configurations:static for manuntility session 
	static wchar_t m_wsConfigFileName[MaxPath];
	static DWORD m_dwTunnelListenPort;
	static DWORD m_dwReMcastTTL ;
	static wchar_t m_wsInstallationID[64];//GUID

	static wchar_t m_DefaultTunnels[256];


};


#endif // !defined(AFX_MCASTFWDSERV_H__64EAEE5D_2DEF_4932_B99A_9F9EFB3A1B0B__INCLUDED_)
