#include "NasCopySource .h"
#include "Log.h"

#include "vstrmuser.h"
#include "PacedIndex.h"
#include "vvx.h"
#include "ErrorCode.h"
#include <time.h> 

#include "VvxParser.h"
#include "VV2Parser.h"

#define NASCOPYSrc			"NasCopySrc"

using namespace ZQ::common;

#define MOLOG (*_pLog)



namespace ZQTianShan {
	namespace ContentProvision {

struct SubFileInfo
{
	char	ext[8];
	long	numerator;
	long	denominator;
	long	direction;				
};

bool getIdxSubFileInfo(const char* szIdxFile, bool bVvx, std::vector<SubFileInfo>& subFiles, MediaInfo& info)
{
	if (bVvx)
	{
		VvxParser vvx;
		if(!vvx.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.videoResolutionH = vvx.getVideoHorizontalSize(); 
		info.videoResolutionV = vvx.getVideoVerticalSize();
		info.bitrate = vvx.GetBitRate();
		info.framerate = atof(vvx.getFrameRateString(vvx.getFrameRateCode()));
		info.playTime = vvx.GetPlayTime();

		subFiles.resize(vvx.getSubFileCount());
		for(int i=0;i<vvx.getSubFileCount();i++)
		{

			vvx.getSubFileExtension(i,subFiles[i].ext, sizeof(subFiles[i].ext));
			vvx.getSubFileSpeed(i, subFiles[i].numerator, *((uint32*)&subFiles[i].denominator));
			if (subFiles[i].numerator<0)
			{
				subFiles[i].numerator = 0 - subFiles[i].numerator;
				subFiles[i].direction = -1;
			}
			else
			{
				subFiles[i].direction = 1;
			}
		}		
	}
	else
	{
		VV2Parser vv2;
		if(!vv2.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.bitrate = vv2.getBitrate();
		info.playTime = vv2.getPlayTime();

		subFiles.resize(vv2.getSubFileCount());
		for(int i=0;i<vv2.getSubFileCount();i++)
		{
			vv2.getSubFileExtension(i,subFiles[i].ext, sizeof(subFiles[i].ext));
			vv2.getSubFileSpeed(i, subFiles[i].numerator, subFiles[i].denominator, subFiles[i].direction);			
		}		
	}

	return true;
}

static void InitVariables(PX *pxP)
{
	memset (pxP, 0, sizeof(*pxP));	// clear all to zero or FALSE

	pxP->vstrmClassHandle		= INVALID_HANDLE_VALUE;
	pxP->RaidLevelSet			= FALSE;
	pxP->RaidLevel				= UNKNOWN_RAID_LEVEL;
	pxP->status					= VSTRM_SUCCESS;

	pxP->StatusSet				= FALSE;
	pxP->activeCopyCount		= 0;
	pxP->Elapsed				= 1;

	pxP->loadParams.Mask =
		LOAD_IOCTL_FILE_LOCAL
		| LOAD_IOCTL_PAUSE_ON_PLAY
		| LOAD_IOCTL_FRAME_COUNT
		| LOAD_IOCTL_MASTER_SESSION_ID
		| LOAD_IOCTL_PRE_BLACK_FRAMES
		| LOAD_IOCTL_POST_BLACK_FRAMES
		| LOAD_IOCTL_TIME_SKIP
		| LOAD_IOCTL_BYTE_SKIP
		| LOAD_IOCTL_SKIP_HEADER
		| LOAD_IOCTL_DEST_PORT_HANDLE
		| LOAD_IOCTL_TERMINATE_ON_EXIT
		| LOAD_IOCTL_PLAYLIST_FLAG
		| LOAD_IOCTL_OBJECT_NAME
		| LOAD_IOCTL_DEBUG
		| LOAD_IOCTL_SPECIFIED_DIRECTORY;

	pxP->NodeName[0]				= '?';
}

void NasCopySource::setVstrmError(HANDLE hVstrmClass, VSTATUS status, const char* szFunc)
{
	char sErrorText[256]={0};
	VstrmClassGetErrorText(hVstrmClass, status, sErrorText, 255);
	int nErrorCode;
	if (status == VSTRM_DISK_FULL)
		nErrorCode = ERRCODE_VSTRM_DISK_FULL;
	else if (status == VSTRM_NETWORK_NOT_READY)
		nErrorCode = ERRCODE_VSTRM_NOT_READY;
	else if (status == VSTRM_NO_MEMORY)
		nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
	else if (status == VSTRM_BANDWIDTH_EXCEEDED)
		nErrorCode = ERRCODE_VSTRM_BANDWIDTH_EXCEEDED;
	else
		nErrorCode = ERRCODE_VSTRM_API_ERROR;				

	std::string strLastErr;
	strLastErr = std::string(szFunc?szFunc:"Vstrm") + std::string(" failed with error: ") + sErrorText;
	SetLastError(strLastErr, nErrorCode);
	MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] %s"), _strLogHint.c_str(), strLastErr.c_str());

	// stop process
	_bStop = true;
}

VSTATUS NasCopySource::sessionDoneCb(PIOCTL_STATUS_BUFFER_LONG sbP, PX *pxP)
{
	ULONG		hours, minutes, seconds, hundreds;
	ULONG		dhours, dminutes, dseconds, dhundreds;

	pxP->status = sbP->status;

	//
	if (sbP->status != VSTRM_SUCCESS)
	{
		pxP->Elapsed = 0;

		if (pxP->StatusSet == FALSE)
		{
			// if status not set, means failed at beginning, it just return
			return VSTRM_SUCCESS;
		}

		char szBuf[512];
		sprintf(szBuf, "VstrmClassCopyObjectEx() from file %s to %s", pxP->InputObjectName, pxP->OutputObjectName);
		setVstrmError(pxP->vstrmClassHandle, pxP->status, szBuf);
	}
	else
	{
		//
		// If session compeleted successfully and user wanted to know how
		// long the play duration was, then SHOW ME THE PLAY DURATION!
		//
		if ((sbP->status == VSTRM_SUCCESS) &&
			(pxP->Elapsed))
		{
			hours = (sbP->playDurationInMs / 1000) / 3600;
			minutes = ((sbP->playDurationInMs / 1000) / 60) % 60;
			seconds = (sbP->playDurationInMs / 1000) % 60;
			hundreds = sbP->playDurationInMs - ((sbP->playDurationInMs / 1000) * 1000);
			dhours = (sbP->sessionDurationInMs / 1000) / 3600;
			dminutes = ((sbP->sessionDurationInMs / 1000) / 60) % 60;
			dseconds = (sbP->sessionDurationInMs / 1000) % 60;
			dhundreds = sbP->sessionDurationInMs - ((sbP->sessionDurationInMs / 1000) * 1000);		

			MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] session %d (on port %d) file %s completed with play duration of %d:%02d:%02d.%02d (session duration %d:%02d:%02d.%02d)"), 
				_strLogHint.c_str(), sbP->sessionId, sbP->portId, pxP->OutputObjectName,
				hours, minutes, seconds, hundreds,
				dhours, dminutes, dseconds, dhundreds);
		}
	}
	
	FreeBw(pxP);

	if (pxP->activeCopyCount>0)
		pxP->activeCopyCount--;

	checkFinish();
	
	return(VSTRM_SUCCESS);
}

void NasCopySource::checkFinish()
{
	bool bDone = true;
	int i;
	for (i=0; i<_nTotalFileCount-1; i++)
	{
		if (gPx[i].activeCopyCount)
		{
			bDone = false;
			break;
		}
	}

	if (bDone)
	{
		SetEvent(_hPartOne);

		if (!gPx[_nTotalFileCount-1].activeCopyCount)
		{
			SetEvent(_hPartTwo);
		}
	}
}

VSTATUS NasCopySource::SessionDoneV2Cb (HANDLE classHandle, PVOID cbParm, PVOID bufP, ULONG bufLen)
{
	PX* pPx = (PX *)cbParm;
	NasCopySource* pThis = pPx->pThis;
	return pThis->sessionDoneCb((PIOCTL_STATUS_BUFFER_LONG)bufP, pPx);	
}


NasCopySource::NasCopySource()
{
	_nOutputCount = 0;
	_nInputCount = 0;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}
	for(i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}

	_bDriverModule = true;
	_bwmgrClientId = 0;
	_bDisableBitrateLimit = 0;
	_nTotalFileCount = 0;

	_hPartOne = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hPartTwo = CreateEvent(NULL, FALSE, FALSE, NULL);
}

NasCopySource::~NasCopySource()
{
	CloseHandle(_hPartOne);
	CloseHandle(_hPartTwo);
}

void NasCopySource::setSourceFileName(const char* srcFilename)
{
	_srcFilename = srcFilename;
}

void NasCopySource::setDestFileName(const char* desFilename)
{
	_desFilename = desFilename;
}

void NasCopySource::setBandwith(int bitrate)
{
	_biterate = bitrate;
}

void NasCopySource::setDisableBitrateLimit(bool bDisableBitrateLimit)
{
	_bDisableBitrateLimit = bDisableBitrateLimit;

	if (bDisableBitrateLimit)
	{
		MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] user disabled bitrate limitation for copying from NAS to Vstrm"),
			_strLogHint.c_str());
	}
}

bool NasCopySource::Init()
{	
	return true;
}

bool NasCopySource::Start()
{

	return true;
}

void NasCopySource::Stop()
{
	// force to cancel provision
	_bStop = true;
	SetEvent(_hPartOne);
	SetEvent(_hPartTwo);
}

void NasCopySource::Close()
{
	_bStop = true;
	SetEvent(_hPartOne);
	SetEvent(_hPartTwo);
}

void NasCopySource::endOfStream()
{

}

const char* NasCopySource::GetName()
{
	return SOURCE_TYPE_NASCOPY;
}

LONGLONG NasCopySource::getProcessBytes()
{
	return GetGraph()->getProcessBytes();
}

VSTATUS NasCopySource::VstrmCopy(PX* pxP)
{
	IOCTL_CONTROL_PARMS_LONG	iocp;

	VSTATUS				(*cb) (HANDLE,PVOID, PVOID,ULONG);

	PVOID				cbp;
	int					pcnt = 0;
	BOOL				noPrivateData = FALSE;
	VSTATUS				status;
	int					percentComplete = 0;
	BOOLEAN				bSourceLocal = FALSE;
	ULONG				delay = 0;  // Default
	ULONG				terminateOnExit = TRUE;

	/* Initialize callback & callback param for use by copy all forks as well */

	pxP->StatusSet = FALSE;
	cb = SessionDoneV2Cb;
	cbp = pxP;

	iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_OBJECT_NAME;
	iocp.u.load.loadParamArray[pcnt].loadValueLength = (USHORT)strlen(pxP->InputObjectName);
	iocp.u.load.loadParamArray[pcnt].loadValueP = pxP->InputObjectName;
	pcnt++;

	iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_OUTPUT_OBJECT_NAME;
	iocp.u.load.loadParamArray[pcnt].loadValueLength = (USHORT)strlen(pxP->OutputObjectName);
	iocp.u.load.loadParamArray[pcnt].loadValueP = pxP->OutputObjectName;
	pcnt++;


	//
	// complete parameter block
	//
	if (pxP->RaidLevelSet)
	{
		iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_RAID_LEVEL;
		iocp.u.load.loadParamArray[pcnt].loadValueP = &pxP->RaidLevel;
		iocp.u.load.loadParamArray[pcnt].loadValueLength = sizeof(pxP->RaidLevel);
		pcnt++;
	}

	iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_TERMINATE_ON_EXIT;
	iocp.u.load.loadParamArray[pcnt].loadValueP = &terminateOnExit;
	iocp.u.load.loadParamArray[pcnt].loadValueLength = sizeof(terminateOnExit);
	pcnt++;

	iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_PERCENT_COMPLETE;
	iocp.u.load.loadParamArray[pcnt].loadValueP = &iocp;
	iocp.u.load.loadParamArray[pcnt].loadValueLength = 4;
	pcnt++;

#if 1 // Changes for ZQ Engineering
	// **PRELIMINARY**
	// LOAD_CODE_THROTTLE_BITRATE value is NOT in Vstrm V6.0 BL0 or BL1.
	iocp.u.load.loadParamArray[pcnt].loadCode = LOAD_CODE_THROTTLE_BITRATE;
	iocp.u.load.loadParamArray[pcnt].loadValueP =  (PVOID)&pxP->bitRate;
	iocp.u.load.loadParamArray[pcnt].loadValueLength = sizeof(pxP->bitRate);  //Sizeof(ULONG64).
	pcnt++;

	// **PRELIMINARY**
	// LOAD_CODE_RETRY_IO_DELAY will not be implemented.

#endif // Changes for ZQ Engineering

	//
	// copy the files
	//
	iocp.u.load.loadParamCount = pcnt;

	iocp.u.load.sessionId = 0;
	status = VstrmClassCopyObjectEx(pxP->vstrmClassHandle,
		&iocp, sizeof(iocp), cb, (PVOID)cbp);
	
	// status returned indicates that the Load() completed but does not
	// guarantee the Play succeeded -- See PlayThread() in VstrmArchive.c
	if (IS_VSTRM_SUCCESS(status))
	{
		pxP->sessionId = iocp.u.load.sessionId;
		pxP->activeCopyCount++;		
	}
	else
	{
		char szBuf[512];
		sprintf(szBuf, "VstrmClassCopyObjectEx() from file %s to %s", pxP->InputObjectName, pxP->OutputObjectName);
		setVstrmError(pxP->vstrmClassHandle, status, szBuf);
	}
	pxP->StatusSet = TRUE;
	pxP->status = status;

	return status;
}

void NasCopySource::setVstrmBwClientId(unsigned int vstrmBwClientId)
{
	_bwmgrClientId = vstrmBwClientId;
}

// Allocate bandwidth
VSTATUS NasCopySource::AllocBw(PX *pxP,ULONG ClientId)
{
	if(!_bwmgrClientId || !pxP->bitRate)
	{
		return VSTRM_SUCCESS;
	}

	char szBuf[256];
	VSTRM_BANDWIDTH_RESERVE_BLOCK brb;
	VSTATUS bwStatus = VSTRM_INVALID_PARAMETER;


	// Allocate Host NIC bandwidth
	memset(&brb, 0, sizeof(brb));
	brb.Type = kVSTRM_BANDWIDTH_TYPE_WRITE;   // Note - Write Bw given current Host NIC Bw Assignment.

	// May make more sense to set to READ Bw, but requires Vstrm Change.
	brb.TargetType = kVSTRM_BANDWIDTH_TARGETTYPE_HOSTNIC;  // Assumes URL source of data...
	brb.ClientId = _bwmgrClientId;
	brb.BwTarget = &pxP->UnprefixedInputObjectName;
	brb.MaxBandwidth = pxP->bitRate;
	brb.MinBandwidth = 0;
	bwStatus = VstrmClassReserveBandwidth(pxP->vstrmClassHandle, &brb, &pxP->bwTicketHostNic);
	if (bwStatus!=VSTRM_SUCCESS)
	{
		sprintf(szBuf, "VstrmClassReserveBandwidth(BW - %s, bitrate - %d)",pxP->InputObjectName,pxP->bitRate);
		setVstrmError(pxP->vstrmClassHandle, bwStatus, szBuf);		
	}
	else
	{
		// Allocate Host File Bandwidth
		memset(&brb, 0, sizeof(brb));
		brb.Type = kVSTRM_BANDWIDTH_TYPE_READ;
		brb.TargetType = kVSTRM_BANDWIDTH_TARGETTYPE_FILE;  // Cluster Destination
		brb.ClientId = ClientId;
		brb.BwTarget = &pxP->OutputObjectName;
		brb.MaxBandwidth = pxP->bitRate;
		brb.MinBandwidth = 0;
		bwStatus = VstrmClassReserveBandwidth(pxP->vstrmClassHandle, &brb, &pxP->bwTicketFile);
		if (bwStatus!=VSTRM_SUCCESS)
		{
			FreeBw(pxP);

			sprintf(szBuf, "VstrmClassReserveBandwidth(BW - %s, bitrate - %d)",pxP->InputObjectName,pxP->bitRate);
			setVstrmError(pxP->vstrmClassHandle, bwStatus, szBuf);		
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] VstrmClassReserveBandwith success - [HostNic Ticket, File Ticket] = [0x%I64x, 0x%I64x]"), _strLogHint.c_str(),pxP->bwTicketHostNic, pxP->bwTicketFile);
		}
	}

	return bwStatus;
}

//Free Bandwidth for one ticket.
VOID NasCopySource::FreeBw(PX *pxP)
{
	if(!_bwmgrClientId || !pxP->bitRate)
		return ;

	char szBuf[256];
	VSTATUS	statusTicket = ERROR_SUCCESS;
	if (pxP->bwTicketHostNic != 0L)
	{
		statusTicket = VstrmClassReleaseBandwidth(pxP->vstrmClassHandle, pxP->bwTicketHostNic);

		if (statusTicket != VSTRM_SUCCESS)
		{	
			VstrmClassGetErrorText(pxP->vstrmClassHandle, statusTicket, szBuf, sizeof(szBuf));

			MOLOG(Log::L_WARNING, CLOGFMT(NASCOPYSrc, "[%s] VstrmClassReleaseBandwidth for HostNic ticket 0x%I64X failed with error %s"), 
				_strLogHint.c_str(),pxP->bwTicketHostNic, szBuf);
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] VstrmClassReleaseBandwidth for HostNic ticket 0x%I64X sucessful"), 
				_strLogHint.c_str(),pxP->bwTicketHostNic);
		}
		pxP->bwTicketHostNic = 0L;
	}

	if (pxP->bwTicketFile != 0L)
	{
		statusTicket = VstrmClassReleaseBandwidth(pxP->vstrmClassHandle, pxP->bwTicketFile);
		if (statusTicket != VSTRM_SUCCESS)
		{	
			VstrmClassGetErrorText(pxP->vstrmClassHandle, statusTicket, szBuf, sizeof(szBuf));

			MOLOG(Log::L_WARNING, CLOGFMT(NASCOPYSrc, "[%s] VstrmClassReleaseBandwidth for ticket 0x%I64X failed with error %s"), 
				_strLogHint.c_str(), pxP->bwTicketFile, szBuf);
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] VstrmClassReleaseBandwidth for ticket 0x%I64X successful"), 
				_strLogHint.c_str(), pxP->bwTicketFile);
		}
		pxP->bwTicketFile = 0L;
	}
}

bool NasCopySource::Run()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(NASCOPYSrc, "[%s] Run() enter"), _strLogHint.c_str());

	char szBuf[512];
	std::string strIdxFile;
	bool bVvx;	
	{
		strIdxFile = _srcFilename + ".vvx";	
		HANDLE fileHandle = CreateFileA(strIdxFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			strIdxFile = _srcFilename + ".vv2";
			fileHandle = CreateFileA(strIdxFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				//error & return
				std::string errmsg;
				getSystemErrorText(errmsg);
				snprintf(szBuf, sizeof(szBuf)-1, "Failed to load VVX/VV2 for (%s) with error: %s", _srcFilename.c_str(), errmsg.c_str());
				MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
				SetLastError(szBuf, ERRCODE_NTFS_READFILE);	
				return false;	
			}
			bVvx = false;
			CloseHandle(fileHandle);
		}
		else
		{
			bVvx = true;
			CloseHandle(fileHandle);
		}
	}

	std::vector<SubFileInfo> subFiles;
	MediaInfo mInfo;
	if(!getIdxSubFileInfo(strIdxFile.c_str(), bVvx, subFiles, mInfo))
	{
		//failed to parse the index file
		snprintf(szBuf, sizeof(szBuf)-1, "Failed to parse VVX/VV2 for (%s)", _srcFilename.c_str());
		MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
		SetLastError(szBuf, ERRCODE_NTFS_READFILE);	
		return false;
	}

	//put index file into the subfiles
	SubFileInfo subf;
	if (bVvx)
		strcpy(subf.ext, ".vvx");
	else
		strcpy(subf.ext, ".vv2");

	subf.numerator = 20;
	subf.denominator = 1;
	subf.direction = 0;	
	subFiles.push_back(subf);
	//make sure the main file speed
	subFiles[0].numerator=1;
	subFiles[0].denominator=1;
	subFiles[0].direction = 0;


	MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] index VVX/VV2 for (%s) loaded"), _strLogHint.c_str(), _srcFilename.c_str());

	_nTotalFileCount = subFiles.size();

	//
	// get the main file size
	//
	LONGLONG filesize=0;
	{
		HANDLE hFile = CreateFile(_srcFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			MOLOG(Log::L_WARNING, CLOGFMT(NASCOPYSrc, "[%s] failed to get file size of [%s] with error: %s"),  _strLogHint.c_str(), 
				_srcFilename.c_str(),errmsg.c_str());
		}
		else
		{
			LARGE_INTEGER larin;								
			if(GetFileSizeEx(hFile, &larin))
			{	
				filesize = larin.QuadPart;
				MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] file[%s] size from NAS [%lld] bytes"),
					_strLogHint.c_str(), _srcFilename.c_str(), filesize);
			}
			else
			{
				std::string errmsg;
				getSystemErrorText(errmsg);
				MOLOG(Log::L_WARNING, CLOGFMT(NASCOPYSrc, "[%s] failed to get file size of [%s] with error: %s"),  _strLogHint.c_str(), 
					_srcFilename.c_str(),errmsg.c_str());
			}
			CloseHandle(hFile);
		}
	}
	if (filesize)
	{
		GetGraph()->setTotalBytes(filesize);
	}
	
	//
	// media info
	//
	GetGraph()->OnMediaInfoParsed(mInfo);

	VSTATUS status;
	int i;
	char   DefaultObjectLocation[MAX_OBJECT_NAME_LONG];
	char progress[255]={0};
	ULONG clientId = kVSTRM_BANDWIDTH_CLIENTID_FACILITY_ZQ | (GetCurrentProcessId()&0x0000FFFF);

	DWORD dwStart = GetTickCount();
	do
	{
		try
		{		
			// This initialization is meant to be illustrative of what operations might be done.
			// It's unlikely that your server will require all the options listed in the PX Struct definition.
			// It does not show how to set the private data, such as bitrate.
			for (i=0; i<_nTotalFileCount; i++)
				InitVariables(&gPx[i]);

			// Get default Destination file prefix
			GetObjectLocation(DefaultObjectLocation, 0);

			for (i=0; i<_nTotalFileCount; i++)
			{
				status = VstrmClassOpenEx(&gPx[i].vstrmClassHandle);
				if (&gPx[i].vstrmClassHandle == INVALID_HANDLE_VALUE)
				{
					sprintf(szBuf, "VstrmClassOpenEx Failed with Status = 0x%x", status);
					MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
					SetLastError(szBuf, ERRCODE_VSTRM_API_ERROR);
					
					break;
				}

				if (subFiles[i].numerator)
				{
					gPx[i].bitRate = _biterate * subFiles[i].denominator /subFiles[i].numerator;
				}
				else
				{
					gPx[i].bitRate = _biterate /6;
				}

				if (_bDisableBitrateLimit)
				{
					gPx[i].bitRate = 0;
				}

				// Create Input file name of form \NTFSDEVICES\\\Server\Share\SourceFile.ext
				//      Where \\Server\Share\SourceFile.ext is supplied by caller.
				// Create output file name of form \DestinationFileSystem\DestinationFile.ext
				//      Where  DestinationFileSystem = \SeafileDevices or \RaidDevices, supplied by Vstrm
				//			  DestinationFile.ext is supplied by caller.
				strcpy((PCHAR)&gPx[i].UnprefixedInputObjectName,  _srcFilename.c_str());
				strcat((PCHAR)&gPx[i].UnprefixedInputObjectName, subFiles[i].ext);
				strcpy((PCHAR)&gPx[i].InputObjectName,  "\\NTFSDEVICES\\");
				strcat((PCHAR)&gPx[i].InputObjectName, (PCHAR)&gPx[i].UnprefixedInputObjectName);
				strcpy((PCHAR)&gPx[i].OutputObjectName, DefaultObjectLocation);
				strcat((PCHAR)&gPx[i].OutputObjectName,_desFilename.c_str());
				strcat((PCHAR)&gPx[i].OutputObjectName, subFiles[i].ext);
				gPx[i].pThis = this;
				gPx[i].fileIndex = i;
				MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] InputObjectName[%d] = %s OutputObjectName[%d] = %s"), 
					_strLogHint.c_str(),i,&gPx[i].InputObjectName,i,&gPx[i].OutputObjectName);

				//Allocate bandwidth ticket.
				status = AllocBw(&gPx[i],clientId);
				if (status != VSTRM_SUCCESS)
				{
					MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] Cannot allocate bandwidth %lld with Status = 0x%x"), _strLogHint.c_str(),gPx[i].bitRate,status);
					if (gPx[i].vstrmClassHandle != INVALID_HANDLE_VALUE)
					{
						VstrmClassCloseEx(gPx[i].vstrmClassHandle);
						gPx[i].vstrmClassHandle = INVALID_HANDLE_VALUE;
					}

					break;
				}
				MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] Succesfully allocate bandwidth %lld with Status = 0x%x"), _strLogHint.c_str(),gPx[i].bitRate,status);
			}

			if (status != VSTRM_SUCCESS)
			{
				break;
			}

			// 
			// Handle copying .mpg and all trickfiles  (omit the .vvx file)
			//
			for ((int)i=0; (i<_nTotalFileCount-1)&&IS_VSTRM_SUCCESS(status); i++)
			{
				status = VstrmCopy(&gPx[i]);
			}
		
			if (status != VSTRM_SUCCESS)
			{
				break;
			}

			while(!_bStop)
			{
				DWORD dwRet = WaitForSingleObjectEx(_hPartOne, 30000, TRUE);
				if (dwRet == WAIT_TIMEOUT)
				{
					try
					{
						ESESSION_CHARACTERISTICS	buffer;
						ULONG						returnSize;

						status=VstrmClassGetSessionChars(gPx[0].vstrmClassHandle,gPx[0].sessionId,&buffer,sizeof(buffer),&returnSize);
						if(VSTRM_SUCCESS==status)
						{
							GetGraph()->OnProgress(buffer.SessionCharacteristics.ByteOffset.QuadPart);				
						}
					}
					catch (...)
					{				
					}
				}
				else if (dwRet != WAIT_IO_COMPLETION)
				{
					break;
				}
			}

			if (_bStop)
				break;

			{
				//
				// copy the vvx file
				//
				MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] Copying index VVX/VV2 file..."), _strLogHint.c_str());	

				ResetEvent(_hPartTwo);
				// Now copy vvx as RAID_1 file
				gPx[_nTotalFileCount-1].RaidLevelSet = TRUE;
				gPx[_nTotalFileCount-1].RaidLevel = RAID_1;
				status = VstrmCopy(&gPx[_nTotalFileCount-1]);

				DWORD dwRet;
				while(!_bStop)
				{
					dwRet = WaitForSingleObjectEx(_hPartTwo, INFINITE, TRUE);
					if (dwRet != WAIT_IO_COMPLETION)
						break;
				}

				if (dwRet == WAIT_OBJECT_0)
				{
					MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] Done Copying VVX file"), _strLogHint.c_str());
				}
			}

			LONGLONG filesize=0;
			if(vsm_GetFileSize(gPx[0].vstrmClassHandle, _desFilename.c_str(),filesize) == 0)
			{
				MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] file size from vstrm [%lld] bytes"),_strLogHint.c_str(), filesize);
			}
			else
			{
				HANDLE hFile = CreateFile(_srcFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					MOLOG(Log::L_WARNING, CLOGFMT(NASCOPYSrc, "[%s] faided to get file size of [%s] with error: %s"),  _strLogHint.c_str(), 
						_srcFilename.c_str(),errmsg.c_str());
				}
				else
				{
					LARGE_INTEGER larin;								
					if(GetFileSizeEx(hFile, &larin))
					{	
						filesize = larin.QuadPart;
						MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] file size from NAS [%lld] bytes"),_strLogHint.c_str(),filesize);
					}
					CloseHandle(hFile);
				}
			}
			if (!filesize)
			{
				filesize = GetGraph()->getProcessBytes();
			}
			GetGraph()->setTotalBytes(filesize);
			GetGraph()->OnProgress(filesize);							
		}
		catch(...)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NASCOPYSrc, "[%s] copying caught unknown exception"), _strLogHint.c_str());	
			SetLastError("copying caught unknown exception", ERRCODE_NTFS_READFILE);	
		}
	}while(0);

	DWORD dwDuration = GetTickCount() - dwStart;	//here maybe wrong, but does not matter, just a log
	DWORD dwTranferRate;
	if (dwDuration)
	{
		dwTranferRate = filesize*8000/dwDuration;
	}
	else
	{
		dwTranferRate = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(NASCOPYSrc, "[%s] Actual copy rate is [%d]bps"), _strLogHint.c_str(), dwTranferRate);

	MOLOG(Log::L_DEBUG, CLOGFMT(NASCOPYSrc, "[%s] Cleaning up..."), _strLogHint.c_str());
	for ((int)i=0; i<_nTotalFileCount; i++)
	{
		FreeBw(&gPx[i]);

		if (gPx[i].vstrmClassHandle != INVALID_HANDLE_VALUE)
		{
			VstrmClassCloseEx(gPx[i].vstrmClassHandle);
			gPx[i].vstrmClassHandle = INVALID_HANDLE_VALUE;
		}
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(NASCOPYSrc, "[%s] Run() left"), _strLogHint.c_str());
	return !GetGraph()->IsErrorOccurred();
}
bool NasCopySource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool NasCopySource::seek(int64 offset, int pos)
{
	return false;
}
}
}
