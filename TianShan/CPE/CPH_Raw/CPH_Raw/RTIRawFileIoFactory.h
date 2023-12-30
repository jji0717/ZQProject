#ifndef _RTIRAW_FILEIO_FACTORY_H
#define _RTIRAW_FILEIO_FACTORY_H

#include "FileIo.h"
#include "Log.h"
namespace ZQTianShan 
{
	namespace ContentProvision
	{
#pragma once

		class RTIRawFileIoFactory : public FileIoFactory
		{
		public:
			RTIRawFileIoFactory(void);
			virtual ~RTIRawFileIoFactory(void);
			virtual ZQ::common::Log* getLog();
			virtual void setLog(ZQ::common::Log* pLog);
			virtual FileIo*	create();
			virtual bool deleteFile(const char* szFileName);
			virtual bool moveFile(const char* szFileName, const char* szNewFileName);
			virtual int64 getFileSize(const char* szFileName);

			virtual int getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr);
			virtual bool initialize();
			virtual void uninitialize();
			virtual void getLastError(std::string& strErr, int& errCode);
		public:
			void setRootDir(const char* szDir);
			std::string getRootDir();
			static bool createDirectoryForFile(const std::string& strFilename);
			static bool createDirectory(const std::string& strDirectory);
		public:
#ifdef ZQ_OS_MSWIN
			virtual void findClose(HANDLE hfile);
			virtual bool findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w);
			virtual HANDLE findFirstFile(char* name, WIN32_FIND_DATAA& w);
#endif
		protected:
			void setLastError(const std::string& strErr, int errCode);
			static std::string getPathOfFile(const std::string& strFilename);

		protected:
			ZQ::common::Log*						_log;
			std::string								_strErr;
			int										_errCode;
			std::string								_strRootPath;
		};
	}}//namespace
#endif
