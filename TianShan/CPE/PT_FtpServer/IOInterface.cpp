
#include "CPECfg.h"
#include "IOInterface.h"
#include "FileIo.h"

#define IoInterface			"IoInterface"
#define MOLOG glog

using namespace ZQ::common;
using namespace ZQTianShan::ContentProvision;


namespace ZQTianShan 
{
	namespace ContentProvision
	{

#ifdef ZQ_OS_MSWIN
		time_t FiletimeToTimet(FILETIME ft)
		{
			LONGLONG ll = *((LONGLONG*)&ft);
			ll -= 116444736000000000;
			if (ll<=0)
				ll = 0;

			return (time_t)(ll/10000000);
		}
#endif

IOInterface::IOInterface(FileIoFactory* pfilefac)
{
	_pFileIoFactory = pfilefac;
	_bwTicket = 0;
	_pFileIo = NULL;
	_bOpened = false;
	_dwBandwidth = 3750000;
	_pFileIo = _pFileIoFactory->create();
	_pFileIo->setLog(&glog);
}

IOInterface::~IOInterface()
{
	if (_pFileIo)
	{
		delete _pFileIo;
		_pFileIo = NULL;
	}
}

bool IOInterface::Open( const char* szFile, int nOpenFlag )
{
	std::string strFile;
	if (szFile[0] == '/' || szFile[0] == '\\')
		strFile = szFile + 1;
	else
		strFile = szFile;

	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),strFile.c_str());
		return false;
	}
	if (!_pFileIo->openfile(strFile.c_str(), 
		FileIo::ACCESS_READ,
		(FileIo::ShareMode)(FileIo::SHARE_READ| FileIo::SHARE_WRITE),
		FileIo::WAY_OPEN_EXISTING,
		FileIo::ATTRIB_INDEXFILE))
	{
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);
		
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "Failed to create output file: %s with error: %s"),
		 strFile.c_str(), strErr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(IoInterface, "open file [%s] successful"),
		strFile.c_str());

	_strFile = szFile;
	_bOpened = true;
	return true;
}

int64 IOInterface::GetFileSize( const char* szFile )
{
	std::string strFile;
	if (szFile[0] == '/' || szFile[0] == '\\')
		strFile = szFile + 1;
	else
		strFile = szFile;

	return _pFileIoFactory->getFileSize(strFile.c_str());
}

int IOInterface::Read( char* pPtr, int nReadLen )
{
	unsigned int dwRet = 0;
	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),_strFile.c_str());
		return false;
	}
	if (!_pFileIo->readfile(pPtr,nReadLen,dwRet))
	{		
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);
		if (nErrorCode == 0x00000026)
		{
			//ERROR_HANDLE_EOF			
			return dwRet;
		}
		
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "Failed to read file: %s with error: %s"),
			 _pFileIo->getFileName().c_str(), strErr.c_str());

		return false;
	}

	return dwRet;
}

bool IOInterface::Write( char* pPtr, int nWriteLen )
{
	if (!_bOpened)
		return false;

	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),_strFile.c_str());
		return false;
	}

	unsigned int  dwWritten = 0;
	if (!_pFileIo->writefile(pPtr,nWriteLen,dwWritten))
	{
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);
				
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "Failed to write file: %s with error: %s"),
			 _pFileIo->getFileName().c_str(), strErr.c_str());

		return false;
	}

	return true;
}

bool IOInterface::Seek(int64 lOffset, int nPosFlag)
{
	if (!_bOpened)
		return false;

	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),_strFile.c_str());
		return false;
	}

	if (!_pFileIo->seekfile(lOffset,(FileIo::Position)nPosFlag))
	{
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);
			
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "Failed to seek file: %s with error: %s"),
			_pFileIo->getFileName().c_str(), strErr.c_str());

		return false;
	}

	return true;
}


void IOInterface::Close()
{
	if (!_bOpened)
		return;

	_pFileIo->closefile();
	_bOpened = false;
}

int IOInterface::GetRecommendedIOSize()
{
	return 64*1024;
}


bool IOInterface::ReserveBandwidth( int nbps )
{
	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),_strFile.c_str());
		return false;
	}
	return _pFileIo->reserveBandwidth(nbps);
}

void IOInterface::ReleaseBandwidth()
{
	if (!_pFileIo)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(IoInterface, "File %s handle is invalid."),_strFile.c_str());
		return;
	}
	
	_pFileIo->releaseBandwidth();
}


}
}
