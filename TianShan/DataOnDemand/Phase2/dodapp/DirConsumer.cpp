#include "DirConsumer.h"
#include "winbase.h"
using namespace ZQ::common;

/************************************************************************/
/*          DirConsumer 类函数                                          */
/************************************************************************/
DirConsumer::DirConsumer(const char* monitorDirectory,long monitorTime,
						 DWORD dirOperation,
						 bool bWatchSubdirectories,
						 bool bSnapshot,
						 bool bNotifyAsFileCome)
{
	glog(Log::L_DEBUG,  "DirConsumer::DirConsumer() ");
	strnset(_monitorDirectory, 0, MAX_PATH);
	initMembers(dirOperation, bWatchSubdirectories, bSnapshot, bNotifyAsFileCome);
	strncpy(_monitorDirectory, monitorDirectory, MAX_PATH - 1);
	_monitorTime = monitorTime;
	glog(Log::L_DEBUG,  "DirConsumer _monitorDirectory <%s>", _monitorDirectory);
}
DirConsumer::~DirConsumer()
{
	MutexGuard guard(_mutex);

	glog(Log::L_DEBUG,  "DirConsumer::~DirConsumer()");

	closeHandles();
	
	if (_pDirMonitor)
	{
		glog(Log::L_DEBUG,  "DirConsumer::Delete DirMonitor pointer");
		delete _pDirMonitor;
		_pDirMonitor = NULL;
	}
}

void DirConsumer::initMembers(DWORD dirOperation, bool bWatchSubdirectories, 
							  bool bSnapshot, bool bNotifyAsFileCome)
{
	_hStop = NULL;
	_hNotify = NULL;
	m_hWaitTime = NULL;
	_interval = 2000;
	_pDirMonitor = NULL;
	_bSnapshot = bSnapshot;
	setDirOperation(dirOperation);
	_bNotifyAsFileCome = bNotifyAsFileCome;
	_notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME;
	_bWatchSubdirectories = bWatchSubdirectories;
}

void DirConsumer::setDirOperation(DWORD dirOperation)
{
	DWORD maxValue = DIR_ADD + DIR_DEL + DIR_MODIFY + DIR_REN_OLD + DIR_REN_NEW;
	_dirOperation = maxValue > dirOperation ? dirOperation : maxValue;
	
	glog(Log::L_DEBUG,  "DirConsumer _dirOperation <%d>", _dirOperation);
}

bool DirConsumer::configure()
{
	DWORD fileAttribute;
	fileAttribute = GetFileAttributesA(_monitorDirectory);
	if ((fileAttribute == ~0)) 
	{
		glog(Log::L_DEBUG,  "DirConsumer GetFileAttributesA error <%d>", GetLastError());
		return false;
	}
	else if (fileAttribute != FILE_ATTRIBUTE_DIRECTORY)
	{
		glog(Log::L_DEBUG,  "DirConsumer GetFileAttributesA value <%d>", fileAttribute);
		return false;
	}
	glog(Log::L_DEBUG,  "DirConsumer GetFileAttributesA <%s> is Directory", _monitorDirectory);
	return true;
}

bool DirConsumer::init(void)
{	
	if (!openHandles())
	{
		return false;
	}

	_pDirMonitor = new DirMonitor(this);
	if (!_pDirMonitor)
	{
		return false;
	}

	if (!configure())
	{
		return false;
	}
	return true;
}
void DirConsumer::uninit()
{
	if(_pDirMonitor)
	  _pDirMonitor->stop();
	stop();
}
int DirConsumer::run(void)
{
	if (!_pDirMonitor->start())
	{
		return 0;
	}

	DWORD interval = _interval;
	bool bContinued = true;
	
	while (bContinued)
	{
		WaitForSingleObject(_hNotify, INFINITE);
		
//		Sleep(_monitorTime*1000);

		if(WaitForSingleObject(m_hWaitTime, _monitorTime*1000) == WAIT_OBJECT_0)
		{
			glog(ZQ::common::Log::L_DEBUG,
				"[monitorDir = %s] Exit DirConsumer::run()  OK",
				_monitorDirectory);	
           return 0;
		}
		if(WaitForSingleObject(_hStop, 0) == WAIT_OBJECT_0)
		{
			glog(ZQ::common::Log::L_DEBUG,
				"[monitorDir = %s] Exit DirConsumer::run()  OK",
				_monitorDirectory);	
			return 1;
		}
		char strtime[30];
		SYSTEMTIME time; 
		GetLocalTime(&time);
		sprintf(strtime,"%04d-%02d-%02d %02d:%02d:%02d:%03d",time.wYear, time.wMonth, 
			time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
//		printf("Notify  %s \n", strtime);
		glog(Log::L_DEBUG,  "Notify time = %s ",strtime);
		processNotify();
		ResetEvent(_hNotify);		
	}
	return 1;
}

/************************************************************************/
/*                针对不同的事件所对应的处理函数                        */
/************************************************************************/
bool DirConsumer::processNotify()
{
    notifyFoldChange();
	return true; 
}
bool DirConsumer::isFileAccess(const char* fileName)
{
	MutexGuard guard(_mutex);
	
	char fullFileName[MAX_PATH];
	strnset(fullFileName, 0, MAX_PATH);
	sprintf(fullFileName, "%s\\%s", _monitorDirectory, fileName);
	
	HANDLE hFile = NULL;
	hFile = CreateFile(fullFileName,
					   GENERIC_READ,
					   FILE_SHARE_READ,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		if (err == ERROR_SHARING_VIOLATION)
		{
			glog(Log::L_DEBUG,  "Other Process is using file <%s>", fullFileName);
			return false;
		}
	}
	CloseHandle(hFile);
	return true;
}

/************************************************************************/
/*            DirMonitor  类函数                                        */
/************************************************************************/
DirMonitor::DirMonitor(DirConsumer* dirConsumer)
:DODAppThread(), 
 _hStop(NULL),
 _hNotify(NULL),
 _directoryEvent(NULL),
 _hSetCallback(NULL),
 _interval(INFINITE),
 _pDirConsumer(dirConsumer),
 _bSetCallbackSucess(false)
{
	strnset(_buffer, 0, MAX_PATH);	
	_overlapped.hEvent = this;
}

DirMonitor::~DirMonitor()
{
	MutexGuard guard(_mutex);
	closeHandles();
	while (_initialContentsQueue.size() > 0)
	{
		_initialContentsQueue.pop();
	}
}

bool DirMonitor::configure()
{
	_directoryEvent = CreateFileA(_pDirConsumer->_monitorDirectory,
								  FILE_LIST_DIRECTORY,
								  FILE_SHARE_READ  | 
								  FILE_SHARE_WRITE | 
								  FILE_SHARE_DELETE,
								  NULL,
								  OPEN_EXISTING,
								  FILE_FLAG_BACKUP_SEMANTICS |
								  FILE_FLAG_OVERLAPPED,
								  NULL);
	if (_directoryEvent == INVALID_HANDLE_VALUE)
	{
		glog(Log::L_DEBUG,  "DirMonitor CreateFileA <%s> error <%d>", 
			 _pDirConsumer->_monitorDirectory, GetLastError());
		
		_bSetCallbackSucess = false;
		SetEvent(_hSetCallback);
		return false;
	}

	glog(Log::L_DEBUG,  "DirMonitor CreateFileA <%s> Success",
		 _pDirConsumer->_monitorDirectory);
	return true;
}

/************************************************************************/
/*                NativeThread vitural函数                              */
/************************************************************************/
bool DirMonitor::start()
{
	DODAppThread::start();

	_hSetCallback = CreateEvent(NULL, false, false, NULL);
	if (_hSetCallback == NULL)
	{
		return false;
	}
	return false;
}

bool DirMonitor::init(void)
{
	if (!openHandles())
	{
		return false;
	}

	if(!configure())
	{
		return false;
	}

	DWORD bytesReturned;
	if (!setMonitorChanges(bytesReturned))
	{
		return false;
	}

	if (_pDirConsumer->_bSnapshot)
	{	
		if (!getDirectorySnapshot(_pDirConsumer->_monitorDirectory,
								  "", "*.*"))
		{
			return false;
		}

		if (!processDirectorySnapshot())
		{
			return false;
		}
	}

	return true;
}

int DirMonitor::run(void)
{
	HANDLE handles[2] = {_hStop, _hNotify} ;

	bool bContinued = true;
	DWORD waitStatus = 0;
	DWORD interval = _interval;
	while (bContinued)
	{
		waitStatus = WaitForMultipleObjectsEx(2, handles, false, interval, true);
		switch(waitStatus)
		{
		case WAIT_OBJECT_0:
			bContinued = false;
			break;
		
		case WAIT_OBJECT_0 + 1:
			processNotify();
			break;

		case WAIT_TIMEOUT:
			processTimeout();
			break;

		default:
			break;
		}
	}
	glog(ZQ::common::Log::L_DEBUG,
		"[monitorDir = %s] Exit DirMonitor::run()  OK",
		 _pDirConsumer->_monitorDirectory);	
	return 1;
}

/*     事件的相应     */
bool DirMonitor::processNotify()
{
	glog(Log::L_DEBUG,  "DirMonitor processNotify");

	if (!getDirectorySnapshot(_pDirConsumer->_monitorDirectory, "", "*.*"))
	{
		return false;
	}

	if (!processDirectorySnapshot())
	{
		return false;
	}
	return true;
}

bool DirMonitor::processDirectorySnapshot()
{
	MutexGuard guard(_mutex);
	
	glog(Log::L_DEBUG,  "DirMonitor processDirectorySnapshot");

	char temp[MAX_PATH];

	int count = _initialContentsQueue.size();
	for (int i = 0; i<count; i++)
	{
		strnset(temp, 0, MAX_PATH);
		strncpy(temp, _initialContentsQueue.front().c_str(), MAX_PATH - 1);
		_initialContentsQueue.pop();
	}
	return true;
}

/************************************************************************/
/*                 私有成员函数                                         */
/************************************************************************/
bool DirMonitor::setMonitorChanges(DWORD& bytesReturned)
{
	if (!ReadDirectoryChangesW(_directoryEvent,
							   _buffer,
							   MAX_PATH - 1,
							   _pDirConsumer->_bWatchSubdirectories,
							   _pDirConsumer->_notifyFilter,
							   &bytesReturned,
							   &_overlapped,
							   DirMonitor::HandleDirChanges))
	{
		glog(Log::L_DEBUG,  "DirMonitor  ReadDirectoryChangesW error <%d>", GetLastError());
		
		_bSetCallbackSucess = false;
		SetEvent(_hSetCallback);

		return false;
	}

	if (!_bSetCallbackSucess)
	{
		_bSetCallbackSucess = true;
		SetEvent(_hSetCallback);
	}
	return true;
}

VOID WINAPI
DirMonitor::HandleDirChanges(DWORD errorCode, DWORD bytes, LPOVERLAPPED pOverlapped)
{
	DirMonitor* pDirMon = (DirMonitor*)pOverlapped->hEvent;
	if (pDirMon)
	{
		DWORD offset;
		FILE_NOTIFY_INFORMATION* pFni = (FILE_NOTIFY_INFORMATION*)pDirMon->_buffer;
		
		do 
		{
			offset = pFni->NextEntryOffset;
			pDirMon->processChangedFile(pFni);
			pFni = (FILE_NOTIFY_INFORMATION*)((BYTE*)pFni + offset);
		} 
		while(offset);

		// 重置命令
		DWORD byteReturned;
		pDirMon->setMonitorChanges(byteReturned);
		memset(pDirMon->_buffer, 0, MAX_PATH);
	}
}

bool DirMonitor::processChangedFile(FILE_NOTIFY_INFORMATION* pFni)
{
	bool bRet = false;

	char changeFilename[MAX_PATH];
	strnset(changeFilename, 0, MAX_PATH);

	WideCharToMultiByte(CP_ACP, 0, pFni->FileName, -1, changeFilename, pFni->FileNameLength, NULL, NULL);

	char strtime[30];
	SYSTEMTIME time; 
	GetLocalTime(&time);
	sprintf(strtime,"%04d-%02d-%02d %02d:%02d:%02d:%03d",time.wYear, time.wMonth, 
		time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
//    printf("Folder  Chanage  %s \n", strtime);
	glog(Log::L_DEBUG,  
		"Folder  Chanage  dir = %s, time = %s ",
		_pDirConsumer->_monitorDirectory,strtime);

	switch(pFni->Action)
	{
	case FILE_ACTION_ADDED:
		if (_pDirConsumer && (_pDirConsumer->_dirOperation & DIR_ADD))
		{
			_pDirConsumer->notify();
		}
		break;

	case FILE_ACTION_REMOVED:
		if (_pDirConsumer && (_pDirConsumer->_dirOperation & DIR_DEL))
		{
			_pDirConsumer->notify();
		}
		break;

	case FILE_ACTION_MODIFIED:
		if (_pDirConsumer && (_pDirConsumer->_dirOperation & DIR_MODIFY))
		{
			_pDirConsumer->notify();
		}
		break;
	
	case FILE_ACTION_RENAMED_OLD_NAME:
		if (_pDirConsumer && (_pDirConsumer->_dirOperation & DIR_REN_OLD))
		{
			_pDirConsumer->notify();
		}
		break;
	
	case FILE_ACTION_RENAMED_NEW_NAME:
		if (_pDirConsumer && (_pDirConsumer->_dirOperation & DIR_REN_NEW))
		{
			_pDirConsumer->notify();
		}
		break;
	
	default:
		break;
	}
	return bRet;
}

bool DirMonitor::getDirectorySnapshot(const char* baseDirectory, 
									  const char* additionalPath,
									  const char* filter)
{
	MutexGuard guard(_mutex);

	char currentPath[MAX_PATH];
	strnset(currentPath, 0, MAX_PATH);
	sprintf(currentPath, "%s\\", baseDirectory);
//	glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot 当前路径<%s>", currentPath);

	if (strcmp(additionalPath, ""))
	{
		strcat(currentPath, additionalPath);
		strcat(currentPath, "\\");
//		glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot 当前路径<%s>", currentPath);
	}
	strcat(currentPath, filter);
//	glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot 当前路径<%s>", currentPath);

	char totalFile[MAX_PATH];
	strnset(totalFile, 0, MAX_PATH);

	WIN32_FIND_DATAA FindData;
	HANDLE hSearch = FindFirstFileA(currentPath, &FindData);
	if (INVALID_HANDLE_VALUE != hSearch)
	{
		char* curFile = FindData.cFileName;
		do 
		{
			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strnset(totalFile, 0, MAX_PATH);
				strcpy(totalFile, additionalPath);
				strcat(totalFile, "\\");
				strcat(totalFile, curFile);
//				glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot 文件相对路径<%s>", totalFile);

				if (fileFilter(totalFile))
				{
					_initialContentsQueue.push(totalFile);
				}
			}
			else if (!strcmp(curFile, "..") || !strcmp(curFile, ".") || !_pDirConsumer->_bWatchSubdirectories)
			{
//				glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot 忽略掉的文件夹 %s \n", curFile);
			}
			else
			{
				char nextLevel[MAX_PATH];
				strnset(nextLevel, 0, MAX_PATH);
				sprintf(nextLevel, "%s", additionalPath);
				if (strcmp(additionalPath, ""))
				{
					strcat(nextLevel, "\\");
				}
				strcat(nextLevel, curFile);
				
				if (!getDirectorySnapshot(baseDirectory, nextLevel, filter))
				{
					FindClose(hSearch);
					return false;
				}
			}
		} while(FindNextFileA(hSearch, &FindData));
	}
	else if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		glog(Log::L_DEBUG,  "DirMonitor  getDirectorySnapshot FindFirstFileA error <%d>", GetLastError());
		FindClose(hSearch);
		return false;
	}
	FindClose(hSearch);
	return true;
}

bool DirMonitor::openHandles()
{
	_hStop = CreateEvent(NULL, false, false, NULL);
	if (_hStop == NULL)
	{
		return false;
	}

	_hNotify = CreateEvent(NULL, false, false, NULL);
	if (_hNotify == NULL)
	{
		return false;
	}

	DWORD status = 0;
	status = WaitForSingleObject(_hSetCallback, INFINITE);
	if (status == WAIT_OBJECT_0)
	{
		if (_bSetCallbackSucess)
		{
			return true;
		}
	}
	return true;
}

void DirMonitor::closeHandles()
{
	if (_directoryEvent)
	{
		CloseHandle(_directoryEvent);
		_directoryEvent = NULL;
	}
	
	if (_hNotify)
	{
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}

	if (_hStop)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}

	if (_hSetCallback)
	{
		CloseHandle(_hSetCallback);
		_hSetCallback = NULL;
	}
}

bool DirConsumer::openHandles()
{
	_hStop = CreateEvent(NULL, false, false, NULL);
	if (_hStop == NULL)
	{
		return false;
	}

	_hNotify = CreateEvent(NULL, true ,false, NULL);
	if (_hNotify == NULL)
	{
		return false;
	}
	m_hWaitTime = CreateEvent(NULL, true,false, NULL);
	if (m_hWaitTime == NULL)
	{
		return false;
	}
	return true;
}

void DirConsumer::closeHandles()
{
	if (_hNotify)
	{
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}

	if (_hStop)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}

		if (m_hWaitTime)
	{
		CloseHandle(m_hWaitTime);
		m_hWaitTime = NULL;
	}
}
