/*****************************************************************************
File Name:     DSAResource.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CDSAResource
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/

#ifndef __CDSAMANAGER_H__
#define __CDSAMANAGER_H__

#include"TCPConnection.h"
#include"ClientParse.h"

//class CClientFactory;
#include"ClientFactory.h"
class CResourceManager;
class CDSAResource
{
 public:
	CDSAResource( CResourceManager *pOwner,char* IP,int Port);
	virtual ~CDSAResource(void);
 private:
    //one DSA has only one connection with SRM 
    CTCPConnection* m_pConnection;
	//CDSAParse* m_pParse;
 private:
	 //pointer to CSingleConnect
	 //whitch connect to DSA and receive send data
	 //from DSA
   CClientFactory   m_pSingleConnect;
   int  m_MaxDODCount;     //max DOD count           
   int  m_CurDODCount;     //current DOD in use     
   int  m_DSAPort;         // DSA listen Port
   char m_DSAIP[16];       // DSA IP 
 public:
    //Start to connect DSA
   void StartConnect();
   void SetConnection(CTCPConnection* pConnection);
   CTCPConnection* GetConnection();
   //Set the connect State
   void SetState(bool State);
   bool GetState();
   void CloseSocket();
   void RestartConnect();
   public:
	  CResourceManager * m_pOwner;
  };

//DSA list
typedef std::list< CDSAResource* > CDSAResourcePtrList;

#endif 
