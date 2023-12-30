
#include "BaseClass.h"
#include "CPH_Cfg.h"
#include "RTFProc.h"
#include "FileLog.h"
#include "FTPSource.h"
#include "getopt.h"
#include <math.h>
#include "strHelper.h"
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "CStdFileIoFactory.h"
#include <list>
#include "FTPMSClientFactory.h"
#include "SystemUtils.h"
#include "ZQ_common_conf.h"
#include "urlstr.h"
#include "CIFSSource.h"
#include "SystemUtils.h"
#include "Guid.h"

#ifdef ZQ_OS_LINUX
#include "CDNFileSetTarget.h"
#include <dlfcn.h> 
#else
#include "CDNFileSetTarget.h"
#endif

//#ifdef ZQ_OS_LINUX
#include "PacingInterface.h"
//#endif
using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define LinuxTrick			"LinuxTrick"
#define MOLOG					(glog)

#define RDS_PROVISION_ERRCODE		1

ZQ::common::FileLog		filelog;

class CTFGen : public ZQTianShan::ContentProvision::BaseGraph, public ZQ::common::NativeThread
{
public:
	CTFGen()
		: _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}

	~CTFGen(){};

public:

	bool preLoad(const std::string& strSrcFile, int nBandwidth, const std::string& strFilename, const std::string& strtrickspeed, const std::string& strPID, const std::string& strPAID, int nTypeH264 = 0, int nSrcFileset = 0, int IndexVVC = 0);

	void OnProgress(int64& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	virtual int run(void);

protected:
	void cleanup();

	bool _bQuit;
	bool _bCleaned;
	ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
	ZQ::common::BufferPool							_pool;		
};

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

std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;
//#ifdef ZQ_OS_LINUX
static PacedIndexFactory*                                         _pPacedIndexFac;

#ifdef ZQ_OS_LINUX
static void*                                                  _pPacedIndexDll;
#else
static HMODULE                                                  _pPacedIndexDll;
#endif

bool InitCPH(std::string& strCfgDir,std::string& strlogName)
{
	filelog.open(strlogName.c_str(), ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);
	_gCPHCfg.setLogger(&filelog);

	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());
		filelog.flush();

		return false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

	}


	CStdFileIoFactory* pFactory = new CStdFileIoFactory();
	_pFileIoFac.reset(pFactory);

	_pFileIoFac->setLog(&filelog);
	if (!_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	//
	// do some module initialize
	//
	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

	filelog.flush();

#ifdef ZQ_OS_MSWIN
	_pPacedIndexDll = LoadLibrary(_gCPHCfg.szPaceDllPath);
	if (!_pPacedIndexDll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "%s failed to load"),_gCPHCfg.szPaceDllPath);
		return false;
	}
	typedef bool (*FunCreatePacedIndexFactory)(PacedIndexFactory**);
	FunCreatePacedIndexFactory _create;
	_create = (FunCreatePacedIndexFactory)GetProcAddress(_pPacedIndexDll,"CreatePacedIndexFactory");
	if (_create != NULL)
	{
		_create(&_pPacedIndexFac);
		if (!_pPacedIndexFac)
		{
			FreeLibrary(_pPacedIndexDll);
			MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to create pacingFactory"));
			return false;
		}
		_pPacedIndexFac->setLog(&glog);
		//CDNSess::_pPacedIndexFac->setConfig("name","value");
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "Successfully load %s"),_gCPHCfg.szPaceDllPath);
	}
	else
	{
		std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		FreeLibrary(_pPacedIndexDll);
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to get CreatePacedIndexFactory entry error[%s]"), strErr.c_str());
		return false;
	}
#else
	_pPacedIndexDll = dlopen(_gCPHCfg.szPaceDllPath,RTLD_LAZY);
	if (!_pPacedIndexDll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "%s failed to load"),_gCPHCfg.szPaceDllPath);
		return false;
	}
	typedef bool (*FunCreatePacedIndexFactory)(PacedIndexFactory**);
	FunCreatePacedIndexFactory _create;
	_create = (FunCreatePacedIndexFactory)dlsym(_pPacedIndexDll,"CreatePacedIndexFactory");
	if (_create != NULL)
	{
		_create(&_pPacedIndexFac);
		if (!_pPacedIndexFac)
		{
			dlclose(_pPacedIndexDll);
			MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to create pacingFactory"));
			return false;
		}
		_pPacedIndexFac->setLog(&glog);
		//CDNSess::_pPacedIndexFac->setConfig("name","value");
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "Successfully load %s"),_gCPHCfg.szPaceDllPath);
	}
	else
	{
		std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		dlclose(_pPacedIndexDll);
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to get CreatePacedIndexFactory entry error[%s]"), strErr.c_str());
		return false;
	}
#endif
	return true;
}

void UninitCPH()
{
	RTFProcess::uninitRTFLib();

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (_pFileIoFac.get())
	{
		_pFileIoFac->uninitialize();
		_pFileIoFac.reset(0);
	}

#ifdef ZQ_OS_LINUX
	typedef bool (*DestroyPacedIndexFactory)(PacedIndexFactory*);
	DestroyPacedIndexFactory _pdestry = (DestroyPacedIndexFactory)dlsym(_pPacedIndexDll,"DestroyPacedIndexFactory");
	if (_pdestry)
	{
		_pdestry(_pPacedIndexFac);
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to get DestroyPacedIndexFactory entry"));
	}
	if (_pPacedIndexDll)
	{
		dlclose(_pPacedIndexDll);
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "Successfully unload %s"),_gCPHCfg.szPaceDllPath);
	}
#else
	typedef bool (*DestroyPacedIndexFactory)(PacedIndexFactory*);
	DestroyPacedIndexFactory _pdestry = (DestroyPacedIndexFactory)GetProcAddress(_pPacedIndexDll,"DestroyPacedIndexFactory");
	if (_pdestry)
	{
		_pdestry(_pPacedIndexFac);
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "failed to get DestroyPacedIndexFactory entry"));
	}
	if (_pPacedIndexDll)
	{
		FreeLibrary(_pPacedIndexDll);
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "Successfully unload %s"),_gCPHCfg.szPaceDllPath);
	}
#endif

	filelog.flush();
	ZQ::common::setGlogger(NULL);
}



bool CTFGen::preLoad(const std::string& strSrcFile, int nBandwidth, const std::string& strFilename, const std::string& strtrickspeed, const std::string& strPID, const std::string& strPAID, int nTypeH264, int nSrcFileset,int IndexVVC)
{
	_pool.initialize(65536, 800, 400);
	bool bIndexVVC = IndexVVC;
	SetLog(&filelog);
	SetMemAlloc(&_pool);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	if (!nBandwidth)
	{
		nMaxBandwidth = 0;
	}
	else
		nMaxBandwidth = nBandwidth;

	MOLOG(Log::L_DEBUG, CLOGFMT(LinuxTrick, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	BaseSource *pSource= NULL;

	{
		ZQ::common::URLStr Url(strSrcFile.c_str());
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] sourceUrl[%s]"), _strLogHint.c_str(),strSrcFile.c_str());

		std::string url = strSrcFile;
		if (0 ==stricmp("ftp", Url.getProtocol()))
		{
			FTPIOSource* ftpSource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP);
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
		else if(0 ==stricmp("cifs", Url.getProtocol()) || 0 ==stricmp("file", Url.getProtocol()) || strSrcFile.substr(0, 1)=="/")
		{
			std::string host = Url.getHost();
			std::string sourceFilename = Url.getPath();
			std::string strOpt,strsystype,strsharePath;
			bool bLocalSrcFlag;
			strsystype = "cifs";

			if (strSrcFile.substr(0, 1)=="/")
			{
				bLocalSrcFlag = true;
				sourceFilename = strSrcFile;
			}
			else if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
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

			CIFSIOSource* fsSource = (CIFSIOSource*)SourceFactory::Create(SOURCE_TYPE_CIFS);
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
				fsSource->setSharePath(strsharePath);
			pSource = fsSource;
		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CDN, "[%s] sourceurl[%s] protocol[%s] not support"), _strLogHint.c_str(),
				strSrcFile.c_str(), Url.getProtocol());
			return false;
		}

		std::string mainFileExt;
		{
			ZQ::common::Guid id;
			id.create();

			ZQ::common::Guid::UUID Uid = id;

			Uid.Data1 ^= *((unsigned long*) Uid.Data4);
			Uid.Data2 ^= *((unsigned short*) Uid.Data4+4);
			Uid.Data3 ^= *((unsigned short*) Uid.Data4+6);
			Uid.Data3 &=0xfff0;

			char buf[32] = "";

			memset(buf, 0, 32);
			snprintf(buf, sizeof(buf)-2, ".0x%08X%04X%04X\0", Uid.Data1, Uid.Data2, Uid.Data3);

			if( 0 != _gCPHCfg.ciscofileext.mode)
			{
				mainFileExt = std::string(buf);
			}
		}

		std::list<float> trickspeed;
		if (strtrickspeed.empty())
			trickspeed.push_back(7.5);
		else
		{
			std::vector<std::string> strVec;
			strVec = ZQ::common::stringHelper::split(strtrickspeed,',');

			for (std::vector<std::string>::iterator it = strVec.begin();it != strVec.end(); it++)
			{
				trickspeed.push_back(atof((*it).c_str()));
			}
		}

		std::map<std::string, int> exMap;
		std::map<std::string, int>::iterator iter;
		trickspeed.sort();
		int index = 0;
		for (int i = 0; i < trickspeed.size(); i++)
		{
			char ex[10]={0};
			char exr[10] ={0};

			if (nTypeH264)
			{
				getUnifiedTrickExt(index,ex);

				exMap.insert(std::make_pair(std::string(ex),i));		
			}
			else
			{
				getTrickExt(index,ex,exr);

				exMap.insert(std::make_pair(std::string(ex),i));
				exMap.insert(std::make_pair(std::string(exr),i));
			}	
			index++;
		}

		int outPutNum = 2 + exMap.size();
		if (outPutNum < 2)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "[%s] Not specify trick speed"), _strLogHint.c_str());
			return false;
		}

		RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF);
		pProcess->setTrickFile(exMap);
		pProcess->settrickSpeedNumerator(trickspeed);
		AddFilter(pProcess);
		if (nTypeH264)
		{
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
		}
		else if (bIndexVVC)
		{
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_MPEG2);
			pProcess->setAssetInfo(strPID, strPAID);
			if ( 0 != _gCPHCfg.ciscofileext.mode) 
				pProcess->setCsicoFileExt(1);
			else
				pProcess->setCsicoFileExt(0);
			pProcess->setCsicoMainFileExt(mainFileExt);
		}
		else
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE_MPEG2);
		
		{
			CDNFilesetTarget* pTarget = (CDNFilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
			if(!AddFilter(pTarget))
				return false;
			pTarget->setPacingFactory(_pPacedIndexFac);

			pTarget->setFilename(strFilename.c_str());
			pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
			pTarget->enableCacheForIndex(_gCPHCfg.enableCacheForIndex);
			pTarget->setBandwidth(nMaxBandwidth);
			pTarget->enableProgressEvent(1);
			pTarget->enableMD5(_gCPHCfg.enableMD5);
			pTarget->setTrickFile(exMap);
			pTarget->setTrickSpeed(trickspeed);
			if (nTypeH264)
			{
				pTarget->setTypeH264();
				pTarget->enablePacing(false);
				pTarget->enableStreamableEvent(false);
			}
			else
			{
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
		MOLOG(Log::L_ERROR, CLOGFMT(LinuxTrick, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	if (!Start())
	{
		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

int CTFGen::run( void )
{
	DWORD dwStart = SYS::getTickCount();

	bool bRet = Run();
	if (!bRet)
	{
		Close();

		MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return bRet;
	}

	char tmp[64];
	snprintf(tmp, sizeof(tmp)-2, FMT64, _llProcBytes);

	Close();

	int nTimeSpentMs = SYS::getTickCount() - dwStart;
	int nActualBps;
	if (nTimeSpentMs)
	{
		nActualBps = int(_llTotalBytes*8000/nTimeSpentMs);
	}
	else
	{
		nActualBps = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);

	return true;
}
void CTFGen::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
}

void CTFGen::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void CTFGen::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(LinuxTrick, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}


void usage()
{
	printf("usage: TrickGen -s <source file/url>\n");	
	printf("       -d <destination filename>\n");		
	printf("       -c [configuration file path], default is curent directory: \".\\\" \n");
	printf("       -b [bandwidth]  the trick generation speed in bps, 0 for no limit, default is 0\n");
	printf("       -t [content type]  M for mpeg2, H for H264, default is M\n");
	printf("       -f [fileset] 1 for source is fileset, 0 for not, default is 0\n");
	printf("       -m multiple trick speed specified.eg:\"7.5\" or \"7.5,15,22.5\"\n");
	printf("       -v vvc index type, 1 for vvc, 0 for not\n");
	printf("       -p set the providerid and providerassetid, format: \"providerid,providerassetid\"\n");
	printf("       -l <specify log file name> default is \"TrickGen.log\"\n");
	printf("       -h display this help\n");	
}

int main(int argc, char** argv)
{
	if (argc <3)
	{
		usage();
		return 0;                                                                                                                                                                                                                                                                       
	}

	std::string strSrc, strFile, strCfgdir, strContentType,trickspeed,strLogName, strPID, strPAID, strPIDStr;
	int nBandwidth = 0;
	int nTypeH264 = 0;
	int nSrcFileset = 0;
	strCfgdir = ".\\";
	strLogName="";
	bool IndexVVC = false;

	int ch;
	while((ch = getopt(argc, argv, "hHs:S:d:D:b:B:c:C:t:T:f:F:m:M:l:L:v:V:p:P")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
		case 'H':
			usage();
			return 0;

		case 's':
		case 'S':
			if (optarg)
			{
				strSrc = optarg;
			}
			break;

		case 'd':
		case 'D':
			if (optarg)
			{
				strFile = optarg;
			}
			break;

		case 'b':
		case 'B':
			if (optarg)
			{
				nBandwidth = atoi(optarg);
			}
			break;

		case 'f':
		case 'F':
			if (optarg)
			{
				nSrcFileset = atoi(optarg);
			}
			break;

		case 't':
		case 'T':
			if (optarg)
			{
				strContentType = optarg;
				if (strContentType=="H" || strContentType=="h")
					nTypeH264 = 1;
			}
			break;

		case 'c':
		case 'C':
			if (optarg)
			{
				strCfgdir = optarg;
			}
			break;
		case 'm':
		case 'M':
			if (optarg)
			{
				trickspeed = optarg;
			}
			break;
		case 'l':
		case 'L':
			if (optarg)
			{
				strLogName = optarg;
			}
			break;
		case 'v':
		case 'V':
			if (optarg)
			{
				IndexVVC = atoi(optarg);;
			}
			break;
		case 'p':
		case 'P':
			if (optarg)
			{
				strPIDStr = optarg;
				size_t t = strPIDStr.find_first_of(',');
				if (t>0)
				{
					strPID = strPIDStr.substr(0, t);
					strPAID = strPIDStr.substr(t+1, strPIDStr.length());
				}
			}
			break;
		default:
			printf("Error: unknown option %c specified\n\n", ch);
			usage();
			return 0;
		}
	}

	if (strLogName.empty())
		strLogName = "TrickGen.log";
	else
	{
		if (stricmp(strLogName.c_str(),".log")!= 0)
			strLogName += ".log";
	}

	if (!InitCPH(strCfgdir,strLogName))
	{
		printf("failed to InitCPH, check log file: %s\n", strLogName.c_str());
		return 0;
	}

	{
		CTFGen aaa;

		do
		{
			if (!aaa.preLoad(strSrc, nBandwidth, strFile,trickspeed, strPID, strPAID, nTypeH264, nSrcFileset, IndexVVC))
			{
				printf("failed to preload with error [%s], check log file: %s\n", aaa.GetLastError().c_str(), strLogName.c_str());
				break;
			}

			aaa.start();
			SYS::sleep(500);			
			DWORD dwStart = SYS::getTickCount();
			LONGLONG total = aaa.getTotalBytes();
			while(1)
			{
				DWORD result= aaa.waitHandle(500);
				if (result)
				{	
					if (total)
					{
						int nTimeSpentMs = SYS::getTickCount() - dwStart;
						int processRate = (aaa.getProcessBytes())*100/total;
						printf("\r time: %d seconds, done: %d%%",nTimeSpentMs/1000,processRate);
					}	
				}
				else if(result == 0)
				{
					int nTimeSpentMs = SYS::getTickCount() - dwStart;
					if (!aaa.IsErrorOccurred())
					{
						printf("\r time: %d seconds, done: 100%%\n",nTimeSpentMs/1000);
					}
					else
					{
						printf("success break\n");
					}
					break;
				}
				else
				{
					printf("\n error occurs");
					break;
				}
			}

			if (aaa.IsErrorOccurred())
			{
				printf("Trick generation failed with error: %s\n", aaa.GetLastError().c_str());
			}
			else
			{
				printf("Trick generation success\n");
			}

		}while(0);	

		aaa.Close();
	}

	UninitCPH();
	return 1;
}
