#include <HostToIP.h>
#include <WPCapThread.h>
#include <PacingInterface.h>

#include <CPHInc.h>
#include "AquaLibTargetFac.h"
#include <RTFProc.h>
#include <NICSelector.h>
#include <AquaFileIoFactory.h>
#include <AquaFileSetTarget.h>
#include <CStdFileIoFactory.h>

#include "CSILibAPI.h"
#include "CSILibErrors.h"
#include "CPH_AquaLib.h"
#include "AquaLibHelper.h"
#include "CPH_AquaLibCfg.h"
#include "ZQResource.h"

#include <CdmiFuseOps.h>
using namespace CSILib;

extern "C"
{
#ifdef ZQ_OS_LINUX
	#include <sys/mount.h>
	#include <dirent.h>  
#endif
}

#define MOLOG					(glog)

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#ifndef stricmp
#define stricmp strcasecmp
#endif

CdmiFuseOps* _pCdmiFuseOps = NULL;
NetworkIFSelector* _nNetSelector = NULL;
std::auto_ptr<FileIoFactory> CPHAquaLibSess::_pFileIoFac;
std::auto_ptr<FileIoFactory> CPHAquaLibSess::_pCifsFileIoFac;
BaseCPHelper* AquaLibHelper::_theHelper =NULL;

PacedIndexFactory*    CPHAquaLibSess::_pPacedIndexFac=NULL;
void*   CPHAquaLibSess::_pPacedIndexDll = NULL;

int64 SparseFileSize = 20000000000;

std::string _mountPath;
typedef struct 
{
	std::string mountPoint;
	int   nMount;
}MountPoint;

typedef std::map<std::string, MountPoint>  MOUNTURLS;
MOUNTURLS _mountURLs;
ZQ::common::Mutex   _lockMountURL;

ZQ::common::NativeThreadPool _cdmiThreadPool;
bool fixpath(std::string& path, bool bIsLocal = true)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;

	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';
	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
			*p = FNSEPC;
	}

	if (!bIsLocal && ':' == pathbuf[1])
		pathbuf[1] = '$';
	else if (bIsLocal && '$' == pathbuf[1])
		pathbuf[1] = ':';

	path = pathbuf;

	delete []pathbuf;
	return true;

}

/*
static unsigned long timeval()
{
	unsigned long rettime = 1;
	
	int64 ntime = ZQTianShan::now();
	ntime &= 0xFFFFFFFF;//keep only the low part
	rettime = (unsigned long)ntime;
	return rettime;
}
*/

void getUnifiedTrickExt(int speedNo, char* ext)
{
	if(0 == speedNo)
	{
		sprintf(ext, ".FFR");
	}
	else
	{
		sprintf(ext, ".FFR%d", speedNo);
	}		
}

void getTrickExt(int speedNo, char* ext1, char* ext2)
{
	if(0 == speedNo)
	{
		sprintf(ext1, ".FF");
		sprintf(ext2, ".FR");
	}
	else
	{
		sprintf(ext1, ".FF%d", speedNo);
		sprintf(ext2, ".FR%d", speedNo);
	}		
}

std::string replaceString(const std::string& inStr)
{
	std::string str = inStr;
	size_t pos = str.find('.');
	while ( std::string::npos != pos)
	{
		str[pos] = '#';
		pos = str.find('.');
	}
	return str;
}

std::string formatSpeed(std::list<float> trickspeeds)
{
	std::string strResult = "";
	for(std::list<float>::iterator itorSpeed = trickspeeds.begin();itorSpeed != trickspeeds.end(); itorSpeed++)
	{
		char buf[128]= "";
		sprintf(buf, "%.2f,", *itorSpeed);
		strResult += buf;
	}
	return strResult;
}
std::string formatSpeed(std::vector<float> trickspeeds)
{
	std::string strResult = "";
	for(std::vector<float>::iterator itorSpeed = trickspeeds.begin();itorSpeed != trickspeeds.end(); itorSpeed++)
	{
		char buf[128]= "";
		sprintf(buf, "%.2f,", *itorSpeed);
		strResult += buf;
	}
	return strResult;
}

int randstring(char* buffer, int bufsize) 
{ 
	if (buffer == 0) 
	{ 
		return 0; 
	} 

	srand(time(NULL) + rand());    
	for(int i = 0; i  < bufsize; i++) 
	{ 
		buffer[i] = rand()%26 + 97;  
	} 

	return bufsize; 
} 
#ifdef ZQ_OS_LINUX
int listdir(const char *path, TianShanIce::StrValues& subDirs) 
{  
	struct dirent *entry;  
	DIR *dp;  

	dp = opendir(path);  

	if (dp == NULL) {  
		return -1;  

	}  
	while((entry = readdir(dp)))  
	{
		if(stricmp(entry->d_name , ".") && stricmp(entry->d_name , "..") && entry->d_type == 4 )
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "list mount directory [%s%s] "),path, entry->d_name);

			subDirs.push_back(entry->d_name);  
		}
	}
	closedir(dp);  
	return 0;  
} 

bool mountURL(std::string strsharePath, std::string strsystype, std::string strOpt, std::string& szMountPoint, const std::string& strLogHint)
{
	MOUNTURLS::iterator itorMountURL;
	ZQ::common::MutexGuard guard(_lockMountURL);
	itorMountURL = _mountURLs.find(strsharePath);
	if(itorMountURL != _mountURLs.end())
	{
		(itorMountURL->second).nMount++;
		szMountPoint = (itorMountURL->second).mountPoint;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "[%s]mount[%s] to target[%s], mount count[%d]"),
			strLogHint.c_str(), strsharePath.c_str(),szMountPoint.c_str(),(itorMountURL->second).nMount);
	}
	else
	{
		char strRandomDir[11] = "";
		randstring(strRandomDir, sizeof(strRandomDir) -1);
		szMountPoint = _mountPath + std::string(strRandomDir);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "[%s]mount[%s] to target[%s] with option[%s]"),
			strLogHint.c_str(), strsharePath.c_str(),szMountPoint.c_str(),strOpt.c_str());
		CStdFileIoFactory::createDirectory(szMountPoint);
		int res = mount(strsharePath.c_str(),szMountPoint.c_str(),strsystype.c_str(),MS_RDONLY,(void*)strOpt.c_str());
		if (res < 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "[%s]Failed to mount[%s] to target[%s] with option[%s], errStr:[%s]"),
				strLogHint.c_str(), strsharePath.c_str(),szMountPoint.c_str(),strOpt.c_str(),strerror(errno));
			rmdir(szMountPoint.c_str());
			return false;
		}
		MountPoint mountpoint ;
		mountpoint.mountPoint = szMountPoint;
		mountpoint.nMount = 1;
		MAPSET(MOUNTURLS, _mountURLs, strsharePath, mountpoint);
	}
	return true;
}
bool umountURL(std::string sharePath, const std::string& strLogHint)
{
	MOUNTURLS::iterator itorMountURL;
	ZQ::common::MutexGuard guard(_lockMountURL);
	itorMountURL = _mountURLs.find(sharePath);
	if(itorMountURL != _mountURLs.end())
	{
		(itorMountURL->second).nMount--;
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaLib, "[%s]umount[%s] to target[%s], mount count[%d]"),
			strLogHint.c_str(), sharePath.c_str(),(itorMountURL->second).mountPoint.c_str(),(itorMountURL->second).nMount);
		if(itorMountURL->second.nMount <= 0)
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "[%s] umount[%s] to target[%s]"),
				strLogHint.c_str(), sharePath.c_str(),(itorMountURL->second).mountPoint.c_str());
			umount((itorMountURL->second).mountPoint.c_str());
			rmdir((itorMountURL->second).mountPoint.c_str());
			_mountURLs.erase(itorMountURL);
		}
	}
	return true;
}
#endif

void logCallback( CSI_SES_HANDLE hSession, const char *pClassString, char *pMessageString)
{

}
extern "C" __EXPORT bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return false;

	// set log handler
	ZQ::common::setGlogger(pEngine->getLogger());
	_gCPHCfg.setLogger(&glog);

	// load configurations
	std::string strCfgDir;
	strCfgDir = pEngine->getConfigDir();
	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	

	std::string strVersion = __STR1__(ZQ_PRODUCT_VER_MAJOR) "." __STR1__(ZQ_PRODUCT_VER_MINOR) "." __STR1__(ZQ_PRODUCT_VER_PATCH) "." __STR1__(ZQ_PRODUCT_VER_BUILD);
	MOLOG(Log::L_INFO,  CLOGFMT(CPH_AquaLib, "version[%s]"), strVersion.c_str());
    CStdFileIoFactory* pCstdFactory = new CStdFileIoFactory();
    CPHAquaLibSess::_pCifsFileIoFac.reset(pCstdFactory);
    CPHAquaLibSess::_pCifsFileIoFac->setLog(&glog);
    if (!CPHAquaLibSess::_pCifsFileIoFac->initialize())
    {
        std::string strErr;
        int nErrCode;
        CPHAquaLibSess::_pCifsFileIoFac->getLastError(strErr, nErrCode);
        printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
        return false;
    }

	FuseOpsConf fuseOpsConf;
	fuseOpsConf.enableCache =  _gCPHCfg.aquaCache.enable;
	fuseOpsConf.attrCache_childrenTTLsec = CACHED_FILE_INFO_TTL_SEC;
	fuseOpsConf.attrCache_TTLsec = CACHED_FILE_INFO_TTL_SEC;

	fuseOpsConf.tankConf.readBlockCount = _gCPHCfg.aquaCache.cacheBuffers;
	fuseOpsConf.tankConf.readCacheBlockSize = _gCPHCfg.aquaCache.cacheBuffersize;
	
	fuseOpsConf.tankConf.writeBlockCount = _gCPHCfg.aquaCache.cacheBuffersForwrite;

	fuseOpsConf.tankConf.writeBufferBlockSize = _gCPHCfg.aquaCache.cacheBuffersizeForwrite;

	fuseOpsConf.tankConf.logFlag = _gCPHCfg.aquaCache.cacheLogFlag;
	fuseOpsConf.tankConf.flushThreadPoolSize = _gCPHCfg.aquaCache.cacheFlushThreads;
	fuseOpsConf.tankConf.bufferInvalidationInterval = _gCPHCfg.aquaCache.cacheForceFlushInterval;

	fuseOpsConf.tankConf.cacheInvalidationInterval =3000;
	fuseOpsConf.tankConf.readAheadCount = _gCPHCfg.aquaCache.cacheReadAheadMax;
	fuseOpsConf.tankConf.readAheadThreshold = _gCPHCfg.aquaCache.cacheReadAheadTrigger;
	fuseOpsConf.tankConf.readAheadIncreamentLogBase = _gCPHCfg.aquaCache.cacheReadAheadPowerBase;
	fuseOpsConf.tankConf.mergableArrayMaxItemSize = _gCPHCfg.aquaCache.cacheReadAheadRecognitions;
	fuseOpsConf.tankConf.maxWriteQueueMergeItemCount= _gCPHCfg.aquaCache.cacheWriteSegmentsMax;

	fuseOpsConf.tankConf.writeThreadsOfYield = 0;
	fuseOpsConf.tankConf.writeYieldMax = 1000;
	fuseOpsConf.tankConf.writeYieldMin = 20 ;
	fuseOpsConf.tankConf.writeAvgWinSizeForYield = 3;
	fuseOpsConf.tankConf.writeBufferCountOfYield = 50;
	fuseOpsConf.attrCache_size = 10*1000;

	fuseOpsConf.tankConf.mergableArrayMaxItemSize = MIN( fuseOpsConf.tankConf.mergableArrayMaxItemSize, 1000);
	fuseOpsConf.tankConf.maxWriteQueueMergeItemCount = MIN( fuseOpsConf.tankConf.maxWriteQueueMergeItemCount, 20 );

	fuseOpsConf.tankConf.writeBufferCountOfYield = MIN(100, MAX(0,fuseOpsConf.tankConf.writeBufferCountOfYield));

	if( fuseOpsConf.tankConf.readAheadIncreamentLogBase < 2 )
		fuseOpsConf.tankConf.readAheadIncreamentLogBase = 2;

	if( fuseOpsConf.tankConf.readBlockCount < 2 )	
		fuseOpsConf.tankConf.readBlockCount = 2;

	if( fuseOpsConf.tankConf.readBlockCount > 1000* 1000 ) 
		fuseOpsConf.tankConf.readBlockCount = 1000 * 1000;

	if( fuseOpsConf.tankConf.writeBlockCount < 2 )
		fuseOpsConf.tankConf.writeBlockCount = 2;

	if( fuseOpsConf.tankConf.writeBlockCount > 1000* 1000 )
		fuseOpsConf.tankConf.writeBlockCount = 1000 * 1000;

	fuseOpsConf.tankConf.maxWriteQueueBufferCount = _gCPHCfg.aquaCache.cacheWriteLengthMax/fuseOpsConf.tankConf.writeBufferBlockSize;
	fuseOpsConf.tankConf.minWriteQueueBufferCount = _gCPHCfg.aquaCache.cacheWriteLengthMin/fuseOpsConf.tankConf.writeBufferBlockSize;

	if( fuseOpsConf.tankConf.readCacheBlockSize < 4096 )
		fuseOpsConf.tankConf.readCacheBlockSize = 4096;

	// childrenTTL should not be more than fileInfoTTL, otherwise will trigger many indivial file query when list dir
	if (fuseOpsConf.attrCache_childrenTTLsec > fuseOpsConf.attrCache_TTLsec)
		fuseOpsConf.attrCache_childrenTTLsec = fuseOpsConf.attrCache_TTLsec;
	if(fuseOpsConf.attrCache_size < 1000 ) {
		fuseOpsConf.attrCache_size = 1000;
	}

	_cdmiThreadPool.resize( _gCPHCfg.aquaServer.maxThreadPoolSize);
    _pCdmiFuseOps = new CdmiFuseOps(glog, _cdmiThreadPool, _gCPHCfg.aquaServer.rootUrl, _gCPHCfg.aquaServer.userDomain, _gCPHCfg.aquaServer.homeContainer, _gCPHCfg.aquaServer.flag, fuseOpsConf,_gCPHCfg.aquaServer.bindIp);
	AquaFileIoFactory* pFactory = new AquaFileIoFactory();
    pFactory->setCdmiOps(_pCdmiFuseOps);
	CPHAquaLibSess::_pFileIoFac.reset(pFactory);
	CPHAquaLibSess::_pFileIoFac->setLog(&glog);

	if (!CPHAquaLibSess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		CPHAquaLibSess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	AquaLibTargetFac * pTargetFac = new AquaLibTargetFac(CPHAquaLibSess::_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

    _nNetSelector = new NetworkIFSelector(glog);
    if(!_nNetSelector)
    {
        MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "Failed to new NetIfSelector object."));
        return false;
    }

    for (std::vector< NetInterface::NetInterfaceHolder >::iterator iter = _gCPHCfg.capture.interfaces.begin();
        iter != _gCPHCfg.capture.interfaces.end(); iter++)
    {
        _nNetSelector->addInterface((*iter).ip,(*iter).bandwidth);
    }

    NetworkIFSelector::InterfaceInfoList infoList;
    _nNetSelector->listInterfaceInfo(infoList);
    if (infoList.size() == 0)
    {
        MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "failed to load config of NetworkInterface for Capture, please check CPH_AquaLib.xml"));
        return false;
    }

    //multicast capture initialize
    {
        WinpCapThreadInterface* pCaptureInterface;
        pCaptureInterface = new WinpCapThreadInterface();
        pCaptureInterface->setKernelBufferBytes(_gCPHCfg.capture.winpcapKernelBuffer*1024*1024);
        pCaptureInterface->setMinBytesToCopy(_gCPHCfg.capture.winpcapMinBufferCopy*1024);

        MulticastCaptureInterface::setInstance(pCaptureInterface);
        pCaptureInterface->setLog(&glog);

        for(size_t i=0;i<_gCPHCfg.capture.interfaces.size();i++)
        {
            std::string strLocalIp;
            if (!HostToIP::translateHostIP(_gCPHCfg.capture.interfaces[i].ip.c_str(), strLocalIp))//translate host name to ip
                strLocalIp = _gCPHCfg.capture.interfaces[i].ip;

            pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.capture.interfaces[i].bandwidth);
        }

        if (!pCaptureInterface->init())
        {
            MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "Failed to initialize capture interface with error: %s"),
                pCaptureInterface->getLastError().c_str());
            return false;
        }
    }

	//
	// helper
	//
	if (!AquaLibHelper::_theHelper)
		AquaLibHelper::_theHelper = new AquaLibHelper(pEngine->getThreadPool(), pEngine);

	//
	// register methods
	//
    for (std::vector< Method::MethodHolder>::iterator iter = _gCPHCfg.provisionMethod.methods.begin();iter != _gCPHCfg.provisionMethod.methods.end();iter++)
	{
		if (iter->enableFlag && iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), AquaLibHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_AQUALIB, 0);

			MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}
#ifdef ZQ_OS_LINUX
	CPHAquaLibSess::_pPacedIndexDll = dlopen(_gCPHCfg.szPaceDllPath,RTLD_LAZY);
	if (!CPHAquaLibSess::_pPacedIndexDll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "%s failed to load"),_gCPHCfg.szPaceDllPath);
		return false;
	}
	typedef bool (*FunCreatePacedIndexFactory)(PacedIndexFactory**);
    FunCreatePacedIndexFactory _create;
    _create = (FunCreatePacedIndexFactory)dlsym(CPHAquaLibSess::_pPacedIndexDll,"CreatePacedIndexFactory");
	if (_create != NULL)
	{
		_create(&CPHAquaLibSess::_pPacedIndexFac);
		if (!CPHAquaLibSess::_pPacedIndexFac)
		{
			dlclose(CPHAquaLibSess::_pPacedIndexDll);
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "failed to create pacingFactory"));
			return false;
		}
		CPHAquaLibSess::_pPacedIndexFac->setLog(&glog);
		//CPHAquaLibSess::_pPacedIndexFac->setConfig("name","value");
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "Successfully load %s"),_gCPHCfg.szPaceDllPath);
	}
	else
	{
        std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		dlclose(CPHAquaLibSess::_pPacedIndexDll);
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "failed to get CreatePacedIndexFactory entry error[%s]"), strErr.c_str());
		return false;
	}

	_mountPath= _gCPHCfg.mountpath.entry;
	int nlen = _mountPath.size();
	if(nlen <=0 )
	{
      _mountPath = "/mnt/CPESvc/";
	  CStdFileIoFactory::createDirectory(_mountPath);
	}
	else
	if(_mountPath[nlen-1] != '/')
		_mountPath += "/";
	TianShanIce::StrValues subdirs;
	listdir(_mountPath.c_str(), subdirs);
	TianShanIce::StrValues::iterator itorSubdir = subdirs.begin();
	while(itorSubdir != subdirs.end())
	{
		std::string subPath = _mountPath + *itorSubdir;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "umount[%s] "),subPath.c_str());
		umount(subPath.c_str());
		rmdir(subPath.c_str());
		itorSubdir++;
	}
#endif
	DWORD logLevel = 7;
	CSI_ERROR error = csiInitializeLibrary( 10, logCallback, logLevel);
	if(error != CSI_MSG_SUCCESS)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI,"failed to Initialize Library"));
		return 0;
	}
	return true;
}

extern "C" __EXPORT void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
    for (std::vector<Method::MethodHolder>::iterator iter = _gCPHCfg.provisionMethod.methods.begin();iter != _gCPHCfg.provisionMethod.methods.end();iter++)
	{
		if ((*iter).enableFlag == 1)
		{
			pEngine->unregisterHelper((*iter).methodName.c_str(), pCtx);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (AquaLibHelper::_theHelper)
	{
		try
		{
			delete AquaLibHelper::_theHelper;
		}
		catch(...){};
		
		AquaLibHelper::_theHelper = NULL;
	}
	
    if (_nNetSelector)
    {
        delete _nNetSelector;
        _nNetSelector = NULL;
    }
    //
    //do some module uninitialize
    //
    MulticastCaptureInterface::destroyInstance();

	RTFProcess::uninitRTFLib();

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (CPHAquaLibSess::_pFileIoFac.get())
	{
		CPHAquaLibSess::_pFileIoFac->uninitialize();
		CPHAquaLibSess::_pFileIoFac.reset(0);
	}

    if (CPHAquaLibSess::_pCifsFileIoFac.get())
    {
        CPHAquaLibSess::_pCifsFileIoFac->uninitialize();
        CPHAquaLibSess::_pCifsFileIoFac.reset(0);
    }
	if(_pCdmiFuseOps)
		delete _pCdmiFuseOps;
	_pCdmiFuseOps = NULL;
#ifdef ZQ_OS_LINUX
	typedef bool (*DestroyPacedIndexFactory)(PacedIndexFactory*);
	DestroyPacedIndexFactory _pdestry = (DestroyPacedIndexFactory)dlsym(CPHAquaLibSess::_pPacedIndexDll,"DestroyPacedIndexFactory");
	if (_pdestry)
	{
		_pdestry(CPHAquaLibSess::_pPacedIndexFac);
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaLib, "failed to get DestroyPacedIndexFactory entry"));
	}
	if (CPHAquaLibSess::_pPacedIndexDll)
	{
		dlclose(CPHAquaLibSess::_pPacedIndexDll);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "Successfully unload %s"),_gCPHCfg.szPaceDllPath);
	}

	ZQ::common::MutexGuard guard(_lockMountURL);
	MOUNTURLS::iterator itorMountURL = _mountURLs.begin();
	while(itorMountURL != _mountURLs.end())
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaLib, "umount[%s] to target[%s]"),
			(itorMountURL->first).c_str(),(itorMountURL->second).mountPoint.c_str());
		umount((itorMountURL->second).mountPoint.c_str());
		rmdir((itorMountURL->second).mountPoint.c_str());
		itorMountURL++;
	}
#endif
	csiCloseLibrary();
//	ZQ::common::setGlogger(NULL);
}
