#include "RTIRawFileIoFactory.h"
#include "RTIRawFileIo.h"
#include "strHelper.h"
#include <vector>
namespace ZQTianShan 
{
	namespace ContentProvision
	{
		RTIRawFileIoFactory::RTIRawFileIoFactory(void)
		{
		}
		RTIRawFileIoFactory::~RTIRawFileIoFactory(void)
		{
			uninitialize();
		}
		ZQ::common::Log* RTIRawFileIoFactory::getLog()
		{
			return _log;
		}

		void RTIRawFileIoFactory::setLog(ZQ::common::Log* pLog)
		{
			_log = pLog;
		}
		FileIo* RTIRawFileIoFactory::create()
		{
			return new RTIRawFileIo(this);
		}
		bool RTIRawFileIoFactory::deleteFile(const char* szFileName)
		{
			return true;
		}
		bool RTIRawFileIoFactory::moveFile(const char* szFileName, const char* szNewFileName)
		{
			return true;
		}
		int64 RTIRawFileIoFactory::getFileSize(const char* szFileName)
		{
			return 0;
		}
		bool RTIRawFileIoFactory::initialize()
		{
			return true;
		}

		void RTIRawFileIoFactory::uninitialize()
		{
		}
		void RTIRawFileIoFactory::setRootDir(const char* szDir)
		{
			_strRootPath = szDir;
			if (FNSEPC != _strRootPath[_strRootPath.length()-1])
				_strRootPath += FNSEPS;
		}
		std::string RTIRawFileIoFactory::getRootDir()
		{
			return _strRootPath;
		}
		void RTIRawFileIoFactory::getLastError(std::string& strErr, int& errCode)
		{
			strErr = _strErr;
			errCode = _errCode;
		}

		void RTIRawFileIoFactory::setLastError(const std::string& strErr, int errCode)
		{
			_strErr = strErr;
			_errCode = errCode;
		}

		bool RTIRawFileIoFactory::createDirectoryForFile(const std::string& strFilename)
		{
			//get path of file
			std::string strDirectory = getPathOfFile(strFilename);	
			return createDirectory(strDirectory.c_str());
		}

		bool RTIRawFileIoFactory::createDirectory(const std::string& strDirectory)
		{
			return true;
		}

		std::string RTIRawFileIoFactory::getPathOfFile(const std::string& strFilename)
		{
			std::string strPath;
			std::string::size_type pos = strFilename.find_last_of(FNSEPS);
			if (pos != std::string::npos)
				strPath.assign(strFilename, 0, pos);
			return strPath;
		}
#ifdef ZQ_OS_MSWIN
		void RTIRawFileIoFactory::findClose(HANDLE hfile)
		{
			::FindClose(hfile);
		}

		bool RTIRawFileIoFactory::findNextFile( HANDLE hfile,WIN32_FIND_DATAA& w )
		{
			return ::FindNextFile(hfile, &w);
		}

		HANDLE RTIRawFileIoFactory::findFirstFile( char* name, WIN32_FIND_DATAA& w )
		{
			HANDLE fileHandle = ::FindFirstFileA(name, &w);
			if(fileHandle == INVALID_HANDLE_VALUE) 
			{
				return INVALID_HANDLE_VALUE;
			}
			return fileHandle;
		}
#endif
		int RTIRawFileIoFactory::getFileStats( char *filepath, FSUtils::fileInfo_t *infoptr )
		{
			return 0;
		}
	}}//namespace





