/*****************************************************************************
File Name:     Parse.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CParse
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/
#ifndef __CCLIENTPARSE_H__
#define __CCLIENTPARSE_H__

#include"scqueue.h"
class CSessionManager;
class CTCPConnection;
class ISCParser
{
public:
	virtual void Parse(CSCMemoryBlockPtr block) = 0;
};

class CClientParse : public ISCParser
{
public:
    CClientParse(CTCPConnection* pConnection);
	virtual ~CClientParse(void);
public:
    CSessionManager * m_pSessionManager;
     
public:
    // @Function	This function Parse the comand from DSA and 
	//transfer the command to client
	/// @Param		connection(IN) the connection receive command from DSA
	/// @Return		if successfully transfer the command return true
	//else return false
	virtual void Parse(CSCMemoryBlockPtr block);
    //set the CSessionManager pointer to m_pSessionManager
	//when it is initialize, it should be setse
	void SetSessionManager(CSessionManager * pManager);
   void SetConnection(CTCPConnection* pConnection);
private:
    CString GetCurrDateTime();
    CTCPConnection* m_Connection; 
};

class CDSAResource;
class CDSAParse : public ISCParser
{
public:
    CDSAResource* m_Connection;
	CDSAParse(CDSAResource* pDSAParse);
	virtual ~CDSAParse(void);
public:
    CSessionManager * m_pSessionManager;
public:
	// @Function	This function Parse the comand from DSA and 
	//transfer the command to client
	/// @Param		connection(IN) the connection receive command from DSA
	/// @Return		if successfully transfer the command return true
	//else return false
	virtual void Parse(CSCMemoryBlockPtr block);
     //set the CSessionManager pointer to m_pSessionManager
	//when it is initialize, it should be set
	void SetSessionManager(CSessionManager * pManager);
	void SetDSAResource(CDSAResource* pConnection);
   };
#endif 