#include "afx.h"
#include <string>
#include "..\..\..\DBSync\DBSyncToLam.h"

bool ProcData(_LAM_to_DBSync* ltb, _DBSync_to_LAM* btl)
{
//	TRACE("%d\n", ltb->_SynComID);
//	TRACE("%S\n", ltb->_SyncPath);
//	TRACE("%d\n", ltb->_SyncType);

	btl->_SynComID = 10;
	wcscpy(btl->_SyncPath, L"you");
	btl->_SyncType = 20;
	wcscpy(btl->_errorCode, L"no error");
	wcscpy(btl->_errorDescription, L"shit");

	return true;
}

int main()
{
	HMODULE hModule = LoadLibrary("JMS_DBSync_LAM_AddIn.dll");
	if (!hModule)
	{
		return 1;
	}

	DBSync_LAM_Proto_Initialize		dlInit;
	DBSync_LAM_Proto_Run			dlRun;
	DBSync_LAM_Proto_UnInitialize	unInit;

	dlInit = (DBSync_LAM_Proto_Initialize)   GetProcAddress(hModule, "DBSync_LAM_Initialize");
	dlRun  = (DBSync_LAM_Proto_Run)          GetProcAddress(hModule, "DBSync_LAM_Run");
	unInit = (DBSync_LAM_Proto_UnInitialize) GetProcAddress(hModule, "DBSync_LAM_UnInitialize");

	char path[] = "e:\\addin.log";
	
	dlInit(path);

	std::string log = "1111";
	int i = 0;

	/****    获取函数指针    ****/
	bool (*pCallback)(_LAM_to_DBSync* ltb, _DBSync_to_LAM* btl) = &ProcData;
	ASSERT(pCallback != NULL);

	dlRun(pCallback);

	unInit();
	return 1;
}
