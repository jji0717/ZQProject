// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/BaseCS/cspNTFS.cpp $
// 
// 7     4/27/12 3:40p Li.huang
// 
// 6     4/11/12 11:35a Li.huang
// add linux build
// 
// 5     4/02/12 8:59p Hui.shao
// 
// 4     1/18/12 6:24p Li.huang
// 
// 3     5/26/11 6:27p Hui.shao
// 
// 2     2/15/11 1:09p Fei.huang
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 13    10-04-19 15:31 Li.huang
// 
// 12    09-07-27 11:46 Fei.huang
// 
// 11    08-12-25 18:15 Fei.huang
// + linux support
// 
// 9     08-11-15 16:32 Hui.shao
// 
// 6     08-11-06 13:24 Hui.shao
// added new entry to test if content is in use
// 
// 5     08-11-03 11:54 Hui.shao
// 
// 4     08-11-03 11:37 Hui.shao
// 
// 3     08-10-29 15:58 Hui.shao
// 
// 2     08-10-07 19:54 Hui.shao
// added volume layer
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 5     08-08-11 18:42 Hui.shao
// added store replica handling
// 
// 4     08-07-31 17:20 Hui.shao
// added the portail enties for provisioning
// 
// 3     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 2     08-07-18 15:12 Hui.shao
// 
// 1     08-07-15 14:19 Hui.shao
// initial check in
// ===========================================================================
#include "../../../Common/FileSystemOp.h"
#include "ContentImpl.h"
#include "Guid.h"
#include "Log.h"
#include "CombString.h"
#include "TimeUtil.h"

#ifdef ZQ_OS_MSWIN

extern "C"
{
	#include <io.h>
};

#define READ_DIR_CHANGE_BUFFER_SIZE 4096
#define READ_DIR_CHANGE_FILETER	 \
        (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE \
		| FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY )
#define READ_DIR_CHANGE_SUBDIR	false
#define READ_DIR_CHANGE_INTERVAL (60*1000) // 1min
#define MAX_MONITOR_DIR_COUNT  (10)

#else

#include <unistd.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DEFAULT_WATCH_MASK (IN_CREATE|IN_MODIFY|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM)
#define BUFFER_SIZE 1024

#endif


namespace ZQTianShan {
namespace ContentStore {

#ifdef ZQ_OS_MSWIN

/// -----------------------------
/// declare class FilesystemSink
/// -----------------------------
class FilesystemSink : public ZQ::common::NativeThread {

public:

	FilesystemSink(ContentStoreImpl& store);
	virtual ~FilesystemSink();
	
	void quit();
	bool insertPathToMonitor(const char* path);

protected:

	virtual bool init(void);
	virtual int run(void);
	virtual void final();

	void wakeup();

	bool   _bQuit;
//	HANDLE _hDir;
	HANDLE _hCompletePort;
//	OVERLAPPED _overlapped;
//	char   _buffer[ READ_DIR_CHANGE_BUFFER_SIZE ]; //buffer for ReadDirectoryChangesW
//	DWORD  _dwBufLength; //length or returned data from ReadDirectoryChangesW -- ignored?...
//	std::string _lastFilename;
//	DWORD _lastActon;

	std::string _monitorRoot;
	TianShanIce::StrValues _monitorPaths;

	typedef struct _DirToMonitor
	{
		std::string path; //used to compare
		HANDLE		hDir;
		char		buf[READ_DIR_CHANGE_BUFFER_SIZE]; //buffer for ReadDirectoryChangesW
		OVERLAPPED	overlapped; // the key
		std::string lastFilename;
		DWORD lastActon;
	} DirToMonitor;

	DirToMonitor _monitorDirs[MAX_MONITOR_DIR_COUNT];
	size_t	 _monitorCount;
	typedef std::map <void*, size_t> OverlapToDirIdx; // the map of POVERLAPPED to the Idx of _monitorDirs
	OverlapToDirIdx _overlapIdx;

	ZQ::common::Mutex _monitorDirsLock;

public:

	ContentStoreImpl& _store;
};

//typedef struct _PortalCtxNTFS {
//	ContentStoreImpl* theStore;
//	FilesystemSink fsSink;
//} PortalCtxNTFS;

#else


class FilesystemSink {

    typedef std::vector<std::string> DIRS;

public:
	
	FilesystemSink(ContentStoreImpl&);
	~FilesystemSink();

public:

	void getMonitorDirs(DIRS& dirs) const {
        std::map<int, std::string>::const_iterator iter = _watchGroup.begin();
        for(; iter != _watchGroup.end(); ++iter) {
            dirs.push_back(iter->second); 
        }
	}

	void setWatchMask(unsigned mask) {
		_mask = mask;
	}

    void addMonitorDir(const std::string& dir) {
        std::string local = dir;
        if(local.at(local.length()-1) != '/') {
            local += '/';
        }     

        int wd = inotify_add_watch(_dirHandle, local.c_str(), _mask);

        if(wd < 0) {
            std::ostringstream os;
            os << "failed to add watch for (" << dir << "): " << strerror(errno);
            throw os.str().c_str();
        }
        _watchGroup[wd] = local;
    }
	
	bool filter(const std::string& name) const {
		if(name == "test") {
			return true;
		}
		return false;
	}

	void startWatch();
	void stopWatch();


public:

	void onFileAdded(const std::string& f) {
		std::cout << "file added: (" << f << ")" << std::endl;
        _store.OnFileEvent(TianShanIce::Storage::fseFileCreated, f, TianShanIce::Properties(), Ice::Current());
	}
	void onFileRenamed(const std::string& oldF, const std::string& newF) {
		std::cout << "file renamed: [old: (" << oldF << ") new: (" << newF << ")]" << std::endl;

        TianShanIce::Properties params;
        params.insert(TianShanIce::Properties::value_type("newFilename", newF));
        _store.OnFileEvent(::TianShanIce::Storage::fseFileRenamed, oldF, params, Ice::Current());
	}

    void onFileModified(const std::string& f) {
		std::cout << "file modified: (" << f << ")" << std::endl;
        _store.OnFileEvent(TianShanIce::Storage::fseFileModified, f, TianShanIce::Properties(), Ice::Current()); 
    }

	void onFileRemoved(const std::string& f) {
		std::cout << "file removed: (" << f << ")" << std::endl;
        _store.OnFileEvent(TianShanIce::Storage::fseFileDeleted, f, TianShanIce::Properties(), Ice::Current());
	}

private:

	unsigned _mask;
	int _dirHandle;
	bool _subTree;	
	bool _stopped;
	char* _eventBuffer;

private:

	static void* watch(void*);
	static void processChanges(FilesystemSink*, ssize_t);

	std::map<int, std::string> _watchGroup;    
    pthread_t _id;

public:

	ContentStoreImpl& _store;

}; 

#endif

void ContentStoreImpl::initializePortal(ContentStoreImpl& store) {
	if (store._ctxPortal) {
		return;
    }

	FilesystemSink* sink = new FilesystemSink(store);
	store._ctxPortal = (void*) sink;
	if (NULL == store._ctxPortal) {
		return;
    }
#ifdef ZQ_OS_LINUX
    sink->startWatch();
#endif
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store) {
	if (NULL != store._ctxPortal) {
		FilesystemSink* pFilesystemSink = reinterpret_cast<FilesystemSink*>(store._ctxPortal);
#ifdef ZQ_OS_MSWIN
		pFilesystemSink->quit();
#else
        try{
        pFilesystemSink->stopWatch();
        }
        catch(...) {throw;}
#endif
		delete ((FilesystemSink*) store._ctxPortal);
	}

	store._ctxPortal = 0;
}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname) {
	return pathname;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* volRootPath)
{
	freeMB = totalMB = 0;

	if (!volRootPath || strlen(volRootPath) <= 0) {
		return;
    }

	if (!store._ctxPortal) {
        return;
    }

	uint64 free=0, avail=0, total=0;

//#define _MY_DUMMY
#ifdef _MY_DUMMY
	if (!GetDiskFreeSpaceEx(volRootPath, (ULARGE_INTEGER*)&avail, (ULARGE_INTEGER*)&total, (ULARGE_INTEGER*)&free))
		return;

	totalMB = total/1000/1000;
	freeMB = free/1000/1000;
#else

	if(FS::getDiskSpace(volRootPath, avail, total, free)) {
		totalMB = total/1000/1000;
		freeMB = free/1000/1000;
	}
#endif // _MY_DUMMY
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType) {
	return true;
}


uint64 ContentStoreImpl::checkResidentialStatus(
            ContentStoreImpl& store, 
            uint64 flagsToTest, 
            const ContentImpl::Ptr pContent, 
            const std::string& fullName,
            const std::string& mainFilePathname) {

	std::string fileToCheck = fixupPathname(store, mainFilePathname); // the main file only in this portal

    uint64 ret = 0;
	if ((flagsToTest & RSDFLAG(frfResidential))) {
        FS::FileAttributes attr(fileToCheck);
        if(attr.exists()) {
			ret |= RSDFLAG(frfResidential);
        }
	}

	if (!ret) {
		return (ret & flagsToTest); 
    }

#ifdef ZQ_OS_MSWIN
    UINT uStyle =0;
    HFILE hFile = HFILE_ERROR;
    OFSTRUCT fdata;

	if (flagsToTest & RSDFLAG(frfWriting)) {
		uStyle = (OF_WRITE | OF_SHARE_EXCLUSIVE);
		hFile = ::OpenFile(fileToCheck.c_str(), &fdata, uStyle);
		if (HFILE_ERROR == hFile) {
			ret |= RSDFLAG(frfWriting);
		}
		else ::_lclose(hFile);
	}

	if (flagsToTest & RSDFLAG(frfReading)) {
		uStyle = (OF_READ | OF_SHARE_EXCLUSIVE);
		hFile = ::OpenFile(fileToCheck.c_str(), &fdata, uStyle);
		if (HFILE_ERROR == hFile) {
			ret |= RSDFLAG(frfReading);
		}
		else ::_lclose(hFile);
	}
#else

    struct stat query;
    if(stat(fileToCheck.c_str(), &query)) {
        fprintf(stderr, "failed to stat file %s\n", fileToCheck.c_str());
        return false;
    }

    DIR* procdir = opendir("/proc");
    if(!procdir) {
        fprintf(stderr, "%s\n", "failed to open /proc");
        return false;
    }

    struct dirent* proc = 0;
    pid_t mypid = getpid();
    while((proc = readdir(procdir)) != 0) {
        if(proc->d_name[0] < '0' || proc->d_name[0] > '9') {
            /* not a process entry */
            continue;
        }

        pid_t curr_pid = atoi(proc->d_name);
        if(curr_pid == mypid) {
            /* skip myself */
            continue;
        }

        char path[255]; 
        snprintf(path, 255, "/proc/%d/fd", curr_pid);
        DIR* tmpdir = opendir(path);
        if(!tmpdir) {
            continue;
        }

        char filepath[255];

        struct dirent* tmpent = 0;
        while((tmpent = readdir(tmpdir))!= 0) {
            snprintf(filepath, 255, "/proc/%d/fd/%s", curr_pid, tmpent->d_name);
            struct stat st;
            if(!stat(filepath, &st) && (st.st_ino == query.st_ino)) {
                struct stat st2;
                lstat(filepath, &st2);

                if(st2.st_mode & S_IRUSR) {
                    ret |= RSDFLAG(frfReading);
                    break;
                }
                if(st2.st_mode & S_IWUSR) {
                    ret |= RSDFLAG(frfWriting);
                    break;
                }
            }
        }
        closedir(tmpdir);

        if(ret) break;
    }
    closedir(procdir);
#endif

	return (ret & flagsToTest);
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath) 
// throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt)
{
	ContentStoreImpl::FileInfos infos;

	std::string searchFor = fixupPathname(store, rootPath ? rootPath : "");
    searchFor += FNSEPS;

    std::vector<std::string> files = FS::searchFiles(searchFor, "*.*");

    std::vector<std::string>::const_iterator iter = files.begin();
    for(; iter != files.end(); ++iter) {
        FS::FileAttributes attr(*iter);
        if(attr.isDirectory()) {
            continue;
        }

		ContentStoreImpl::FileInfo info;
		info.filename = attr.name();
        
        char buf[80];
        if(ZQ::common::TimeUtil::Time2Iso(attr.mtime(), buf, 80)) {
            info.stampLastWrite = buf;
        }

        infos.push_back(info);

    }

	return infos;
}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename) {
	return memberFilename;
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname) {
    FS::FileAttributes attr(pathOfVolume);

    /* create if not exists */
    if(!attr.exists()) {
        return FS::createDirectory(pathOfVolume);
    }

    if(!attr.isDirectory()) {
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, 
                    "createPathOfVolume() foldername[%s] conflict with existing file"), pathOfVolume.c_str());
		return false;
    }
    else {
		// start monitoring changes of this directory
		FilesystemSink* pFilesystemSink = reinterpret_cast<FilesystemSink*>(store._ctxPortal);
#ifdef ZQ_OS_MSWIN
		pFilesystemSink->insertPathToMonitor(pathOfVolume.c_str());
#else
		pFilesystemSink->addMonitorDir(pathOfVolume);
#endif
	}

	return true;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume) {
    FS::FileAttributes attr(pathOfVolume);

    if(!attr.exists()) {
        return true;
    }
    if(attr.exists() && !attr.isDirectory()) {
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, 
                    "createPathOfVolume() foldername[%s] conflict with existing file"), pathOfVolume.c_str());
		return false;
	}

	return FS::remove(pathOfVolume);
}

bool ContentStoreImpl::deleteFileByContent(
            ContentStoreImpl& store, 
            const ContentImpl& content, 
            const std::string& mainFilePathname) {

	FS::remove(mainFilePathname);

	return (0 == checkResidentialStatus(store, RSDFLAG(frfResidential), const_cast<ContentImpl *>(&content), "", mainFilePathname));
}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname) {
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStorePortal, 
                "populateAttrsFromFile() content[%s] popluate attributes from main file [%s]"), 
                content.ident.name.c_str(), mainFilePathname.c_str());

    FS::FileAttributes attr(mainFilePathname);
    
    if(!attr.exists() || attr.isDirectory()) {
		return false;
	}

	TianShanIce::Properties::const_iterator md = content.metaData.find(METADATA_FileSize);
    std::ostringstream oss;
    oss << attr.size();
    if(md == content.metaData.end()) {
        content.metaData.insert(TianShanIce::Properties::value_type(METADATA_FileSize, oss.str())); 
    }
    else {
        content.metaData[METADATA_FileSize] = oss.str();
    }
    
	return true;
}

bool ContentStoreImpl::completeRenaming(
            ContentStoreImpl& store, 
            const std::string& mainFilePathname, 
            const std::string& newPathname) {
	return (0 != checkResidentialStatus(store, RSDFLAG(frfResidential), NULL, "", newPathname));
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const std::string& contentName,
            const std::string& sourceUrl, 
            const std::string& sourceType, 
            const std::string& startTimeUTC, 
            const std::string& stopTimeUTC, 
            const int maxTransferBitrate)
//				throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
//                   ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
            
	return 0;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(
            ContentStoreImpl& store, 
            const ContentImpl& content, 
            const std::string& contentName,
            std::string& pushUrl, 
            const std::string& sourceType, 
            const std::string& startTimeUTC, 
            const std::string& stopTimeUTC, 
            const int maxTransferBitrate)
//			throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt)
{
	return (0);
}

std::string ContentStoreImpl::getExportURL(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const TianShanIce::ContentProvision::ProvisionContentKey& contentkey,
            const ::std::string& transferProtocol, 
            Ice::Int transferBitrate, 
            Ice::Int& ttl, 
            TianShanIce::Properties& params) {

    std::ostringstream oss;
    oss << "cifs://192.168.81.107" << content.identVolume.name << LOGIC_FNSEPC << content.ident.name;

    std::replace(oss.str().begin(), oss.str().end(), FNSEPC, LOGIC_FNSEPC);

	return oss.str();
}

void ContentStoreImpl::cancelProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const std::string& provisionTaskPrx) 
//            throw (::TianShanIce::ServerError, TianShanIce::InvalidStateOfArt)
{ 
}

void ContentStoreImpl::notifyReplicasChanged(
            ContentStoreImpl& store, 
            const TianShanIce::Replicas& replicasOld, 
            const TianShanIce::Replicas& replicasNew) {
}


/// -----------------------------
/// impl class FilesystemSink
/// -----------------------------
#ifdef ZQ_OS_MSWIN

FilesystemSink::FilesystemSink(ContentStoreImpl& store)
: _store(store), _bQuit(false), _hCompletePort(NULL), _monitorCount(0)
{
	ZQ::common::MutexGuard gd(_monitorDirsLock);
	for (size_t i=0; i < sizeof(_monitorDirs) / sizeof(DirToMonitor); i++)
	{
		_monitorDirs[i].hDir = NULL;
		memset(_monitorDirs[i].buf, 0x00, sizeof(_monitorDirs[i].buf));
		memset(&_monitorDirs[i].overlapped, 0x00, sizeof(_monitorDirs[i].overlapped));

		_overlapIdx.insert(OverlapToDirIdx::value_type(&_monitorDirs[i].overlapped, i));
	}
}

bool FilesystemSink::insertPathToMonitor(const char* path)
{
	if (NULL == path || strlen(path) <=0)
		return false;

	size_t i = 0;

	ZQ::common::MutexGuard gd(_monitorDirsLock);
	for (; i < _monitorCount; i++)
	{
		if (0 == strnicmp(path, _monitorDirs[i].path.c_str(), _monitorDirs[i].path.length()))
			break;
	}

	if (i < _monitorCount)
		return true; // already under monitoring

	if (_monitorCount >= sizeof(_monitorDirs) / sizeof(DirToMonitor))
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "insertPathToMonitor() no quota to monitor more directory"));
		return false;
	}

	_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FilesystemSink, "insertPathToMonitor() monitoring a new path[%s]"), path);

	_monitorDirs[_monitorCount].path = path;
	if (FNSEPC != _monitorDirs[_monitorCount].path[strlen(path) -1])
		_monitorDirs[_monitorCount].path +=FNSEPS;

	_monitorDirs[_monitorCount].hDir = ::CreateFile(path, 
								FILE_LIST_DIRECTORY, 
								FILE_SHARE_READ | FILE_SHARE_WRITE , //| FILE_SHARE_DELETE, <-- removing FILE_SHARE_DELETE prevents the user or someone else from renaming or deleting the watched directory. This is a good thing to prevent.
								NULL, //security attributes
								OPEN_EXISTING,
								FILE_FLAG_BACKUP_SEMANTICS | //<- the required priviliges for this flag are: SE_BACKUP_NAME and SE_RESTORE_NAME.  CPrivilegeEnabler takes care of that.
                                FILE_FLAG_OVERLAPPED, //OVERLAPPED!
								NULL);

	if( INVALID_HANDLE_VALUE == _monitorDirs[_monitorCount].hDir)
	{
		DWORD dwError = ::GetLastError();
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "insertPathToMonitor() couldn't open directory [%s] for monitoring. err:%d"), path, dwError);
		return false;
	}

	_hCompletePort = ::CreateIoCompletionPort(_monitorDirs[_monitorCount].hDir, _hCompletePort, (ULONG_PTR) &_monitorDirs[_monitorCount], 0);

	DWORD dwBufLength = sizeof(_monitorDirs[_monitorCount].buf);

	if( !::ReadDirectoryChangesW(_monitorDirs[_monitorCount].hDir, _monitorDirs[_monitorCount].buf, // FILE_NOTIFY_INFORMATION records are put into this buffer
		READ_DIR_CHANGE_BUFFER_SIZE, READ_DIR_CHANGE_SUBDIR, READ_DIR_CHANGE_FILETER,// needs to watch subdirectories
		&dwBufLength, //this var not set when using asynchronous mechanisms...
		&_monitorDirs[_monitorCount].overlapped, NULL) ) //no completion routine!
	{
		::CloseHandle(_monitorDirs[_monitorCount].hDir);
		_monitorDirs[_monitorCount].hDir = NULL;

		DWORD err = ::GetLastError();
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "insertPathToMonitor() ReadDirectoryChangesW(%s) error:%d"), path, err);
		return false;
	}

	if (0 == _monitorCount++)
		start();

	return true;
}

FilesystemSink::~FilesystemSink()
{
	quit();

	ZQ::common::MutexGuard gd(_monitorDirsLock);
	if( _hCompletePort )
		::CloseHandle( _hCompletePort );
	_hCompletePort = NULL;

	for (size_t i=0; i < sizeof(_monitorDirs) / sizeof(DirToMonitor); i++)
	{
		if( _monitorDirs[i].hDir && INVALID_HANDLE_VALUE != _monitorDirs[i].hDir)
			::CloseHandle( _monitorDirs[i].hDir );
		_monitorDirs[i].hDir = NULL;
	}
}
	
void FilesystemSink::quit()
{
	_bQuit = true;
	wakeup();
}

void FilesystemSink::wakeup()
{
	// wake up the thread and yield for other thread
	if (NULL != _hCompletePort)
	{
		DirToMonitor* pZero = & _monitorDirs[0];
		if (!::PostQueuedCompletionStatus(_hCompletePort, sizeof(pZero), (ULONG_PTR)pZero, NULL))
		{
			// error occured here
			DWORD dwError = ::GetLastError();
			_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "quit() PostQueuedCompletionStatus() error occured: %d"), dwError);
		}

		::Sleep(1);
	}
}

bool FilesystemSink::init(void)
{
	return (!_bQuit);
}

#ifndef min
#  define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

int FilesystemSink::run(void)
{
    DWORD nBytes =0;
	DirToMonitor* pDirChanged =NULL;
	LPOVERLAPPED pOverlapped =NULL;

	while (!_bQuit)
	{
		// Retrieve the directory info for this directory through the io port's completion key
		if( ! ::GetQueuedCompletionStatus(_hCompletePort, &nBytes,
			(PULONG_PTR) &pDirChanged,
			&pOverlapped,
			READ_DIR_CHANGE_INTERVAL) )
		{
			if (!_bQuit)
			{
				DWORD err = ::GetLastError();
				if (WAIT_TIMEOUT != err)
				{
					_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "run() GetQueuedCompletionStatus() error:%d"), err);
					::Sleep(200);
				}
			}

			pOverlapped =NULL;
		}

		if (_bQuit)
			break;

		std::string filename;

		if (NULL == pDirChanged || pDirChanged < &_monitorDirs[0] || pDirChanged > &_monitorDirs[_monitorCount] || pOverlapped != &pDirChanged->overlapped)
		{
			// TODO: log this is not the event we are watching
			pDirChanged = NULL;
			pOverlapped = NULL;
		}
		else
		{
			// process the event
			ZQ::common::MutexGuard gd(_monitorDirsLock);
			filename = pDirChanged->path;
			PFILE_NOTIFY_INFORMATION pCurrentRecord = (PFILE_NOTIFY_INFORMATION) pDirChanged->buf;
			while (!_bQuit && NULL != pCurrentRecord)
			{
				try {
					WCHAR wcFileName[ MAX_PATH + 1];
					int len = (min (MAX_PATH, pCurrentRecord->FileNameLength) +1) /sizeof(WCHAR);
					::memcpy(wcFileName, pCurrentRecord->FileName, len * sizeof(WCHAR));
					wcFileName[len] =0;
					filename = pDirChanged->path + (const char*) ZQ::common::CombString(wcFileName);
					::TianShanIce::Properties params;

					switch(pCurrentRecord->Action)
					{
					case FILE_ACTION_ADDED:		// a file was added!
						_store.OnFileEvent(::TianShanIce::Storage::fseFileCreated, filename, params, ::Ice::Current()); break;

					case FILE_ACTION_REMOVED:	//a file was removed
						_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, filename, params, ::Ice::Current()); break;

					case FILE_ACTION_MODIFIED:
						_store.OnFileEvent(::TianShanIce::Storage::fseFileModified, filename, params, ::Ice::Current()); break;

					case FILE_ACTION_RENAMED_OLD_NAME:
						pDirChanged->lastFilename = filename; break;

					case FILE_ACTION_RENAMED_NEW_NAME:
						if (FILE_ACTION_RENAMED_OLD_NAME == pDirChanged->lastActon && !pDirChanged->lastFilename.empty())
						{
							params.insert(::TianShanIce::Properties::value_type("newFilename", filename));
							_store.OnFileEvent(::TianShanIce::Storage::fseFileRenamed, pDirChanged->lastFilename, params, ::Ice::Current());
							pDirChanged->lastFilename = "";
						}
						break;

					default: //unknown action
						_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(FilesystemSink, "run() Unknown file action: %d"), pCurrentRecord->Action);
						break;
					}
				}
				catch(...)
				{
					_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(FilesystemSink, "run() caught unknown exception while processing file event"));
				}

				pDirChanged->lastActon = pCurrentRecord->Action;
				if (0 == pCurrentRecord->NextEntryOffset)
					break;

				pCurrentRecord = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)pCurrentRecord + pCurrentRecord->NextEntryOffset);
			}

			//	Changes have been processed, reissue the watch command
			DWORD dwBufLength = sizeof(pDirChanged->buf);
			if( ! ::ReadDirectoryChangesW(pDirChanged->hDir, pDirChanged->buf, // FILE_NOTIFY_INFORMATION records are put into this buffer
				READ_DIR_CHANGE_BUFFER_SIZE,
				READ_DIR_CHANGE_SUBDIR, // needs to watch subdirectories
				READ_DIR_CHANGE_FILETER,
				&dwBufLength, //this var not set when using asynchronous mechanisms...
				&pDirChanged->overlapped, NULL) )//no completion routine!
			{
				DWORD err = ::GetLastError();
				_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FilesystemSink, "run() ReadDirectoryChangesW error:%d"), err);
				::Sleep(100);
			}
		}
		
	}

	return 0; //thread is ending
}


void FilesystemSink::final()
{
}

#else

FilesystemSink::FilesystemSink(ContentStoreImpl& store)
:_mask(DEFAULT_WATCH_MASK), 
_dirHandle(-1),
_stopped(false),
_eventBuffer(0),
_store(store){

	_eventBuffer = new char[BUFFER_SIZE];	
	memset(_eventBuffer, '\0', BUFFER_SIZE);

	_dirHandle = inotify_init();

	if(_dirHandle < 0) {
        std::ostringstream os;
        os << "failed to init inotiry: " << strerror(errno);
		throw os.str().c_str();
	}
}

FilesystemSink::~FilesystemSink() {
}

void FilesystemSink::startWatch() {

	if(pthread_create(&_id, NULL, watch, (void*)this)) {
        std::ostringstream os;
        os << "failed to start watch thread: " << strerror(errno);
		throw os.str().c_str();
	}
}

void FilesystemSink::stopWatch() {
    std::map<int , std::string>::iterator iter = _watchGroup.begin();
    for(; iter != _watchGroup.end(); ++iter) {
        inotify_rm_watch(_dirHandle, iter->first);
    }

    void* res = 0;
    pthread_cancel(_id);
    pthread_join(_id, &res);

    if(res == PTHREAD_CANCELED) {
        _stopped = true;
    }

	if(_eventBuffer) {
		delete _eventBuffer;
		_eventBuffer = 0;
	}
}

void* FilesystemSink::watch(void* params) {
   // pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    FilesystemSink* This = (FilesystemSink*)params;

    try {
        while(ssize_t len = read(This->_dirHandle,  This->_eventBuffer, BUFFER_SIZE)) {
            processChanges(This, len);	
       } 
    }
    catch(...) {
        throw;
    }

    return (0);
}

void FilesystemSink::processChanges(FilesystemSink* This, ssize_t len) {

	char* offset = This->_eventBuffer;
	struct inotify_event* event = (inotify_event*)This->_eventBuffer;

    static std::string oldF;

	while(offset - This->_eventBuffer < len) {
		
        std::string name;

        std::map<int, std::string>::iterator iter = This->_watchGroup.find(event->wd);
        if(iter != This->_watchGroup.end()) {

            if(event->mask & IN_ISDIR) {

                name = iter->second + event->name + "/";

                if(event->mask & IN_CREATE) {
                    
                    std::cout << "add folder: " << name << std::endl;

                    int wd = inotify_add_watch(This->_dirHandle, name.c_str(), This->_mask);

                    This->_watchGroup[wd] = name; 
                }

                if(event->mask & IN_DELETE) {
                    std::cout << "folder delete: (" << name << ")" << std::endl;
                }

                return;
			}
            else {
                name = iter->second + event->name;
            }
		}
        else {
            break;
        }

		if(event->mask & IN_CREATE) {
			This->onFileAdded(name);
		}

        if(event->mask & IN_MODIFY) {
			This->onFileModified(name);
        }

		if(event->mask & IN_DELETE) {
			This->onFileRemoved(name);
		}

		if(event->mask & IN_MOVED_TO || event->mask & IN_MOVED_FROM) {

			if(event->mask & IN_MOVED_FROM) {
                oldF = name;
			}
			else {
                This->onFileRenamed(oldF, name);
                oldF.clear();
			}

		}

		size_t tmp = sizeof(inotify_event) + event->len;
		event = (inotify_event*)(offset + tmp);

		offset += tmp;
	}
}

#endif

}} // namespace
