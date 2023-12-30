/*****************************************************************************
File Name:     Connection.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CConnection
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/
#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include <list>
#include"sccommonsender.h"
#include"sccommonreceiver.h"
class CConnection
{
public:
	CConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver/*,CParse *pParse*/);
	//SetConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver);
	virtual ~CConnection(void);
	//have a send thread to send data
    CSCCommonSender*   m_pSender;
	//have a recv thread to receive data
	CSCCommonReceiver* m_pRecerver;
 };
typedef std::list< CConnection* > CConnectionPtrList;

#endif

