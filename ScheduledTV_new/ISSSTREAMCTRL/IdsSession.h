#ifndef _H_IDS_SESSION
#define _H_IDS_SESSION

#include "Ids_Def.h"
#include <vector>

#include "StreamOpCtrl.h"

class IdsSession : public ItvStatus
{
public:
	typedef OBJECTLIST APPNAME;
	typedef std::vector<APPNAME> apps_t;
public:
	IdsSession();
	virtual ~IdsSession();
	BOOL	Initialize();
	BOOL	UnInitialize();
	static void	IdsCallBack(LPITVSTATUSBLOCK pItvSb);
	static void IdsConnectionLostCallBack(IDSSESS *pIdsSess);
	BOOL  Bind(const wchar_t* host, const wchar_t* user=L"", const WORD flags=IDS_READ_ONLY);
	BOOL  UnBind(void);
	
	DWORD ListApplications(apps_t &apps);

	BOOL  GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, METADATA& metaData);
	BOOL  GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, std::wstring& wstrVal);
	BOOL  GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, DWORD& dwVal);
protected:
	IDSSESS	m_IdsSess;
};

#endif // _H_IDS_SESSION