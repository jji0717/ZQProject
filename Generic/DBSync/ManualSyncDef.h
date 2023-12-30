#ifndef _DBSYNCTOLAM_H_
#define _DBSYNCTOLAM_H_
#include "Log.h"

#define WAIT_TIME 2000


// data information for DBSync service
struct _LAM_to_DBSync
{
	// SyncCommand id
	int		_SynComID;
	

	// SyncCommand type
	int		_SyncType;


	// SyncCommand path
	wchar_t _SyncPath[MAX_PATH];
	
};


// data information for LAM service
struct _DBSync_to_LAM : public _LAM_to_DBSync
{
	// error code
	wchar_t _errorCode[MAX_PATH];

	// error description
	wchar_t _errorDescription[MAX_PATH];
};


// ------------------------------------------------
/// This API will get called when Initialize() is called.
/// This usually waits request from LAM_QUEUE and sends result to DBSYNC_QUEUE
extern "C" bool ManualSync_Init(void* pFunc, ZQ::common::Log * pLog);
typedef bool (*ManualSync_Proto_Init)(void* pFunc, ZQ::common::Log * pLog);
#define MANUAL_SYNC_INIT "ManualSync_Init"


extern "C" bool ManualSync_Run();
typedef bool (*ManualSync_Proto_Run)();
#define MANUAL_SYNC_RUN "ManualSync_Run"

// ------------------------------------------------
/// This API will get called when Add-in is released.  
/// This usually happens when DBSync service stops.
extern "C" void ManualSync_Stop();
typedef void (*ManualSync_Proto_Stop)();
#define MANUAL_SYNC_STOP "ManualSync_Stop"

// ------------------------------------------------
// The Count of Functions
#define    MANUAL_SYNC_COUNT     3


// ------------------------------------------------
/// This API is a CALLBACK function for DBSync and called by Add-In  
/// @param[in]	_LAM_to_DBSync parsed data from XML
/// @param[out] _DBSync_to_LAM result data from DBSync
typedef bool	(*ManualSync_ProcData_Callback)(_LAM_to_DBSync*, _DBSync_to_LAM*);

#endif