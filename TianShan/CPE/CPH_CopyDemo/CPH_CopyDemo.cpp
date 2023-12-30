#include "CPH_CopyDemo.h"
#include "UrlStr.h"

const char* outputdir = "d:\\temp\\";

ZQ::common::NativeThreadPool gPool;

ZQTianShan::ContentProvision::BaseCPHelper* CopyDemoHelper::_theHelper =NULL;

extern "C" __declspec(dllexport) bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return FALSE;

	if (!CopyDemoHelper::_theHelper)
		CopyDemoHelper::_theHelper = new CopyDemoHelper(gPool, pEngine);

	pEngine->registerHelper(METHODTYPE_COPYDEMO, CopyDemoHelper::_theHelper, pCtx);
	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_COPYDEMO, pCtx);

	if (CopyDemoHelper::_theHelper)
	{
		try
		{
			delete CopyDemoHelper::_theHelper;
		}
		catch(...){};

		CopyDemoHelper::_theHelper = NULL;
	}
}


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

CopyDemoSess::CopyDemoSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		: BaseCPHSession(helper, pSess), _hSource(INVALID_HANDLE_VALUE), _hDest(INVALID_HANDLE_VALUE),
		_filesize(0), _copied(0), _maxKbps(56), _buf(NULL), _bQuit(false)
{
}

CopyDemoSess::~CopyDemoSess()
{
	ZQ::common::MutexGuard gd(*this);
	doCleanup();
}

void CopyDemoSess::doCleanup()
{
	if (INVALID_HANDLE_VALUE != _hSource)
		::CloseHandle(_hSource);
	_hSource = INVALID_HANDLE_VALUE;
	
	if (INVALID_HANDLE_VALUE != _hDest)
		::CloseHandle(_hDest);
	_hDest = INVALID_HANDLE_VALUE;

	if (NULL != _buf)
		_helper.getMemoryAllocator()->free(_buf);

	_buf = NULL;
}


bool CopyDemoSess::preLoad()
{
	if (!_sess || 0!= _sess->methodType.compare(METHODTYPE_COPYDEMO))
		return false;

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
		return false;

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtProvisionBandwidth))
		return false;

	TianShanIce::ValueMap& res =  _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (res.end() == res.find(CPHPM_SOURCEURL))
		return false;
	
	TianShanIce::Variant& var = res[CPHPM_SOURCEURL];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
		return false;

	{
		ZQ::common::URLStr Url(var.strs[0].c_str());
		
		if (0 !=stricmp("file", Url.getProtocol()))
		{
			//		error(ZQ::common::Log::L_ERROR, "protocol \"%s\" is not supported", srcUrl.getProtocol());
			return false;
		}
		
		std::string host = Url.getHost();
		_sourceFilename = Url.getPath();
		
		if (host.empty() || 0 == host.compare(".") || 0 == host.compare("localhost"))
			fixpath(_sourceFilename, true);
		else
		{
			fixpath(_sourceFilename, false);
			_sourceFilename = std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + Url.getHost() + LOGIC_FNSEPS +_sourceFilename; 
			fixpath(_sourceFilename, false);
		}
	}
		
	if (res.end() == res.find(CPHPM_FILENAME))
		return false;
	
	var = res[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
		return false;
	
	{
		_destFilename = outputdir + var.strs[0];

	}

	res =  _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (res.end() != res.find(CPHPM_BANDWIDTH) && res[CPHPM_BANDWIDTH].lints.size()>0)
		_maxKbps = (int) (res[CPHPM_BANDWIDTH].lints[0] / 1000);

	BaseCPHSession::preLoad();
	return (!(_sourceFilename.empty() || _destFilename.empty()));
}

void CopyDemoSess::terminate(bool cleanupIncompletedOutput)
{
	_bQuit = true;
}

bool CopyDemoSess::getProgress(Ice::Long& offset, Ice::Long& total)
{

	return true;
}

int CopyDemoSess::run(void)
{
	std::string strSessId = _sess->ident.name;

	// copy file at a given speed
	_hSource = ::CreateFile(_sourceFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == _hSource)
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "failed to open source file: %s", _sourceFilename.c_str());
		return -3;
	}
	
	// verify the free space of the content
	DWORD dwSizeLow, dwSizeHigh;
	dwSizeLow = GetFileSize( _hSource, &dwSizeHigh);
	_filesize = dwSizeHigh;
	_filesize <<=32;
	_filesize +=dwSizeLow;

	
	_hDest = ::CreateFile(_destFilename.c_str(), GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	if (INVALID_HANDLE_VALUE == _hDest)
	{
		// error(ZQ::common::Log::L_ERROR, "failed to open destination file to write: %s", destPath.c_str());
		::CloseHandle(_hSource);
		_hSource = INVALID_HANDLE_VALUE;
		return -4;
	}

	TianShanIce::Properties startparams;
	notifyStarted(startparams);

#define MAX_BUF_SZ (1024*64)
	
	DWORD nBytesRead=0;
	DWORD nByteWritten=0;

	DWORD nMaxRead = MAX_BUF_SZ;
	if (_maxKbps >0)
	{
		nMaxRead = _maxKbps * 1024/8/2; // should be read within 500ms ea
	}
	
	if (nMaxRead > MAX_BUF_SZ)
		nMaxRead = MAX_BUF_SZ;
	
	_buf = (char*)_helper.getMemoryAllocator()->alloc(nMaxRead);

	if (NULL == _buf)
	{
		// error(ZQ::common::Log::L_ERROR, "out of memory for read buffer");
		::CloseHandle(_hSource);
		::CloseHandle(_hDest);
		return -5;
	}
	
	try
	{
		int lasttimer = timeval();
		
		// attempt an asynchronous read operation
		while(!_bQuit && ::ReadFile(_hSource, _buf, nMaxRead, &nBytesRead, NULL))
		{
			::WriteFile(_hDest, _buf, nBytesRead, &nByteWritten, NULL);
			if (nByteWritten != nBytesRead)
			{
				// error(ZQ::common::Log::L_ERROR, "failed to write: ByteToWrite=%d but ByteWritten=%d", nBytesRead, nByteWritten);
				::CloseHandle(_hSource);
				::CloseHandle(_hDest);
				return -5;
			}
			
			_copied +=nByteWritten;
			
			if (_maxKbps >0)
			{
				int expected = nByteWritten *8 / _maxKbps; // in msec
				expected *= 0.99; // decrease a bit for this computing cycle
				
				int now = timeval();
				int interval = (lasttimer > now) ? 
					(((long)(-1) - lasttimer) + now + 1) 
					: now - lasttimer;    //if overflow
				
				if (!_bQuit && interval < expected)
				{
					//						printf("\nprocessed %dbytes expected=%dms but used %dms\n", nByteWritten, expected, interval);
					::Sleep(expected - interval);
				}
				
				lasttimer = timeval();
			}
			
//			updateProgress(_copied, _filesize);

			if (nBytesRead < nMaxRead)
				break;
		}
	}
	catch(...) {}

	::TianShanIce::Properties params;
	printf("%s call notifyStopped\n",strSessId.c_str ());
	notifyStopped(false, params);
	return 0;
}
