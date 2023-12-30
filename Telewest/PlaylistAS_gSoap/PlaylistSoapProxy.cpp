// PlaylistSoapProxy.cpp: implementation of the PlaylistSoapProxy class.
//
//////////////////////////////////////////////////////////////////////
#include "Log.h"
#include "PlaylistSoapProxy.h"
#include "SoapImplSoapWrapProxy.h"
#include "SoapWrap.nsmap"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void PlaylistSoapProxy::logSoapError(struct soap *soap)
{
	const char **s;
	if (!*soap_faultcode(soap))
		soap_set_fault(soap);
	
	glog(ZQ::common::Log::L_ERROR, "[SOAP] ERROR: ErrorCode=%d  FaultCode=<%s> FaultString=<%s>",soap->error, *soap_faultcode(soap), *soap_faultstring(soap));

	s = soap_faultdetail(soap);
	if (s && *s)
	{
		glog(ZQ::common::Log::L_ERROR, "[SOAP] ERROR: Details <%s>", *s);
	}
}

PlaylistSoapProxy::PlaylistSoapProxy(wchar_t* par_WsdlFile)
{
	wcscpy(_wszSoapWSDLFilePath, par_WsdlFile);
	_dwTimeout = DEFAULT_TIMEOUT;
}

PlaylistSoapProxy::~PlaylistSoapProxy()
{
}

AELIST PlaylistSoapProxy::OnSetupSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId)
{
	char	server[512];
	wcstombs(server, _wszSoapWSDLFilePath, 512);
	int nret;
	AELIST outList;
	memset(&outList, 0x00, sizeof(AELIST));


	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	SoapWrap	client;
	struct ns2__getPlaylistDetailInformationResponse	result;

	client.endpoint = server;

	//////////////////////////////////////////////////////////////////////////
	// invoke method and check result
	nret = client.ns2__getPlaylistDetailInformation(homeId, deviceId, playlistId, streamId, result);
	if(nret != SOAP_OK)
	{
		logSoapError(client.soap);
		return outList;
	}

	glog(ZQ::common::Log::L_DEBUG, "[SOAP] PlaylistSoapProxy::OnSetupSoapCall()  success");
	//////////////////////////////////////////////////////////////////////////
	// compose AELIST
	long i,elemNum=0;
	
	elemNum = (long)(result._getPlaylistDetailInformationReturn->elements->__size);
	
	// if no element, return empty list
	if(elemNum==0) {
		outList.AECount = elemNum;
		return outList;
	}

	// has elements, so allocate new array for it
	AELEMENT* pNewAEs = new AELEMENT[elemNum];
	memset(pNewAEs, 0x00, sizeof(AELEMENT)*elemNum); 

	DWORD dwAEUID;
	
	outList.AECount = elemNum;
	
	for(i=0; i<elemNum; i++)
	{
		std::string** ppae = result._getPlaylistDetailInformationReturn->elements->__ptr;
		std::string* pae = ppae[i];
		std::string strAEID = *pae;
		sscanf(strAEID.c_str(), "%ld", &dwAEUID);

		pNewAEs[i].AEUID = dwAEUID;
		pNewAEs[i].PlayoutEnabled = TRUE;
		pNewAEs[i].EncryptionVendor = ITV_ENCRYPTION_VENDOR_None;
		pNewAEs[i].EncryptionDevice = ITV_ENCRYPTION_DEVICE_None;
	}

	outList.AELlist = (PAEARRAY)pNewAEs;
	return outList;
}

long PlaylistSoapProxy::OnPlaySoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId)
{
	char	server[512];
	wcstombs(server, _wszSoapWSDLFilePath, 512);
	int nret;
	AELIST outList;
	memset(&outList, 0x00, sizeof(AELIST));


	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	SoapWrap	client;
	int			result;

	client.endpoint = server;

	//////////////////////////////////////////////////////////////////////////
	// invoke method and check result
	nret = client.ns2__playPlaylist(homeId, deviceId, playlistId, streamId, result);
	if(nret != SOAP_OK)
	{
		logSoapError(client.soap);
	}
	glog(ZQ::common::Log::L_DEBUG, "[SOAP] PlaylistSoapProxy::OnPlaySoapCall()  success");
	return result;
}

long PlaylistSoapProxy::OnTeardownSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId, char* errorCode)
{
	char	server[512];
	wcstombs(server, _wszSoapWSDLFilePath, 512);
	int nret;
	AELIST outList;
	memset(&outList, 0x00, sizeof(AELIST));


	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	SoapWrap	client;
	int			result;

	client.endpoint = server;

	//////////////////////////////////////////////////////////////////////////
	// invoke method and check result
	nret = client.ns2__teardownPlaylist(homeId, deviceId, playlistId, streamId, errorCode, result);
	if(nret != SOAP_OK)
	{
		logSoapError(client.soap);
	}
	glog(ZQ::common::Log::L_DEBUG, "[SOAP] PlaylistSoapProxy::OnTeardownSoapCall()  success");
	return result;
}

void PlaylistSoapProxy::releaseAEList(AELIST exAElist)
{
	if(exAElist.AELlist)
		delete[] (PAELEMENT)exAElist.AELlist;
	
	exAElist.AELlist = NULL;
}