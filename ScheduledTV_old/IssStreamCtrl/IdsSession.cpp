#include "Log.h"
using namespace ZQ::common;

#include "IdsSession.h"
#include "Ids_Interfaces.h"

#define	WAIT_INDEFINITELY	0
#define	SIZE_LIMITED		1024

IdsSession::IdsSession()
{
	memset(&m_IdsSess, 0, sizeof(m_IdsSess));
}

IdsSession::~IdsSession()
{
}

BOOL IdsSession::Initialize()
{
	m_ItvVersion.VersionComponents.byteMajor = 1; // ITV_VERSION_CURRENT_MAJOR;
	m_ItvVersion.VersionComponents.byteMinor = ITV_VERSION_CURRENT_MINOR;

	ITVSTATUS status = IdsInitialize(&m_ItvVersion, 
							NULL
					   );
	return (ITV_SUCCESS == status);
}

BOOL IdsSession::UnInitialize()
{
	ITVSTATUS status = IdsUninitialize();
	return (ITV_SUCCESS == status);
}
void IdsSession::IdsCallBack(LPITVSTATUSBLOCK pItvSb)
{
	if (ITV_SUCCESS != pItvSb->Status && ITV_PENDING != pItvSb->Status)
	{
		// error log
		;
	}
}
void IdsSession::IdsConnectionLostCallBack(IDSSESS *pIdsSess)
{
	glog(Log::L_DEBUG, _T("In IdsConnectionLostCallBack"));
}

BOOL IdsSession::Bind(const wchar_t* host, const wchar_t* user, const WORD flags)
{	
	glog(Log::L_DEBUG, _T("Enter IdsSession::Bind()"));
	ITVSTATUS status = IdsBind((wchar_t *)host, (wchar_t *)user, flags, WAIT_INDEFINITELY, &m_IdsSess,
								GetStatusBlock(), NULL, IdsConnectionLostCallBack);
	
	if (ITV_SUCCESS != status && ITV_PENDING != status) 
	{
		glog(Log::L_ERROR, _T("IdsSession Bind() Fail"));
		return FALSE;
	}

	status = IdsSetOptions(&m_IdsSess, IDS_OPT_ENTRYLIMIT, SIZE_LIMITED);
	if (ITV_SUCCESS != status && ITV_PENDING != status)
	{
		glog(Log::L_ERROR, _T("IdsSession Bind, IdsSetOptions Fail"));
		return FALSE;
	}
	return TRUE;
}

BOOL IdsSession::UnBind(void)
{
	glog(Log::L_DEBUG, _T("Enter IdsSession::UnBind()"));
	ITVSTATUS status = IdsUnbind(&m_IdsSess, IDS_ABORT, 
			GetStatusBlock(), IdsCallBack);
	if (ITV_SUCCESS != status && ITV_PENDING != status)
	{
		glog(Log::L_ERROR, _T("IdsSession UnBind() Fail"));
		return FALSE;
	}
	return TRUE;
}

DWORD IdsSession::ListApplications(apps_t &apps)
{
	APPNAME* pApps = NULL;
	DWORD dwNumFound = 0;
	glog(Log::L_DEBUG, _T("Enter IdsSession::ListApplications"));
	ITVSTATUS status = IdsListApplications(&m_IdsSess, &pApps, &dwNumFound, 
				GetStatusBlock(), NULL); //IdsCallBack);
	if (ITV_SUCCESS != status ) //&& ITV_PENDING != status)
	{
		glog(Log::L_DEBUG, _T("IdsSession IdsListAppilcation fail"));
	}
	else // success
	{
			// copy the site infos to local and free the memory in ITV api
		for (DWORD i = 0; i < dwNumFound; i++)
			apps.push_back(pApps[i]);
		if (dwNumFound > 0)
			IdsFree(pApps);
		return apps.size();
	}
	return 0;
}

BOOL IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, METADATA& metaData)
{
	if (metaDataName == NULL || *metaDataName == 0x00)
		return FALSE;

	DWORD dwNumMeta;
	METADATA* Mds2Read[2], * pMDs, Md2Read;
	
	// init the query input
	memset(&Md2Read, 0x00, sizeof(Md2Read));
	Md2Read.Version = m_ItvVersion;

	BOOL succ = FALSE;

	wcscpy(Md2Read.wszMdName, L"*");
	
	Mds2Read[0] = &Md2Read;
	Mds2Read[1] = NULL;

	ITVSTATUS status  = IdsReadApplication(&m_IdsSess, appUid, Mds2Read, 1, &pMDs, 
							&dwNumMeta, GetStatusBlock(), NULL); // performed synchronously
														//IdsCallBack);
	if (ITV_SUCCESS != status)	// && ITV_PENDING != status)
	{
		glog(Log::L_ERROR, _T("IdsSession::GetAppMeta, IdsReadApplication Fail"));
	}
	else
	{
		for (DWORD i = 0; i < dwNumMeta; i++)
		{
			if (wcscmp(metaDataName, pMDs[i].wszMdName) ==0)
			{
				succ = TRUE;
				memcpy(&metaData, &(pMDs[i]), sizeof(metaData));
				break;
			}
		}
		
		IdsFreeMd(pMDs, dwNumMeta);
	} 
	
	return succ;
}

BOOL IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, std::wstring& wstrVal)
{
	METADATA result;

	BOOL succ = GetAppMetaData(appUid, metaDataName, result);

	if (!succ || result.wMdType != 1)
		return FALSE;

	succ= TRUE;
	WCHAR buf[32];

	switch (result.wMdType)
	{
	case 1: // integer value, then convert it to wstring
		swprintf(buf, L"%08x", result.iVal);
		wstrVal = buf;
		break;
		
	case 4: // string value
		wstrVal = result.sVal;
		break;
		
	default:
		swprintf(buf, L"other valuetype: %08x", result.wMdType);
		wstrVal = buf;
		succ= FALSE;
		break;
	}
	
	return succ;
}

BOOL IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, DWORD& dwVal)
{
	METADATA result;

	BOOL succ = GetAppMetaData(appUid, metaDataName, result);

	if (!succ || result.wMdType != 1)
		return FALSE;

	dwVal = result.iVal;
	
	return TRUE;
}