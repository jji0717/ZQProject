
#include "BaseClass.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_FSCPCfg.h"
#include "RTFProc.h"
#include "FileLog.h"
#include "QueueBufMgr.h"
#include "NTFSFileSetSource .h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_FSCP			"CPH_FSCP"
#define MOLOG					(glog)

#define FSCP_PROVISION_ERRCODE		1

ZQ::common::FileLog		filelog;

class FSCPGen : public ZQTianShan::ContentProvision::BaseGraph, public QueueBufMgr
{
public:
	FSCPGen()
		: _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}
	
	~FSCPGen(){};
	
public:
	
	bool preLoad(std::string& strSrcFile, std::string& strFilename);

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
	filelog.open("FSCPGen.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);

	if (!_gCPHCfg.loadWithConfigFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePathName());		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePathName());
	}	
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


bool FSCPGen::preLoad(std::string& strSrcFile, std::string& strFilename)
{
	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth = 10000000;//bps
	std::string sourcedir = strSrcFile.substr(0, strSrcFile.find_last_of('\\'));
	strSrcFile = strSrcFile.substr(strSrcFile.find_last_of('\\')+1, strSrcFile.size()-strSrcFile.find_last_of('\\')-1);
	std::string cachdir = _gCPHCfg.szCacheDir;
	NTFSFileSetSource* pSource =  (NTFSFileSetSource*)SourceFactory::Create(SOURCE_TYPE_NTFS_FILESET);
	AddFilter(pSource);
	pSource->setFilename(strSrcFile.c_str());
	pSource->setDirectory(cachdir.c_str(),sourcedir.c_str());
	
	
	if (_gCPHCfg.enableTestNTFS)
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

		std::string strPath = _gCPHCfg.szNTFSOutputDir;
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
		pTarget[0]->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		pTarget[0]->enableProgressEvent(_gCPHCfg.enableProgEvent);
		_pMainTarget = pTarget[0];
		
		if (!ConnectTo(pSource, 0, pTarget[0], 0))
			return false;
		if (!ConnectTo(pSource, 1, pTarget[1], 0))
			return false;
		if (!ConnectTo(pSource, 2, pTarget[2], 0))
			return false;
		if (!ConnectTo(pSource, 3, pTarget[3], 0))
			return false;
	}
	else
	{
		VstrmFsTarget* pTarget = (VstrmFsTarget*)TargetFactory::Create(TARGET_TYPE_VSTRMFS);
		
		if(!AddFilter(pTarget))
			return false;

		pTarget->setFilename(strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->enablePacing(true);
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		_pMainTarget = pTarget;
		
		if (!ConnectTo(pSource, 0, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 1, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 2, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 3, pTarget, 0))
			return false;
	}
	
	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);
	
	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}
	
	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

bool FSCPGen::run()
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

void FSCPGen::OnProgress(LONGLONG& prcvBytes)
{
}

void FSCPGen::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void FSCPGen::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

void usage()
{
	printf("usage: FSCPGen <sourcefile> <targetfile> <config file path>\n");
}

void main(int argc, char** argv)
{
	if (argc < 4)
	{
		usage();
		return;
	}

	std::string strSrc, strFile, strCfgdir;
	strSrc = argv[1];
	strFile = argv[2];
	strCfgdir = argv[3];

	InitCPH(strCfgdir);

	do
	{
		FSCPGen aaa;
		if (!aaa.preLoad(strSrc, strFile))
		{
			printf("failed to preload with error [%s], check FSCPGen.log\n", aaa.GetLastError().c_str());
			break;
		}

		if(!aaa.run())
		{
			printf("failed to run with error [%s], check FSCPGen.log\n", aaa.GetLastError().c_str());
		}
		else
		{
			printf("FSCP trick generation success\n");
		}

		aaa.Close();
	}while(0);	

	UninitCPH();
}
