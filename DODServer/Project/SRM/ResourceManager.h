/*****************************************************************************
File Name:     ResourceManager.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CResourceManager
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/
#ifndef __CRESOURCEMANAGER_H__
#define __CRESOURCEMANAGER_H__

#include"DSAResource.h"
#include<list>
#include"ClientParse.h"
#include"SessionManager.h"

class CSRManager;
//DSA to connect
typedef struct DSATAG
{
   int  IPPort;
   char	IPAddress[16];
 
}strDSA;
typedef std::list<strDSA> DSAList;

class CResourceManager
{ 
public:
	CResourceManager(CSRManager* pOwner);
    virtual ~CResourceManager(void);
public:
    CDSAResourcePtrList  m_DSAResourcePtrList;//DSA connection list
    DSAList              m_DSAAddrList;//DSA list
    // CDSAParse            m_parse;
public:
    /// @Function	This function get the current CDSAResource and set the
	//current CDSAResource to next
	/// @Param		CDSAResource*(IN,OUT),
	/// @Return		if successfully get the current CDSAResource return ture
	//else if the  CDSAResource list is null return false 
    //bool GetDSAResource(CDSAResource* pDSAResource);

//	void Initialize();
    /// @Function	This function add a DSA to DSA list.
	/// @Param		char* IPAddress(IN): the IP of the DSA to connect to
	/// @Param		int port (IN): the port of the DSA listening 
	/// @Return		It returns viod.
	void AddOneDSA(char* IPAddress,int port);
    /// @Function	for each DSA This function create a CDSAResource and add it to list
	/// @Param		void 
	/// @Return		It returns viod.
	void Create();

    void notify(CSCTCPSocket* pSocket);
	void Destroy(void);

    CDSAResource* SelectAResource(void);

    static DWORD WINAPI ThreadEntry(LPVOID pParam);

	//static DWORD WINAPI ThreadEntry((LPVOID pParam);
	void ThreadProc();
	void SendHeartBeat();
	void CloseThread();

    void SetSessionManager( CSessionManager* pSessionManager);


    private:
    //strDSA* GetOneDSA(void);
    CRITICAL_SECTION  m_ResourceLock;
    HANDLE		 m_hThread;              
	HANDLE		 m_hEvent;
            
    CDSAResource* m_CurResource;//pointer to current CDSAResource
    CDSAResource*  m_PreResource;
private:
    void Lock() {EnterCriticalSection(&m_ResourceLock);};
    void UnLock(){LeaveCriticalSection(&m_ResourceLock);};
	//void SendHeartBeat();

	

public:
    CSRManager* m_pOwner;
private:
    CSessionManager* m_pSessionManager;
    
};
#endif


