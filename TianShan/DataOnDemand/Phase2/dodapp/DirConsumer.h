// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: DirConsumer.h,v 1.0 2006/11/30 10:530:56 Chen Shuai Exp $
// Branch: $Name:  $
// Author: Chen Shuai
// Desc  : Define DirConsumer
//

// ===========================================================================

#ifndef __DIRCONSUMER_H__
#define __DIRCONSUMER_H__

#include "afx.h"
//#include "NativeThread.h"
#include "Locks.h"
#include <list>
#include <queue>
#include "Log.h"
#include "DODAppThread.h"
using namespace std;

#define OUT
#define DIR_ADD     0x00000001
#define DIR_DEL     0x00000002
#define DIR_MODIFY  0x00000004
#define DIR_REN_OLD 0x00000008
#define DIR_REN_NEW 0x00000010

/************************************************************************/
/*            DirConsumer 类定义                                        */
/************************************************************************/
class DirConsumer :  public DODAppThread
{	
	friend class DirMonitor;
public:
	DirConsumer(const char* monitorDirectory,long monitorTime,
				DWORD dirOperation = DIR_ADD | DIR_DEL | DIR_MODIFY | DIR_REN_OLD | DIR_REN_NEW,
				bool bWatchSubdirectories = true, 
			    bool bSnapshot = true,
				bool bNotifyAsFileCome = false);

	virtual ~DirConsumer();

	void stop() { SetEvent(_hStop);SetEvent(_hNotify); 
	          SetEvent(m_hWaitTime);waitHandle(INFINITE);};

    /************************************************************************/
	/*           事件的响应                                                 */
	/************************************************************************/
	virtual bool notify()
	{ SetEvent(_hNotify);return true;};

	virtual bool notifyFoldChange(){return true;}

protected:
	virtual bool init(void);
	virtual void uninit();
	virtual int run(void);
	virtual bool configure();
	virtual bool processNotify();
private:
	DirConsumer();
	bool openHandles();
	void closeHandles();
	bool isFileAccess(const char* fileName);
	void setDirOperation(DWORD dirOperation);

	void initMembers(DWORD dirOperation, bool bWatchSubdirectories, 
					 bool bSnapshot, bool bNotifyAsFileCome);

protected:
	HANDLE _hStop;
	HANDLE _hNotify;
	HANDLE m_hWaitTime;

	DWORD _dirOperation;
	DWORD _notifyFilter;
	bool _bWatchSubdirectories;
	bool _bSnapshot;
	char _monitorDirectory[MAX_PATH];
	bool _bNotifyAsFileCome;
 
private:
	DWORD _interval;
	DirMonitor* _pDirMonitor;
	ZQ::common::Mutex _mutex;
	long _monitorTime;//second
};


/************************************************************************/
/*              DirMonitor  类定义                                      */
/************************************************************************/
class DirMonitor : public DODAppThread
{
	typedef queue<string> StringQueue;
	
	friend class DirConsumer;
protected:
	DirMonitor(DirConsumer* dirConsumer);
	virtual ~DirMonitor();
	
	void stop() { SetEvent(_hStop); waitHandle(INFINITE);;};

	virtual bool configure();
	
	/************************************************************************/
	/*                NativeThread virtual函数                              */
	/************************************************************************/
	virtual bool start();
	virtual bool init(void);
	virtual int  run(void);

	/************************************************************************/
	/*                事件的响应                                            */
	/************************************************************************/
	virtual bool notify() { return (SetEvent(_hNotify)); };
	virtual bool processTimeout() { processNotify(); return true;};
	virtual bool processNotify();

	/************************************************************************/
	/*                对监视目录的操作                                      */
	/************************************************************************/
	virtual bool getDirectorySnapshot(const char* baseDirectory,
									  const char* additionalPath,
									  const char* filter);
	virtual bool processDirectorySnapshot();
	virtual int  fileFilter(const char* str) { return 1; };
	
private:
	DirMonitor();

	bool setMonitorChanges(DWORD&);
	bool processChangedFile(FILE_NOTIFY_INFORMATION* pFni);
	static VOID WINAPI HandleDirChanges(DWORD, DWORD, LPOVERLAPPED);

	bool openHandles();
	void closeHandles();

protected:
	HANDLE _hStop;
	HANDLE _hNotify;
	HANDLE _directoryEvent;
	HANDLE _hSetCallback;

	DWORD _interval;

private:
	char _buffer[MAX_PATH];
	OVERLAPPED _overlapped;

	StringQueue _initialContentsQueue;
	DirConsumer* _pDirConsumer;
	bool _bSetCallbackSucess;
	ZQ::common::Mutex _mutex;
};
#endif