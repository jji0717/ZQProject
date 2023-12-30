#include "FileSystemOp.h"
#include "ContentImpl.h"
#include "TimeUtil.h"
#include "ContentProvisionWrapper.h"
#include "MCCSCfg.h"
#include "ContentSysMD.h"

#include "NativeCS.h"
#include "CPHInc.h"
#include "ContentUser.h"

#ifndef min
#  define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

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
#endif

#define DEFAULT_WATCH_MASK (IN_CREATE|IN_MODIFY|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM)
#define BUFFER_SIZE 1024


namespace ZQTianShan {
namespace ContentStore {


class UrlGenerate
{
public:
	UrlGenerate(const std::string & urlExpr, ZQ::common::Log& log)
		:_urlExpr(urlExpr), _log(log)
	{
	}
	
	// <ContentStore ...>
	//     ...
	//		<ExportURL expression="ftp://192.168.81.23/${FOLDPATH}/${ContentName}" />
	// </ContentStore>
	// assume the full path of the current content is /$/folder1/folder2/contentA
	// then vol=$, ${FOLDPATH}="folder1/folder2/"; ${ContentName} = "contentA"
	// so the final output url= "ftp://192.168.81.23/folder1/folder2/contentA"
	//std::string             _urlExpr; // read from the configuration
	std::string operator()(const TianShanIce::ContentProvision::ProvisionContentKey& contentkey)
	{
		std::string url(_urlExpr);
        size_t sepratePos = _urlExpr.find("${");
		if (std::string::npos == sepratePos)
			return url;

		url.erase(sepratePos);
		while (std::string::npos != sepratePos)
		{
			fixUpFolder(url, contentkey.volume, sepratePos);
			fixUpContentName(url, contentkey.content, sepratePos);
			sepratePos = _urlExpr.find("${", sepratePos + 1);
		}

		return url;
	};

private:
	int fixUpFolder(std::string& urlResuslt, const std::string& volume, size_t sepratePos)
	{
		size_t markFolder = _urlExpr.find("FolderPath", sepratePos);
		size_t markPos = _urlExpr.rfind("${", markFolder);
		size_t notSpace = _urlExpr.find_first_not_of("  ", markPos + strlen("${"));
		size_t curlyBraces = _urlExpr.find_first_of("}", sepratePos);

		if (std::string::npos != markFolder && std::string::npos != markPos			
			&& notSpace == markFolder && markFolder < curlyBraces)
		{
            size_t rootPath = volume.find("$");
			if(std::string::npos != rootPath)
			{
				if (std::string::npos != volume.find("$/"))
					urlResuslt += volume.substr(rootPath + strlen("$/"), volume.length());
				else
                    urlResuslt += volume.substr(rootPath + strlen("$"), volume.length());
			}
		}

		return true;
	};
	
	int fixUpContentName(std::string& urlResuslt, const std::string& contentName, size_t sepratePos)
	{
		size_t markContentName = _urlExpr.find("ContentName", sepratePos);
		size_t markPos  = _urlExpr.rfind("${", markContentName);
		size_t notSpace = _urlExpr.find_first_not_of("  ", markPos + strlen("${"));
		size_t curlyBraces = _urlExpr.find_first_of("}", sepratePos);

		if (std::string::npos != markContentName && std::string::npos != markPos
			&& notSpace == markContentName && markContentName < curlyBraces)
		{
            if (urlResuslt.find_last_not_of(" ", urlResuslt.length()) != urlResuslt.rfind("/"))//if last is not "/"
				urlResuslt += "/";

			urlResuslt += contentName;
		}

		return true;
	};

private:
	ZQ::common::Log&    _log;
	const std::string&  _urlExpr;	
};


#ifdef ZQ_OS_MSWIN

/// -----------------------------
/// declare class FilesystemSink
/// -----------------------------
class FilesystemSink : public ZQ::common::NativeThread
{
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
	HANDLE _hCompletePort;
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

typedef struct __tagCtx
{
	FilesystemSink* sink;
	ContentProvisionWrapper* prov;
} Context;


NativeCS::NativeCS()
  :_urlExpr(configGroup.mccsConfig.urlExpr.expression), _isUrlGenActive(false)
{
	if (!_urlExpr.empty())
		_isUrlGenActive = true;
}

NativeCS::~NativeCS(){};

void NativeCS::initializePortal(ContentStoreImpl& store)
{
	//   printf("initializePortal: %p\n", &store._log);
	_ctx = new Context();
	_ctx->sink = new FilesystemSink(store);
	_ctx->prov = new ContentProvisionWrapper(store._log);

	assert(_ctx->sink != 0); 
	assert(_ctx->prov != 0);
	if (!_ctx) 
	{
		return;
	}

	Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);

	using namespace TianShanIce::Storage;
	ContentStoreExPrx csPrx = ContentStoreExPrx::uncheckedCast(store._adapter->createProxy(csIdent));		

	if (!_ctx->prov->init(store._adapter->getCommunicator(), 
		csPrx, configGroup.mccsConfig.cpcEndPoint, configGroup.mccsConfig.registerInterval)) 
	{		
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NativeCS, "initializePortal() init CPC failed"));
		return;
	}

	_ctx->prov->setTrickSpeeds(configGroup.mccsConfig.trickSpeedCollection);
	_ctx->prov->setNoTrickSpeedFileRegex(configGroup.mccsConfig.noTrickSpeeds.enable, configGroup.mccsConfig.noTrickSpeeds.expressionList);

#ifdef ZQ_OS_MSWIN
	_ctx->sink->start();
#else
	_ctx->sink->startWatch();
#endif
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NativeCS, "initializePortal() init succeed."));
}

void NativeCS::uninitializePortal(ContentStoreImpl& store) 
{
	if (_ctx) 
	{
		Context* ctx = _ctx;
		FilesystemSink* sink = _ctx->sink;
		ContentProvisionWrapper* prov = _ctx->prov;

		_ctx = NULL;
#ifdef ZQ_OS_MSWIN
		sink->quit();
		prov->unInit();
#else
		try{
			sink->stopWatch();
			prov->unInit();
		}
		catch(...) {
			throw;
		}
#endif//ZQ_OS_MSWIN
		delete prov;
		delete sink;
		delete (ctx);
	}

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NativeCS, "uninitializePortal() succeed."));
}

std::string NativeCS::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
}

void NativeCS::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* volRootPath)
{
	freeMB = totalMB = 0;
	if (!volRootPath || strlen(volRootPath) <= 0)
		return;

	if (!_ctx)
		return;

	uint64 free=0, avail=0, total=0;
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

bool NativeCS::validateMainFileName(ContentStoreImpl& store, std::string& contentName, const std::string& contentType)
{
	return true;
}


uint64 NativeCS::checkResidentialStatus(
	ContentStoreImpl& store, 
	uint64 flagsToTest, 
	const ContentImpl::Ptr pContent, 
	const std::string& fullName,
	const std::string& mainFilePathname) 
{
	//int64 funStartTime = ZQ::common::TimeUtil::now();
	std::string fileToCheck = fixupPathname(store, mainFilePathname); // the main file only in this portal

	uint64 ret = 0;
	if ((flagsToTest & RSDFLAG(frfResidential)))
	{
		FS::FileAttributes attr(fileToCheck);
		if(attr.exists()) 
		{
			ret |= RSDFLAG(frfResidential);
		}
	}

	if (!ret) 
	{
		return (ret & flagsToTest); 
	}

	store._log(ZQ::common::Log::L_DEBUG, LOGFMT("NativeCS checkResidentialStatus() checking file[%s]"), fileToCheck.c_str());
#ifdef ZQ_OS_MSWIN
	UINT uStyle =0;
	HFILE hFile = HFILE_ERROR;
	OFSTRUCT fdata;

	if (flagsToTest & RSDFLAG(frfWriting)) 
	{
		uStyle = (OF_WRITE | OF_SHARE_EXCLUSIVE);
		hFile = ::OpenFile(fileToCheck.c_str(), &fdata, uStyle);
		if (HFILE_ERROR == hFile) 
		{
			ret |= RSDFLAG(frfWriting);
		}
		else ::_lclose(hFile);
	}

	if (flagsToTest & RSDFLAG(frfReading)) 
	{
		uStyle = (OF_READ | OF_SHARE_EXCLUSIVE);
		hFile = ::OpenFile(fileToCheck.c_str(), &fdata, uStyle);
		if (HFILE_ERROR == hFile) 
		{
			ret |= RSDFLAG(frfReading);
		}
		else ::_lclose(hFile);
	}
#else
	struct stat query = {0};
	if(stat(fileToCheck.c_str(), &query))      //&& lstat(fileToCheck.c_str(), &st2))
	{
		store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to stat file %s"), fileToCheck.c_str());
		return false;
	}
	DIR* procdir = opendir("/proc");
	if(!procdir) 
	{
		store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to open the proc directory [/proc]"));
		return false;
	}
	struct dirent* proc = 0;
	pid_t mypid = getpid();
	int64 outLoop = 0;
	while((proc = readdir(procdir)) != 0) 
	{
		if(proc->d_name[0] < '0' || proc->d_name[0] > '9') 
		{
			/* not a process entry */
			continue;
		}
		pid_t curr_pid = atoi(proc->d_name);
		if(curr_pid == mypid)
		{
			/* skip myself */
			continue;
		}
		char path[255]; 
		snprintf(path, 255, "/proc/%d/fd", curr_pid);
		DIR* tmpdir = opendir(path);
		if(!tmpdir)
		{
			continue;
		}
		char filepath[255];
		struct dirent* tmpent = 0;
		while((tmpent = readdir(tmpdir))!= 0) 
		{
			snprintf(filepath, 255, "/proc/%d/fd/%s", curr_pid, tmpent->d_name);
			struct stat st;
			if(!stat(filepath, &st) && (st.st_dev == query.st_dev) && (st.st_ino == query.st_ino)) 
			{
				struct stat st2;
				lstat(filepath, &st2);
				if(st2.st_mode & S_IWUSR) 
				{
					outLoop |= RSDFLAG(frfWriting);
					store._log(ZQ::common::Log::L_DEBUG, LOGFMT("NativeCS file %s opened for writting by %d"), fileToCheck.c_str(), curr_pid);
				}
				if(st2.st_mode & S_IRUSR) 
				{
					outLoop |= RSDFLAG(frfReading);
					store._log(ZQ::common::Log::L_DEBUG, LOGFMT("NativeCS file %s opened for reading by %d"), fileToCheck.c_str(), curr_pid);
				}
				break;
			}
		}//while((tmpent = readdir(tmpdir))!= 0) 
		closedir(tmpdir);
		if(outLoop) break;
	}//while((proc = readdir(procdir)) != 0) 
	ret |= outLoop;
	closedir(procdir);
#endif //ZQ_OS_MSWIN
	//store._log(ZQ::common::Log::L_INFO, LOGFMT("NativeCS the ret is [%lld] ,flagsToTest is [%lld] ,using time [%d]ms"), ret, flagsToTest, (int)(ZQ::common::TimeUtil::now() - funStartTime));
	return (ret & flagsToTest);
}

ContentStoreImpl::FileInfos NativeCS::listMainFiles(ContentStoreImpl& store, const char* rootPath) 
{
	ContentStoreImpl::FileInfos infos;
	std::string searchFor = fixupPathname(store, rootPath ? rootPath : "");
	searchFor += FNSEPS;

	std::vector<std::string> files = FS::searchFiles(searchFor, "*");
	std::vector<std::string>::const_iterator iter = files.begin();
	for(; iter != files.end(); ++iter) 
	{
		FS::FileAttributes attr(*iter);
		if(attr.isDirectory()) 
		{
			continue;
		}

		ContentStoreImpl::FileInfo info;
		info.filename = attr.name();

		char buf[80];
		if(ZQ::common::TimeUtil::Time2Iso(attr.mtime(), buf, 80)) 
		{
			info.stampLastWrite = buf;
		}

		infos.push_back(info);
	}

	return infos;
}

std::string NativeCS::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename) 
{
	return memberFilename;
}

bool NativeCS::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname) 
{
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
		FilesystemSink* sink = _ctx->sink;
#ifdef ZQ_OS_MSWIN
		sink->insertPathToMonitor(pathOfVolume.c_str());
#else
		sink->addMonitorDir(pathOfVolume);
#endif
	}

	return true;
}

bool NativeCS::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume) 
{
	FS::FileAttributes attr(pathOfVolume);

	if(!attr.exists()) 
	{
		return true;
	}
	if(attr.exists() && !attr.isDirectory()) 
	{
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStorePortal, 
			"createPathOfVolume() foldername[%s] conflict with existing file"), pathOfVolume.c_str());
		return false;
	}

	return FS::remove(pathOfVolume);
}

bool NativeCS::deleteFileByContent(
	ContentStoreImpl& store, 
	const ContentImpl& content, 
	const std::string& mainFilePathname) 
{
		FS::remove(mainFilePathname);

		return (0 == checkResidentialStatus(store, RSDFLAG(frfResidential), const_cast<ContentImpl *>(&content), "", mainFilePathname));
}

bool NativeCS::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname) 
{
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStorePortal, 
		"populateAttrsFromFile() content[%s] popluate attributes from main file [%s]"), 
		content.ident.name.c_str(), mainFilePathname.c_str());

	FS::FileAttributes attr(mainFilePathname);

	if(!attr.exists() || attr.isDirectory()) 
	{
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

bool NativeCS::completeRenaming(
	ContentStoreImpl& store, 
	const std::string& mainFilePathname, 
	const std::string& newPathname) 
{
		return (0 != checkResidentialStatus(store, RSDFLAG(frfResidential), NULL, "", newPathname));
}

TianShanIce::ContentProvision::ProvisionSessionPrx NativeCS::submitProvision(
	ContentStoreImpl& store, 
	ContentImpl& content, 
	const std::string& contentName,
	const std::string& sourceUrl, 
	const std::string& sourceType, 
	const std::string& startTimeUTC, 
	const std::string& stopTimeUTC, 
	const int maxTransferBitrate) 
	//throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) 
{
	/* provision session exists */
	std::string strProvisionSess = content.provisionPrxStr;
	if(!strProvisionSess.empty())
	{
		TianShanIce::ContentProvision::ProvisionSessionPrx session;

		try {
			session = TianShanIce::ContentProvision::ProvisionSessionPrx::checkedCast(
				store._adapter->getCommunicator()->stringToProxy(strProvisionSess));
		}catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_WARNING, 
				LOGFMT("[%s] Open provision session[%s] for updateScheduledTime() caught exception[%s]"),
				contentName.c_str(), strProvisionSess.c_str(), ex.ice_name().c_str());
		}

		try {
			std::string start, stop;
			session->getScheduledTime(start, stop);

			// need to change the time to tianshan time to compare, because IM use localtime+timezero sometime, but we always use utc
			if(start != startTimeUTC || stop != stopTimeUTC) 
			{
				store._log(ZQ::common::Log::L_INFO, LOGFMT("[%s] update schedule time: [start (%s --> %s) stop (%s --> %s"),
					contentName.c_str(), start.c_str(), startTimeUTC.c_str(), stop.c_str(), stopTimeUTC.c_str());

				session->updateScheduledTime(startTimeUTC, stopTimeUTC);
			}
		} catch (const Ice::Exception& ex) {
			store._log(ZQ::common::Log::L_ERROR, LOGFMT("failed to update schedule time [start: (%s) stop: (%s)] for (%s): (%s)"),
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
	if (!transferBitrate) 
	{
		transferBitrate = configGroup.mccsConfig.defaultProvisionBW;
	}

	::TianShanIce::Properties sessMdata;
	{
		::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
		::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_IndexType);
		if (it!=metaDatas.end())
		{
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, it->second.c_str()));
		}
		else
		{
			if (stricmp(configGroup.mccsConfig.strDefaultIndexType.c_str(),"VVC") == 0)
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVC"));
			else
				sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_INDEXTYPE, "VVX"));
		}
		it = metaDatas.find(METADATA_ProviderId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERID, it->second));


		it = metaDatas.find(METADATA_ProviderAssetId);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PROVIDERASSETID, it->second));

		it = metaDatas.find(METADATA_AugmentationPIDs);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_AUGMENTATIONPIDS, it->second));

		it = metaDatas.find(METADATA_Preencryption);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_PREENCRYPTION, it->second));

		it = metaDatas.find(METADATA_WishedTrickSpeeds);
		if (it!=metaDatas.end())
			sessMdata.insert(::TianShanIce::Properties::value_type(CPHPM_WISHEDTRICKSPEEDS, it->second));
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	TianShanIce::Storage::ContentPrx contentPrx = 
		TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

	TianShanIce::ContentProvision::ProvisionContentKey contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper* cpWrapper = _ctx->prov;
	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = 0;

	try {
		if(store._storeAggregate)
		{
			pPrx = cpWrapper->activeProvision(contentPrx, contentKey, strFilePathName, sourceUrl, 
				sourceType,
				startTimeUTC, stopTimeUTC, transferBitrate,	sessMdata, false);
		}else{
			pPrx = cpWrapper->activeProvision(contentPrx, contentKey, strFilePathName, sourceUrl, 
				"NativeCS",
				startTimeUTC, stopTimeUTC, transferBitrate,	sessMdata, false);
		}
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

TianShanIce::ContentProvision::ProvisionSessionPrx NativeCS::bookPassiveProvision(
	ContentStoreImpl& store, 
	const ContentImpl& content, 
	const std::string& contentName,
	std::string& pushUrl, 
	const std::string& sourceType, 
	const std::string& startTimeUTC, 
	const std::string& stopTimeUTC, 
	const int maxTransferBitrate)
	//throw (TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) 
{
		std::string strFilePathName = content.getMainFilePathname(Ice::Current());

		TianShanIce::Storage::ContentPrx contentPrx = 
			TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));

		TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
		contentKey.content = contentName;
		contentKey.contentStoreNetId = store._netId;
		contentKey.volume = content.identVolume.name;

		ContentProvisionWrapper* cpWrapper = _ctx->prov;

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

std::string NativeCS::getExportURL(
	ContentStoreImpl& store, 
	ContentImpl& content, 
	const TianShanIce::ContentProvision::ProvisionContentKey& contentkey,
	const ::std::string& transferProtocol, 
	Ice::Int transferBitrate, 
	Ice::Int& ttl, 
	TianShanIce::Properties& params) 
{
		ContentProvisionWrapper* cpWrapper = _ctx->prov;
		int transBitrate = transferBitrate;
		int nTTL = 0;
		int permittedBitrate;

		if(transferProtocol != TianShanIce::Storage::potoFTP)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(store._log,
				EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol (%s) not supported"), transferProtocol.c_str() );
		}

		if (_isUrlGenActive)
		{
			UrlGenerate urlGenerater(_urlExpr, store._log);
			return urlGenerater(contentkey);
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
}

void NativeCS::cancelProvision(
	ContentStoreImpl& store, 
	ContentImpl& content, 
	const std::string& provisionTaskPrx) 
	//throw (::TianShanIce::ServerError, TianShanIce::InvalidStateOfArt) 
{ 
		ContentProvisionWrapper* cpWrapper = _ctx->prov;
		std::string contentName = content._name();

		cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

void NativeCS::notifyReplicasChanged(
	ContentStoreImpl& store, 
	const TianShanIce::Replicas& replicasOld, 
	const TianShanIce::Replicas& replicasNew) 
{
}

#ifdef ZQ_OS_MSWIN

FilesystemSink::FilesystemSink(ContentStoreImpl& store)
:_store(store), _bQuit(false), _hCompletePort(NULL), _monitorCount(0)
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
	{
		start();
	}

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

#else//ZQ_OS_MSWIN
FilesystemSink::FilesystemSink(ContentStoreImpl& store)
	:_mask(DEFAULT_WATCH_MASK), 
	_dirHandle(-1),
	_stopped(false),
	_eventBuffer(0),
	_store(store)
{
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

FilesystemSink::~FilesystemSink() 
{
}

void FilesystemSink::startWatch() 
{

	if(pthread_create(&_id, NULL, watch, (void*)this)) {
        std::ostringstream os;
        os << "failed to start watch thread: " << strerror(errno);
		throw os.str().c_str();
	}
}

void FilesystemSink::stopWatch()
{
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

void* FilesystemSink::watch(void* params)
{
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

void FilesystemSink::processChanges(FilesystemSink* This, ssize_t len) 
{
	char* offset = This->_eventBuffer;
	struct inotify_event* event = (inotify_event*)This->_eventBuffer;

	static std::string oldF;

	while(offset - This->_eventBuffer < len) 
	{
		std::string name;

		std::map<int, std::string>::iterator iter = This->_watchGroup.find(event->wd);
		if(iter != This->_watchGroup.end()) {

			if(event->mask & IN_ISDIR) {

				name = iter->second + event->name + "/";

				if(event->mask & IN_CREATE)
				{
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


#endif//ZQ_OS_MSWIN
}} 
