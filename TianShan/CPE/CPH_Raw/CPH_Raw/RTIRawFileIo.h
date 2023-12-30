#ifndef _RTIRAQ_FILEIO_H
#define _RTIRAQ_FILEIO_H

#include "FileIo.h"

namespace ZQTianShan 
{
	namespace ContentProvision
	{
		class RTIRawFileIo : public FileIo
		{
			friend class RTIRawFileIoFactory;
		public:
			RTIRawFileIo(RTIRawFileIoFactory* pFileIoFactory);
			virtual ~RTIRawFileIo(void);
			
			virtual ZQ::common::Log* getLog();
			virtual void setLog(ZQ::common::Log* pLog);
			virtual bool openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib);
			
			virtual bool setOption() {return true;}
			virtual bool readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen){return true;}

			virtual bool writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen);
			virtual bool seekfile(int64 offset, Position pos);
			virtual bool setEndOfFile(int64 offset);
			virtual bool closefile();

			virtual bool enableSparse(){return false;};

			virtual std::string getFileName();

			virtual std::string getFileExtension();

			virtual void setFileExtension(const char* szFileExt);

			virtual void setMainFileFalg(){};
			//bandwidth management
			virtual bool reserveBandwidth(unsigned int dwBandwidth){return true;}
			virtual void releaseBandwidth(){}

			virtual void getLastError(std::string& strErr, int& errCode);
		protected:
			void setLastError(const std::string& strErr, int errCode);
		protected:

			//HANDLE							_hOutputFile;
			FILE*								_hOutputFile;
			ZQ::common::Log*					_log;
			std::string							_strFileName;
			std::string							_strFileExtension;

			std::string							_strErr;
			int									_errCode;

			std::string							_strLogHint;
			std::string							_strPath;

		};
	}}//namespace
#endif