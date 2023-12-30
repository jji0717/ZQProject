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
// $Log: /ZQProjs/ScheduledTV_new/ISSSTREAMCTRL/StreamSession.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     06-04-30 17:46 Bernie.zhao
// 
// 1     05-08-30 18:29 Bernie.zhao
// 
// 23    05-03-24 14:53 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 22    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// ===========================================================================

#include "../Common/Log.h"
using namespace ZQ::common;

#include <assert.h>
#include "Mod.h"
#include "StreamSession.h"

#include "StreamData.h"

#include "../MainCtrl/ScheduleTV.h"

CStreamSession* CStreamSession::m_instance = NULL;
CStreamDataManager CStreamSession::m_streamDatas;

extern ScheduleTV gSTV;
DWORD g_AppUID;

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
	if(m_instance != NULL)
		delete m_instance;
	m_instance = NULL;
}

CStreamSession::CStreamSession(TYPEINST typeInst,  DWORD dwAppUID) 
		: m_TypeInst(typeInst)

{
	m_dwAppUid = dwAppUID;
	g_AppUID = dwAppUID;

	m_CbCallBackMask = CB_NEWSTREAM | CB_STATECHG | CB_RDYASSET | CB_AEXITION | CB_STRMOP;
	m_bSubSet = TRUE;
	m_RdyAssetOpCtl = SOPCTL_APPROVE;
	m_StateChangeMask = DNETRDY       |
				        STRMSTART     |
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
}

BOOL CStreamSession::Initialize(const wchar_t* wchXmlPath)
{
	glog(Log::L_DEBUG, _T("(ISS) - Enter CStreamSession::Initialize()"));
	m_ItvStatus.SetBlockUserParam((DWORD)this);

	// load StreamData from XML
	m_streamDatas.LoadStreamDoc(wchXmlPath);

	// Initialize the ContralTable
	m_StreamOptb.GeneralInit();

	// initialize the ISS
	glog(Log::L_INFO, _T("(ISS) - IssInit with parameter: TypeInst.type %x, TypeInst.Inst %d, dwAppUid %d, CallBackMask %d, StateChangeMask %d"),
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
								NULL //GetComplRoutine()
							  );

	
	if (ITV_SUCCESS != status)
	{
		glog(Log::L_ERROR, _T("(ISS) - CStreamSession::Initialize() Fail! Status = %x"), status);
		return FALSE;
	}

	glog(Log::L_INFO, _T("(ISS) - CStreamSession::Initialize() successfully"));
	return TRUE;
}

BOOL CStreamSession::UnInitialize()
{
	glog(Log::L_DEBUG, ("(ISS) - Enter CStreamSession::UnInitialize()"));
	ITVSTATUS sts = IssTerm(m_TypeInst, 
						    m_ItvStatus.GetStatusBlock(), 
						    NULL //GetComplRoutine()
					        );
	if(ITV_SUCCESS == sts)
		glog(Log::L_INFO, ("(ISS) - CStreamSession::UnInitialize() Succeed"));

	// delete the streamdata xml file
	m_streamDatas.DeleteStreamDoc();

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
	glog(Log::L_DEBUG, _T("(ISS) - Enter CStreamSession::IssComplCallBack"));
	if (ITV_SUCCESS != pItvSb->Status && ITV_PENDING != pItvSb->Status)
	{
		// asynchronous invoking failed
		glog(Log::L_DEBUG, _T("(ISS) - CStreamSession::IssComplCallBack ERROR, General Status = %x, Extend Status = %x"), 
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
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_NEWSTREAM");
		return OnNewStream(Sid, &pCbBlk->NS);

	case CB_AEXITION:
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_AEXITION");
		return OnAETransition(Sid, &pCbBlk->AX);

	case CB_LOADCHG:
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_LOADCHG");
		return OnLoadChanged(Sid);

	case CB_STATECHG:
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_STATECHG");
		return OnStateChanged(Sid, &pCbBlk->SC);

	case CB_RDYASSET:
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
			Sid.dwStreamIdNumber, CbType, L"CB_RDYASSET");
		return OnAssetReady(Sid, &pCbBlk->RA.AEList);

	case CB_REQUESTDATA:
		break;

	case CB_STRMOP:
		glog(Log::L_INFO, _T("(ISS) - [SID=%X]  Enter CStreamSession::IssCallBackDispather, CALLBACK --- TYPE:%d(%s)"), 
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
	BYTE   nVer;
	BYTE   nCnt = 0;
	BYTE   nTag;
	BYTE   nLth;
	BYTE   *pPtr;
	WORD   i;

	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  SS -> AS: AssetId = %x, PurchaseID = %d, NodeGroupID = %d, NewStreamType = %d, CurrentState = %d"), 
		sid.dwStreamIdNumber, pNewStream->dwAssetId, pNewStream->PurchaseID, pNewStream->NodeGroupID, pNewStream->CbNewStreamType, 
		pNewStream->TypeParam.RecoveredStatus.CurrentState);

	BOOL bRet = TRUE;
	switch(pNewStream->CbNewStreamType)
	{
	case START:
		{
			if(pNewStream->pUserParam == NULL)
			{
				glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Cannot get any UserParam, cosz pUserParam == NULL"), sid.dwStreamIdNumber);
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

						 glog(Log::L_WARNING, L"CStreamSession::OnNewStream - pUserData is invalid: Version=%d,Length=%d",
							 pAppData->byteVersion, pUserData->wLength);
						 
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

/*
			get device id from settop info, and use this device id to pass the purchase id
			DWORD dwPurchaseID = 0;

			WORD wLow, wHigh;
			wLow  = MAKEWORD(pNewStream->ClientId.settop[18], pNewStream->ClientId.settop[17]);
			wHigh = MAKEWORD(pNewStream->ClientId.settop[16], pNewStream->ClientId.settop[15]);
			
			dwPurchaseID = MAKELONG(wLow, wHigh);
			// printf("PurchaseID = %d\n", dwPurchaseID);
*/
			
//			dwPurchaseId = dwHomeId;	// coz we are now using home-id instead of purchaseID
			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  PURCHASEID=%ld"), sid.dwStreamIdNumber, dwPurchaseId);

			
			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AS -> SS: Began AcceptStream Operation"), sid.dwStreamIdNumber);
			
			assert(m_instance != NULL);

			StreamOpCtrl strmOpCtrl(m_instance, sid);
			bRet = strmOpCtrl.AcceptStream(m_instance);
			if(bRet)
			{
				// add the current newstream data to the list.
				CStreamData newStreamData(dwPurchaseId, sid, pNewStream->dwAssetId);
				
				m_streamDatas.Add(newStreamData);

				glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AcceptStream Operation SUCCEED"), sid.dwStreamIdNumber);
			}
			else
				glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AcceptStream Operation FAILED"), sid.dwStreamIdNumber);

		}
		break;
		
	case RECONNECT:
	case ATTACH:
		{
//			if(pNewStream->TypeParam.RecoveredStatus.CurrentState == ISS_STATE_CM_TERMINATED)
//			{
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
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  SS -> AS: OnAssetReady"), sid.dwStreamIdNumber);

	// log the original AEList
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Original AEList returned by Ready Asset, AssetUID = %x, AECount = %d"), 
			sid.dwStreamIdNumber, pOriginalAEList->AssetUID, pOriginalAEList->AECount);

	PAEARRAY pOrgAEs = pOriginalAEList->AELlist;
	for(int i=0; i<pOriginalAEList->AECount; i++)
	{
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  NO %d Element: AEUID = %x"), 
			sid.dwStreamIdNumber, i+1, (*pOrgAEs)[i].AEUID);
	}
		
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AS -> SS: Began AcceptReadyAsset Operation"), sid.dwStreamIdNumber);

	AELIST NewAEList;
	DWORD dwPurchaseID=m_streamDatas.GetPurchaseID(sid);

	if(STVSUCCESS!=gSTV.OnGetAElist(dwPurchaseID, NewAEList) )
	{
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Cannot get Asset Elements according to PurchaseID = %d, stream will be broadcasting dummy asset"), sid.dwStreamIdNumber, dwPurchaseID);

//		// terminate stream by force, set aelist to 0
//		pOriginalAEList->AELlist = NULL;
	}
	else 
	{
	
		PAELEMENT pCpyAEs = (PAELEMENT)NewAEList.AELlist;
		long lNewAECount = NewAEList.AECount;
		if(pCpyAEs != NULL)
		{
			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  NEW AEList Count = %d"), sid.dwStreamIdNumber, lNewAECount);

			pOriginalAEList->AECount = lNewAECount;
			pOriginalAEList->AELlist = (PAEARRAY)pCpyAEs;

			for(int i=0; i<lNewAECount; i++)
			{
				glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  NEW Asset Element ID = %x"), sid.dwStreamIdNumber, pCpyAEs[i].AEUID);
			}
		}

		assert(m_instance != NULL);
	}

	StreamOpCtrl strmOpCtrl(m_instance, sid);
	BOOL bRet = strmOpCtrl.AcceptReadyAsset(pOriginalAEList);

	if(bRet)
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AS -> SS: AcceptReadyAsset Operation Succeed"), sid.dwStreamIdNumber);
	else {
		if(m_streamDatas.Remove(sid))
		{
			StreamOpCtrl termCtrl(m_instance, sid);
			termCtrl.TerminateStream();
			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AS -> SS : AcceptReadyAsset Operation Failed"), sid.dwStreamIdNumber);
		}
	}

	gSTV.OnFreeAElist(dwPurchaseID);
	
	return TRUE;
}

BOOL CStreamSession::OnAETransition(STREAMID sid, LPAEXITION pAEXition)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  SS -> AS: OnAETransition"), sid.dwStreamIdNumber);
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  AE Transiton: PreAEUID = %x, NextAEUID = %x, Sequence = %d "), 
		sid.dwStreamIdNumber, pAEXition->dwPrevAE, pAEXition->dwNewAE, pAEXition->dwAeXSeqNum);

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
		gSTV.OnSendAEStatus(playNoti.dwPurchaseID, playNoti);
		break;
	case NOTITYPE_PS:   // stop and play notification
		gSTV.OnSendAEStatus(stopNoti.dwPurchaseID, stopNoti);  // the previous AE stopped.
		gSTV.OnSendAEStatus(playNoti.dwPurchaseID, playNoti);  // the new AE started.
		break;
	case NOTITYPE_STOP: // only stop notification
//		gSTV.OnSendAEStatus(stopNoti.dwPurchaseID, stopNoti);
		break;
	default:
		break;
	}
	return TRUE;
}

BOOL CStreamSession::OnStateChanged(STREAMID sid, LPSTATECHG stateChg)
{
	SSAENotification abortNoti;
	DWORD dwPurchaseID = 0;
	
	switch(stateChg->Chg)
	{
	case DNETRDY:	
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"DNETRDY");
		break;
	case STRMSTART: 
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMSTART");
		break;
	case STRMEND:	// Change AEList succeed
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMEND");
		break;
	case STRMABORT:
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMABORT");

		// pass the notification message to the MainCtrl if necessary
//		abortNoti.dwPurchaseID = m_streamDatas.GetPurchaseID(sid);
//		abortNoti.dwAeUID = UNKNOWN_UID;
//		abortNoti.wOperation = SAENO_ABORT;
//		abortNoti.dwStatus = SAENS_SUCCEED;
//		gSTV.OnSendAEStatus(abortNoti.dwPurchaseID, abortNoti);
		
//		if(m_streamDatas.Remove(sid))
//		{
//			// Terminate the stream;
//			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), TerminateStream After STRMABORT"), 
//				sid.dwStreamIdNumber);
//			StreamOpCtrl strmOpCtrl(m_instance, sid);
//			strmOpCtrl.TerminateStream();
//		}
		
		break;
	case STRMTERM:  // the stream was unavailable, so remove it from the list
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d(%s)"), 
			sid.dwStreamIdNumber, stateChg->Chg, L"STRMTERM");

		// pass the notification message to the MainCtrl if necessary
		abortNoti.dwPurchaseID = m_streamDatas.GetPurchaseID(sid);
		abortNoti.dwAeUID = UNKNOWN_UID;
		abortNoti.wOperation = SAENO_STOP;
		abortNoti.dwStatus = SAENS_SUCCEED;
		
		if(m_streamDatas.Remove(sid))
		{
//			gSTV.OnSendAEStatus(abortNoti.dwPurchaseID, abortNoti);
			
			// Terminate the stream;
			glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), TerminateStream After STEAMTERM"), 
				sid.dwStreamIdNumber);
			StreamOpCtrl strmOpCtrl(m_instance, sid);
			strmOpCtrl.TerminateStream();
		}
		
		break;
	case STRMPLAYING:
		break;
	default:
		glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  CStreamSession::OnStateChanged(), LPSTATECHG::STATECHGTYPE = %d"), 
			sid.dwStreamIdNumber, stateChg->Chg);
		break;
	}
	return TRUE;
}

BOOL CStreamSession::OnLoadChanged(STREAMID sid)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  OnLoadChanged"), sid.dwStreamIdNumber);
	return TRUE;
}

BOOL CStreamSession::OnPrimeVideoStore(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnPrimeVideoStore"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);
		
		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnPlayStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnPlayStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnFFStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnFFStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnRewStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnRewStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnPauseStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnPauseStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnResumeStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnResumeStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
		}
	}
	return TRUE;
}

BOOL CStreamSession::OnStopStream(STREAMID sid, DWORD dwOpSeqnum, OPCTL ExpectedAction)
{
	glog(Log::L_DEBUG, _T("(ISS) - [SID=%X]  Enter OnStopStream"), sid.dwStreamIdNumber);
	if (SOPCTL_APPROVE == ExpectedAction)
	{
		assert(m_instance != NULL);

		StreamOpCtrl strmOpCtrl(m_instance, sid);
		if(! strmOpCtrl.AcceptOperation(dwOpSeqnum)) {
			// TODO: log error
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
