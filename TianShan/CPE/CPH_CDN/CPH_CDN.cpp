
#include "BaseClass.h"
#include "CPH_CDN.h"
#include "BaseClass.h"
#include "CPH_Cfg.h"
#include "RTFProc.h"
#include "ErrorCode.h"
#include "FTPSource.h"
#include "FTPFilesetSource.h"
#include "EncodeConvert.h"
#include <math.h>
#include <list>
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "CStdFileIoFactory.h"
#include "FTPMSClientFactory.h"
#include "FTPPropSource.h"
#include "HTTPPropSource.h"
#include "CIFSSource.h"
#include "C2PullSrc.h"
#include "SystemUtils.h"
#include "NormalizeSparseFile.h"
#include "Guid.h"
#include "TianShanDefines.h"
#ifdef ZQ_OS_LINUX
#include "CDNFileSetTarget.h"
extern "C"
{
#include <sys/mount.h>
#include <dirent.h>  
}
#else
#include "FilesetTarget.h"
#endif

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#ifdef ZQ_OS_LINUX
	#ifndef stricmp
		#define stricmp strcasecmp
	#endif
#endif


#define CPH_CDN			"CPH_CDN"
#define MOLOG					(glog)

std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> CDNSess::_pFileIoFac;
ZQTianShan::ContentProvision::BaseCPHelper* CDNHelper::_theHelper =NULL;
FTPMSClientFactory* CDNSess::_pFTPClientFactory = NULL;
HTTPClientFactory* CDNSess::_pHttpClientFactory = NULL;
#ifdef ZQ_OS_LINUX
PacedIndexFactory*       CDNSess::_pPacedIndexFac=NULL;
void*   CDNSess::_pPacedIndexDll=NULL;
#endif

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
#ifdef ZQ_OS_LINUX

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
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "list mount directory [%s%s] "),path, entry->d_name);

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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s]mount[%s] to target[%s], mount count[%d]"),
			strLogHint.c_str(), strsharePath.c_str(),szMountPoint.c_str(),(itorMountURL->second).nMount);
	}
	else
	{
		char strRandomDir[11] = "";
		randstring(strRandomDir, sizeof(strRandomDir) -1);
		szMountPoint = _mountPath + std::string(strRandomDir);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s]mount[%s] to target[%s] with option[%s]"),
			strLogHint.c_str(), strsharePath.c_str(),szMountPoint.c_str(),strOpt.c_str());
		CStdFileIoFactory::createDirectory(szMountPoint);
		int res = mount(strsharePath.c_str(),szMountPoint.c_str(),strsystype.c_str(),MS_RDONLY,(void*)strOpt.c_str());
		if (res < 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s]Failed to mount[%s] to target[%s] with option[%s], errStr:[%s]"),
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
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_CDN, "[%s]umount[%s] to target[%s], mount count[%d]"),
			strLogHint.c_str(), sharePath.c_str(),(itorMountURL->second).mountPoint.c_str(),(itorMountURL->second).nMount);
		if(itorMountURL->second.nMount <= 0)
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] umount[%s] to target[%s]"),
				strLogHint.c_str(), sharePath.c_str(),(itorMountURL->second).mountPoint.c_str());
			umount((itorMountURL->second).mountPoint.c_str());
			rmdir((itorMountURL->second).mountPoint.c_str());
			_mountURLs.erase(itorMountURL);
		}
	}
	return true;
}
#endif

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	
#ifdef ZQ_OS_MSWIN
	//
	// do some module initialize
	//
	std::string errmsg;
	if (_gCPHCfg.enableTestNTFS)
	{
		CStdFileIoFactory* pFactory = new CStdFileIoFactory();
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		CDNSess::_pFileIoFac.reset(pFactory);
	}
	else
	{
		//VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		//pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		//pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		//_pFileIoFac.reset(pFactory);
	}
#else
	CStdFileIoFactory* pFactory = new CStdFileIoFactory();
//	pFactory->setRootDir(_gCPHCfg.szTargetDir);
	pFactory->setFileSync(_gCPHCfg.filesync.maxSyncTimeout,_gCPHCfg.filesync.bytesToSync);
	CDNSess::_pFileIoFac.reset(pFactory);
#endif
	CDNSess::_pFileIoFac->setLog(&glog);
	if (!CDNSess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		CDNSess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(CDNSess::_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

	CDNSess::_pFTPClientFactory = new FTPMSClientFactory();
	if (!CDNSess::_pFTPClientFactory)
	{
		return false;	
	}
	CDNSess::_pFTPClientFactory->setLog(&glog);

	CDNSess::_pHttpClientFactory = new HTTPClientFactory();
	if (!CDNSess::_pHttpClientFactory)
	{
		return false;	
	}
	CDNSess::_pHttpClientFactory->setLog(&glog);

	//
	// helper
	//
	if (!CDNHelper::_theHelper)
		CDNHelper::_theHelper = new CDNHelper(pEngine->getThreadPool(), pEngine);

	//
	// register methods
	//
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->enableFlag && iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), CDNHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_CDN, 0);

			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}
#ifdef ZQ_OS_LINUX
	CDNSess::_pPacedIndexDll = dlopen(_gCPHCfg.szPaceDllPath,RTLD_LAZY);
	if (!CDNSess::_pPacedIndexDll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "%s failed to load"),_gCPHCfg.szPaceDllPath);
		return false;
	}
	typedef bool (*FunCreatePacedIndexFactory)(PacedIndexFactory**);
        FunCreatePacedIndexFactory _create;
        _create = (FunCreatePacedIndexFactory)dlsym(CDNSess::_pPacedIndexDll,"CreatePacedIndexFactory");
	if (_create != NULL)
	{
		_create(&CDNSess::_pPacedIndexFac);
		if (!CDNSess::_pPacedIndexFac)
		{
			dlclose(CDNSess::_pPacedIndexDll);
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "failed to create pacingFactory"));
			return false;
		}
		CDNSess::_pPacedIndexFac->setLog(&glog);
		//CDNSess::_pPacedIndexFac->setConfig("name","value");
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "Successfully load %s"),_gCPHCfg.szPaceDllPath);
	}
	else
	{
                std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		dlclose(CDNSess::_pPacedIndexDll);
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "failed to get CreatePacedIndexFactory entry error[%s]"), strErr.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "umount[%s] "),subPath.c_str());
		umount(subPath.c_str());
		rmdir(subPath.c_str());
		itorSubdir++;
	}
#endif

	return true;
}

extern "C" __EXPORT void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if ((*iter).enableFlag == 1)
		{
			pEngine->unregisterHelper((*iter).methodName.c_str(), pCtx);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (CDNHelper::_theHelper)
	{
		try
		{
			delete CDNHelper::_theHelper;
		}
		catch(...){};
		
		CDNHelper::_theHelper = NULL;
	}
	
	//
	//do some module uninitialize
	//
	RTFProcess::uninitRTFLib();

	if (CDNSess::_pFTPClientFactory)
	{
		try
		{
			delete CDNSess::_pFTPClientFactory;
		}
		catch(...){};

		CDNSess::_pFTPClientFactory = NULL;
	}

	if (CDNSess::_pHttpClientFactory)
	{
		try
		{
			delete CDNSess::_pHttpClientFactory;
		}
		catch(...){};

		CDNSess::_pHttpClientFactory = NULL;
	}

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (CDNSess::_pFileIoFac.get())
	{
		CDNSess::_pFileIoFac->uninitialize();
		CDNSess::_pFileIoFac.reset(0);
	}

#ifdef ZQ_OS_LINUX
	typedef bool (*DestroyPacedIndexFactory)(PacedIndexFactory*);
	DestroyPacedIndexFactory _pdestry = (DestroyPacedIndexFactory)dlsym(CDNSess::_pPacedIndexDll,"DestroyPacedIndexFactory");
	if (_pdestry)
	{
		_pdestry(CDNSess::_pPacedIndexFac);
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "failed to get DestroyPacedIndexFactory entry"));
	}
	if (CDNSess::_pPacedIndexDll)
	{
		dlclose(CDNSess::_pPacedIndexDll);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "Successfully unload %s"),_gCPHCfg.szPaceDllPath);
	}

	ZQ::common::MutexGuard guard(_lockMountURL);
	MOUNTURLS::iterator itorMountURL = _mountURLs.begin();
	while(itorMountURL != _mountURLs.end())
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "umount[%s] to target[%s]"),
			(itorMountURL->first).c_str(),(itorMountURL->second).mountPoint.c_str());
		umount((itorMountURL->second).mountPoint.c_str());
		rmdir((itorMountURL->second).mountPoint.c_str());
		itorMountURL++;
	}
#endif

//	ZQ::common::setGlogger(NULL);
}

#include "urlstr.h"

static bool fixpath(std::string& path, bool bIsLocal = true)
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
CDNSess::~CDNSess()
{
	cleanup();
}

void CDNSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool CDNSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	_pRTFProc = NULL;
	_strMethod = _sess->methodType;
	if(stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation) 
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_FTPRTF)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_FTPRTFH264)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_NTFSRTF)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_NTFSRTFH264)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_C2Pull)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_C2PullH264)
		&& stricmp(_strMethod.c_str(),METHODTYPE_CDN_HTTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			_strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	bool bH264Type = false;
	if (stricmp(_strMethod.c_str(),METHODTYPE_CDN_FTPRTFH264) == 0 || stricmp(_strMethod.c_str(),METHODTYPE_CDN_NTFSRTFH264) == 0
		|| stricmp(_strMethod.c_str(),METHODTYPE_CDN_C2PullH264) == 0)
	{
		bH264Type = true;
	}
	std::string strFilename;
	std::string strSrcUrl;
	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	strFilename = var.strs[0];

	if (resURI.end() != resURI.find(CPHPM_SOURCEURL))
	{
		TianShanIce::Variant& var2 = resURI[CPHPM_SOURCEURL];
		if (var2.type == TianShanIce::vtStrings && var2.strs.size()>0)
		{
			strSrcUrl = var2.strs[0];
		}
	}

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
    int nBandwidth = (int)var1.lints[0];
    _nBandwidth = nBandwidth;

	bool bIndexVVC = false;
	std::string providerId,providerAssetId;
	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);
	if (it!=prop.end())
	{
		if (stricmp(it->second.c_str(),"VVC") == 0)
		{
			bIndexVVC = true;				
		}
	}
//	if (bIndexVVC)
//	{
		it = prop.find(CPHPM_PROVIDERID);
		if (it!=prop.end())
			providerId = it->second;

		it = prop.find(CPHPM_PROVIDERASSETID);
		if (it!=prop.end())
			providerAssetId = it->second;
//	}

	int augmentationPidCount = 0;
	uint16 augmentationPids[ MAX_AUGMENTATION_PIDS ];
	memset((void*) augmentationPids, 0 , sizeof(augmentationPids));

	std::string strPIDs = "";
	it = prop.find(CPHPM_AUGMENTATIONPIDS);
	if (it!=prop.end())
	{
		strPIDs = it->second;
	}
	else
	{
		strPIDs = _gCPHCfg.strAugmentationPids;
	}
    if(!strPIDs.empty())
	{
		::TianShanIce::StrValues  strAugmentationPids;
		::ZQ::common::stringHelper::SplitString(strPIDs, strAugmentationPids, ";,");
		int i;
		for(i = 0; i < strAugmentationPids.size() && i < MAX_AUGMENTATION_PIDS; ++i)
		{
			augmentationPids[i] = atoi(strAugmentationPids[i].c_str());
		}
		augmentationPidCount = strAugmentationPids.size();
	}

	bool enableNoTrickSpeed = false;
	it = prop.find(CPHPM_NOTRICKSPEEDS);
	if (it != prop.end())
	{
		int major = 0, minor = 0;
		RTFProcess::getCTFLibVersion(major, minor);
		if(major < 3)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(CPH_CDN, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
		}
		enableNoTrickSpeed = true;
	}


	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	if (_gCPHCfg.bandwidthLimitRate)
	{
		nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
	}
	else
		nMaxBandwidth = nBandwidth;
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_CDN, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation))
	{
		FTPPropSource* pSource = new FTPPropSource(_pFTPClientFactory,_pFileIoFac.get());
		if (!pSource)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] create FTP source failed"), _strLogHint.c_str());
			return false;
		}
		AddFilter(pSource);
		pSource->setURL(strSrcUrl.c_str());
		pSource->setTargetDir(_gCPHCfg.szTargetDir);
		pSource->setLog(_pLog);
		pSource->setLocalNetIf(_gCPHCfg.szLocalNetIf);
		pSource->setCacheDir(_gCPHCfg.szCacheDir);
		pSource->setFilename(strFilename.c_str());
		pSource->setMaxBandwidth(nMaxBandwidth);
		pSource->setEnableResumeDownload(_gCPHCfg.enableResumeForDownload);
		//pSource->setBandwidthCtrlInterval(_gCPHCfg.bandwidthCtrlInterval);
		pSource->setWaitTimeForGrowing(_gCPHCfg.timeoutForGrowing);
		pSource->enableMD5(true);	
		pSource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
	}
	else if(!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		std::string transferIP = _gCPHCfg.transferip;

		ZQ::common::URLStr srcUrl(strSrcUrl.c_str());
		std::string fileset = srcUrl.getVar("fileset");
		if(!stricmp(fileset.c_str(), "true"))
			transferIP = _gCPHCfg.transferIpForCache; //for cache gateway

		HTTPPropSource* pSource = new HTTPPropSource(_pHttpClientFactory,_pFileIoFac.get());
		if (!pSource)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] create HTTP source failed"), _strLogHint.c_str());
			return false;
		}
		AddFilter(pSource);
		pSource->setURL(strSrcUrl.c_str());
		pSource->setLog(_pLog);
		pSource->setLocalNetIf(_gCPHCfg.szLocalNetIf);
		pSource->setCacheDir(_gCPHCfg.szCacheDir);
		pSource->setFilename(strFilename.c_str());
		pSource->setMaxBandwidth(nMaxBandwidth);
		pSource->setEnableResumeDownload(_gCPHCfg.enableResumeForDownload);
		//pSource->setBandwidthCtrlInterval(_gCPHCfg.bandwidthCtrlInterval);
		pSource->setWaitTimeForGrowing(_gCPHCfg.timeoutForGrowing);
		pSource->enableMD5(true);	
		pSource->setPIDAndPAID(providerId, providerAssetId);
		pSource->setIngressCapcaity(_gCPHCfg.ingressCapcaity * 1000);
		pSource->setLocateFileIP(_gCPHCfg.bindip, transferIP);
		pSource->setTransferServerPort(_gCPHCfg.transferPort);
		pSource->setTransferDelay(_gCPHCfg.transferdelay);
		pSource->setSpeed(_gCPHCfg.nspeed);
		pSource->setTimeout(_gCPHCfg.timeout);
		pSource->setSleeptime(_gCPHCfg.sleeptime.timeInterval);
	}
	else
	{
		std::list<float> trickspeed;
		std::list<float> trickspeedHD;

		::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;

		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_CDN, "trickSpeed from session [%s]"), formatSpeed(trickcol).c_str());

		if (trickcol.size() == 0 || (trickcol.size() == 1 && fabs(trickcol[0]) <=1e-6 )) //add compare folat 
			trickcol.push_back(7.5);
      
		bool bFoundHD = false;
		for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
		{
			if(fabs(*iterTrick) <=1e-6)
			{
				bFoundHD = true;
				continue;
			}
			if(bFoundHD)
			{
				trickspeedHD.push_back((*iterTrick));
			}
			else
				trickspeed.push_back((*iterTrick));		
		}

		if(trickspeed.empty() && trickspeedHD.size() > 0)
		{
			trickspeed.assign(trickspeedHD.begin(), trickspeedHD.end());
		}

		if(!trickspeed.empty() && !trickspeedHD.empty() && trickspeed.size() != trickspeedHD.size())
		{
			trickspeed.sort();
			trickspeedHD.sort();

			if(trickspeed.size() < trickspeedHD.size())
			{
				int npos = trickspeedHD.size() - trickspeed.size();
				std::list<float>::iterator itorSpeed =  trickspeedHD.begin();
				for(uint i = 0; i < trickspeedHD.size() - npos ; i++)
				{
					itorSpeed++;
				}
				for(uint i = 0; i < npos ; i++)
				{
					trickspeed.push_back(*itorSpeed);
					itorSpeed++;
				}
			}
			else
			{
				int npos = trickspeed.size() - trickspeedHD.size();
				std::list<float>::iterator itorSpeed =  trickspeed.begin();
				for(uint i = 0; i < trickspeed.size() - npos ; i++)
				{
					itorSpeed++;
				}
				for(uint i = 0; i < npos ; i++)
				{
					trickspeedHD.push_back(*itorSpeed);
					itorSpeed++;
				}
			}
			trickspeed.sort();
			trickspeedHD.sort();
		}
		
		_bPushTrigger = false;

		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] sourceUrl[%s], SD trickspeed [%s], HD trickSpeed[%s]"),
			_strLogHint.c_str(),strSrcUrl.c_str(), formatSpeed(trickspeed).c_str(), formatSpeed(trickspeedHD).c_str());

		std::string url = strSrcUrl;
		ZQ::common::URLStr Url(strSrcUrl.c_str());
		BaseSource *pSource= NULL;

		if (0 ==stricmp("ftp", Url.getProtocol()))
		{
			FTPIOSource* ftpSource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP,  &_helper._pool);
			AddFilter(ftpSource);    //only after this, the log handle will be parsed in
			ftpSource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
			ftpSource->setURL(url.c_str());
			ftpSource->setMaxBandwidth(nMaxBandwidth);
			ftpSource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
			if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
			{
				ftpSource->setSourceUrlUTF8(true);
			}
			pSource = ftpSource;
		}
		else if(0 ==stricmp("cifs", Url.getProtocol()) || 0 ==stricmp("file", Url.getProtocol()))
		{
			std::string host = Url.getHost();
			std::string sourceFilename = Url.getPath();
			std::string strOpt,strsystype,strsharePath;
			bool bLocalSrcFlag;
			strsystype = "cifs";
			if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
			{
				bLocalSrcFlag = true;
				fixpath(sourceFilename, true);
				strOpt = "username=,password=";
			}
			else
			{
				bLocalSrcFlag = false;
				fixpath(sourceFilename, false);
				strsharePath =std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + Url.getHost() + LOGIC_FNSEPS + sourceFilename.substr(0,sourceFilename.find_first_of(FNSEPC));
				sourceFilename = sourceFilename.substr(sourceFilename.find_first_of(FNSEPC)+1);
				fixpath(sourceFilename, false);
				strOpt = "username=" + std::string(Url.getUserName()) + ",password=" + std::string(Url.getPwd());
			}

			CIFSIOSource* fsSource = (CIFSIOSource*)SourceFactory::Create(SOURCE_TYPE_CIFS, &_helper._pool);
			if (!fsSource)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] create CIFS source failed"), _strLogHint.c_str());
				return false;
			}
			AddFilter(fsSource);

			if (!stricmp(_gCPHCfg.szUrlEncode, "utf-8"))
			{
				fsSource->setSourceUrlUTF8(true);
			}
			fsSource->setIOFactory(_pFileIoFac.get());
			fsSource->setFileName(sourceFilename.c_str());
			fsSource->setMaxBandwidth(nMaxBandwidth);
			fsSource->setMountOpt(strOpt);
			fsSource->setSystemType(strsystype);
			fsSource->setLocalSourceFlag(bLocalSrcFlag);
			if (!bLocalSrcFlag)
			{
				fsSource->setSharePath(strsharePath);
#ifdef ZQ_OS_LINUX
				std::string strMountPoint;
				bool bMount = mountURL(strsharePath, strsystype, strOpt, strMountPoint, _strLogHint);
				if(!bMount)
					return false;
				_bSuccessMount = true;
				_sharePath = strsharePath;
				fsSource->setMountPath(strMountPoint);

#endif
			}
			pSource = fsSource;
		}
		else if(0 ==stricmp("c2pull", Url.getProtocol()))
		{
			if(strFilename.size() == 0)
			{
				strFilename = providerAssetId + providerId;
			}

			C2PullSource* c2pullSource = (C2PullSource*)SourceFactory::Create(SOURCE_TYPE_C2PULL, &_helper._pool);
			if (!c2pullSource)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] create C2PULL source failed"), _strLogHint.c_str());
				return false;
			}
			
			AddFilter(c2pullSource);
			c2pullSource->setCacheDir(_gCPHCfg.szCacheDir);
			c2pullSource->setFilename(strFilename.c_str());
			c2pullSource->setHttpClientFac(_pHttpClientFactory);
            c2pullSource->setURL(strSrcUrl.c_str());
			c2pullSource->setPIDAndPAID(providerId, providerAssetId);
			c2pullSource->setIngressCapcaity(_gCPHCfg.ingressCapcaity * 1000);
			c2pullSource->setLocateFileIP(_gCPHCfg.bindip, _gCPHCfg.transferip);
			c2pullSource->setTransferServerPort(_gCPHCfg.transferPort);
			c2pullSource->setTransferDelay(_gCPHCfg.transferdelay);
			c2pullSource->setSpeed(_gCPHCfg.nspeed);
			c2pullSource->setTimeout(_gCPHCfg.timeout);
            c2pullSource->setMaxBandwidth(nMaxBandwidth);

			pSource = c2pullSource;

		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] url protocol[%s] not support"), _strLogHint.c_str(),Url.getProtocol());
			return false;
		}

		ZQ::common::MD5ChecksumUtil md5;
		md5.checksum(strFilename.c_str(), strFilename.length());
		std::string strMd5 = md5.lastChecksum();

		unsigned short uFileCiscoExt = atoi(strMd5.substr(14, 2).c_str());
	    uFileCiscoExt &= 0xfff0;

		transform (strMd5.begin(), strMd5.end(), strMd5.begin(), (int(*)(int))toupper);
		std::string strFileCiscoExt = strMd5.substr(0, 14);

		char buf[64] = "";
		memset(buf, 0, 64);
		snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
		std::string mainFileExt="";
		std::string replacePAID = replaceString(providerAssetId);
		std::string replacePID = replaceString(providerId);
		if( 1 == _gCPHCfg.ciscofileext.mode)
		{
			mainFileExt = std::string(buf);
		}
		else if( 2 == _gCPHCfg.ciscofileext.mode)
		{
			memset(buf, 0, 64);
			snprintf(buf, sizeof(buf) - 2, ".00%s_%s\0", replacePAID.c_str(), replacePID.c_str());
			mainFileExt =  std::string(buf);
		}
		FileExtensions exMap;
		FileExtensions::iterator iter;
		std::string key="";
		std::map<std::string , int> exMapOutPutfile;
//		trickspeed.sort();
		int index = 0;

		for (int i = 0; i < (int)trickspeed.size(); i++)
		{
			char ex[10]={0};
			char exr[10] ={0};
         
			FileExtension fileExt;
			if (bH264Type && _gCPHCfg.unifiedtrickfile.enable) //.h264 wgk
			{
				memset(buf, 0, 64);
				getUnifiedTrickExt(index,ex);
				key = std::string(ex);
				if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
				{
					++uFileCiscoExt;
					snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else if( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
				{
					memset(buf, 0, 64);
					snprintf(buf, sizeof(buf)-2, ".B%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else
				{
				    exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
				}
				fileExt.ext = ex;
				fileExt.extForCisco = buf;
				fileExt.position = i;
				exMap.insert(std::make_pair(key, fileExt));
//				exMap.push_back(fileExt);
			}
			else
			{
				getTrickExt(index,ex,exr);
				key = std::string(ex);
				memset(buf, 0, 64);
				if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
				{
					++uFileCiscoExt;	
					snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else if( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC)// NVOD1234567890abcdef_cctv.com.F0NVOD1234567890abcdef_cctv$com
				{
					memset(buf, 0, 64);
					snprintf(buf, sizeof(buf)-2, ".F%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else
					exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
				fileExt.ext = ex;
				fileExt.extForCisco = buf;
				fileExt.position = i;
				exMap.insert(std::make_pair(key, fileExt));
//                exMap.push_back(fileExt);


				key = std::string(exr);
				memset(buf, 0, 64);
				if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
				{
					++uFileCiscoExt;
					snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else if( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC) // NVOD1234567890abcdef_cctv.com.R0NVOD1234567890abcdef_cctv$com
				{
					memset(buf, 0, 64);
					snprintf(buf, sizeof(buf)-2, ".R%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
					exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
					key = std::string(buf);
				}
				else
					exMapOutPutfile.insert(std::make_pair(std::string(exr), i));
				fileExt.ext = exr;
				fileExt.extForCisco = buf;
				fileExt.position = i;
				exMap.insert(std::make_pair(key, fileExt));
		//		exMap.push_back(fileExt);
			}
			index++;
		}

		int outPutNum = 2 + exMap.size();

		if(enableNoTrickSpeed)
			outPutNum = 2;

		if ( !enableNoTrickSpeed && outPutNum < 2 )
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Not specify trick speed"), _strLogHint.c_str());
			return false;
		}

		RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
		AddFilter(pProcess);
		if(!enableNoTrickSpeed)
		{
			pProcess->setTrickFileEx(exMap);
			pProcess->settrickSpeedNumerator(trickspeed);
			pProcess->settrickSpeedNumeratorHD(trickspeedHD);
		}
		pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
		if( 0 != _gCPHCfg.ciscofileext.mode)
			pProcess->setCsicoFileExt(1);
//		pProcess->setCsicoFileExt(_gCPHCfg.ciscofileext.enable);
		pProcess->setCsicoMainFileExt(mainFileExt);
		if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPRTF) || !stricmp(_strMethod.c_str(), METHODTYPE_CDN_NTFSRTF))
		{
			pProcess->setAugmentationPids(augmentationPids ,augmentationPidCount);
		}
		if (bH264Type)
		{
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
		}
#ifndef ZQ_OS_MSWIN
		else if (bIndexVVC)
		{
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_MPEG2);
			pProcess->setAssetInfo(providerId,providerAssetId);
		}
#endif
		else
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE_MPEG2);
		_pRTFProc = pProcess;
		{
#ifdef ZQ_OS_LINUX
			CDNFilesetTarget* pTarget = (CDNFilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
			pTarget->setPacingFactory(_pPacedIndexFac);
			pTarget->enableCacheForIndex(_gCPHCfg.enableCacheForIndex);
#else
			FilesetTarget* pTarget = (FilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
#endif   
			if(!AddFilter(pTarget))
				return false;
			pTarget->setFilename(strFilename.c_str());
			pTarget->setBandwidth(nMaxBandwidth);
			pTarget->enableProgressEvent(true);
			pTarget->enableMD5(_gCPHCfg.enableMD5);
			if(!enableNoTrickSpeed)
			{
				pTarget->setTrickFile(exMapOutPutfile);

				pTarget->setTrickSpeed(trickspeed);
			}
			pTarget->setCsicoMainFileExt(mainFileExt);
			if (bH264Type)
			{
				pTarget->setTypeH264();
				pTarget->enablePacing(_gCPHCfg.enablePacing);

				pTarget->enableStreamableEvent(false);
			}
			else
			{
                pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
				pTarget->setIndexType(bIndexVVC);
				pTarget->enablePacing(_gCPHCfg.enablePacing);
				pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
			}

			_pMainTarget = pTarget;

			InitPins();

			if (!ConnectTo(pSource, 0, pProcess, 0))
				return false;

			for (int i = 0; i < outPutNum; i++)
				if (!ConnectTo(pProcess, i, pTarget, i))
					return false;
		}
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
#ifdef ZQ_OS_LINUX
		if(_bSuccessMount)
		{
			umountURL(_sharePath, _strLogHint);
		}
#endif
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void CDNSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
		cleanup();
		try
		{
			delete this;
		}
		catch (...)
		{			
		}
	}
	else
	{
		//
		// thread already started, let final to delete this
		//
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool CDNSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
} 

int CDNSess::run(void)
{	
	bool bRet;

	Ice::Long llStart = ZQTianShan::now();
    _helper.increaseLoad(_strMethod,_nBandwidth);
	if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation))
	{
		FTPPropSource* pSource = (FTPPropSource*)getSourceFilter();
		if (!pSource)
		{
			setErrorInfo(ERRCODE_INVALID_SRCURL, ("Source file is invalid."));			
			return false;
		}
		bRet = pSource->Run();

	}
	else if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
		{
			HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
			if (!pSource)
			{
				setErrorInfo(ERRCODE_INVALID_SRCURL, ("Source file is invalid."));			
				return false;
			}
			bRet = pSource->Run();

		}
	else
		bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);

#ifdef ZQ_OS_LINUX
	if(_bSuccessMount)
	{
		umountURL(_sharePath, _strLogHint);
	}
#endif

	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			

		if (!_bStartEventSent)
		{
			notifyStarted(params);
		}

        char tmp[40];
		sprintf(tmp, "False");
		params[EVTPM_OPENFORWRITE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s]OpenForWrite [%s]"), _strLogHint.c_str(),tmp);

		notifyStopped(true, params);

		if(!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
		{
			HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
			pSource->deleteOutput(_gCPHCfg.deleteOnFail);
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());

		Close();

		return 0;
	}

	// get the range info from 
	std::map<std::string, std::string> fileinfomap;
	std::map<std::string, std::string>::iterator iter;
	std::string extCol;
	// query the available range info, and fill in params
	if (_pRTFProc)
	{
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		pProcess->getOutputFileInfo(fileinfomap);
		pProcess->getOutputFileExtCol(extCol);
	}
	else if(!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
		pSource->getOutputFileInfo(fileinfomap);
		pSource->getOutputFileExtCol(extCol);
	}
	// set to params
	for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
	{
		params[iter->first.c_str()] = iter->second;
		//MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
		params[EVTPM_MEMBERFILEEXTS] = extCol;
		//MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
	}

	Close();

	int64 supportFileSize=0;
	std::string md5;
    std::vector <std::string> outputfilelist;
	std::string cacherDir = "";
	int indexType, outputfilecount;
	bool bNormalize = true;
	    
#ifdef ZQ_OS_MSWIN
	if (_gCPHCfg.enableTestNTFS)
#endif
	{
		if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation))
		{
			FTPPropSource* pSource = (FTPPropSource*)getSourceFilter();
			pSource->getSupportFileSize(supportFileSize);
			pSource->getMD5(md5);
			_llProcBytes = pSource->getProcessSize();
			_llTotalBytes = pSource->getTotalSize();
			pSource->getOutputFiles(outputfilelist, outputfilecount, indexType);
			bNormalize = pSource->getNormalize();
		}
		else if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
			{
				HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
				pSource->getSupportFileSize(supportFileSize);
				pSource->getMD5(md5);
				_llProcBytes = pSource->getProcessSize();
				_llTotalBytes = pSource->getTotalSize();
				pSource->getOutputFiles(outputfilelist, outputfilecount, indexType);
			}
		else
		{
			if (_pMainTarget)
			{
#ifdef ZQ_OS_LINUX
				CDNFilesetTarget* pTarget = (CDNFilesetTarget*)_pMainTarget;
#else
				FilesetTarget* pTarget = (FilesetTarget*)_pMainTarget;
#endif
				supportFileSize = pTarget->getSupportFileSize();
				pTarget->getMD5(md5);
                pTarget->getOutputFiles(outputfilelist, outputfilecount, indexType);
			}
		}
		
	}

	if(bNormalize && _gCPHCfg.enableNSF && (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPRTF) || !stricmp(_strMethod.c_str(), METHODTYPE_CDN_NTFSRTF) || !stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation) || !stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation)))
	{
		CStdFileIoFactory* pFactory = (CStdFileIoFactory*)CDNSess::_pFileIoFac.get();
		cacherDir = pFactory->getRootDir();
		for(int i = 0 ; i < outputfilelist.size(); i ++)
		{
			outputfilelist[i] =  cacherDir +  outputfilelist[i];
		}
		NormalizeSparseFile normallize(_pLog, outputfilecount, outputfilelist, indexType, false);
		int bRet = normallize.normalizeSparseFileSet();
		if(bRet)
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] provision done, status[failure], error[failed to normalize sparse file]"), 
				_strLogHint.c_str());
			return 0;
		}
	}

	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, FMT64, _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, FMT64, supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPPropagation))
		{
			sprintf(tmp, FMT64, _llTotalBytes*8000/_bitrate);
			params[EVTPM_PLAYTIME] = tmp;
		}
		else if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
		{
			sprintf(tmp, FMT64, _llTotalBytes*8000/_bitrate);
			params[EVTPM_PLAYTIME] = tmp;
		}
		else
		{
			sprintf(tmp, FMT64, _llProcBytes*8000/_bitrate);
			params[EVTPM_PLAYTIME] = tmp;
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else 
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	sprintf(tmp, "False");
	params[EVTPM_OPENFORWRITE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);
   

	if (_pRTFProc && (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_FTPRTF) || !stricmp(_strMethod.c_str(), METHODTYPE_CDN_NTFSRTF)))
	{
		RTFProcess* pRtfprocess = (RTFProcess*)_pRTFProc;
		bool bEncrypt = pRtfprocess->getPreEncrypt();
		int augmentedBitRate;
		int originalBitRate;
		std::string augmentionpids;
		if(bEncrypt)
		{
			pRtfprocess->getAugmentationPids(augmentionpids);	
			params[EVTPM_AUGMENTATIONPIDS] = augmentionpids;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] AugmentationPids[%s]"), _strLogHint.c_str(),augmentionpids.c_str());

			pRtfprocess->getPreEncryptBitRate(augmentedBitRate, originalBitRate);

			sprintf(tmp, "%d", originalBitRate);
			params[EVTPM_ORIGINALBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] OrignalBiterate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "%d", augmentedBitRate);
			params[EVTPM_AUGMENTATEDBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] AugmentatedBitarate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "1");
			params[EVTPM_PREENCRYPTION] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] PreEncryption[%s]"), _strLogHint.c_str(),tmp);
		}
	}
	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);

	int nTimeSpentMs = int(ZQTianShan::now() - llStart);
	int nActualBps;
	if (nTimeSpentMs)
	{
		nActualBps = int(_llProcBytes*8000/nTimeSpentMs);
	}
	else
	{
		nActualBps = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);
	return 0;
}

void CDNSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool CDNHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "provision session is 0"));
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_CDN_FTPPropagation) 
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_FTPRTF)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_FTPRTFH264)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_NTFSRTF)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_NTFSRTFH264)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_C2Pull)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_C2PullH264)
		&& stricmp(strMethod.c_str(),METHODTYPE_CDN_HTTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	if(stricmp(strMethod.c_str(), METHODTYPE_CDN_FTPPropagation)==0 || (stricmp(strMethod.c_str(),METHODTYPE_CDN_FTPRTF) == 0) || (stricmp(strMethod.c_str(),METHODTYPE_CDN_FTPRTFH264) == 0))
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,6);
		if (stricmp(strpro.c_str(),"ftp://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Can't find the FTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url(%s)" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if(stricmp(strMethod.c_str(),METHODTYPE_CDN_NTFSRTF) == 0 || stricmp(strMethod.c_str(),METHODTYPE_CDN_NTFSRTFH264) == 0)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}
		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,7);
		if (stricmp(strpro.c_str(),"cifs://")!= 0 && stricmp(strpro.c_str(),"file://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Can't find the CIFS protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if(stricmp(strMethod.c_str(),METHODTYPE_CDN_C2Pull) == 0 || stricmp(strMethod.c_str(),METHODTYPE_CDN_C2PullH264) == 0)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}
		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,9);
		if (stricmp(strpro.c_str(),"c2pull://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Can't find the c2pull protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if(stricmp(strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation)==0)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,9);
		if (stricmp(strpro.c_str(),"c2http://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] Can't find the c2 HTTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_CDN, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the c2 HTTP protocol from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		MOLOG(Log::L_WARNING, CLOGFMT(CPH_CDN, "[%s] Unknown method[%s], setSessionType() with default type"), sess->ident.name.c_str(), strMethod.c_str());
	}

	return true;
}

bool CDNSess::prime()
{
	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	if (!BaseCPHSession::prime())
		return false;

	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void CDNSess::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);

	::TianShanIce::Properties params;
	char tmp[40];
	// query the available range info, and fill in params
	if (_pRTFProc)
	{
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pProcess->getOutputFileInfo(fileinfomap);
		pProcess->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
			params[EVTPM_MEMBERFILEEXTS] = extCol;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		}
	}

/*	else if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pSource->getOutputFileInfo(fileinfomap);
		pSource->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
			params[EVTPM_MEMBERFILEEXTS] = extCol;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		}
	}
*/
	sprintf(tmp, "True");
	params[EVTPM_OPENFORWRITE] = tmp;
	//MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);

	updateProgress(_llProcBytes, _llTotalBytes, params);
}

void CDNSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void CDNSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	if (_pRTFProc)
	{
		std::string indextype;
		
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		pProcess->getIndexType(indextype);

		params[EVTPM_INDEXEXT] = indextype;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] IndexTypeExt[%s]"), _strLogHint.c_str(),indextype.c_str());
		
	}
	
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CDN, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

CDNHelper::CDNHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	CPHConfig::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

CDNHelper::~CDNHelper()
{
	MethodCostList::iterator it = _methodCostList.begin();
	for (;it!=_methodCostList.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodCostList.clear();
}

bool CDNHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (stricmp(methodType, METHODTYPE_CDN_FTPPropagation) && 
		stricmp(methodType, METHODTYPE_CDN_FTPRTF)&&
		stricmp(methodType,METHODTYPE_CDN_FTPRTFH264)&&
		stricmp(methodType,METHODTYPE_CDN_NTFSRTF)&&
		stricmp(methodType,METHODTYPE_CDN_NTFSRTFH264)&&
		stricmp(methodType,METHODTYPE_CDN_C2Pull)&&
		stricmp(methodType,METHODTYPE_CDN_C2PullH264)&&
		stricmp(methodType,METHODTYPE_CDN_HTTPPropagation))
		return false;
	
	bpsAllocated =0;

	CPHConfig::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
			break;
	}
	if (methodIter == _gCPHCfg.methods.end())
		return false;

	bpsMax=(*methodIter).maxBandwidth;
	initCost =0;
	return true;
}

/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool CDNHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	CPHConfig::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
			break;
	}
	if (methodIter == _gCPHCfg.methods.end())
		return false;

	maxKbps = (*methodIter).maxBandwidth;
	maxSessins = (*methodIter).maxSession;
	
	getCurrentLoad(methodType,allocatedKbps,sessions);

	return true;
}


