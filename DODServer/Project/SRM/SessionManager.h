/*****************************************************************************
File Name:     SessionManager.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CSessionanager
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/

#ifndef __CSESSIONMANAGER_H__
#define __CSESSIONMANAGER_H__

//#include"Connection.h"
#include"ResourceManager.h"
#include"DSAResource.h"
//class CCnection;
class CSRManager;
//define the session state 
enum SESSION_STATUS
{
    normal=1,
    lost,
	err,
};
/////////////////////////////////////////////////////
class CDODSession
{
public:
	CDODSession();
	CDODSession(CTCPConnection* pClientConnection,CDSAResource* pDSAConnection,
		DWORD SessionID, SESSION_STATUS State) 
	{
		try
		{
			m_pClientConnection=pClientConnection;
			m_pDSAConnection=pDSAConnection;
			m_wSessionID=SessionID;
			m_eStatus=State;
		}
		catch(...)
		{
			Clog( LOG_DEBUG, _T("catch Error in CDODSession::CDODSession") );
		}

	};
	virtual ~CDODSession(void)
	{
	};
private:
	CTCPConnection* m_pClientConnection;    //connection to client app
	CDSAResource* m_pDSAConnection;         //resource and connection to DSA
	DWORD m_wSessionID;                     //identify the session, named rule- create time of session
	SESSION_STATUS m_eStatus;               //session status
	DWORD     m_wDODPortID;                 //other DSA parameter to identify, reserved  
public:
	CTCPConnection* GetClientConnction(void)
	{
		return m_pClientConnection;
	}
	void SetClientConnction(CTCPConnection* pconnection)
	{
		try
		{
			m_pClientConnection=pconnection;
		}
		catch(...)
		{
			Clog( LOG_DEBUG, _T("catch Error in CDODSession::SetClientConnction") );
		}
	}
	CDSAResource* GetDSAConnection(void)
	{
		return m_pDSAConnection;
	}
	void SetDSAConnection(CDSAResource* pDSAResource)
	{
		m_pDSAConnection=pDSAResource;
	}
	DWORD GetSessionID(void)
	{
		return m_wSessionID;
	}
	int GetState()
	{
		return m_eStatus;
	}
	void SetState(SESSION_STATUS State)
	{
		m_eStatus=State;
	}
};

typedef std::list<CDODSession> DODSessionList;
//////////////////////////////////////////////////////
class CSessionManager
{
public:
	CSessionManager(CSRManager* pOwner);
	virtual ~CSessionManager(void);
public:
	DODSessionList m_SessionList;                //session list
	CResourceManager*  m_pResourceManager;    //pointer to CResourceManager in CSRManager
	CRITICAL_SECTION  m_sessionLock;
	CRITICAL_SECTION  m_timeLock;
	HANDLE		 m_hThreadCheck;              
	HANDLE		 m_hEventCheck;
private:
	CSRManager* m_pOwner;                     //pointer to CSRManager
public:
	//void CreateSession();
	//generate a session ID
	DWORD GenerateSessionID();
	//set CResourceManager pointer to m_pResourceManager
	void SetResourceManger(CResourceManager*  pManager);
	// @Function	This function Get the CConnection pointer from session list
	// @Param		int SesionID(IN),SesionID identified the session
	// @Return		if session list have the Session ID return the CConnection
	//else return NULL
	CTCPConnection* GetConnection(int SesionID);
	// @Function	This function Get the CDSResource pointer from session list
	// @Param		int SesionID(IN),SesionID identified the session
	// @Return		if session list have the Session ID return the CDSResource
	//else return NULL
	CDSAResource* GetDSResource(int SesionID);
	// @Function	This function find the session in session list 
	//whose id is SesionID,if find delete it ,else do nothing 
	// @Param		int SesionID(IN),SesionID identified the session
	bool DeleteSession(int SesionID);

	bool DeleteSession(CDSAResource* pDSAConnection);   

	void   Lock();
	void   UnLock();
	void   LockTime();
	void   UnlockTime();
	//check the session state in session list
	void   CheckSession();
	void AddASession(CDODSession Session);
	void StartCheck();
	static DWORD WINAPI EntryCheck(LPVOID pParam);
	void ThreadProc();
	void Close();
	void Destroy(void);
};
#endif
