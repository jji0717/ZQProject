
#include "BaseClass.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_RDSCfg.h"
#include "TrickImportUser.h"
#include "FileLog.h"
#include "QueueBufMgr.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTFRDS			"CPH_RTFRDS"
#define MOLOG					(glog)

#define RDS_PROVISION_ERRCODE		1

ZQ::common::FileLog		filelog;

class TrickGen : public ZQTianShan::ContentProvision::BaseGraph, public QueueBufMgr
{
public:
	TrickGen()
		: _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}
	
	~TrickGen(){};
	
public:
	
	bool preLoad(std::string& strSrcFile, int nBandwidth, std::string& strFilename);

	void OnProgress(LONGLONG& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	bool run();
	
protected:
	void cleanup();
	
	bool _bQuit;
	bool _bCleaned;
	ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
};

void InitCPH(std::string& strCfgDir)
{
	filelog.open("TrickGen.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);

	if (!_gCPHRDSCfg.loadWithConfigFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to load configuration [%s]"), 
			_gCPHRDSCfg.getConfigFilePathName());		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Load configuration from [%s] successfully"),
			_gCPHRDSCfg.getConfigFilePathName());
	}	

	//
	// do some module initialize
	//
}

void UninitCPH()
{
	//
	//do some module uninitialize
	//
	VstrmFsTarget::uninitMyVstrm();

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


bool TrickGen::preLoad(std::string& strSrcFile, int nBandwidth, std::string& strFilename)
{
	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	if (_gCPHRDSCfg.bandwidthLimitRate)
	{
		nMaxBandwidth = int(((float)nBandwidth) * _gCPHRDSCfg.bandwidthLimitRate / 100);
	}
	else
		nMaxBandwidth = nBandwidth;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTFRDS, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHRDSCfg.bandwidthLimitRate, nMaxBandwidth);
	
	NTFSIOSource* pSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC);
	AddFilter(pSource);
	pSource->setMaxBandwidth(nMaxBandwidth);
	pSource->setFileName(strSrcFile.c_str());

	CTrickImportUser* pProcess = (CTrickImportUser*)ProcessFactory::Create(PROCESS_TYPE_TRICKGEN);
	AddFilter(pProcess);
	
	if (_gCPHRDSCfg.enableTestNTFS)
	{
		NTFSTarget* pTarget[4] = {0};
		pTarget[0]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[1]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[2]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[3]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		

		if(!AddFilter(pTarget[0]))
			return false;
		if(!AddFilter(pTarget[1]))
			return false;
		if (!AddFilter(pTarget[2]))
			return false;
		if (!AddFilter(pTarget[3]))
			return false;

		std::string strPath = _gCPHRDSCfg.szNTFSOutputDir;
		if(!( strPath[strPath.length()-1]=='\\' || strPath[strPath.length()-1]=='/'))
			strPath+="\\";
		std::string strFile = strPath + strFilename;
		
		pTarget[0]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".ff";
		pTarget[1]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".fr";
		pTarget[2]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".vvx";
		pTarget[3]->setFilename(strFile.c_str());
		pTarget[0]->enableStreamableEvent(_gCPHRDSCfg.enableStreamEvent);
		pTarget[0]->enableProgressEvent(_gCPHRDSCfg.enableProgEvent);
		_pMainTarget = pTarget[0];
		
		if (!ConnectTo(pSource, 0, pProcess, 0))
			return false;
		if (!ConnectTo(pProcess, 0, pTarget[0], 0))
			return false;
		if (!ConnectTo(pProcess, 1, pTarget[1], 0))
			return false;
		if (!ConnectTo(pProcess, 2, pTarget[2], 0))
			return false;
		if (!ConnectTo(pProcess, 3, pTarget[3], 0))
			return false;
	}
	else
	{
		VstrmFsTarget* pTarget = (VstrmFsTarget*)TargetFactory::Create(TARGET_TYPE_VSTRMFS);

		if(!AddFilter(pTarget))
			return false;
		pTarget->setFilename(strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHRDSCfg.szCacheDir);
		pTarget->enablePacing(true);
		pTarget->enablePacingTrace(_gCPHRDSCfg.enablePacingTrace);
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->enableStreamableEvent(_gCPHRDSCfg.enableStreamEvent);
		pTarget->enableProgressEvent(_gCPHRDSCfg.enableProgEvent);
		_pMainTarget = pTarget;
	

		if (!ConnectTo(pSource, 0, pProcess, 0))
			return false;
		if (!ConnectTo(pProcess, 0, pTarget, 0))
			return false;
		if (!ConnectTo(pProcess, 1, pTarget, 1))
			return false;
		if (!ConnectTo(pProcess, 2, pTarget, 2))
			return false;
		if (!ConnectTo(pProcess, 3, pTarget, 3))
			return false;
	}

	SetMediaSampleSize(_gCPHRDSCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

bool TrickGen::run()
{	
	bool bRet = Run();
	if (!bRet)
	{
		Close();
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStopped() called"), _strLogHint.c_str());
		return bRet;
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);

	Close();
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStopped() called"), _strLogHint.c_str());
	return true;
}

void TrickGen::OnProgress(LONGLONG& prcvBytes)
{
}

void TrickGen::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void TrickGen::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHRDSCfg.streamReqSecs/8);
	}
}

void usage()
{
	printf("usage: TrickGen <sourcefile> <targetfilename> <config file path> [bandwidth]\n");
}

void main(int argc, char** argv)
{
	if (argc <4)
	{
		usage();
		return;
	}

	std::string strSrc, strFile, strCfgdir;
	strSrc = argv[1];
	strFile = argv[2];
	strCfgdir = argv[3];

	int nBandwidth = 0;
	if (argc >4)
		nBandwidth = atoi(argv[4]);

	InitCPH(strCfgdir);

	do
	{
		TrickGen aaa;
		if (!aaa.preLoad(strSrc, nBandwidth, strFile))
		{
			printf("failed to preload with error [%s], check TrickGen.log\n", aaa.GetLastError().c_str());
			break;
		}

		if(!aaa.run())
		{
			printf("failed to run with error [%s], check TrickGen.log\n", aaa.GetLastError().c_str());
		}
		else
		{
			printf("trick generation success\n");
		}

		aaa.Close();
	}while(0);	

	UninitCPH();
}
