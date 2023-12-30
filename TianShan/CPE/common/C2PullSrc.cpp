#include "C2PullSrc.h"
#include "Log.h"
#include "ErrorCode.h"
#include "urlstr.h"
#include "assert.h"
#include "SystemUtils.h"
#include "TimeUtil.h"
#include "ParseIndexFile.h"

//if this macro defined, the stream dump will be in capture thread
//if not, dump will be in trick generation thread
//the diffrent is:
//1. if disk io is not quick enough, when in capture thread, maybe it would infect the stream capture
//2. dump in capture thread would get more data
#define DUMP_IN_CAPTURE_THREAD


#define C2PullSrc			"C2PullSrc"

using namespace ZQ::common;

#define MOLOG (*_pLog)
namespace ZQTianShan {
	namespace ContentProvision {

/*bool getIdxSubFileInfo(const char* szIdxFile, std::string  strIndexExt, std::vector<SubFileInfo>& subFiles, MediaInfo& info)
{
	int64 SparseFileSize = 20000000000;
	if (stricmp(strIndexExt.c_str(), "VVX") == 0)
	{
		VvxParser vvx;
		if(!vvx.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.videoResolutionH = vvx.getVideoHorizontalSize(); 
		info.videoResolutionV = vvx.getVideoVerticalSize();
		info.bitrate = vvx.GetBitRate();
		info.framerate = atof(vvx.getFrameRateString(vvx.getFrameRateCode()));
		info.playTime = vvx.GetPlayTime();

//		subFiles.resize(vvx.getSubFileCount());
		for(int i=0;i<vvx.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;
			subinfo.bIsSparseFile = false;

			vvx.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vvx.getSubFileSpeed((uint32)i,subinfo.numerator,*(uint32*)&subinfo.denominator);
			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}
			subFiles.push_back(subinfo);
		}		
	}
	else if(stricmp(strIndexExt.c_str(), "VV2") == 0)
	{
		VV2Parser vv2;
		if(!vv2.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.bitrate = vv2.getBitrate();
		info.playTime = vv2.getPlayTime();

//		subFiles.resize(vv2.getSubFileCount());
		for(unsigned int i=0;i<vv2.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;
			subinfo.bIsSparseFile = false;
			vv2.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vv2.getSubFileSpeed(i, subinfo.numerator, subinfo.denominator, subinfo.direction);	
			subFiles.push_back(subinfo);		
		}	

		
	}
	else if(stricmp(strIndexExt.c_str(), "index") == 0)
	{
        ZQ::IdxParser::IdxParserEnv  env;
		env.AttchLogger(&glog);
		ZQ::IdxParser::IndexFileParser idxParser(env);
		ZQ::IdxParser::IndexData	idxData;
		if(!idxParser.ParserIndexFileFromCommonFS("",idxData, true, szIdxFile)) {
			return false;
		}
		
		info.videoResolutionH = idxData.getVideoHorizontalSize(); 
		info.videoResolutionV = idxData.getVideoVerticalSize();
		info.bitrate = idxData.getMuxBitrate();
		info.framerate = atof(idxData.getFrameRateString());
		info.playTime = idxData.getPlayTime();

//		subFiles.resize(idxData.getSubFileCount());
		for(int i=0;i<idxData.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			ZQ::IdxParser::IndexData::SubFileInformation subfileinfo;
			idxData.getSubFileInfo(i, subfileinfo);
			subinfo.firstOffset = subfileinfo.startingByte;
			subinfo.finalOffset = subfileinfo.endingByte;

			if(subinfo.firstOffset > 0 && subinfo.finalOffset > 0 && subinfo.firstOffset > SparseFileSize)
				subinfo.bIsSparseFile = true;
			else
				subinfo.bIsSparseFile = false;
			subinfo.totalFilesize = subfileinfo.fileSize;
			const std::string& strExtension =  idxData.getSubFileName( i ) ;
			memset(subinfo.ext, 0 , sizeof(subinfo.ext));
			strncpy(subinfo.ext, strExtension.c_str(), sizeof(subinfo.ext));
			
			SPEED_IND speed = idxData.getSubFileSpeed((uint32)i);
			subinfo.numerator = speed.numerator;
			subinfo.denominator = speed.denominator;

			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}

			subFiles.push_back(subinfo);
		}		
	}
	else
		return false;

	return true;
}*/
C2PullSource::C2PullSource()	
{
	_strProvideAssetId = "";
	_strProvideId = "";
	_timeout = 10;
	_nspeed = 1;
	_transferDelay = 0;

	_pMediaSample = NULL;

	_bProcessDataError = false;

	_nOutputCount = 1;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}
	for(i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}

	_llProcBytes = 0;
	_bDriverModule = false;
	_offset = 0;
	_bIsEndOfData = false;
}

C2PullSource::~C2PullSource()
{	
	Close();
}
void C2PullSource::setCacheDir(const char* szCachDir)
{
	if (szCachDir[strlen(szCachDir)-1] != FNSEPC)
		_tempDir = szCachDir + std::string(FNSEPS);
	else
		_tempDir = szCachDir;
}
void C2PullSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;

	MOLOG(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}
bool C2PullSource::Init()
{
	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] init()"), _strLogHint.c_str());

	_pMediaSample = acquireOutputBuffer();
	if (!_pMediaSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] failed to initializ C2PullSource with error: Failed to allocate media sample "),_strLogHint.c_str());
		SetLastError(std::string("initializ C2PullSource with error: Failed to allocate media sample") , ERRCODE_BUFFERQUEUE_FULL);
		return false;
	}

	ZQ::common::URLStr srcUrl(_srcURL.c_str());
	_strServer = srcUrl.getHost();
	int nPort = srcUrl.getPort();
	{
		char strTempPort[64] = "";
		itoa(nPort, strTempPort, 10);
		_strPort = strTempPort;
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%s] SourceUrl=%s, Host=%s, Port=%s"),_strLogHint.c_str(), _srcURL.c_str(), _strServer.c_str(), _strPort.c_str());

//	ZQTianShan::ContentProvision::C2HttpClient* pc2HttpClient = __pHttpClientFactory->create();
	ZQTianShan::ContentProvision::C2HttpClient* pc2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, _pLog,_timeout, _bindIP,_transferip);
	if (!pc2HttpClient)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] failed to create HTTPClient instance"), _strLogHint.c_str());
		SetLastError(std::string("initializ C2PullSource with error: failed to create HTTPClient instance ") , ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		return false;
	}
	_pHttpDownloader.reset(pc2HttpClient);
	_pHttpDownloader->setIngressCapacity(_IngressCapcaity);
	_pHttpDownloader->setPIdAndPaid(_strProvideId, _strProvideAssetId);
	
	char szBuf[512];
	//cache index file in a temp directory
	_strTempIndexFile = _tempDir +  _strProvideAssetId + _strProvideId + ".index";	
	if (!downloadIndexFile())
		return false;

	//get IndexFile extension
	int npos = _strTempIndexFile.rfind('.');
	if(npos <  0)
	{
		snprintf(szBuf, sizeof(szBuf)-1, "failed to parser index file %s extension", _strTempIndexFile.c_str());	
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
		SetLastError(std::string(szBuf) , ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		remove(_strTempIndexFile.c_str());
		return false;
	}
	std::string strIndexExt = _strTempIndexFile.substr(npos + 1);
	// 	std::string strOrgFile =  indexFilename.substr(0, npos);

	std::vector<SubFileInfo> subFiles;
	std::vector<SubFileInfo>::iterator iter;

	if(!ParseIndexFileInfo::getIdxSubFileInfo(_strTempIndexFile.c_str(), strIndexExt, subFiles, _info))
	{
		//failed to parse the index file
		snprintf(szBuf, sizeof(szBuf)-1, "Failed to parse VVX/VV2/INDEX index file for [PID: %s][PAID: %s]", _strProvideId.c_str() , _strProvideAssetId.c_str());
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
		SetLastError(std::string(szBuf) , ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		remove(_strTempIndexFile.c_str());
		return false;
	}

	remove(_strTempIndexFile.c_str());

	//get Main file Info;
	iter = subFiles.begin();

	_mainFileset.filename      = iter->ext;;
	_mainFileset.firstOffset   = iter->firstOffset;
	_mainFileset.finalOffset   = iter->finalOffset;
	_mainFileset.totalFilesize = iter->totalFilesize;
	_mainFileset.bIsSparseFile = iter->bIsSparseFile;

	MOLOG(Log::L_WARNING, CLOGFMT(C2PullSrc, "[%s] initialize C2Pull source: MainFile name[%s], totoalsize[%lld], firstOffset[%lld],finalOffset[%lld]"), 
		_strLogHint.c_str(),_mainFileset.filename.c_str(), _mainFileset.totalFilesize, _mainFileset.firstOffset, _mainFileset.finalOffset);

	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] initialized"), _strLogHint.c_str());
	return true;
}

bool C2PullSource::Start()
{
	return NativeThread::start();
}
int C2PullSource::run(void)
{
	bool bRet = true;
	if (!downloadMainFile())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] failed to download main file"), _strLogHint.c_str());
		bRet = false;
	}

	_hDownLoadComplete.signal();

	if (_pMediaSample)
	{
		releaseOutputBuffer(_pMediaSample);
		_pMediaSample = NULL;
	}
	if(!_bIsEndOfData)
		endOfStream();

	return bRet;
}
void C2PullSource::Stop()
{
	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] Stop() called"), _strLogHint.c_str());
	if(_pHttpDownloader.get())
		_pHttpDownloader->setError(true, "C2PullSource closed");

	_hDownLoadComplete.wait();

	if (_pMediaSample)
	{
		releaseOutputBuffer(_pMediaSample);
		_pMediaSample = NULL;
	}
	if(!_bIsEndOfData)
		endOfStream();
}

void C2PullSource::Close()
{
	if (_pMediaSample)
	{
		releaseOutputBuffer(_pMediaSample);
		_pMediaSample = NULL;
	}

	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] Close() enter"), _strLogHint.c_str());		

	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
		if (_downloaded.size())
		{
			MOLOG(Log::L_WARNING, CLOGFMT(C2PullSrc, "[%s] there is still [%d] captured data sample not processed"), _strLogHint.c_str(), _downloaded.size());

			while(_downloaded.size()>0)
			{
				MediaSample* pSample = _downloaded.front();		
				_downloaded.pop_front();
				GetGraph()->freeMediaSample(pSample);
			}
		}
	}	

	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] Closed"), _strLogHint.c_str());		
}

void C2PullSource::endOfStream()
{
	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] endOfStream() called"), _strLogHint.c_str());

	_bIsEndOfData = true;
}

const char* C2PullSource::GetName()
{
	return SOURCE_TYPE_C2PULL;
}

int64 C2PullSource::getProcessBytes()
{
	return _llProcBytes;
}

bool C2PullSource::getSample(MediaSample*& pSample)
{	
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
	if (_downloaded.size()>0)
	{
		pSample = _downloaded.front();		
		_downloaded.pop_front();
		return true;
	}

	pSample = NULL;
	return !_bIsEndOfData;
}

MediaSample* C2PullSource::GetData(int nOutputIndex)
{
	MediaSample* pSample;

	while(getSample(pSample))
	{
		if (pSample)
		{		
			return pSample;
		}
		SYS::sleep(1);
	};

	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] End of download file"), _strLogHint.c_str());
	return NULL;
}

MediaSample* C2PullSource::acquireOutputBuffer()
{
	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] acquireOutputBuffer() failed to allocate mediasample"), _strLogHint.c_str());		
		SetLastError(std::string("McastCap: failed to allocate mediasample"), ERRCODE_BUFFERQUEUE_FULL);
	}

	return pSample;
}

void C2PullSource::releaseOutputBuffer( MediaSample* pSample )
{
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] releaseOutputBuffer() with NULL pointer"), _strLogHint.c_str());
		return;
	}
	if (pSample->getDataLength())
	{
		pSample->setOffset(_offset);
		_offset += pSample->getDataLength();

		_lock.enter();
		_downloaded.push_back(pSample);
		_lock.leave();
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] In releaseOutputBuffer(), the dataLen parameter is 0"), _strLogHint.c_str());		
		GetGraph()->freeMediaSample(pSample);
	}
}
bool C2PullSource::downloadIndexFile()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%s][%s][%s]download index file"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str());
	int64 lStart = ZQ::common::now();

	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	
	int64 maxLen = -1;

	_pHttpDownloader->setSubfile("index");
//	std::string deleteUrl = std::string("http://") + _strServer + ":" + _strPort;
    std::string deleteUrl = _srcURL;

//	url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
	url = _srcURL;

	locateRequest.pid  = _strProvideId;
	locateRequest.paid = _strProvideAssetId;
	locateRequest.subFile = "index";
	locateRequest.beginPos = -1;
	locateRequest.endPos = -1;
	locateRequest.bitRate = 3750000;
	locateRequest.transferDelay = _transferDelay;

	if(!_pHttpDownloader->prepareLocateFile(url, locateRequest, locateResponse))
	{
		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s][%s][%s]failed to prepare locate index file with error: %s"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str(),errstr.c_str());
		SetLastError(std::string("C2PullSrc:prepare down index file  with error: ") + errstr, ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		return false;
	}

	char urlPort[65] = "";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", _transferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;

	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
	transferUrl = buf;

	maxLen  = -1;
	if(locateResponse.OpenForWrite)
	{
		int64 nBeginPos = 0;
		int64 nEndPos = 0;

		if(_pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
			maxLen = nEndPos;
		else
			maxLen = 65536;
	}

	if( _pHttpDownloader->downloadFile(transferUrl, _strTempIndexFile, 0, maxLen, false)< 0)
	{
		_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);

		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s][%s][%s]failed to download index file with error: %s"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str(), errstr.c_str());
		SetLastError(std::string("C2PullSrc:down index file  with error: ") + errstr, ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		return false;
	}

	_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);

	MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] index file downloaded to local %s took %d"), _strLogHint.c_str(), 
		 _strTempIndexFile.c_str(), (int)(ZQ::common::now() - lStart));
	return true;
}
bool C2PullSource::downloadMainFile()
{
	bool result = true;

	int64 lStart = ZQ::common::now();

	if(_mainFileset.filename == "")
		_mainFileset.filename ="forward/1";

	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	
	int64 maxLen = -1;
	int64 startOffset = 0;

//	url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
	url = _srcURL;

	locateRequest.pid  = _strProvideId;
	locateRequest.paid = _strProvideAssetId;
	locateRequest.beginPos = _mainFileset.firstOffset;
	locateRequest.endPos  = -1;
	locateRequest.transferDelay = _transferDelay;
	locateRequest.subFile = _mainFileset.filename;

    if(_nBandwidthBps <= 0)
		locateRequest.bitRate = _info.bitrate * _nspeed;
	else
		locateRequest.bitRate = _nBandwidthBps * _nspeed;

	_pHttpDownloader->setSubfile(_mainFileset.filename);
	if(!_pHttpDownloader->prepareLocateFile(url, locateRequest, locateResponse))
	{
		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s]: %s"), _strLogHint.c_str(), errstr.c_str());
		return false;
	}

	char urlPort[65] = "";
	snprintf(urlPort, sizeof(urlPort) -1, "%u", _transferServerPort);

	if (locateResponse.transferportnum.length() <= 0)
		locateResponse.transferportnum = urlPort;

	char buf[256] = "";
	snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());
    
	transferUrl = buf;

	int64 filesize = _pHttpDownloader->downloadPartialFile(&C2PullSource::processData,(void *)this,  transferUrl, _mainFileset.firstOffset, -1);
	if(filesize < 0)
	{
//		url = std::string("http://") + _strServer + ":" + _strPort; 
		_pHttpDownloader->deleteTransferId(url, locateResponse.transferId, locateResponse.transferHost);

		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s]: %s"), _strLogHint.c_str(), errstr.c_str());
		SetLastError(errstr, ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		return false;
	}

//	url = std::string("http://") + _strServer + ":" + _strPort; 
	_pHttpDownloader->deleteTransferId(url, locateResponse.transferId, locateResponse.transferHost);

	if(_bProcessDataError)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] failed to allocate media sample"), _strLogHint.c_str());
		result = false;
	}
	else if((_mainFileset.finalOffset - _mainFileset.firstOffset +1) > filesize)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(C2PullSrc, "[%s] download file size smaller than original file size"), _strLogHint.c_str());
		std::string errstr = "download file size smaller than original file size";
		SetLastError(errstr, ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		result = false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(C2PullSrc, "[%s] file[PID: %s][PAID: %s] copied took %d ms"), 
			_strLogHint.c_str(),  _strProvideId.c_str(), _strProvideAssetId.c_str(), 
			(int)(ZQ::common::now() - lStart));
	}
	return result;
}
bool C2PullSource::processData(void* pCtx, char*pBuf, int64 nBufLen)
{
//	static unsigned int processdata = 0;
	C2PullSource* pC2PullSource = (C2PullSource*) pCtx;
	//
	// check after receive this buffer, whether exceed the buffer size
	//
	if(!pC2PullSource->_pMediaSample || pC2PullSource->_bStop)
	{
		pC2PullSource->_bProcessDataError = true;
		pC2PullSource->_pHttpDownloader->setError(true, std::string("Failed to allocate media sample or C2PullSource closed"));
		pC2PullSource->SetLastError(std::string("Failed to allocate media sample  or C2PullSource closed") , ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
		return false;
	}

//	unsigned int freesize = pC2PullSource->_pMediaSample->getFreeSize();
//	unsigned int bufsize = pC2PullSource->_pMediaSample->getBufSize();
//	unsigned int datalength = pC2PullSource->_pMediaSample->getDataLength();

//	glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%s]BufLen[%lld] MediaSample[%u], RemainderData[%u], MediaSampleBufsize[%u], bufFreeSize[%u], datalen(offsetSet)[%u], [%u]"), 
//		pC2PullSource->_strLogHint.c_str(), nBufLen, (unsigned int)(nBufLen/ bufsize), (unsigned int)(nBufLen % bufsize), bufsize, freesize,datalength, freesize + datalength); 

	if( pC2PullSource->_pMediaSample->getFreeSize() > static_cast<size_t>(nBufLen))
	{
		glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "Fill data to free buf, BufLen[%d]"),nBufLen);
		//not full 
		memcpy(pC2PullSource->_pMediaSample->getFreeBufferPointer(), pBuf, nBufLen);
		pC2PullSource->_pMediaSample->increaseDataLength(nBufLen);

//		processdata += nBufLen;
	}
	else 
	{	
		int cpySize = pC2PullSource->_pMediaSample->getFreeSize();
		int leftSize = nBufLen - cpySize;
        int offset = 0;

//		glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "Fill data to remainder free buf from bufOffset[%d], copyBuf[%d] leftsize[%d] BufLen[%d][%p    ----   %p]"), offset, cpySize, leftSize, leftSize + offset, pC2PullSource->_pMediaSample->getPointer(),pC2PullSource->_pMediaSample->getFreeBufferPointer());
		// make the buffer full
		memcpy(pC2PullSource->_pMediaSample->getFreeBufferPointer(), pBuf, cpySize);
		pC2PullSource->_pMediaSample->increaseDataLength(cpySize);
		// release the full buffer
		pC2PullSource->releaseOutputBuffer(pC2PullSource->_pMediaSample);

//		processdata += cpySize;
 //       int i  = 1;

		offset = cpySize;
		while (leftSize >= 0)
		{
			// acquire new buffer
			if (!(pC2PullSource->_pMediaSample = pC2PullSource->acquireOutputBuffer()))
			{
				pC2PullSource->_bProcessDataError = true;
				pC2PullSource->_pHttpDownloader->setError(true, std::string("Failed to allocate media sample"));
				pC2PullSource->SetLastError(std::string("Failed to allocate media sample") , ERRCODE_C2PULL_DOWNLOADFILE_FAIL);
				return false;
			}

			pC2PullSource->_pMediaSample->setDataLength(0);

			if(leftSize >= pC2PullSource->_pMediaSample->getBufSize())
			{
//				glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d]alloc new buffer to fill data from bufOffset[%d] copyBuf[%d] BufLen[%d]"),i, offset, pC2PullSource->_pMediaSample->getBufSize(), leftSize + offset);

				memcpy(pC2PullSource->_pMediaSample->getPointer(), pBuf + offset , pC2PullSource->_pMediaSample->getBufSize());
				pC2PullSource->_pMediaSample->increaseDataLength(pC2PullSource->_pMediaSample->getBufSize());
				offset += pC2PullSource->_pMediaSample->getBufSize();

// 				glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d]full buffer, freesize[%u],datalength[%u],bufsize[%u] BufLen[%d] "),
//					i, pC2PullSource->_pMediaSample->getFreeSize(),pC2PullSource->_pMediaSample->getDataLength(), pC2PullSource->_pMediaSample->getBufSize(), leftSize + offset);
				// release the full buffer
				pC2PullSource->releaseOutputBuffer(pC2PullSource->_pMediaSample);

			    leftSize = leftSize - pC2PullSource->_pMediaSample->getBufSize();

//				glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d] leftsize[%d] offset[%d] BufLen[%d]"),i, leftSize, offset, leftSize + offset);
//				i++;
//				processdata += pC2PullSource->_pMediaSample->getBufSize();
			}
			else
			{
				// copy the left buffer
				if(leftSize > 0) 
				{
//					glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d]*alloc new buffer to fill data, from bufOffset[%d],  copyBuf[%d] BufLen[%d]"),i, offset, leftSize, leftSize + offset);
					memcpy(pC2PullSource->_pMediaSample->getPointer(), pBuf + offset, leftSize);
					pC2PullSource->_pMediaSample->setDataLength(leftSize);
					offset+= leftSize;

//					glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d]full buffer, freesize[%u],datalength[%u],bufsize[%u] BufLen[%d] "),
//						i, pC2PullSource->_pMediaSample->getFreeSize(),pC2PullSource->_pMediaSample->getDataLength(), pC2PullSource->_pMediaSample->getBufSize(), leftSize + offset);

//					processdata += leftSize;	
// 					glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "[%d]*leftsize[%d] offset[%d] BufLen[%d], bufferFreeSize[%d]"),i, 0, offset, 0 + offset, pC2PullSource->_pMediaSample->getFreeSize());
					leftSize = -1;	
				}
				else
					leftSize = -1;
			}
		}
	}
	return true;
//	glog(Log::L_DEBUG, CLOGFMT(C2PullSrc, "processdata [%u]]"), processdata);
}
bool C2PullSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool C2PullSource::seek(int64 offset, int pos)
{
	return false;
}
}}
