/*****************************************************************************
File Name:     SRManager.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CSRManager
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/

#ifndef __CSRMANAGER_H__
#define __CSRMANAGER_H__

#include"ClientManager.h"
#include"ResourceManager.h"
#include"Sessionmanager.h"

class CSRManager
{
public:
    CSRManager(void);
	virtual ~CSRManager(void);
public:
    CClientManager*    m_ClientManager;
    CResourceManager*  m_ResourceManager;
    CSessionManager *  m_Sessionmanager;
private:
	//Transite the pointer
	void Initialize();
	// @Function	this function load the information in XML file and transit the 
	//parameter to correct class,
	// @Param		void 
	// @Return		if successfully read the xml file return ture
	//else false
	bool LoadConfiguration();
    // @Function	this function worked after  Initialize and LoadConfiguration
	//the function make the class CClientManager CResourceManager CSessionManager
	//to work
	void Start();
private:
    int		m_nError;								// error code.
	char	m_strError[MAX_PATH];					// error description.
    char    m_strXMLFileName[MAX_PATH];		
public:
	BOOL create();
	void Destroy(void);
    BOOL Pause();
    BOOL Resume();

 };
#endif
