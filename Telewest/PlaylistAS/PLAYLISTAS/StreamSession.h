// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: StreamData.cpp,v 1.1 2004/10/06 10:00:00 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : impl the data and data-management of the session between AS and SS
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTAS/StreamSession.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     05-03-24 16:50 Bernie.zhao
// added soap call when acceptrdyasset failed
// 
// 1     05-02-16 11:56 Bernie.zhao
// 
// 2     04-12-01 17:38 Ken.qian
// 
// 1     04-10-11 17:04 Ken.qian
// 
// Revision 1.1  2004/10/06 10:00:00 Ken Qian
//   definition and implemention
//
// ===========================================================================

#ifndef _H_STREAMSESSION
#define _H_STREAMSESSION

#include "IssApi.h"
#include "StreamOpCtrl.h"
#include "PlaylistSoapProxy.h"

class CStreamDataManager;

class CStreamSession  
{
public:
	/// Create an instance of CStreamSession. There is only one instance 
	///   in the program.
	/// @param typeInst - the unique identification which to use SSAPI.
	/// @param dwAppUID  - the unique identification the AS ID.
	/// @return - the instance of CStreamSession.
	static CStreamSession* Instance(TYPEINST typeInst, DWORD dwAppUID);

	/// free the CStreamSession instance created by Instance().
	static void FreeInstance(void);

protected:
	/// constructor: must be protected to avoid creating CStreamSession instance
	///   directly thru constructor.
	/// @param typeInst - the unique identification which to use SSAPI.
	/// @param dwAppUID  - the unique identification the AS ID.
	/// @return - the instance of CStreamSession.
	CStreamSession(TYPEINST	typeInst, DWORD dwAppUID); 

	/// destructor
	virtual ~CStreamSession() {}

public:
	/// Initialize the ISSAPI to prepare for the communication between AS and SS.
	BOOL	Initialize();
	/// Terminate the session between AS and SS.
	BOOL	UnInitialize();

//private:
public:
	//////////////////////////////////////////////////////////////////////////
	/// Get the private attributes of CStreamSession.
	TYPEINST	GetTypeInst(void) { return m_TypeInst; }
	LPOPCTLTBL  GetOpCtrlTable(void) {	return m_StreamOptb.GetOpCtrlTable();};
	LPITVSTATUSBLOCK GetStatusBlock(void) { return m_ItvStatus.GetStatusBlock(); }

	/// set WSDL file path for SOAP client
	///@param[in]	wsdlFilePath	the path of the file
	void	SetSoapWSDLFilePath(wchar_t* wsdlFilePath) { wcscpy(m_SoapWSDLFilePath, wsdlFilePath); }

	/// set WSML file path for MSSOAP client
	///@param[in]	wsmlFilePath	the path of the file
	void	SetSoapWSMLFilePath(wchar_t* wsmlFilePath) { wcscpy(m_SoapWSMLFilePath, wsmlFilePath); }

	/// set SOAP web service name
	///@param[in]	soapServicename	the service name
	void	SetSoapServiceName(wchar_t* soapServicename) { wcscpy(m_SoapServiceName, soapServicename); }
	
	/// set SOAP web service port
	///@param[in]	soapPort		the service port
	void	SetSoapPort(wchar_t* soapPort) { wcscpy(m_SoapPort, soapPort); }
	
	/// set SOAP web service namespace
	///@param[in]	soapNamespace	the service namespace
	void	SetSoapNamespace(wchar_t* soapNamespace) { wcscpy(m_SoapNamespace, soapNamespace); }
	
	/// set SOAP timeout
	///@param[in]	soapTimeout		the timeout to set, in milli-second
	void	SetSoapTimeout(DWORD soapTimeout) { m_SoapTimeout = soapTimeout; }

	//////////////////////////////////////////////////////////////////////////
	// CallBack Routine Pointer
	static LPCOMPLROUTINE GetComplRoutine();
	static LPSSCALLBACKROUTINE	GetCallBackRoutine();


	//////////////////////////////////////////////////////////////////////////
	// these following process the identified type CallBack
	static BOOL	OnNewStream(STREAMID sid, LPNEWSTREAM pNewStream);
	static BOOL	OnAssetReady(STREAMID sid, PAELIST pOriginalAEList, PAELIST pNewAes = NULL);
	static BOOL	OnAETransition(STREAMID sid, LPAEXITION pAEXition);
	static BOOL	OnStateChanged(STREAMID sid, LPSTATECHG stateChg);
	static BOOL	OnLoadChanged(STREAMID sid);
	static BOOL	OnPrimeVideoStore(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnPlayStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnFFStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnRewStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnPauseStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnResumeStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);
	static BOOL	OnStopStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction);

private:
	/// the asynchronization API processing result returned.
	static void IssComplCallBack(LPITVSTATUSBLOCK pItvSb);
	/// the call back dispatcher
	static BOOL IssCallBackDispather(TYPEINST TypeInst, STREAMID Sid, 
							CALLBACKTYPE CbType, LPCBKBLOCK	pCbBlk);

	/// Make a copy of the new AEList
	/// @param pNewAEList - the source AEList that will be copyed.
	/// @param lNewAECount(out) - How many Asset Element in the AEList.
	/// @return - the copyed Asset Elements array. 
	///           the AELEMENT array's memory was allocated in this fun, and should be freed outside
	static PAELEMENT CopyNewAelements(const PAELIST pNewAEList, long& lNewAECount);

	/// log COM error info
	///@param[in]	pMessage	appended message
	///@param[in]	Error		com error
	static void LogSoapError(wchar_t* pMessage, const _com_error& Error);
	
private:
	TYPEINST    m_TypeInst;
	DWORD       m_dwAppUid;

	BOOL         m_bSubSet;
	OPCTL        m_RdyAssetOpCtl;
	CALLBACKTYPE m_CbCallBackMask;
	STATECHGTYPE m_StateChangeMask;

	ItvStatus		   m_ItvStatus;
	StreamOpCtrlTable  m_StreamOptb;

	static CStreamSession* m_instance;
	static CStreamDataManager m_streamDatas;

	/// path of SOAP WSDL file
	static wchar_t		m_SoapWSDLFilePath[MAX_PATH];

	/// path of MSSOAP WSML file
	static wchar_t		m_SoapWSMLFilePath[MAX_PATH];

	/// SOAP web service name
	static wchar_t		m_SoapServiceName[MAX_PATH];

	/// SOAP web service port
	static wchar_t		m_SoapPort[MAX_PATH];

	/// SOAP web service namespace
	static wchar_t		m_SoapNamespace[MAX_PATH];

	/// SOAP timeout, in milli-second
	static DWORD		m_SoapTimeout;
};

#endif // _H_STREAMSESSION
