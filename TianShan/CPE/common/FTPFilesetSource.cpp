
#include "FTPFilesetSource.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"

/*#include "vvx.h"*/
#include "VvxParser.h"
#include "VV2Parser.h"

#include "vstrmuser.h"
#include "vsiolib.h"
#include "VstrmBase.h"

#include "FtpDownload.h"
#include "FTPMSClient.h"


#define FTPFilesetSrc			"FTPFilesetSrc"
const std::string SRCFILTER_PROTO_FTP   = "ftp";



#pragma comment(lib, "Wininet.lib")

extern bool validatePath( const char *     szPath );
using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {
bool FTPFilesetSource::getIdxSubFileInfo(const char* szIdxFile, bool bVvx, std::vector<SubFileInfo>& subFiles, MediaInfo& info)
{
	if (bVvx)
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

		subFiles.resize(vvx.getSubFileCount());
		for(int i=0;i<vvx.getSubFileCount();i++)
		{

			vvx.getSubFileExtension(i,subFiles[i].ext, sizeof(subFiles[i].ext));
			vvx.getSubFileSpeed(i, subFiles[i].numerator, *((uint32*)&subFiles[i].denominator));
			if (subFiles[i].numerator<0)
			{
				subFiles[i].numerator = 0 - subFiles[i].numerator;
				subFiles[i].direction = -1;
			}
			else
			{
				subFiles[i].direction = 1;
			}
		}		
	}
	else
	{
		VV2Parser vv2;
		if(!vv2.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.bitrate = vv2.getBitrate();
		info.playTime = vv2.getPlayTime();

		subFiles.resize(vv2.getSubFileCount());
		for(unsigned int i=0;i<vv2.getSubFileCount();i++)
		{
			vv2.getSubFileExtension(i,subFiles[i].ext, sizeof(subFiles[i].ext));
			vv2.getSubFileSpeed(i, subFiles[i].numerator, subFiles[i].denominator, subFiles[i].direction);			
		}		
	}

	return true;
}

FTPFilesetSource::FTPFilesetSource()
	:_pFtpDownloader(new FTPMSClient())
{
	_bDriverModule = true;

	_llProcBytes = 0;
	_nBandwidthBps = 0;
	
	_bStop = false;

	_tempDir = "C:\\"; //temp use;
	_totalBytes = 0;
//	_objectId = 0;

	_bEnableMD5 = false;
	_bwTicket = 0;
	_supportFilesize = 0;
	_OrigFileSize = 0;
}

FTPFilesetSource::~FTPFilesetSource()
{
	if (_pFtpDownloader.get())
	{
		_pFtpDownloader->closeFile();
		_pFtpDownloader->close();
		_pFtpDownloader.reset(0);
	}
}

void FTPFilesetSource::setFilename(const char* szFilename)
{
	//set the target file name
	_strFilename = szFilename;
}

bool FTPFilesetSource::Init()
{
	//char ftpcmd[MAX_PATH];
	DWORD dwErr = 0;
	DWORD buflen = MAX_PATH;
	SubFileInfo subf;
	std::vector<SubFileInfo> subFiles;
	std::vector<SubFileInfo>::iterator iter;
	char szBuf[512];
	std::string errmsg;
	//std::string strSourcePath ;

	MOLOG(Log::L_DEBUG, CLOGFMT(FTPFilesetSrc, "[%s] Enter Init()"), _strLogHint.c_str());
	
	ZQ::common::URLStr srcUrl(_srcURL.c_str());
	std::string strServer = srcUrl.getHost();
	std::string strOrgFile = srcUrl.getPath();
	std::string strUserName = srcUrl.getUserName();
	std::string strPassword = srcUrl.getPwd();
	int nPort = srcUrl.getPort();	

	if (_bDecodeSourceURL)
	{
		char strTemp[512] = "";
		memset(strTemp, 0, 512);

		URLStr::decode(strServer.c_str(), strTemp, 512);
		strServer = strTemp;

		memset(strTemp, 0, 512);
		URLStr::decode(strOrgFile.c_str(), strTemp, 512);
		strOrgFile = strTemp;

		memset(strTemp, 0, 512);
		URLStr::decode(strUserName.c_str(), strTemp, 512);
		strUserName = strTemp;

		memset(strTemp, 0, 512);
		URLStr::decode(strPassword.c_str(), strTemp, 512);
		strPassword = strTemp;
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(FTPFilesetSrc, "[%s] SourceUrl=%s, Host=%s, Port=%d, User=%s, Password=%s, File=%s"),
		_strLogHint.c_str(), _srcURL.c_str(), strServer.c_str(),nPort, strUserName.c_str(), strPassword.c_str(), strOrgFile.c_str());

	_pFtpDownloader->setLog(_pLog);
	_pFtpDownloader->setTransmitMode(FTPDownload::binary);
	_pFtpDownloader->setFTPMode(FTPDownload::active);
	_pFtpDownloader->setIoTimeout(30000);
	if (!_pFtpDownloader->open(strServer, nPort, strUserName, strPassword, _strLocalNetIf))
	{
		std::string strErr = "failed to connect to url " + _srcURL + " with error: " + _pFtpDownloader->getLastError();

		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "[%s] %s"), _strLogHint.c_str(), strErr.c_str());
		SetLastError(strErr, ERRCODE_INVALID_SRCURL);

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] ftp server connected"), _strLogHint.c_str());

	bool  bIndexVvx = isRemoteVvxIndex(strOrgFile);

	std::string strRemoteIndexFile;
	std::string strTempIndexFile;
	if (bIndexVvx)
	{
		strRemoteIndexFile = getVvxIndexFileName(strOrgFile);
		strTempIndexFile = getVvxIndexFileName(_tempDir + _strFilename);
	}
	else
	{
		strRemoteIndexFile = getVv2IndexFileName(strOrgFile);
		strTempIndexFile = getVv2IndexFileName(_tempDir + _strFilename);
	}

	if (!validatePath(_tempDir.c_str()))
		return false;
	//cache index file in a temp directory
	if (!downloadIndexFile(strRemoteIndexFile, strTempIndexFile))
		return false;

	if(!getIdxSubFileInfo(strTempIndexFile.c_str(), bIndexVvx, subFiles, _info))
	{
		//failed to parse the index file
		snprintf(szBuf, sizeof(szBuf)-1, "Failed to parse VVX/VV2 for (%s)", strOrgFile.c_str());
		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "[%s] %s"), _strLogHint.c_str(), szBuf);
		SetLastError(szBuf, ERRCODE_NTFS_READFILE);	
		return false;
	}

	if (bIndexVvx)
		strcpy(subf.ext, ".vvx");
	else
		strcpy(subf.ext, ".vv2");

	subFiles.push_back(subf);
	for (iter = subFiles.begin();iter != subFiles.end();iter ++)
	{
		_srcFileset.push_back(strOrgFile + iter->ext);
		_targetFileset.push_back(_strFilename + iter->ext);
	}

	std::vector<std::string>::iterator fsIter;
	for (fsIter = _srcFileset.begin(); fsIter != _srcFileset.end(); fsIter++)
	{		
		__int64 tempSize = _pFtpDownloader->getFileSize(*fsIter);
		_totalBytes += tempSize;
		MOLOG(Log::L_DEBUG, CLOGFMT(FTPFilesetSrc, "[%s] file size of [%s] is %lld bytes"), 
			_strLogHint.c_str(),(*fsIter).c_str(), tempSize);

		if (fsIter==_srcFileset.begin())
			_OrigFileSize = tempSize;

		if (fsIter!=_srcFileset.begin())
		{
			_supportFilesize += tempSize;
		}
	}
	
	GetGraph()->setTotalBytes(_totalBytes);
	_bitrateCtrl.setBitrate(_nBandwidthBps);
	return true;
}

void FTPFilesetSource::Stop()
{
	_bStop = true;
}

void FTPFilesetSource::Close()
{
	_bStop = true;
	if (_pFtpDownloader.get())
	{
		_pFtpDownloader->closeFile();
		_pFtpDownloader->close();
		_pFtpDownloader.reset(0);
	}
}

void FTPFilesetSource::endOfStream()
{
	
}

const char* FTPFilesetSource::GetName()
{
	return SOURCE_TYPE_FTPFileset;
}

LONGLONG FTPFilesetSource::getProcessBytes()
{
	return _llProcBytes;
}

void FTPFilesetSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPFilesetSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void FTPFilesetSource::setTargetDir(const char* szDir)
{
	if (szDir[strlen(szDir)-1] != '\\')
		_outputDir = std::string(szDir) + std::string("\\");
	else
		_outputDir = szDir;
}

void FTPFilesetSource::setCacheDir(const char* szCachDir)
{
	if (szCachDir[strlen(szCachDir)-1] != '\\')
		_tempDir = szCachDir + std::string("\\");
	else
		_tempDir = szCachDir;
}

bool FTPFilesetSource::Run()
{
	//parsed the media info getting from index file
	GetGraph()->OnMediaInfoParsed(_info);

	_bitrateCtrl.start();

	DWORD dwStart = GetTickCount();
	std::vector<std::string>::iterator fsIter;
	unsigned int nFileIndex;
	for (nFileIndex = 0; nFileIndex < _srcFileset.size(); nFileIndex++)
	{
		std::string inputFileName = _srcFileset[nFileIndex];
		std::string outputFileName;
		if (_bNtfsTest)
			outputFileName = _outputDir + _targetFileset[nFileIndex];
		else
			outputFileName = _targetFileset[nFileIndex];

		bool bopentargetSuc;
	//	HANDLE hFileTarget;
		if (!openRemoteFile(inputFileName))
		{
			break;
		}
			
		bopentargetSuc = openTargetFile(outputFileName);
		if (!bopentargetSuc)
		{
			_pFtpDownloader->closeFile();
			break;
		}
		//reserve bandwidth
		if (!_bNtfsTest && !reserveVBw())
			return false;
		
		while(!_bStop)
		{
			char buf[64*1024];	
			int nReadData = 0;
			bool bRead = _pFtpDownloader->readFile(buf, sizeof(buf), nReadData);
			if(!bRead)
			{
				std::string errmsg = _pFtpDownloader->getLastError();
				SetLastError(std::string("Read file failed with error: ") + errmsg, ERRCODE_NTFS_READFILE);
				MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "[%s] failed to copy file [%s] with error: %s"), _strLogHint.c_str(), outputFileName.c_str(), errmsg);
				break;
			}

			if(nReadData>0)
			{
				if (!writeTargetFile(buf, nReadData))
				{
					break;
				}

				if (_bEnableMD5 && !nFileIndex)
					_md5ChecksumUtil.checksum(buf, nReadData);
			}
			else
			{
				MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] file [%s] copied"), _strLogHint.c_str(), outputFileName.c_str());
				break;
			}

			_llProcBytes += nReadData;
			_bitrateCtrl.control(_llProcBytes);

			GetGraph()->OnProgress(_llProcBytes);
		}

		if (!_bNtfsTest)
			releaseVBW();
		closeTargetFile();		
		_pFtpDownloader->closeFile();
	}


	_pFtpDownloader->close();

	if (GetGraph()->IsErrorOccurred())
	{
		return false;
	}

	// caculate the bitrate
	{
		DWORD dwDuration = GetTickCount() - dwStart;	//here maybe wrong, but does not matter, just a log
		DWORD dwTranferRate;
		if (dwDuration)
		{
			dwTranferRate = (DWORD)(_totalBytes*8000/dwDuration);
		}
		else
		{
			dwTranferRate = 0;
		}
		MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] Actual copy rate is [%d]bps"), _strLogHint.c_str(), dwTranferRate);
	}
		
	return true;
}

bool FTPFilesetSource::releaseVBW()
{	
	if(0 == _nBandwidthBps)
		return true;

	if (!_pFileIo.get())
		return false;

	_pFileIo->releaseBandwidth();
	_bwTicket = 0;
	MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] ReleaseBandwidth(BW - %dbps) succeed."),  _strLogHint.c_str(), _nBandwidthBps);

	return true;
}

bool FTPFilesetSource::openTargetFile(const std::string& targetName)
{
	if (!_pFileIoFac)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "File IO factory is NULL"));
		return false;
	}
	_pFileIo.reset(_pFileIoFac->create());

	if (!_pFileIo->openfile(targetName.c_str(), 
		FileIo::ACCESS_WRITE,
		(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
		FileIo::WAY_CREATE_ALWAYS,
		(FileIo::FileAttrib)(FileIo::ATTRIB_NONE)))
	{
		std::string errStr;
		int errCode;
		_pFileIo->getLastError(errStr,errCode);

		char tmp[256];
		sprintf(tmp, "Failed to open file %s for write err[%s]", targetName.c_str(),errStr.c_str());

		SetLastError(tmp);
		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "Failed to open file %s for write"), targetName.c_str(),errStr.c_str());
		return false;
	}

	_pFileIo->setOption();
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPFilesetSrc, "[%s] Open file %s for write successful"), _strLogHint.c_str(), targetName.c_str());
	return true;
}

bool FTPFilesetSource::reserveVBw()
{
	if (!_pFileIo.get())
		return false;
	_pFileIo->reserveBandwidth(_nBandwidthBps);
	return true;
}

void FTPFilesetSource::closeTargetFile()
{
   if (_pFileIo.get())
   {
	   _pFileIo->closefile();
   }
}

bool FTPFilesetSource::writeTargetFile(char* pBuf, int nLen)
{
	unsigned int writeLen;
	if (_pFileIo->writefile(pBuf,nLen,writeLen))
		return true;
	else
		return false;
}

void FTPFilesetSource::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

void FTPFilesetSource::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

std::string FTPFilesetSource::getVvxIndexFileName( const std::string& strMainFile )
{
	return strMainFile + ".vvx";
}

std::string FTPFilesetSource::getVv2IndexFileName( const std::string& strMainFile )
{
	return strMainFile + ".vv2";
}

bool FTPFilesetSource::isRemoteVvxIndex(const std::string& strRemoteMainFile)
{
	if (_pFtpDownloader->getFileSize(getVvxIndexFileName(strRemoteMainFile))>0)
		return true;

	return false;
}

bool FTPFilesetSource::downloadIndexFile( const std::string& strIndexFile, const std::string& strLocalFile)
{
	if (!_pFtpDownloader->downloadFile(strIndexFile, strLocalFile))
	{
		std::string errstr = _pFtpDownloader->getLastError();
		SetLastError(std::string("failed to download file ") + strIndexFile + " with error:" + errstr, ERRCODE_NTFS_READFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "[%s] failed to download file %s with error: %s"), _strLogHint.c_str(), 
			strIndexFile.c_str(), errstr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] index file %s downloaded to local %s"), _strLogHint.c_str(), 
		strIndexFile.c_str(), strLocalFile.c_str());

	return true;
}

bool FTPFilesetSource::openRemoteFile( const std::string& sourceName )
{
	if (!_pFtpDownloader->openFile(sourceName))
	{
		std::string errstr = _pFtpDownloader->getLastError();
		SetLastError(std::string("failed to open remote file ") + sourceName + " with error:" + errstr, ERRCODE_NTFS_READFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(FTPFilesetSrc, "[%s] failed to open remote file %s with error: %s"), _strLogHint.c_str(), 
			sourceName.c_str(), errstr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPFilesetSrc, "[%s] remote file %s opened"), _strLogHint.c_str(), sourceName.c_str());

	return true;
}
void FTPFilesetSource::setDecodeSourceURL(bool bDecodeSourceURL)
{
	_bDecodeSourceURL = bDecodeSourceURL;
}

bool FTPFilesetSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool FTPFilesetSource::seek(int64 offset, int pos)
{
	return false;
}


}}