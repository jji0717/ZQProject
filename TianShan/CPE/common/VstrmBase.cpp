
#include "VstrmBase.h"
#include "Vstrmuser.h"


HANDLE VstrmBaseFunc::_hVstrmClassHandle = INVALID_HANDLE_VALUE;
unsigned int VstrmBaseFunc::_uVstrmBwCltID = 0;

#pragma comment(lib, "VstrmDLLEx.lib")


bool VstrmBaseFunc::init(int nBandwidthClientID,  std::string& errmsg)
{	
	VSTATUS vStatus;
	char szBuf[255] = {0};

	HANDLE hvstrm = INVALID_HANDLE_VALUE;
	vStatus = VstrmClassOpenEx(&hvstrm);
	if (vStatus != VSTRM_SUCCESS) 
	{
		VstrmClassGetErrorText(hvstrm, vStatus, szBuf, sizeof(szBuf));
		errmsg = szBuf;

		return false;
	} 
	_hVstrmClassHandle = hvstrm;
	_uVstrmBwCltID = nBandwidthClientID;

	vStatus = VstrmClassReleaseAllBandwidth(hvstrm, nBandwidthClientID, 0);
	return true;
}

void VstrmBaseFunc::unInit()
{
	if(_hVstrmClassHandle != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		VstrmClassCloseEx(_hVstrmClassHandle);
		_hVstrmClassHandle = INVALID_HANDLE_VALUE;
	}
}

int VstrmBaseFunc::getVstrmError(std::string& strErr)
{
	if(_hVstrmClassHandle == INVALID_HANDLE_VALUE)
		return 0;

	char sErrorText[256]={0};

	int status = VstrmGetLastError();

	VstrmClassGetErrorText(_hVstrmClassHandle, status, sErrorText, 255);

	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);

	strErr = std::string(sErrorText)+ errcode;
	return status;
}

void VstrmBaseFunc::getVstrmError(unsigned int status, std::string& strErr)
{
	if(_hVstrmClassHandle == INVALID_HANDLE_VALUE)
		return;

	char sErrorText[256]={0};
	VstrmClassGetErrorText(_hVstrmClassHandle, status, sErrorText, 255);

	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);

	strErr = std::string(sErrorText)+ errcode;
}

long VstrmBaseFunc::reserveBandwidth(DWORD dwBandwidth, ULONG64& ulTicket)
{
	if(0 == _uVstrmBwCltID)
	{
		ulTicket = 0;
		return VSTRM_SUCCESS;
	}

	VSTATUS	statusTicket = ERROR_SUCCESS;
	VSTRM_BANDWIDTH_RESERVE_BLOCK   rbFile = {0};
	PVSTRM_BANDWIDTH_RESERVE_BLOCK	pRbFile=&rbFile;

	// The Bw Mgr considers bandwidth requests
	// to be from the perspective of the PCI Bus, not the disks. So, to get data
	// onto the disks they must READ from the PCI Bus, so ask for READ BW here,
	// even tho we are putting data onto the disks using writes. 
	rbFile.ClientId         = _uVstrmBwCltID;
	rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
	rbFile.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_FILE;

	rbFile.BwTarget         = (void*)"VstrmBase"; 

	rbFile.MaxBandwidth		= dwBandwidth;	// passed in with request
	rbFile.MinBandwidth		= dwBandwidth;	// passed in with request
	rbFile.ReservedBandwidth = NULL;

	return VstrmClassReserveBandwidth(_hVstrmClassHandle, pRbFile, &ulTicket);
}

void VstrmBaseFunc::releaseBandwidth(const ULONG64& ulTicket)
{
	if(0 == _uVstrmBwCltID || !ulTicket)
		return;

	VSTATUS	statusTicket = ERROR_SUCCESS;
	statusTicket = VstrmClassReleaseBandwidth(_hVstrmClassHandle, ulTicket);
}