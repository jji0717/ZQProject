#include "Log.h"
#include "ErrorCode.h"
#include "BaseClass.h"

#include "RTIRawFileIoFactory.h"

#include "RTIRawFileIo.h"

#define MOLOG			(*_log)
using namespace ZQ::common;
namespace ZQTianShan 
{
	namespace ContentProvision
	{
		RTIRawFileIo::RTIRawFileIo(RTIRawFileIoFactory* pFileIoFactory) 
			: FileIo(pFileIoFactory)
		{
			_strPath = pFileIoFactory->getRootDir();
			_log = pFileIoFactory->getLog();
			_hOutputFile = NULL;
		}

		RTIRawFileIo::~RTIRawFileIo(void)
		{
			closefile();
		}
		ZQ::common::Log* RTIRawFileIo::getLog()
		{
			return _log;
		}

		void RTIRawFileIo::setLog(ZQ::common::Log* pLog)
		{
			_log = pLog;
		}
		void RTIRawFileIo::setLastError(const std::string& strErr, int errCode)
		{
			_strErr = strErr;
			_errCode = errCode;
		}
		void RTIRawFileIo::getLastError(std::string& strErr, int& errCode)
		{
			strErr = _strErr;
			errCode = _errCode;
		}

		bool RTIRawFileIo::openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib)
		{	
			_strLogHint = szFileName;
			_strFileName = _strPath + szFileName;
			_hOutputFile = fopen(_strFileName.c_str(),"w");
			if (_hOutputFile == 0)
			{
				setLastError(std::string("CreateFile() failed with error: ") , ERRCODE_NTFS_CREATEFILE);
				MOLOG(Log::L_ERROR, CLOGFMT(RTIRawFileIo, "[%s] Failed to create output file: %s"),
					_strLogHint.c_str(), _strFileName.c_str());
				return false;
			}
			MOLOG(Log::L_INFO, CLOGFMT(RTIRawFileIo, "[%s] create output file [%s] success"),_strLogHint.c_str(), _strFileName.c_str());
			return true;
		}
		bool RTIRawFileIo::writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen)
		{
			size_t actualWriteLen;
			actualWriteLen = fwrite(pBuf,1,bufLen,_hOutputFile);
			if (actualWriteLen == 0)
			{
				setLastError(std::string("WriteFile() failed with error: ") , ERRCODE_NTFS_WRITEFILE);
				MOLOG(Log::L_ERROR, CLOGFMT(RTIRawFileIo, "[%s] Failed to WriteFile() with error: %s"),
					_strLogHint.c_str());
				return false;
			}
			writeLen = actualWriteLen;
			return true;
		}
		bool RTIRawFileIo::seekfile(int64 offset, Position pos)
		{
			return !fseek(_hOutputFile,offset,SEEK_SET);
		}
		bool RTIRawFileIo::setEndOfFile(int64 offset)
		{
			return true;
		}

		bool RTIRawFileIo::closefile()
		{
			if (_hOutputFile != NULL)
			{
				fclose(_hOutputFile);
				_hOutputFile = NULL;
			}
			return true;
		}
		std::string RTIRawFileIo::getFileName()
		{
			return _strFileName;
		}
		std::string RTIRawFileIo::getFileExtension()
		{
			return _strFileExtension;
		}

		void RTIRawFileIo::setFileExtension(const char* szFileExt)
		{
			_strFileExtension = szFileExt;
		}
	}}//namespace



