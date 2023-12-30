#include "CPH_CopyDemo.h"
#include "urlstr.h"
#include "TianShanDefines.h"

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
}

namespace {
    /* remeber the last timestamp and session number within a millisecond */
    int64 times = ZQTianShan::now();
    short cnt = 0;
    ZQ::common::Mutex tlock;
}

const char* outputdir = "/tmp/";

ZQ::common::NativeThreadPool gPool;

ZQTianShan::ContentProvision::BaseCPHelper* CopyDemoHelper::_theHelper =NULL;

extern "C"
{
bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if(pEngine == NULL)
		return false;

	if(!CopyDemoHelper::_theHelper)
		CopyDemoHelper::_theHelper = new CopyDemoHelper(gPool, pEngine);

	pEngine->registerHelper(METHODTYPE_COPYDEMO, CopyDemoHelper::_theHelper, pCtx);
	return true;
}

void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if(pEngine == NULL)
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
}


CopyDemoSess::CopyDemoSess(ZQTianShan::ContentProvision::BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
		: BaseCPHSession(helper, pSess), _hSource(NULL), _hDest(NULL), _bLocal(false),
		_filesize(0), _copied(0),  _maxKbps(56), _buf(NULL), _bQuit(false)
{
}

CopyDemoSess::~CopyDemoSess()
{
	ZQ::common::MutexGuard gd(*this);
	doCleanup();
}

void CopyDemoSess::doCleanup()
{

	if(_hSource != NULL)
	{
		fclose(_hSource);
		_hSource = NULL;
	}
	
	if(_hDest != NULL )
	{
		fclose(_hDest);
		_hDest = NULL;
	}

	if (_buf != NULL)
	{
		_helper.getMemoryAllocator()->free(_buf);
		_buf = NULL;
	}
	
	umountFile(_strTarget);
}

bool CopyDemoSess::mountFile(const std::string& strurl, const std::string& session)
{

	char chT[50];
    memset(chT, '\0', 50);

    {
        ZQ::common::MutexGuard gd(tlock);

        int64 times2 = ZQTianShan::now();
        if(times2 == times) {
            sprintf(chT, "%ld-%d", times2, ++cnt);
        }
        else {
            sprintf(chT,"%ld",times2);
            cnt = 0;
            times = times2;
        }
    }

	_strTarget = std::string("/mnt/") + chT;

	std::string source = "//";
	std::string stropt;

	char* p = strchr(strurl.c_str(),'@');
	if(p != NULL)
	{
		ZQ::common::URLStr url(strurl.c_str());
		std::string username = url.getUserName();
		std::string password = url.getPwd();
		source += p+1;
		stropt = "username=" + username + ",password=" + password;
	}
	else
	{
		p = strchr(strurl.c_str(),':');
		if(p == NULL)
			return false;
			
		source = p+1;
		stropt = "username=,password=";
	}
	
	const char* sbegin = source.c_str();
	sbegin += 2;
	if(*sbegin == '/')//local file 
	{
		_bLocal = true;
		_sourceFilename = sbegin;
		return true;
	}
	
	for(int b = 1; b < 3; b++)
	{
		p = strchr(sbegin,'/');
		if(p == NULL)
			return false;
		sbegin = p+1;
	}	
	
	_sourceFilename = _strTarget + p;
	*p = '\0';
	
	int res = mkdir(_strTarget.c_str(), 0755);
	if(res != 0) {
		EngSessLog(ZQ::common::Log::L_ERROR, 
            "failed to create directory [%s] for [%s], error: [%d] [%m]", 
            _strTarget.c_str(), session.c_str(), errno);
		return false;
	}	
    else {
    	EngSessLog(ZQ::common::Log::L_INFO, 
            "created directory (%s) for content (%s)", _strTarget.c_str(), session.c_str());
    }
		
	short retry = 0;
mount:
	res = mount(source.c_str(),_strTarget.c_str(),"cifs",MS_RDONLY,(void*)stropt.c_str());
	if(res != 0) {
        if(errno == EAGAIN) {
			++retry;
            EngSessLog(ZQ::common::Log::L_ERROR, 
                "temporarily fail to mount [%s] to [%s] for [%s] retry [%d]", 
				source.c_str(),_strTarget.c_str(), session.c_str(), retry);

			if(retry <= 3) {
				sleep(1);
				goto mount;
			}
        }
		rmdir(_strTarget.c_str());
		EngSessLog(ZQ::common::Log::L_ERROR, 
            "failed to mount [%s] to target [%s] for [%s] with option [%s], error: [%d] [%m]", 
            source.c_str(),_strTarget.c_str(), session.c_str(), stropt.c_str(),errno);
		return false;
	}
	
	EngSessLog(ZQ::common::Log::L_INFO, 
            "mount [%s] to target for [%s] successfully", 
            source.c_str(),_strTarget.c_str(), session.c_str());
	
	return true;
}

void CopyDemoSess::umountFile(const std::string& strTarget)
{
	if(strTarget.empty() || _bLocal) return;

	try {
		int res = umount(strTarget.c_str());
		if(res != 0) {
			EngSessLog(ZQ::common::Log::L_WARNING, 
					"umount target [%s] failed, error: [%d] [%m]", strTarget.c_str(), errno);
        }
        else {
			EngSessLog(ZQ::common::Log::L_WARNING, 
					"umount target [%s] successfully", strTarget.c_str());
            rmdir(strTarget.c_str());
        }
	}
	catch(...){}
}


bool CopyDemoSess::preLoad()
{
	if (!_sess || 0!= _sess->methodType.compare(METHODTYPE_COPYDEMO))
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the method type is not match, type[%s]",METHODTYPE_COPYDEMO);
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the session not [TianShanIce::SRM::rtURI] source");
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtProvisionBandwidth))
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the session not [TianShanIce::SRM::rtProvisionBandwidth] source");
		return false;
	}

	TianShanIce::ValueMap& res =  _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (res.end() == res.find(CPHPM_SOURCEURL))
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the session not [%s] source",CPHPM_SOURCEURL);
		return false;
	}
	
	TianShanIce::Variant& var = res[CPHPM_SOURCEURL];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the session not [%s] source",CPHPM_SOURCEURL);
		return false;
	}
	std::string src = var.strs[0];

	if (res.end() == res.find(CPHPM_FILENAME))
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() not find [%s]",CPHPM_FILENAME);
		umountFile(_strTarget);
		return false;
	}
	
	var = res[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() [%s] is invalidate",CPHPM_FILENAME);
		umountFile(_strTarget);
		return false;
	}
	std::string filename = var.strs[0];
	EngSessLog(ZQ::common::Log::L_INFO, "CopyDemoSess::preLoad() content file name [%s]",filename.c_str());
	std::string::size_type pos = filename.find_last_of('/');
	if(pos != std::string::npos) {
//		filename = filename.substr(pos+1);
		_destFilename = filename;
	}
	else
		_destFilename = outputdir + filename;

	if(_destFilename[0] != '/')
		_destFilename = "/" + _destFilename;
	
	res =  _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (res.end() != res.find(CPHPM_BANDWIDTH) && res[CPHPM_BANDWIDTH].lints.size()>0)
		_maxKbps = (int) (res[CPHPM_BANDWIDTH].lints[0] / 1000);

	{
		ZQ::common::URLStr Url(src.c_str());
		
		if (0 !=strcasecmp("file", Url.getProtocol()))
		{
			//		error(ZQ::common::Log::L_ERROR, "protocol \"%s\" is not supported", srcUrl.getProtocol());
			EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() the URL [%s] is not [file] protocol",var.strs[0].c_str());
			return false;
		}
	
		bool res = mountFile(src, filename);
		if(!res)
		{
			EngSessLog(ZQ::common::Log::L_ERROR, "CopyDemoSess::preLoad() failed to mount URL[%s]",var.strs[0].c_str());
			return false;
		}
	}

	BaseCPHSession::preLoad();
	return true;
}

void CopyDemoSess::terminate(bool cleanupIncompletedOutput)
{
	_bQuit = true;
}

bool CopyDemoSess::getProgress(Ice::Long& offset, Ice::Long& total)
{
	total=_filesize;
	offset = _copied;
	return true;
}

int CopyDemoSess::run(void)
{
	struct stat pstat;
	int result = stat(_sourceFilename.c_str(), &pstat);
	if(result == -1)
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "failed to get file [%s] size, error [%d] [%m] ", _sourceFilename.c_str(), errno);
		umountFile(_strTarget);
		return -1;
	}
	_filesize = pstat.st_size;
	EngSessLog(ZQ::common::Log::L_INFO, "file size is [%lld]", _filesize);
		

	std::string strSessId = _sess->ident.name;

	// copy file at a given speed
	_hSource = fopen(_sourceFilename.c_str(), "r");
	if (_hSource == NULL)
	{
		EngSessLog(ZQ::common::Log::L_ERROR, "failed to open source file: %s", _sourceFilename.c_str());
		umountFile(_strTarget);
		return -1;
	}
	
	// verify the free space of the content

	
	_hDest = fopen(_destFilename.c_str(), "w+");
	if (_hDest == NULL)
	{
		// error(ZQ::common::Log::L_ERROR, "failed to open destination file to write: %s", destPath.c_str());
		EngSessLog(ZQ::common::Log::L_ERROR, "failed to open destination file: %s", _destFilename.c_str());
		umountFile(_strTarget);
		fclose(_hSource);
		_hSource = NULL;
		return -2;
	}

	TianShanIce::Properties startparams;
	char tmp[64];
	sprintf(tmp, "%ld", static_cast<int64>(_filesize));
	startparams[EVTPM_TOTOALSIZE] = tmp;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d",_maxKbps);
	startparams[EVTPM_MPEGBITRATE] = tmp;
	notifyStarted(startparams);

#define MAX_BUF_SZ (1024*64)
	
	uint32 nBytesRead=0;
	uint32 nByteWritten=0;

	uint32 nMaxRead = MAX_BUF_SZ;
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
		EngSessLog(ZQ::common::Log::L_ERROR, "get read buffer memory failed");
		umountFile(_strTarget);
		fclose(_hSource);
		_hSource = NULL;
		fclose(_hDest);
		_hDest = NULL;
		return -3;
	}
	
	try
	{
		int64 lasttimer = ZQTianShan::now();
		
		// attempt an asynchronous read operation
		while(!_bQuit && (nBytesRead = fread(_buf,1, nMaxRead, _hSource)))
		{
			nByteWritten = fwrite(_buf, 1, nBytesRead,_hDest);
			if (nByteWritten != nBytesRead)
			{
				// error(ZQ::common::Log::L_ERROR, "failed to write: ByteToWrite=%d but ByteWritten=%d", nBytesRead, nByteWritten);
				EngSessLog(ZQ::common::Log::L_ERROR, "write file [%s] failed",_destFilename.c_str());
				umountFile(_strTarget);
				fclose(_hSource);
				_hSource = NULL;
				fclose(_hDest);
				_hDest = NULL;
				return -4;
			}
			
			_copied +=nByteWritten;
			
			if (_maxKbps >0)
			{
				int expected = nByteWritten *8 / _maxKbps; // in msec
				expected = int(0.99*expected); // decrease a bit for this computing cycle
				
				int64 now = ZQTianShan::now();
				int interval = (lasttimer > now) ? 
					(((int64)(-1) - lasttimer) + now + 1) 
					: now - lasttimer;    //if overflow
				
				if (!_bQuit && interval < expected)
				{
					//						printf("\nprocessed %dbytes expected=%dms but used %dms\n", nByteWritten, expected, interval);
					usleep((expected - interval)*1000);
				}
				
				lasttimer = ZQTianShan::now();
			}
			updateProgress(_copied, _filesize);
			if (nBytesRead < nMaxRead)
				break;
		}
	}
	catch(...) {}

	EngSessLog(ZQ::common::Log::L_INFO, "Copied file size is [%lld]", _copied);
	TianShanIce::Properties params;
	notifyStopped(false, params);
	
	/*
	fclose(_hSource);
	_hSource = NULL;
	fclose(_hDest);
	_hDest = NULL;
	umountFile(_strTarget);
	*/

	return 0;
}
