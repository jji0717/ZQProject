// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE_CDNFILESETTARGET_H
#define ZQTS_CPE_CDNFILESETTARGET_H


#include "BaseClass.h"
#include "MD5CheckSumUtil.h"
#include "FileIo.h"
#include "PacingInterface.h"
#include <list>

#define	TARGET_TYPE_FILESET	"CDNFilesetIO"

#define MAX_QUEUED_SAMPLES	200


namespace ZQTianShan 
{
	namespace ContentProvision
	{

		class CDNFilesetTarget : public BaseTarget,public PacedIndexWrite
		{
		public:
			CDNFilesetTarget(FileIoFactory* pFileIoFac);

			virtual ~CDNFilesetTarget();

			virtual void InitPins();

			virtual bool Init();

			virtual bool Start();
			virtual void Stop();

			virtual void Close();

			virtual void endOfStream();

			virtual const char* GetName();

			virtual void setFilename(const char* szFile);

			void setBandwidth(unsigned int uBandwidthBps);

			void enablePacing(bool bEnable = true){_bPacing = bEnable;};

			void setTargetDeletion(int targetDelete = 1){_DeleteTargetOnFail = targetDelete;}

			void enablePacingTrace(bool bTrace = true){
				_bPacingTrace = bTrace;
			}

			void setWriteLatencyWarning(int nWarningMs);
			void setTypeH264(){_bTypeH264=true;}

			void enableMD5(bool bEnable = true);

			virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

			bool Receive(MediaSample* pSample, int nInputIndex=0);

			void setTrickFile(std::map<std::string, int>& temp){_SpeedAndFileExt = temp;}
			void setTrickSpeed(std::list<float>& trickspeed);
			void setPacingFactory(PacedIndexFactory* pFac){_pPacedIndexFac = pFac;}
            
			bool setPacedIndexInfo();

			void getMD5(std::string& strMD5);
			uint64 getSupportFileSize();
			void delOutput();

			void enableCacheForIndex(bool bEnable = false);

			/// set cache path
			void setCacheDirectory(const std::string& path);

			// get the cache path
			std::string getCacheDirectory() { return _cachePath; };

			void setIndexType(bool bIndexType = false){_bIndexVVC = bIndexType;}
			bool getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType);

			bool resetMainfile();
			void setCsicoMainFileExt(std::string& mainFileExt){_mainFileExt = mainFileExt;};
		protected:
			virtual void processOutPut();
			virtual uint64 getFileSize(const char* szFile);
			virtual bool deleteFile(const char* szFile);

			void fileIoFailure(std::auto_ptr<FileIo>& pFileIo, const char* szLogHeader, bool bSetLastError = true);

			struct SubFile
			{
				std::string		strFilename;
				std::string		strFileExt;
				std::string     strPacename;

				std::auto_ptr<FileIo>	pFileIo;

				uint32          bwPercentageN;   // numerator for calculating bandwidth
				uint32          bwPercentageD;   // denominator for calculating bandwidth
				uint32          reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)
				bool			bIndexFile;		//true is index file
				const void*		pacingIndexCtx;	//context of pacing
				CDNFilesetTarget*	pThis;			//point to FilesetTarget object

				int64		llProcOffset;
				std::vector<MediaSample*>	samples;
			};

			virtual bool write(const char* buf, int len);
			virtual bool seek(int offset);

			virtual bool reserveBandwith();
			virtual void releaseBandwith();

			virtual bool writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen);

			virtual int decideOpenFileFlag(bool bIndexFile);

			bool writeData(SubFile& subFile, char* pBuf, int dwBufLen);

			bool writeSubFileNoIndex(SubFile& subFile, MediaSample* pSample);
			bool writeSubFileIndex(SubFile& subFile, MediaSample* pSample);
			bool writeSubFile(SubFile& subFile, MediaSample* pSample);

			void initSubFiles();
			bool openSubFiles();
			void closeSubFiles();
       protected:
			bool			_enableCacheForIndex;

			SubFile			_subFiles[10];

			int             _DeleteTargetOnFail;
			bool			_bCleanup;

			std::string		_strFilename;
			uint32			_dwBandwidth;
			bool			_bTypeH264;
			bool			_bPacing;
			static bool		_bPacingTrace;
			bool			_bEnableMD5;
			::ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

			std::map<std::string, int> _SpeedAndFileExt;//used to specify speed num
			float _trickspeed[4];

			FileIoFactory*		_pFileIoFac;		//file io factory
			int					_nWriteLatencyWarningMs;

			std::string	        _cachePath;
			bool                _bIndexVVC;
			PacedIndexFactory*  _pPacedIndexFac;
			PacedIndex*         _pPacedIndexObj;
			std::string         _mainFileExt;
		};

	}}


#endif

