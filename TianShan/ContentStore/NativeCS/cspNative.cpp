#include "FileSystemOp.h"
#include "ContentImpl.h"
#include "TimeUtil.h"
#include "ProvWrapper.h"
#include "NativeServiceConfig.h"
#include "ContentSysMD.h"

#include <pthread.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DEFAULT_WATCH_MASK (IN_CREATE|IN_MODIFY|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM)
#define BUFFER_SIZE 1024


namespace ZQTianShan {
namespace ContentStore {


class FilesystemSink {

    typedef std::vector<std::string> DIRS;

public:
	
	FilesystemSink(ContentStoreImpl& store);
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
            ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
                _store._log,
                EXPFMT(FileSystemSink, csexpInternalError, 
                "failed to add watch for (%s): [%s]"), 
                dir.c_str(),
                strerror(errno)
            );
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
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FileSystemSink, "file added: (%s)"), f.c_str());
        _store.OnFileEvent(TianShanIce::Storage::fseFileCreated, f, TianShanIce::Properties(), Ice::Current());
	}
	void onFileRenamed(const std::string& oldF, const std::string& newF) {
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FileSystemSink, "file renamed: [old: (%s) new: (%s)]"),oldF.c_str(), newF.c_str());

        TianShanIce::Properties params;
        params.insert(TianShanIce::Properties::value_type("newFilename", newF));
        _store.OnFileEvent(::TianShanIce::Storage::fseFileRenamed, oldF, params, Ice::Current());
	}

    void onFileModified(const std::string& f) {
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FileSystemSink, "file modified: (%s)"), f.c_str());
        _store.OnFileEvent(TianShanIce::Storage::fseFileModified, f, TianShanIce::Properties(), Ice::Current()); 
    }

	void onFileRemoved(const std::string& f) {
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FileSystemSink, "file removed: (%s)"), f.c_str());
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

typedef struct __tagCtx {
    FilesystemSink* sink;
    ContentProvisionWrapper* prov;
} Context;

void ContentStoreImpl::initializePortal(ContentStoreImpl& store) {
	if (store._ctxPortal) {
		return;
    }
 //   printf("initializePortal: %p\n", &store._log);
    Context* ctx = new Context();
    ctx->sink = new FilesystemSink(store);
    ctx->prov = new ContentProvisionWrapper(store._log);

    assert(ctx->sink != 0); 
    assert(ctx->prov != 0);

	store._ctxPortal = (void*)ctx;
	if (!store._ctxPortal) {
		return;
    }

    Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);

    using namespace TianShanIce::Storage;
	ContentStoreExPrx csPrx = ContentStoreExPrx::uncheckedCast(store._adapter->createProxy(csIdent));		
	
	if (!ctx->prov->init(store._adapter->getCommunicator(), 
        csPrx, configGroup.mccsConfig.cpcEndPoint, configGroup.mccsConfig.registerInterval)) {		
		return;
	}	

	ctx->prov->setTrickSpeeds(configGroup.mccsConfig.trickSpeedCollection);
    ctx->sink->startWatch();
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store) {
	if (store._ctxPortal) {
		FilesystemSink* sink = static_cast<Context*>(store._ctxPortal)->sink;
        ContentProvisionWrapper* prov = static_cast<Context*>(store._ctxPortal)->prov;

        try{
            sink->stopWatch();
            delete sink;

            prov->unInit();
//            delete prov;
        }
        catch(...) {
            throw;
        }
	}

	store._ctxPortal = 0;
}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname) {
	return pathname;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* volRootPath) {
	freeMB = totalMB = 0;

	if (!volRootPath || strlen(volRootPath) <= 0)
		return;

	if (!store._ctxPortal)
        return;

	int64 free=0, avail=0, total=0;
	if(FS::getDiskSpace(volRootPath, free, avail, total))
	{
		// On Linux, free means total size of free blocks in the file system, while avail means
		// the total size of free blocks available to a non-privileged process.
		// we should take whichever is smaller as free space
		free = min(avail, free);
		
		totalMB = total/1000/1000;
		freeMB = free/1000/1000;
	}
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

    struct stat query;
    if(stat(fileToCheck.c_str(), &query)) {
        store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to stat file %s"), fileToCheck.c_str());
        return false;
    }

    DIR* procdir = opendir("/proc");
    if(!procdir) {
        store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to open /proc"));
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

	return (ret & flagsToTest);
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath) {
	ContentStoreImpl::FileInfos infos;

	std::string searchFor = fixupPathname(store, rootPath ? rootPath : "");
    searchFor += FNSEPS;

    std::vector<std::string> files = FS::searchFiles(searchFor, "*");

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

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume) {
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
		FilesystemSink* sink = static_cast<Context*>(store._ctxPortal)->sink;
		sink->addMonitorDir(pathOfVolume);
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
			throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) {

    /* provision session exists */
	std::string strProvisionSess = content.provisionPrxStr;
	if(!strProvisionSess.empty()) {
		TianShanIce::ContentProvision::ProvisionSessionPrx session;

		try {
 			session = TianShanIce::ContentProvision::ProvisionSessionPrx::checkedCast(
 				store._adapter->getCommunicator()->stringToProxy(strProvisionSess));
		}
		catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_WARNING, 
				LOGFMT("[%s] Open provision session[%s] for updateScheduledTime() caught exception[%s]"),
				contentName.c_str(), strProvisionSess.c_str(), ex.ice_name().c_str());
		}

		try {
			std::string start, stop;
			session->getScheduledTime(start, stop);

			//
			// need to change the time to tianshan time to compare, because IM use localtime+timezero sometime, but we always use utc
			//
			if(start != startTimeUTC || stop != stopTimeUTC) {
				store._log(ZQ::common::Log::L_INFO, 
					LOGFMT("[%s] update schedule time: [start (%s --> %s) stop (%s --> %s"),
					contentName.c_str(), start.c_str(), startTimeUTC.c_str(), stop.c_str(), stopTimeUTC.c_str());

				session->updateScheduledTime(startTimeUTC, stopTimeUTC);
			}
		} 
		catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to update schedule time [start: (%s) stop: (%s)] for (%s): (%s)"),
				startTimeUTC.c_str(), stopTimeUTC.c_str(), contentName.c_str(), ex.ice_name().c_str());

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(
				store._log,
				EXPFMT(MediaClusterCS, csexpInternalError, "failed to updateScheduledTime() with start[%s] stop[%s]"), 
				startTimeUTC.c_str(),
				stopTimeUTC.c_str()
            );
		}

		return session;
	}

	int transferBitrate = maxTransferBitrate;
	if (!transferBitrate) {
		transferBitrate = configGroup.mccsConfig.defaultProvisionBW;
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

    TianShanIce::Storage::ContentPrx contentPrx = 
        TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

	TianShanIce::ContentProvision::ProvisionContentKey contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;
	
	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = 0;

    try {
        pPrx = cpWrapper->activeProvision(
                contentPrx,
                contentKey,
                strFilePathName,	
                sourceUrl,
                sourceType, 
                startTimeUTC,
                stopTimeUTC, 
                transferBitrate,
                false);
    }
    catch(const TianShanIce::InvalidParameter& ex) {
        ex.ice_throw();
    }
    catch(const TianShanIce::InvalidStateOfArt& ex) {
        ex.ice_throw();
    }
    catch(const Ice::Exception& ex) {
        ZQTianShan::_IceThrow<TianShanIce::ServerError>(
            store._log,
            EXPFMT(NativeService, csexpInternalError, "failed to submit provision [%s]"), 
            ex.ice_name().c_str()
        );
    }
    
	return pPrx;
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
			throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) {

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	TianShanIce::Storage::ContentPrx contentPrx = 
            TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->passiveProvision(
		contentPrx,
		contentKey,
		strFilePathName,					  
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		maxTransferBitrate,
		pushUrl);

	return pPrx;
}

std::string ContentStoreImpl::getExportURL(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const TianShanIce::ContentProvision::ProvisionContentKey& contentkey,
            const ::std::string& transferProtocol, 
            Ice::Int transferBitrate, 
            Ice::Int& ttl, 
            TianShanIce::Properties& params) {

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;

	int transBitrate = transferBitrate;
	int nTTL = 0;
	int permittedBitrate;

	/* invalidate the protocol. */
	if(transferProtocol != TianShanIce::Storage::potoFTP){
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
			store._log,
			EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol (%s) not supported"), transferProtocol.c_str()
			);
	}

	std::string strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentkey, transBitrate, nTTL, permittedBitrate);

	ttl = nTTL;

	{
		std::ostringstream oss;
		oss << ttl;		
		params[expTTL] = oss.str();

		oss.str("");
		oss << permittedBitrate;
		params[expTransferBitrate] = oss.str();		
	}

	time_t now = time(0);
	char window[255];

	ZQ::common::TimeUtil::Time2Iso(now, window, 255);
	std::string stStart = window;
	params[expTimeWindowStart] = stStart;

	ZQ::common::TimeUtil::Time2Iso(now+ttl, window, 255);
	std::string stEnd = window;
	params[expTimeWindowEnd] = stEnd;


	store._log(ZQ::common::Log::L_DEBUG, 
		LOGFMT("(%s) getExportURL [URL: (%s) ttl: (%d) timeWindowStart: (%s) timeWindowEnd: (%s) bitrate: (%d)"), 
		contentkey.content.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;

/*
    std::ostringstream oss;
    oss << "cifs://192.168.81.107" << content.identVolume.name << LOGIC_FNSEPC << content.ident.name;

    std::replace(oss.str().begin(), oss.str().end(), FNSEPC, LOGIC_FNSEPC);

	return oss.str();
*/
}

void ContentStoreImpl::cancelProvision(
            ContentStoreImpl& store, 
            ContentImpl& content, 
            const std::string& provisionTaskPrx) 
            throw (::TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) { 

	ContentProvisionWrapper* cpWrapper = static_cast<Context*>(store._ctxPortal)->prov;
	
	std::string contentName = content._name();
	cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

void ContentStoreImpl::notifyReplicasChanged(
            ContentStoreImpl& store, 
            const TianShanIce::Replicas& replicasOld, 
            const TianShanIce::Replicas& replicasNew) {
}


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
        ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
            store._log,
            EXPFMT(FileSystemSink, csexpInternalError, 
            "failed to init inotify [%s]"), 
            strerror(errno)
        );
	}
}

FilesystemSink::~FilesystemSink() {
}

void FilesystemSink::startWatch() {

	if(pthread_create(&_id, NULL, watch, (void*)this)) {
        ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
            _store._log,
            EXPFMT(FileSystemSink, csexpInternalError, 
            "failed to start watch thread [%s]"), 
            strerror(errno)
        );
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
                    
//                    std::cout << "add folder: " << name << std::endl;

                    int wd = inotify_add_watch(This->_dirHandle, name.c_str(), This->_mask);

                    This->_watchGroup[wd] = name; 
                }

                if(event->mask & IN_DELETE) {
//                   std::cout << "folder delete: (" << name << ")" << std::endl;
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


}} 
