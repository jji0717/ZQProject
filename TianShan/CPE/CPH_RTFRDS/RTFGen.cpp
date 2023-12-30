
#include "BaseClass.h"
#include "NTFSSource.h"
#include "BaseClass.h"
#include "FilesetTarget.h"
#include "CPH_RTFRDSCfg.h"
#include "RTFProc.h"
#include "FileLog.h"
#include "QueueBufMgr.h"
#include "FTPSource.h"
#include "FTPFilesetSource.h"
#include "getopt.h"
#include <math.h>
#include "strHelper.h"
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "NtfsFileIoFactory.h"
#include "VstrmFileIoFactory.h"
#include <list>

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTFRDS			"CPH_RTFRDS"
#define MOLOG					(glog)

#define RDS_PROVISION_ERRCODE		1

ZQ::common::FileLog		filelog;

class RTFGen : public ZQTianShan::ContentProvision::BaseGraph, public QueueBufMgr,public ZQ::common::NativeThread
{
public:
	RTFGen()
		: _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}
	
	~RTFGen(){};
	
public:
	
	bool preLoad(std::string& strSrcFile, int nBandwidth, std::string& strFilename,std::string& strtrickspeed, int nTypeH264 = 0, int nSrcFileset = 0);

	void OnProgress(LONGLONG& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	virtual int run(void);
	
protected:
	void cleanup();
	
	bool _bQuit;
	bool _bCleaned;
	ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
};

std::string getErrMsg(DWORD dwErrCode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(     FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	if (lpMsgBuf != NULL)
	{
		std::string strErr = (LPCTSTR) lpMsgBuf;
		LocalFree( lpMsgBuf );
		return strErr;
	}
	else 
	{
		std::string strErrMsg;
		char wszErrMsg [256];
		sprintf(wszErrMsg, "Error Code = %d", dwErrCode);
		strErrMsg = wszErrMsg;
		return strErrMsg;
	}
}

// ------------------------------------------------------------------------- //
std::string getErrMsg()
{
	DWORD dwErrCode = ::GetLastError();
	return getErrMsg(dwErrCode);
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
bool validatePath  ( const char * wszPath )
{
	if (-1 != ::GetFileAttributes(wszPath))
		return true;

	DWORD dwErr = ::GetLastError();
	if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
	{
		if (!::CreateDirectory(wszPath, NULL))
		{
			dwErr = ::GetLastError();
			if ( dwErr != ERROR_ALREADY_EXISTS)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(Common, "Failed to create directory %s Reason : %s"),
					wszPath, getErrMsg(dwErr).c_str() );
				return false;
			}
		}
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(Common,"Failed to access directory %s Reason : %s"), 
			wszPath, getErrMsg(dwErr).c_str());
		return false;
	}

	return true;
}

bool SetRegEdit(LPCTSTR dataset,char* module)
{
	HKEY hKEY;
	DWORD typesz=REG_SZ;
	DWORD typed =REG_DWORD;

	DWORD onlyFlag;
	DWORD defaultOnlyFlag=0;//atol("0");
	DWORD interval;
	DWORD defaultInterva=1000;//atol("1000");
	DWORD cbData=80;
	DWORD position;

	std::string firstV = "PaceMainFileOnly";
	std::string secondV = "PacingIntervalMs";
	std::string thirdV = "PacingModule";

	long ret0=(::RegCreateKeyEx(HKEY_LOCAL_MACHINE,dataset,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKEY,&position));
	if(ret0!=ERROR_SUCCESS)
	{
		if (position == REG_CREATED_NEW_KEY)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "The key %s did not exist then failed to create it"),
				dataset);
		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "The key %s existed then failed to open it"),
				dataset);
		}
		return FALSE;
	}

	long ret1=::RegQueryValueEx(hKEY,(LPCSTR)firstV.c_str(),NULL,&typed,(LPBYTE)&onlyFlag,&cbData);
	if(ret1!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)firstV.c_str(),NULL,REG_DWORD,(LPBYTE)&defaultOnlyFlag,sizeof(DWORD));
		if(ret1!=ERROR_SUCCESS)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to set configuration [%s] = [%u],error is %d"),
				firstV.c_str(),defaultOnlyFlag,GetLastError());
			::RegCloseKey(hKEY);
			return FALSE;
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Set configuration [%s] = [%u]"),
			firstV.c_str(),defaultOnlyFlag);
	}

	long ret2=::RegQueryValueEx(hKEY,(LPCSTR)secondV.c_str(),NULL,&typed,(LPBYTE)&interval,&cbData);
	if(ret2!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)secondV.c_str(),NULL,REG_DWORD,(LPBYTE)&defaultInterva,sizeof(DWORD));
		if(ret1!=ERROR_SUCCESS)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to set configuration [%s] = [%u],error is %d"),
				secondV.c_str(),defaultInterva,GetLastError());
			::RegCloseKey(hKEY);
			return FALSE;
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Set configuration [%s] = [%u]"),
			secondV.c_str(),defaultInterva);
	}

	LPBYTE temp = new BYTE[80];
	long ret3 =::RegQueryValueEx(hKEY,(LPCSTR)thirdV.c_str(),NULL,&typesz,temp,&cbData);
	if(ret3!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)thirdV.c_str(),NULL,REG_SZ,(LPBYTE)module,strlen(module)+1);
		if(ret1!=ERROR_SUCCESS)
		{	
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to set configuration [%s] = [%u],error is %d"),
				thirdV.c_str(),module,GetLastError());
			::RegCloseKey(hKEY);
			delete []temp;
			return FALSE;
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Set configuration [%s] = [%u]"),
			thirdV.c_str(),module);
	}

	::RegCloseKey(hKEY);
	delete []temp;
	return true;
}

bool CheckRegEdit()
{
	LPCTSTR   data_Set="Software\\SeaChange\\PacedIndex\\VVX\\";
	LPCTSTR   data_Set1="Software\\SeaChange\\PacedIndex\\VV2\\";

	char  defaultModule[80] = "PacedVVX.dll";
	char  secondModule[80] = "PacedVV2.dll";

	if (!SetRegEdit(data_Set,defaultModule))
		return false;

	if (!SetRegEdit(data_Set1,secondModule))
		return false;

	return TRUE;
}

std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;

bool InitCPH(std::string& strCfgDir,std::string& strlogName)
{
	filelog.open(strlogName.c_str(), ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);
	_gCPHCfg.setLogger(&filelog);

	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());
		filelog.flush();

		return false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}

	validatePath(_gCPHCfg.szCacheDir);
	if (_gCPHCfg.enableTestNTFS)
	{
		validatePath(_gCPHCfg.szNTFSOutputDir);
	}	

	CheckRegEdit();

	if (_gCPHCfg.enableTestNTFS)
	{
		NtfsFileIoFactory* pFactory = new NtfsFileIoFactory();
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		_pFileIoFac.reset(pFactory);
	}
	else
	{
		VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		_pFileIoFac.reset(pFactory);
	}

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

	return true;
}

void UninitCPH()
{
	//
	//do some module uninitialize
	//

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

	filelog.flush();
	ZQ::common::setGlogger(NULL);
}

///////////////
#include "UrlStr.h"

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
	
	return true;
	
}

static unsigned long timeval()
{
	unsigned long rettime = 1;
	
	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;
	
	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);
	
	return rettime;
}


bool RTFGen::preLoad(std::string& strSrcFile, int nBandwidth, std::string& strFilename,std::string& strtrickspeed, int nTypeH264, int nSrcFileset)
{
	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	if (_gCPHCfg.bandwidthLimitRate)
	{
		nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
	}
	else
		nMaxBandwidth = nBandwidth;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTFRDS, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);
	
	BaseSource *pSource= NULL;
	const char* szFtpProtocol = "ftp://";
	if (nSrcFileset)
	{
		if (!strnicmp(strSrcFile.c_str(), szFtpProtocol, strlen(szFtpProtocol)))
		{
			std::string url = strSrcFile;
			std::string strPath = _gCPHCfg.szNTFSOutputDir;
			if(!( strPath[strPath.length()-1]=='\\' || strPath[strPath.length()-1]=='/'))
				strPath+="\\";
			std::string strFile = strPath + strFilename;


			FTPFilesetSource* ftpsource = (FTPFilesetSource*)SourceFactory::Create(SOURCE_TYPE_FTPFileset);
			AddFilter(ftpsource);
			ftpsource->setURL(url.c_str());
			ftpsource->setFilename(strFilename.c_str());
			ftpsource->setMaxBandwidth(nMaxBandwidth);
			ftpsource->setCacheDir(_gCPHCfg.szCacheDir);
			ftpsource->setTargetDir(strPath.c_str());
			ftpsource->setMode(_gCPHCfg.enableTestNTFS);
			pSource = ftpsource;
		}
	}
	else
	{
		if (strnicmp(strSrcFile.c_str(), szFtpProtocol, strlen(szFtpProtocol)))
		{
			std::string srcFilename = strSrcFile;
			fixpath(srcFilename);

			NTFSIOSource* ntfsSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC);
			AddFilter(ntfsSource);    //only after this, the log handle will be parsed in
			ntfsSource->setMaxBandwidth(nMaxBandwidth);	
			ntfsSource->setFileName(srcFilename.c_str());
			//ntfsSource->setUtf8(bUtf8Flag);
			pSource = ntfsSource;
		}
		else
		{
			std::string url = strSrcFile;

			FTPIOSource* ftpsource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP);
			AddFilter(ftpsource);
			ftpsource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
			ftpsource->setURL(url.c_str());
			ftpsource->setMaxBandwidth(nMaxBandwidth);
			pSource = ftpsource;
		}	

		
		std::list<int> trickspeed;
		if (strtrickspeed.empty())
			trickspeed.push_back(1);
		else
		{
			std::vector<std::string> strVec;
			strVec = ZQ::common::stringHelper::split(strtrickspeed,',');
			
			for (std::vector<std::string>::iterator it = strVec.begin();it != strVec.end(); it++)
			{
				trickspeed.push_back(int(ceil((atof((*it).c_str()))/float(7.5))));
			}
		}

		std::map<std::string, int> exMap;
		std::map<std::string, int>::iterator iter;
		int i = 0;
		trickspeed.sort();
		for (std::list<int>::iterator iter = trickspeed.begin();iter != trickspeed.end();iter++)
		{
			char ex[10]={0};
			char exr[10] ={0};

			if (nTypeH264)
			{
				getUnifiedTrickExt(i,ex);

				exMap.insert(std::make_pair(std::string(ex),(*iter)-1));		
			}
			else
			{
				getTrickExt(i,ex,exr);

				exMap.insert(std::make_pair(std::string(ex),(*iter)-1));
				exMap.insert(std::make_pair(std::string(exr),(*iter)-1));
			}
			i++;
		}

		int outPutNum = 2 + exMap.size();
		if (outPutNum < 2)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Not specify trick speed"), _strLogHint.c_str());
			return false;
		}

		RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF);
		AddFilter(pProcess);
		pProcess->setTrickFile(exMap);
		if (nTypeH264)
		{
#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
			pProcess->setTrickGenParam(RTF_INDEX_TYPE_VV2, RTF_VIDEO_CODEC_TYPE_H264);
#else
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
#endif
		}
		
		{
			FilesetTarget* pTarget = (FilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);

			if(!AddFilter(pTarget))
				return false;
			pTarget->setFilename(strFilename.c_str());
			pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
			pTarget->enableCacheForIndex(_gCPHCfg.enableCacheForIndex);
//			pTarget->enablePacing(false);
			pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);

			pTarget->setBandwidth(nMaxBandwidth);

			pTarget->enableProgressEvent(1);
			pTarget->enableMD5(_gCPHCfg.enableMD5);
			pTarget->setTrickFile(exMap);
			if (nTypeH264)
			{
				pTarget->setTypeH264();
				pTarget->enablePacing(true);
				pTarget->enableStreamableEvent(false);
			}
			else
			{
				pTarget->enablePacing(true);
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

int RTFGen::run( void )
{
	DWORD dwStart = GetTickCount();

	bool bRet = Run();
	if (!bRet)
	{
		Close();

		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return bRet;
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);

	Close();

	int nTimeSpentMs = GetTickCount() - dwStart;
	int nActualBps;
	if (nTimeSpentMs)
	{
		nActualBps = int(_llTotalBytes*8000/nTimeSpentMs);
	}
	else
	{
		nActualBps = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);

	return true;
}
void RTFGen::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
}

void RTFGen::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTFGen::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

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
	printf("       -l <specify log file name> default is \"RTFGen.log\"\n");
	printf("       -h display this help\n");	
}

void main(int argc, char** argv)
{
	if (argc <3)
	{
		usage();
		return;                                                                                                                                                                                                                                                                       
	}

	std::string strSrc, strFile, strCfgdir, strContentType,trickspeed,strLogName;
	int nBandwidth = 0;
	int nTypeH264 = 0;
	int nSrcFileset = 0;
    strCfgdir = ".\\";
	strLogName="";

	int ch;
	while((ch = getopt(argc, argv, "hHs:S:d:D:b:B:c:C:t:T:f:F:m:M:l:L:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
		case 'H':
			usage();
			return;

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
		default:
			printf("Error: unknown option %c specified\n\n", ch);
			usage();
			return;
		}
	}

	if (strLogName.empty())
		strLogName = "RTFGen.log";
	else
	{
		if (_stricmp(strLogName.c_str(),".log")!= 0)
			strLogName += ".log";
	}

	InitCPH(strCfgdir,strLogName);

	{
		RTFGen aaa;

		do
		{
			if (!aaa.preLoad(strSrc, nBandwidth, strFile,trickspeed, nTypeH264, nSrcFileset))
			{
				printf("failed to preload with error [%s], check RTFGen.log\n", aaa.GetLastError().c_str());
				break;
			}

			aaa.start();

			DWORD dwStart = GetTickCount();
			LONGLONG total = aaa.getTotalBytes();
			while(1)
			{
				DWORD result= aaa.waitHandle(500);
				if (result == WAIT_TIMEOUT)
				{	
					if (total)
					{
						int nTimeSpentMs = GetTickCount() - dwStart;
						int processRate = (aaa.getProcessBytes())*100/total;
						printf("\r time: %d seconds, done: %d%%",nTimeSpentMs/1000,processRate);
					}	
				}
				else if(result == WAIT_OBJECT_0)
				{
					int nTimeSpentMs = GetTickCount() - dwStart;
					printf("\r time: %d seconds, done: 100%%\n",nTimeSpentMs/1000);
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
}
