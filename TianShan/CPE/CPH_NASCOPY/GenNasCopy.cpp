
#include "BaseClass.h"
#include "NTFSSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_NasCopyCfg.h"
#include "ErrorCode.h"
#include "NasCopySource .h"
#include "TsStorage.h"
#include "FileLog.h"
#include "QueueBufMgr.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_NasCopy			"CPH_NasCopy"
#define MOLOG					(glog)

ZQ::common::FileLog		filelog;

class GenNasCopy : public ZQTianShan::ContentProvision::BaseGraph, public QueueBufMgr
{
public:
	GenNasCopy()
		: _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}

	~GenNasCopy(){};

public:

	bool preLoad(std::string& strSrcFile, std::string& strFilename,int bitrate);

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
	filelog.open("GenNasCopy.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);
	_gCPHCfg.setLogger(&glog);

	// load configurations
	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		
		_gCPHCfg.snmpRegister("");
	}	
}

void UninitCPH()
{
	//
	//do some module uninitialize

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
	
	static const char* szProtocol[] = {
		"file:",
		"cifs:",
		"nfs:"
	};
	static int nProto = sizeof(szProtocol)/sizeof(const char*);

	int i;
	for(i=0;i<nProto;i++)
	{
		if (!strnicmp(pathbuf, szProtocol[i], strlen(szProtocol[i])))
		{
			path = pathbuf + strlen(szProtocol[i]);
			break;
		}
	}
	if (i>=nProto)
	{
		//not found
		path = pathbuf;
	}
	
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



bool GenNasCopy::preLoad(std::string& srcFilename, std::string& strFilename,int bitrate)
{
	//
	// convert the file name if need
	//
	fixpath(srcFilename, false);
	
	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	NasCopySource* pSource = (NasCopySource*)SourceFactory::Create(SOURCE_TYPE_NASCOPY);//new NasCopySource();
	if (!pSource)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to initialize NasCopySource object"), _strLogHint.c_str(), _strLastErr.c_str());
	    return false;
	}
	AddFilter(pSource);
	pSource->setSourceFileName(srcFilename.c_str());
	pSource->setDestFileName(strFilename.c_str());
	pSource->setBandwith(bitrate);
	pSource->setCopyMainFileOnly(_gCPHCfg.bCopyMainFileOnly);	
	pSource->setVstrmBwClientId(_gCPHCfg.vstrmBwClientId);	
	

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}


	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}


bool GenNasCopy::run(void)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] run() called"), _strLogHint.c_str());
	::TianShanIce::Properties params;
	
	bool bRet = Run();
	if (!bRet)
	{			
		Close();
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}
	Close();
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return true;
}


void GenNasCopy::OnProgress(LONGLONG& prcvBytes)
{
}

void GenNasCopy::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void GenNasCopy::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
}

void usage()
{
	printf("usage: NasCopyGen <sourcefile> <targetfile> <config file path><bitrate>\n");
}

void main(int argc, char** argv)
{
	if (argc < 5)
	{
		usage();
		return;
	}

	std::string strSrc, strFile, strCfgdir;
	int bitrate;
	strSrc = argv[1];
	strFile = argv[2];
	strCfgdir = argv[3];
	bitrate = atoi(argv[4]);

	InitCPH(strCfgdir);

	do
	{
		GenNasCopy aaa;
		if (!aaa.preLoad(strSrc, strFile,bitrate))
		{
			printf("failed to preload with error [%s], check NasCopyGen.log\n", aaa.GetLastError().c_str());
			break;
		}

		if(!aaa.run())
		{
			printf("failed to run with error [%s], check NasCopyGen.log\n", aaa.GetLastError().c_str());
		}
		else
		{
			printf("NasCopyGen trick generation success\n");
		}

		aaa.Close();
	}while(0);	

	UninitCPH();
}
