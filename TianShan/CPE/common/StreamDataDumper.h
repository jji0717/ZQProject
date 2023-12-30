


#ifndef _STREAM_DATA_DUMPER_HEADER_
#define _STREAM_DATA_DUMPER_HEADER_

#include <string>

class StreamDataDumper
{
public:
	StreamDataDumper();
	~StreamDataDumper();

	void setPath(const char* szPath);

	void setFile(const char* szFile) ;
	
	bool init();

	void close(bool bSucc=true);

	bool dump(char* buf, int len);

	void enable(bool bEnable=true);

	void deleteOnSuccess(bool bDelOnSucc=true);
private:

	std::string		_strPath;
	std::string		_strFile;
	std::string		_strPathFile;
	bool			_bDeleteOnSuccess;
	bool			_bEnableDump;
    FILE*           _hFile;
};


#endif

