#ifndef _DBSYNCADILAM_H_
#define _DBSYNCADILAM_H_

#pragma warning (disable: 4786)

#include "ManualSyncDef.h"
#include "DSInterface.h"
#include "DSCallBack.h"

class CDSInterface;

class ManualSyncAdi
{
	friend class CDSInterface;
public:
	ManualSyncAdi();
	~ManualSyncAdi();

	void           SetAddinPath(wchar_t* addinPath);
	bool           IsValid()  { return  m_enabled; };

	static void    ProcData           (IN     _LAM_to_DBSync* ltb, 
		                               OUT    _DBSync_to_LAM* btl);

	static void    SetErrCode         (IN     _DBSync_to_LAM* btl,
									   IN     DWORD err);

public:
	bool      init();
	int       run();
	void      final();

private:
	bool m_enabled;
	HMODULE m_hLib;

	wchar_t m_addinPath[MAX_PATH];

	std::vector<std::string>  m_procNameVec;

	static DWORD  m_syncID;
};

class ManualSyncFolderCallback : public DSCallBackBase
{
	friend class CDSInterface;

public:
	ManualSyncFolderCallback(HANDLE notifyHandle, DWORD triggerID, CString& syncFolder, DWORD& syncResult);
	virtual ~ManualSyncFolderCallback();

public:
	DWORD Process(void);

protected:
	bool  CombinePath(CString& path, CString& syncFolderName);
	DWORD GetParentUID(CString& path);
	bool  ReserveAdjustFolder(const WCHAR*  ori_root, CString& outString);
	int   ManuallySyncFolder(CString& syncFolderName);

private:
	CString m_syncFolder;
	HANDLE  m_hCompleteNotify;
	DWORD   m_triggerID;
	DWORD&  m_syncResult;
};

class ManualSyncMDDCallback : public DSCallBackBase
{
	friend class CDSInterface;

public:
	ManualSyncMDDCallback(HANDLE notifyHandle, DWORD triggerID, DWORD& syncResult);
	virtual ~ManualSyncMDDCallback();

public:
	DWORD Process(void);

private:
	HANDLE  m_hCompleteNotify;
	DWORD   m_triggerID;
	DWORD&  m_syncResult;
};

#endif