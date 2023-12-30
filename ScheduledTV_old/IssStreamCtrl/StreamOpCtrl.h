#ifndef _H_STREAM_OP_CTRL
#define _H_STREAM_OP_CTRL
#include "IssApi.h"
#include <tchar.h>
class CStreamSession;

class ItvStatus
{
public:
	ItvStatus() {
		m_ItvVersion.VersionComponents.byteMajor = ITV_VERSION_CURRENT_MAJOR;
		m_ItvVersion.VersionComponents.byteMinor = ITV_VERSION_CURRENT_MINOR;
		memcpy(&m_ItvStatusBlk.Version, &m_ItvVersion, sizeof(m_ItvStatusBlk.Version));
	}
	virtual ~ItvStatus() {}

	const LPITVSTATUSBLOCK GetStatusBlock() { return &m_ItvStatusBlk; }
	const PITVVERSION GetItvVersion() { return &m_ItvVersion; }

	void SetBlockUserParam(DWORD dwUserParam) { m_ItvStatusBlk.dwUserParam = dwUserParam; };

	static const char* DumpItvSb(char *buf, LPITVSTATUSBLOCK pItvSb)
	{
		if (buf == NULL || pItvSb==NULL)
			return NULL;
		
		sprintf(buf, "ver:%08x; resv:%04x, status:%08x, exsts:%08x, upm:%08x",
			pItvSb->Version,
			pItvSb->wReserved,
			pItvSb->Status,
			pItvSb->dwExtendedStatus,
			pItvSb->dwUserParam
			);
		return buf;
	}

	ITVVERSION		m_ItvVersion;
	ITVSTATUSBLOCK	m_ItvStatusBlk; // status block
};

class StreamOpCtrlTable
{
public:
	StreamOpCtrlTable();
	StreamOpCtrlTable(const StreamOpCtrlTable& opCtlTbl);

	virtual ~StreamOpCtrlTable() {}

	void  GeneralInit(void);

	BOOL SetOpCtrl(STREAMOP operation, OPCTL control);
	const OPCTL GetOpCtrl(STREAMOP operation);
	LPOPCTLTBL GetOpCtrlTable();

	BOOL SetPrimVideoStoreCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamPlayCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamFFCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamRewCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamPauseCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamResumeCtrl(OPCTL control = SOPCTL_ALLOW);
	BOOL SetStreamStopCtrl(OPCTL control = SOPCTL_ALLOW);

private:

	// data declare
	OPCTLTBLELEMENT m_OpCtrlTable[SOP_LAST];

};

/** StreamOpCtrl class.
 *
 *  the stream operation control wrapper.
 *  it derives StreamOpCtrlTable as its per-stream stream operation ctrls
*/

class StreamOpCtrl 
{
public:

	StreamOpCtrl(CStreamSession* pStreamSession, STREAMID SId);

	virtual ~StreamOpCtrl();


	// State of the stream presently
	ISSSTREAMSTATE GetCurrentState();
	
	// The previous state of the stream. This may be of interest in cases where the
	// stream has been aborted.

	ISSSTREAMSTATE GetPreviousState();

	time_t GetPlayStartTime();
	
	// The list of possible AEs for this stream.
	PAELIST GetAEList();

	// The list of active AEs for this stream
	PAELIST GetActiveList();

	// The index into ActiveList of the AE currently playing on the stream,
	// or which was playing when the stream was aborted or incurred an error
	DWORD  GetCurrentActiveIndex();

	// A sequential number representing the sequence of the currently playing file,
	// or that which was playing when the stream was aborted or incurred. an error
	// over the lifetime of the entire stream.

	DWORD  GetSequenceNumber();

	// Current position information for the stream - an indicator of the position in
	// the current stream which is independent of the ActiveList.
	PAPP_NPT  GetCurrentPostion();

	// output port with TSid
	PITVPORT  GetOutputPort();


	
	// operations per stream
	BOOL AcceptStream(void* userParam = NULL);

	BOOL AcceptReadyAsset(PAELIST pNewAes);

	BOOL ModifyStream(MODOP ModOp, PAELIST pNewAes);

	BOOL AcceptOperation(DWORD dwSeqNum);

    BOOL TerminateStream();
	
protected:

	CStreamSession* m_pStreamSession;
	
	STREAMID m_SID;

	LPSTREAMSTATUS m_pStreamStatus;

};

#endif // _H_STREAM_OP_CTRL