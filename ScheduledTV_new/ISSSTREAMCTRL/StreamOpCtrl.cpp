#include "Log.h"

#include "StreamOpCtrl.h"
#include "StreamSession.h"

/** StreamOpCtrlTable class.
 *
 *  the stream operation control table
 */

StreamOpCtrlTable::StreamOpCtrlTable()
{
	// reset the table with value pair <SOP_NOOP, SOPCTL_NOP>
	for (DWORD i = 0; i < SOP_LAST; i++)
	{
		m_OpCtrlTable[i].StreamOperator	= SOP_NOOP;
		m_OpCtrlTable[i].OpCtlValue		= SOPCTL_NOP;
	}
}

StreamOpCtrlTable::StreamOpCtrlTable(const StreamOpCtrlTable& OpCtlTbl)
{
	// the copier
	memcpy(m_OpCtrlTable, OpCtlTbl.m_OpCtrlTable, sizeof(m_OpCtrlTable));
}

void  StreamOpCtrlTable::GeneralInit(void)
{
	SetStreamPlayCtrl(SOPCTL_ALLOW);
	SetStreamFFCtrl(SOPCTL_APPROVE);
	SetStreamRewCtrl(SOPCTL_APPROVE);
	SetStreamPauseCtrl(SOPCTL_APPROVE);
	SetStreamResumeCtrl(SOPCTL_APPROVE);
	SetStreamStopCtrl(SOPCTL_APPROVE);
}

const OPCTL StreamOpCtrlTable::GetOpCtrl(STREAMOP operation)
{
	if (operation <= SOP_NOOP || operation >= SOP_LAST)
		return SOPCTL_NOP; // out of range
	
	// scan the current table to locate
	DWORD i = 0;
	for (; i < SOP_LAST && m_OpCtrlTable[i].StreamOperator != SOP_NOOP; i++)
	{
		if (m_OpCtrlTable[i].StreamOperator == operation)
			break; 
	}
	
	return m_OpCtrlTable[i].OpCtlValue;
}

BOOL StreamOpCtrlTable::SetOpCtrl(STREAMOP operation, OPCTL control)
{
	if (operation <= SOP_NOOP || operation >= SOP_LAST)
		return FALSE; // out of range
	
	// scan the current table to locate the place to insert
	DWORD i = 0;
	for (; i < SOP_LAST && m_OpCtrlTable[i].StreamOperator != SOP_NOOP; i++)
	{
		if (m_OpCtrlTable[i].StreamOperator == operation)
			break; 
	}
	
	// set the new ctrl value pair
	m_OpCtrlTable[i].StreamOperator = operation;
	m_OpCtrlTable[i].OpCtlValue		= control;
	
	return TRUE; // successful
}

LPOPCTLTBL StreamOpCtrlTable::GetOpCtrlTable()
{
	return m_OpCtrlTable;
}

BOOL StreamOpCtrlTable::SetPrimVideoStoreCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_PRIME, control);
}

BOOL StreamOpCtrlTable::SetStreamPlayCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_PLAY, control);
}

BOOL StreamOpCtrlTable::SetStreamFFCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_FF, control);
}

BOOL StreamOpCtrlTable::SetStreamRewCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_REW, control);
}

BOOL StreamOpCtrlTable::SetStreamPauseCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_PAUSE, control);
}

BOOL StreamOpCtrlTable::SetStreamResumeCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_RESUME, control);
}

BOOL StreamOpCtrlTable::SetStreamStopCtrl(OPCTL control /* = SOPCTL_ALLOW */)
{
	return SetOpCtrl(SOP_STOP, control);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


StreamOpCtrl::StreamOpCtrl(CStreamSession* pStreamSession, STREAMID sid) 
			: m_pStreamSession(pStreamSession), m_SID(sid)
{
	m_pStreamStatus = NULL;
}

StreamOpCtrl::~StreamOpCtrl()
{
	if (m_pStreamStatus)
	{
		ITVSTATUS status = IssFreeBlock(m_pStreamSession->GetTypeInst(),
								m_pStreamStatus
						   );
		if (ITV_SUCCESS != status)
		{
			// error log
			printf("IssFreeBlock ERROR \n");
		}
	}
	m_pStreamStatus = NULL;
}


ISSSTREAMSTATE StreamOpCtrl::GetCurrentState()
{
	if (m_pStreamStatus)
		return m_pStreamStatus->CurrentState;
	return ISS_STATE_NONE;
}

ISSSTREAMSTATE StreamOpCtrl::GetPreviousState()
{
	if (m_pStreamStatus)
		return m_pStreamStatus->PreviousState;
	return ISS_STATE_NONE;
}

time_t StreamOpCtrl::GetPlayStartTime()
{
	if (m_pStreamStatus)
		return m_pStreamStatus->PlayStartTime;
	return -1;
}

PAELIST StreamOpCtrl::GetAEList()
{
	if (m_pStreamStatus)
		return &(m_pStreamStatus->AEList);
	return NULL;
}

PAELIST StreamOpCtrl::GetActiveList()
{
	if (m_pStreamStatus)
		return &(m_pStreamStatus->ActiveList);
	return NULL;
}

DWORD StreamOpCtrl::GetCurrentActiveIndex()
{
	if (m_pStreamStatus)
		return m_pStreamStatus->dwCurrentAE;
	return 0;
}

DWORD StreamOpCtrl::GetSequenceNumber()
{
	if (m_pStreamStatus)
		return m_pStreamStatus->dwSequenceNumber;
	return 0;
}

PAPP_NPT StreamOpCtrl::GetCurrentPostion()
{
	if (m_pStreamStatus)
		return &(m_pStreamStatus->NPT);
	return NULL;
}

PITVPORT StreamOpCtrl::GetOutputPort()
{
	if (m_pStreamStatus)
		return &(m_pStreamStatus->OutputPort);
	return NULL;
}

BOOL StreamOpCtrl::AcceptStream(void* userParam /* = NULL */)
{
	// attention: m_pStreamSession->GetOpCtrlTable() ???
	// ?????????
	StreamOpCtrlTable  streamOptb;

	ITVSTATUS	status;
	status = IssAcceptStream(m_pStreamSession->GetTypeInst(),
							m_SID,
							//m_pStreamSession->GetOpCtrlTable(),
							streamOptb.GetOpCtrlTable(),
							APPSTATUS_SUCCESS,
							(USERPARAM*)userParam,
							m_pStreamSession->GetStatusBlock(),
							NULL
							);
	return (ITV_SUCCESS == status) || (ITV_PENDING == status);
}

BOOL StreamOpCtrl::AcceptReadyAsset(PAELIST pNewAes)
{
	ITVSTATUS	status;
	status = IssAcceptRdyAsset(m_pStreamSession->GetTypeInst(),
							m_SID,
							APPSTATUS_SUCCESS,
							pNewAes,
							m_pStreamSession->GetStatusBlock(),
							NULL
							);
	return (ITV_SUCCESS == status) || (ITV_PENDING == status);
}

BOOL StreamOpCtrl::ModifyStream(MODOP ModOp, PAELIST pNewAes)
{
	m_pStreamStatus = new STREAMSTATUS;
	ITVSTATUS	status;
	status = IssModifyStream(m_pStreamSession->GetTypeInst(),
							m_SID,
							ModOp,
							pNewAes,
							&m_pStreamStatus,
							ISS_STRMSTS_FULL,
							m_pStreamSession->GetStatusBlock(),
							NULL
							);

	if(ITV_SUCCESS == status)
	{
		printf("ModifyStream Operation return ITV_SUCCESS\n");
		return TRUE;
	}
	else if(ITV_PENDING == status)
	{
		printf("ModifyStream Operation return ITV_PENDING\n");
		return TRUE;
	}
	return (ITV_SUCCESS == status);
}

BOOL StreamOpCtrl::AcceptOperation(DWORD dwSeqNum)
{
	ITVSTATUS	status;
	status = IssAcceptStreamOp(	
							m_pStreamSession->GetTypeInst(),
							m_SID,
							dwSeqNum,
							APPSTATUS_SUCCESS,
							m_pStreamSession->GetStatusBlock(),
							NULL
							);		
	return (ITV_SUCCESS == status) || (ITV_PENDING == status);			
}

BOOL StreamOpCtrl::TerminateStream()
{
	ITVSTATUS	status;

	status = IssTerminateStream(	
							m_pStreamSession->GetTypeInst(),
							m_SID,
							ITV_APPSTATUS_SUCCESS,
							NULL,
							ISS_STRMSTS_NONE, 
							m_pStreamSession->GetStatusBlock(),
							NULL
							);		
	return (ITV_SUCCESS == status) || (ITV_PENDING == status);			
}
