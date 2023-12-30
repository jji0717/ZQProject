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
// $Log: /ZQProjs/Telewest/PlaylistAS_gSoap/StreamSession.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     09-08-04 11:04 Build
// log change
// 
// 3     09-07-31 10:12 Build
// fix the issue inproper userdata passed into AcceptStream
// 
// 2     05-07-29 16:16 Bernie.zhao
// 
// 1     05-06-28 11:38 Bernie.zhao
// 
// 3     05-04-29 19:57 Bernie.zhao
// 
// 2     05-03-24 16:50 Bernie.zhao
// added soap call when acceptrdyasset failed
// 
// 1     05-02-16 11:56 Bernie.zhao
// ===========================================================================

// system includes
#include <assert.h>
#include <string>

// common includes
#include "Log.h"

// local includes
#include "Mod.h"
#include "StreamSession.h"
#include "StreamData.h"
#include "PlaylistSoapProxy.h"

using namespace ZQ::common;

CStreamSession* CStreamSession::m_instance = NULL;
CStreamDataManager CStreamSession::m_streamDatas;

wchar_t		CStreamSession::m_SoapWSDLFilePath[MAX_PATH];
wchar_t		CStreamSession::m_SoapWSMLFilePath[MAX_PATH];
wchar_t		CStreamSession::m_SoapServiceName[MAX_PATH];
wchar_t		CStreamSession::m_SoapPort[MAX_PATH];
wchar_t		CStreamSession::m_SoapNamespace[MAX_PATH];
DWORD		CStreamSession::m_SoapTimeout;

CStreamSession* CStreamSession::Instance(TYPEINST typeInst, DWORD dwAppUID)
{
	if(NULL == m_instance)
	{
		m_instance = new CStreamSession(typeInst, dwAppUID); 
	}
	return m_instance;
}

void CStreamSession::FreeInstance(void)
{
	if(m_instance != NULL) {
		delete m_instance;
	}
	m_instance = NULL;
}

CStreamSession::CStreamSession(TYPEINST typeInst,  DWORD dwAppUID) 
		: m_TypeInst(typeInst)
{
	m_dwAppUid = dwAppUID;

	m_CbCallBackMask = CB_NEWSTREAM | CB_STATECHG | CB_RDYASSET | CB_AEXITION | CB_STRMOP;
	m_bSubSet = TRUE;
	m_RdyAssetOpCtl = SOPCTL_APPROVE;
	m_StateChangeMask = DNETRDY       |
						STRMSTART	  | 
						STRMBEGIN     | 
                        STRMEND       | 
                        STRMTERM      | 
                        STRMABORT     | 
                        STRMLOSTCONN  |
                        STRMPLAYING   |
                        STRMFSTFWDING | 
                        STRMREWINDING | 
                        STRMPAUSED    | 
                        STRMSTOPPED;

	wcscpy(m_SoapWSDLFilePath, L"./PlaylistSoapInterface.wsdl");
	wcscpy(m_SoapWSMLFilePath, L"./PlaylistSoapInterface.wsml");
	wcscpy(m_SoapServiceName, L"PlaylistSoapInterfaceService");
	wcscpy(m_SoapPort, L"PlaylistSoapInterface");
	wcscpy(m_SoapNamespace, L"http://192.168.80.13:7001/services/PlaylistSoapInterface");
	m_SoapTimeout = DEFAULT_TIMEOUT;
}

BOOL CStreamSession::Initialize()
{
	glog(Log::L_DEBUG, _T("Enter CStreamSession::Initialize()"));
	m_ItvStatus.SetBlockUserParam((DWORD)this);

	// Initialize the ContralTable
	m_StreamOptb.GeneralInit();

	// initialize the ISS
	glog(Log::L_DEBUG, _T("IssInit with parameter: TypeInst.type %x, TypeInst.Inst %d, dwAppUid %x, CallBackMask %d, StateChangeMask %d"),
		m_TypeInst.s.dwType, m_TypeInst.s.dwInst, m_dwAppUid, m_CbCallBackMask, m_StateChangeMask);

	ITVSTATUS status = IssInit(	m_TypeInst,
								m_ItvStatus.GetItvVersion(),
								m_dwAppUid,
								m_bSubSet,
								m_StreamOptb.GetOpCtrlTable(),
								m_RdyAssetOpCtl,
								m_CbCallBackMask,
								GetCallBackRoutine(),
								m_StateChangeMask,  
								m_ItvStatus.GetStatusBlock(),
								NULL
							  );

	
	if (status != ITV_SUCCESS)
	{
		glog(Log::L_ERROR, _T("CStreamSession::Initialize() - IssInit() Failed with Status = %x"), status);
		return FALSE;
	}

	glog(Log::L_DEBUG, _T("CStreamSession::Initialize() successfully"));
	return TRUE;
}

BOOL CStreamSession::UnInitialize()
{
	glog(Log::L_DEBUG, ("Enter CStreamSession::UnInitialize()"));
	ITVSTATUS sts = IssTerm(m_TypeInst, 
						    m_ItvStatus.GetStatusBlock(), 
						    NULL
					        );
	if(ITV_SUCCESS == sts) {
		glog(Log::L_DEBUG, ("CStreamSession::UnInitialize() - IssTerm() Succeed"));
	}
	else {
		glog(Log::L_DEBUG, ("CStreamSession::UnInitialize() - IssTerm() Failed with Status = %x"),sts);
	}
	
	return (ITV_SUCCESS == sts); 
}

// CallBack Routine Pointer
LPCOMPLROUTINE CStreamSession::GetComplRoutine()
{
	return IssComplCallBack;
}

LPSSCALLBACKROUTINE CStreamSession::GetCallBackRoutine()
{
	return IssCallBackDispather;
}

void CStreamSession::IssComplCallBack(LPITVSTATUSBLOCK pItvSb)
{
	glog(Log::L_DEBUG, _T("Enter CStreamSession::IssComplCallBack"));
	if (ITV_SUCCESS != pItvSb->Status && ITV_PENDING != pItvSb->Status)
	{
		// asynchronous invoking failed
		glog(Log::L_DEBUG, _T("CStreamSession::IssComplCallBack ERROR, General Status = %x, Extend Status = %x"), 
				pItvSb->Status, pItvSb->dwExtendedStatus);
		return;
	}
}

BOOL CStreamSession::IssCallBackDispather(TYPEINST TypeInst, STREAMID Sid, 
										  CALLBACKTYPE CbType, LPCBKBLOCK	pCbBlk)
{
	switch(CbType) 
	{
	case CB_NOCALLBACKS: // no callbacks
		break;

	case CB_NEWSTREAM:
		glog(Log::L_DEBUG, _T("[ISS]SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_NEWSTREAM");
		return OnNewStream(Sid, &pCbBlk->NS);

	case CB_AEXITION:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_AEXITION");
		return OnAETransition(Sid, &pCbBlk->AX);

	case CB_LOADCHG:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_LOADCHG");
		return OnLoadChanged(Sid);

	case CB_STATECHG:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_STATECHG");
		return OnStateChanged(Sid, &pCbBlk->SC);

	case CB_RDYASSET:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_RDYASSET");
		return OnAssetReady(Sid, &pCbBlk->RA.AEList);

	case CB_REQUESTDATA:
		break;

	case CB_STRMOP:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_STRMOP");
		switch(pCbBlk->SO.Op) 
		{
		case SOP_NOOP:	// no operation
			break;
		case SOP_PRIME:	// Prime the videostore
			return OnPrimeVideoStore(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_PLAY:	// Start streaming the ActiveList
			return OnPlayStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_FF:	// Fast Forward
			return OnFFStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_REW:	// Rewind
			return OnRewStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_PAUSE:	// Pause
			return OnPauseStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_RESUME:// Resume
			return OnResumeStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		case SOP_STOP:	// Terminate streaming (but not the stream)
			return OnStopStream(Sid, pCbBlk->SO.dwOpSeqnum, pCbBlk->SO.ExpectedAction);
		default:
			;
		}
		break;
	default:
		;
	}
	return TRUE;
}

PAELEMENT CStreamSession::CopyNewAelements(const PAELIST pNewAEList, long& lNewAECount)
{
	PAELEMENT pCpyAelements = NULL;
	lNewAECount = 0;
	if(pNewAEList != NULL && (lNewAECount=pNewAEList->AECount) != 0)
	{
		pCpyAelements = new AELEMENT[lNewAECount];
		memcpy(pCpyAelements, pNewAEList->AELlist, lNewAECount*sizeof(AELEMENT));

		return pCpyAelements;
	}
	return NULL;
}

BOOL CStreamSession::OnNewStream(STREAMID sid, LPNEWSTREAM pNewStream)
{
	DWORD  dwBillingId    = 0;
	time_t tPurchaseTime  = 0;
	DWORD  dwPlayTimeLeft = 0;
	DWORD  dwHomeId       = 0;
	DWORD  dwSmartCardId  = 0;
	DWORD  dwPurchaseId   = 0;
	DWORD  dwAnalogCopyPurchase   = 0;
	DWORD  dwPackageId = 0;
	DWORD  dwContextId = 0;
	
	BoxMacAddr DeviceId;
	memset(&DeviceId, 0, sizeof(BoxMacAddr));
	
	BYTE   nVer;
	BYTE   nCnt = 0;
	BYTE   nTag;
	BYTE   nLth;
	BYTE   *pPtr;
	WORD   i;

	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t SS -> AS: OnNewStream"), sid.dwStreamIdNumber);

	BOOL bRet = TRUE;
	switch(pNewStream->CbNewStreamType)
	{
	case START:
		{
			//////////////////////////////////////////////////////////////////////////
			// Fetch Set-top mac
			for(int macIndex=0; macIndex<6; macIndex++) {
				DeviceId.byteMac[macIndex] = pNewStream->ClientId.settop[13+macIndex];
			}
			
			//////////////////////////////////////////////////////////////////////////
			// Fetch user param
			if(pNewStream->pUserParam == NULL)
			{
				glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Cannot get any UserParam, cosz pUserParam == NULL"), sid.dwStreamIdNumber);
			}
			else
			{
				USERPARAM* pUserData = pNewStream->pUserParam;

				if ( pUserData->wType != MOD_APP_DATA_TYPE || pUserData->wLength <= 0 )
				{
					glog(Log::L_DEBUG, L"CStreamSession::OnNewStream - pUserParam is invalid: Type=%d,Length=%d. stream will be terminated by force",
							 pUserData->wType, pUserData->wLength);

					// terminate stream by force
					if(m_streamDatas.Remove(sid))
					{
						StreamOpCtrl strmOpCtrl(m_instance, sid);
						strmOpCtrl.TerminateStream();
					}

					return TRUE;
				}

				// We have data so check the type, length, and version of data
				MODAPPDATA* pAppData = (MODAPPDATA *) (((BYTE*)pUserData) + sizeof(USERPARAM));

				switch ( pAppData->byteVersion )
				{
					 case MOD_APP_DATA_VERSION_1:

						 if ( pUserData->wLength != 5 && pUserData->wLength != 9 )
							{
							 glog(Log::L_DEBUG, L"CStreamSession::OnNewStream - pUserData is invalid: Version=%d,Length=%d",
									 pAppData->byteVersion, pUserData->wLength);
							 return ( ITV_BAD_PARAM );
							}

						 dwBillingId = ntohl ( pAppData->v1.dwBillingId );

						 if ( pUserData->wLength == 9 )
							{
							 // A non-zero purchase time indicates client is resuming a suspended movie
							 tPurchaseTime = ntohl ( pAppData->v1.tPurchaseTime );
							}
						 break;

					 case MOD_APP_DATA_VERSION_2:

						 nVer = pAppData->v2.byteVersion;
						 nCnt = pAppData->v2.byteDescriptorCnt;
						 pPtr = &pAppData->v2.byteDescriptorCnt;
						 nLth = 1;

						 for (i = 0; i < nCnt; i++)
							 {
							  pPtr += nLth; // Advance pointer to Tag
							  nTag = *pPtr;
							  pPtr++;       // Advance pointer to Length
							  nLth = *pPtr;
							  pPtr++;       // Advance pointer to Value
							  void *pVal = pPtr;

							  if (nTag == SSP_SC_BILLINGID      && nLth == sizeof(DWORD))
								  dwBillingId = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_PURCHASETIME   && nLth == sizeof(DWORD))
								  tPurchaseTime = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_TIMEREMAINING  && nLth == sizeof(DWORD))
								  dwPlayTimeLeft = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_HOMEID         && nLth == sizeof(DWORD))
								  dwHomeId = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_PURCHASEID     && nLth == sizeof(DWORD))
								  dwPurchaseId = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_SMARTCARDID    && nLth == sizeof(DWORD))
								  dwSmartCardId = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_SIGANALOGCOPYPURCHASE    && nLth == sizeof(DWORD))
								  dwAnalogCopyPurchase = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_PACKAGEID    && nLth == sizeof(DWORD))
								  dwPackageId = ntohl ( *(DWORD *)pVal );
							  else
							  if (nTag == SSP_SC_CONTEXTID    && nLth == sizeof(DWORD))
								  dwContextId = ntohl ( *(DWORD *)pVal );
							  else
								  glog(Log::L_DEBUG, L"CStreamSession::OnNewStream - pUserData:Tag=%d,Lth=%d ignored", nTag, nLth );
							 }
						 break;

					 default:
						 glog(Log::L_DEBUG, L"CStreamSession::OnNewStream - pAppData:Version=%d(Invalid Version), stream will be terminated by force", 
							 pAppData->byteVersion);

						// terminate stream by force
						 if(m_streamDatas.Remove(sid))
						{
							StreamOpCtrl strmOpCtrl(m_instance, sid);
							strmOpCtrl.TerminateStream();
						 }
						
						return TRUE;
				 }

			}

			glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t SS -> AS: StreamID = %.8x, HomeID = %.10x, DeviceID = %.2x%.2x%.2x%.2x%.2x%.2x, PurchaseID = %.10x")
				, sid.dwStreamIdNumber
				, sid.dwStreamIdNumber
				, dwHomeId
				, pNewStream->ClientId.settop[13], pNewStream->ClientId.settop[14], pNewStream->ClientId.settop[15], pNewStream->ClientId.settop[16], pNewStream->ClientId.settop[17], pNewStream->ClientId.settop[18]
				, dwPurchaseId);
						
			glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AS -> SS: Began AcceptStream Operation"), sid.dwStreamIdNumber);
			
			assert(m_instance != NULL);

			StreamOpCtrl strmOpCtrl(m_instance, sid);
			bRet = strmOpCtrl.AcceptStream();
			if(bRet)
			{
//	//////////////////////////////////////////////////////////////////////////
//	// this block is for test only, hardcode some parameters
//	// test begin:
//	dwHomeId = 268499533;
//	
//	DeviceId.byteMac[0] = 0x54; 
//	DeviceId.byteMac[1] = 0x0B;
//	DeviceId.byteMac[2] = 0xD6;
//	DeviceId.byteMac[3] = 0x44;
//	DeviceId.byteMac[4] = 0x09;
//	DeviceId.byteMac[5] = 0xFA;
//	
//	dwPurchaseId = 120;
//	// test end:
//	//////////////////////////////////////////////////////////////////////////

				// add the current newstream data to the list.
				CStreamData newStreamData(dwHomeId, DeviceId, dwPurchaseId, sid, pNewStream->dwAssetId);
				m_streamDatas.Add(newStreamData);

				glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AcceptStream Operation SUCCEED"), sid.dwStreamIdNumber);
			}
			else
				glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AcceptStream Operation FAILED"), sid.dwStreamIdNumber);

		}
		break;
		
	case RECONNECT:
	case ATTACH:
		{
//			if(pNewStream->TypeParam.RecoveredStatus.CurrentState == ISS_STATE_CM_TERMINATED)
//			{
				glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t RECONNECTED or ATTACH: remove stream"), sid.dwStreamIdNumber);
				StreamOpCtrl strmOpCtrl(m_instance, sid);
				strmOpCtrl.TerminateStream();
//			}
		}
		break;
	}

	return TRUE;
}
	

BOOL CStreamSession::OnAssetReady(STREAMID sid, PAELIST pOriginalAEList, PAELIST pNewAes)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t SS -> AS: OnAssetReady"), sid.dwStreamIdNumber);

	// log the original AEList
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Original AEList returned by Ready Asset, AssetUID = %x, AECount = %d"), 
			sid.dwStreamIdNumber, pOriginalAEList->AssetUID, pOriginalAEList->AECount);

	PAEARRAY pOrgAEs = pOriginalAEList->AELlist;
	for(int i=0; i<pOriginalAEList->AECount; i++)
	{
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t NO %d Element: AEUID = %x"), 
			sid.dwStreamIdNumber, i+1, (*pOrgAEs)[i].AEUID);
	}
		
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AS -> SS: Began AcceptReadyAsset Operation"), sid.dwStreamIdNumber);

	AELIST NewAEList; 
	char buffHomeID[16], buffDeviceID[20], buffPurchaseID[16], buffStreamID[16], buffErrorcode[16];
	long statusRet;
	
	DWORD dwHomeID = m_streamDatas.GetHomeID(sid);
	BoxMacAddr DeviceID = m_streamDatas.GetDeviceID(sid);
	DWORD dwPurchaseID = m_streamDatas.GetPurchaseID(sid);

	sprintf(buffHomeID,		"0x%010X", dwHomeID);
	sprintf(buffDeviceID,	"%02X:%02X:%02X:%02X:%02X:%02X", DeviceID.byteMac[0],DeviceID.byteMac[1],DeviceID.byteMac[2],DeviceID.byteMac[3],DeviceID.byteMac[4],DeviceID.byteMac[5]);
	sprintf(buffPurchaseID, "0x%010X", dwPurchaseID);
	sprintf(buffStreamID,	"0x%08X", sid.dwStreamIdNumber);

	// call SOAP client to get AEList
	PlaylistSoapProxy tmpSoapProxy(m_SoapWSDLFilePath);
	tmpSoapProxy.setTimeout(m_SoapTimeout);

	try
	{
		NewAEList = tmpSoapProxy.OnSetupSoapCall(buffHomeID, buffDeviceID, buffPurchaseID, buffStreamID);
	}
	catch(...)
	{
		glog(Log::L_ERROR, _T("Error when SETUP: Unknown error"));
		if(m_streamDatas.Remove(sid))
		{
			StreamOpCtrl strmOpCtrl(m_instance, sid);
			strmOpCtrl.TerminateStream();
		}
		return TRUE;
	}
	
	// check if get elements; if 0, terminate the stream
	if(NewAEList.AECount==0) {
		glog(Log::L_WARNING, _T("[ISS] SID=%x:\t AS -> SS : AcceptReadyAsset Operation got no element via SOAP"), sid.dwStreamIdNumber);
		if(m_streamDatas.Remove(sid))
		{
			StreamOpCtrl strmOpCtrl(m_instance, sid);
			strmOpCtrl.TerminateStream();
		}
		return TRUE;
	}

	PAELEMENT pCpyAEs = (PAELEMENT)NewAEList.AELlist;
	long lNewAECount = NewAEList.AECount;
	if(pCpyAEs != NULL)
	{
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t NEW AEList Count = %d"), sid.dwStreamIdNumber, lNewAECount);

		pOriginalAEList->AECount = lNewAECount;
		pOriginalAEList->AELlist = (PAEARRAY)pCpyAEs;

		// clear the AEIDs from array
		m_streamDatas.ClearAEIDs(sid);
		for(int i=0; i<lNewAECount; i++)
		{
			// add the AEID to the array
			m_streamDatas.AddAEID(sid, pCpyAEs[i].AEUID);

			glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t New NO %d Element: AEUID = %x"), sid.dwStreamIdNumber, i+1,pCpyAEs[i].AEUID);
		}
	}

	assert(m_instance != NULL);

	StreamOpCtrl strmOpCtrl(m_instance, sid);
	ITVSTATUS bRet = strmOpCtrl.AcceptReadyAsset(pOriginalAEList);

	if(bRet == ITV_SUCCESS)
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AS -> SS: AcceptReadyAsset Operation Succeed"), sid.dwStreamIdNumber);
	else {
		if(m_streamDatas.Remove(sid))
		{
			sprintf(buffErrorcode,	"0x%08x", bRet);

			// call SOAP client to notify Stream can not start
			
			try
			{
				statusRet = tmpSoapProxy.OnTeardownSoapCall(buffHomeID, buffDeviceID, buffPurchaseID, buffStreamID, buffErrorcode);
			}
			catch(...)
			{
				glog(Log::L_ERROR, _T("Error when TEARDOWN: Unknown error"));
			}

			StreamOpCtrl termCtrl(m_instance, sid);
			termCtrl.TerminateStream();
		}
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AS -> SS : AcceptReadyAsset Operation Failed"), sid.dwStreamIdNumber);
	}

	if(NewAEList.AELlist)
	{
		delete[] (PAEARRAY)NewAEList.AELlist;
		NewAEList.AELlist = NULL;
	}

	return TRUE;
}

BOOL CStreamSession::OnAETransition(STREAMID sid, LPAEXITION pAEXition)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t SS -> AS: OnAETransition"), sid.dwStreamIdNumber);
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t AE Transiton: PreAEUID = %x, NextAEUID = %x "), 
		sid.dwStreamIdNumber, pAEXition->dwPrevAE, pAEXition->dwNewAE);

	SSAENotification stopNoti, playNoti;
	// save the previous AEUID and new AEUID and generate the notification data
	int nRet = m_streamDatas.GenSSAENotification(sid, pAEXition->dwPrevAE, pAEXition->dwNewAE, 
												pAEXition->dwAeXSeqNum, stopNoti, playNoti);
//	int nRet = -1;
	// pass the notification message to the MainCtrl
	switch(nRet)
	{
	case NOTITYPE_NONE: // Error - no notification
		break;
	case NOTITYPE_PLAY: // only play notification
		break;
	case NOTITYPE_PS:   // stop and play notification
		break;
	case NOTITYPE_STOP: // only stop notification
		break;
	default:
		break;
	}
	return TRUE;
}

BOOL CStreamSession::OnStateChanged(STREAMID sid, LPSTATECHG stateChg)
{
	char buffHomeID[16], buffDeviceID[20], buffPurchaseID[16], buffStreamID[16], buffErrorcode[16];
	DWORD dwHomeID;
	BoxMacAddr DeviceID;
	DWORD dwPurchaseID;
	long statusRet;
	
	// soap client
	PlaylistSoapProxy tmpSoapProxy(m_SoapWSDLFilePath);
	tmpSoapProxy.setTimeout(m_SoapTimeout);
	
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t SS -> AS: OnStateChanged"), sid.dwStreamIdNumber);
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t StateChg Type: %d  State: 0x%08x"), sid.dwStreamIdNumber, stateChg->Chg, stateChg->Sts);
	switch(stateChg->Chg)
	{
	case DNETRDY:	
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"DNETRDY");
		break;
	case STRMSTART: 
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMSTART");
		break;
	case STRMEND:	// Change AEList succeed
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMEND");
		break;
	case STRMABORT:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMABORT");
		break;
	case STRMTERM:  // the stream was unavailable, so remove it from the list
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMTERM");
		
		dwHomeID = m_streamDatas.GetHomeID(sid);
		DeviceID = m_streamDatas.GetDeviceID(sid);
		dwPurchaseID = m_streamDatas.GetPurchaseID(sid);
		statusRet;
		
		if(m_streamDatas.Remove(sid))
		{
			// call soap
			sprintf(buffHomeID,		"0x%010X", dwHomeID);
			sprintf(buffDeviceID,	"%02X:%02X:%02X:%02X:%02X:%02X", DeviceID.byteMac[0],DeviceID.byteMac[1],DeviceID.byteMac[2],DeviceID.byteMac[3],DeviceID.byteMac[4],DeviceID.byteMac[5]);
			sprintf(buffPurchaseID, "0x%010X", dwPurchaseID);
			sprintf(buffStreamID,	"0x%08X", sid.dwStreamIdNumber);
			sprintf(buffErrorcode,	"0x%08x", stateChg->Sts);

			// call SOAP client to notify TEARDOWN
			
			try
			{
				statusRet = tmpSoapProxy.OnTeardownSoapCall(buffHomeID, buffDeviceID, buffPurchaseID, buffStreamID, buffErrorcode);
			}
			catch(...)
			{
				glog(Log::L_ERROR, _T("Error when TEARDOWN: Unknown error"));
			}

			// Terminate the stream;
			glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), TerminateStream After STEAMTERM"), 
				sid.dwStreamIdNumber);
			StreamOpCtrl strmOpCtrl(m_instance, sid);
			strmOpCtrl.TerminateStream();
		}
		break;
	case STRMPLAYING:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMPLAYING");
		
		// call soap
		dwHomeID = m_streamDatas.GetHomeID(sid);
		DeviceID = m_streamDatas.GetDeviceID(sid);
		dwPurchaseID = m_streamDatas.GetPurchaseID(sid);
		
		sprintf(buffHomeID,		"0x%010X", dwHomeID);
		sprintf(buffDeviceID,	"%02X:%02X:%02X:%02X:%02X:%02X", DeviceID.byteMac[0],DeviceID.byteMac[1],DeviceID.byteMac[2],DeviceID.byteMac[3],DeviceID.byteMac[4],DeviceID.byteMac[5]);
		sprintf(buffPurchaseID, "0x%010X", dwPurchaseID);
		sprintf(buffStreamID,	"0x%08X", sid.dwStreamIdNumber);

		if(m_streamDatas.GetIsFirstPlay(sid))
		{	// is the first time PLAY message got
			// call SOAP client to notify PLAY
			try
			{
				statusRet = tmpSoapProxy.OnPlaySoapCall(buffHomeID, buffDeviceID, buffPurchaseID, buffStreamID);
			}
			catch(...)
			{
				glog(Log::L_ERROR, _T("Error when PLAY: Unknown error"));
			}

			m_streamDatas.SetIsFirstPlay(sid, false);
		}
		break;

	default:
		glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d"), 
			sid.dwStreamIdNumber, stateChg->Chg);
		break;
	}
	return TRUE;
}

BOOL CStreamSession::OnLoadChanged(STREAMID sid)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t OnLoadChanged"), sid.dwStreamIdNumber);
	return TRUE;
}

BOOL CStreamSession::OnPrimeVideoStore(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnPrimeVideoStore"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);
		
		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnPlayStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnPlayStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnFFStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnFFStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnRewStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnRewStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnPauseStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnPauseStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnResumeStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnResumeStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnStopStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("[ISS] SID=%x:\t Enter OnStopStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(!strmOpCtrl.AcceptOperation(dwOpSeqnum))
		{ // TODO: log error
		}
	}
	return TRUE;
}

#pragma comment (lib, "IssApi" VODLIBEXT)
#pragma comment (lib, "Locks" VODLIBEXT)
#pragma comment (lib, "MCastSvc" VODLIBEXT)
#pragma comment (lib, "ManPkgU" VODLIBEXT)
#pragma comment (lib, "MtTcpComm" VODLIBEXT)
#pragma comment (lib, "Reporter" VODLIBEXT)
#pragma comment (lib, "atomic_queues" VODLIBEXT)
#pragma comment (lib, "cfgpkgU" VODLIBEXT)
#pragma comment (lib, "CLog" VODLIBEXT)
#pragma comment (lib, "ScThreadPool" VODLIBEXT)
#pragma comment (lib, "idsapi" VODLIBEXT)
#pragma comment (lib, "queue" VODLIBEXT)
#pragma comment (lib, "psapi.lib")
#pragma comment (lib, "Ws2_32.lib")
