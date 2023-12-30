#pragma once

#include "ZQ_common_conf.h"
#include "Log.h"
#include "FileLog.h"
#include <string>
#include <vector>

typedef std::vector <std::string> FilesLists;
namespace ZQTianShan {
	namespace ContentProvision {
class NormalizeSparseFile
{
public:
	NormalizeSparseFile(ZQ::common::Log* log, int outputFileCount, FilesLists& outputFileNames, int indexType, bool bGenerateTTS = false);
	~NormalizeSparseFile(void);
public:
	int normalizeSparseFileSet();
protected:
    int _outputFileCount;
	std::string _indexFilepath;
	ZQ::common::Log *_log;
	FilesLists& _outputFileNames;
	bool _bGenerateTTS;
	int _indexType;
//	std::string _cacheDir;

	std::string _tempIndexFile;
protected:
	int normalizeSparseIndexFileVVX( int indexFile, int tempFile, int *pFileCount,
		int64 *pFirstOffsets, int64 *pFinalOffsets );
	int normalizeSparseIndexFileVV2( int indexFile, int tempFile, int *pFileCount,
		int64 *pFirstOffsets, int64 *pFinalOffsets );
	int normalizeSparseIndexFileVVC( int indexFile, int tempFile, int *pFileCount,
		int64 *pFirstOffsets, int64 *pFinalOffsets );
	int  normalizeSparseIndexFile(int *pFileCount,
		int64 *pFirstOffsets, int64 *pFinalOffsets );
	int  normalizeSparseSpeedFile(int fileNumber,
		int64 *pFirstOffsets, int64 *pFinalOffsets );
	int  deleteIndexFile();

};
}}
