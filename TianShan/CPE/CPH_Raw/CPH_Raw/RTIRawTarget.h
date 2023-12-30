#ifndef _RTIRAW_TARGET_H
#define _RTIRAW_TARGET_H

#include "BaseClass.h"
#include "FileIo.h"

#define	TARGET_TYPE_RTIRAW	"RTIRawTarget"
#define	TARGET_TYPE_FILESET	"FilesetIO"
#define MAX_QUEUED_SAMPLES	200

#pragma once
namespace ZQTianShan 
{
	namespace ContentProvision
	{
		class RTIRawTarget : public BaseTarget
		{
		public:
			RTIRawTarget(FileIoFactory* pFileIoFac);
			virtual ~RTIRawTarget(void);

			virtual bool Init();
			virtual void Close(){closeSubFiles();}
			virtual void endOfStream(){}
			virtual bool Start(){return true;}
			
			virtual const char* GetName();
			virtual void setFilename(const char* szFile);
			/*void setBandwidth(unsigned int uBandwidthBps);*/

			virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

			bool Receive(MediaSample* pSample, int nInputIndex=0);

			/// set cache path
			void setCacheDirectory(const std::string& path)
			{
				_cachePath = path;
			}
			// get the cache path
			std::string getCacheDirectory() { return _cachePath; };
		protected:
			void fileIoFailure(std::auto_ptr<FileIo>& pFileIo, const char* szLogHeader, bool bSetLastError = true);

			struct SubFile
			{
				std::string		strFilename;
				std::string		strFileExt;
				std::auto_ptr<FileIo>	pFileIo;
				uint32          reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)
				const void*		pacingIndexCtx;	//context of pacing
				RTIRawTarget*	pThis;			//point to RTIRawTarget object
				int64		llProcOffset;
				std::vector<MediaSample*>	samples;
			};		
			virtual bool writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen);

			bool writeData(SubFile& subFile, char* pBuf, int dwBufLen);

			bool writeSubFileNoIndex(SubFile& subFile, MediaSample* pSample);
			
			void initSubFiles();
			bool openSubFiles();
			void closeSubFiles();

		public:
			SubFile			_subFile;

			std::string		_strFilename;
			uint32			_dwBandwidth;
			FileIoFactory*		_pFileIoFac;		//file io factory
			std::string	        _cachePath;
			int				_nWriteLatencyWarningMs;
		};

	}}//namespace

#endif