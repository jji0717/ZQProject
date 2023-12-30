#ifndef _NLEEMULATOR_FILEPARSER_FILE__
#define _NLEEMULATOR_FILEPARSER_FILE__
#include "NLEDefine.h"
#include "FileLog.h"
class FileParser
{
public:
	FileParser(std::string& filepath, ZQ::common::FileLog* log);
	~FileParser(void);
public:
	bool parserFile();
	FileInfos getFileItems() {return _fileitems;};
protected:
	std::string _filepath;
    FileInfos _fileitems;
	ZQ::common::FileLog* _log;
};
#endif//_NLEEMULATOR_FILEPARSER_FILE__
