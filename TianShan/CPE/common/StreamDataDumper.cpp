

#include "ZQ_common_conf.h"
#include "StreamDataDumper.h"
#ifdef ZQ_OS_MSWIN
  #include "NtfsFileIoFactory.h"
#else
  #include "CStdFileIoFactory.h"
#endif


StreamDataDumper::StreamDataDumper()
{
	_bDeleteOnSuccess = true;
	_bEnableDump = false;
	_hFile = 0;
}

StreamDataDumper::~StreamDataDumper()
{
	close(false);
}

void StreamDataDumper::setPath(const char* szPath)
{
	if (!szPath || !szPath[0])
	{
		_strPath = "";
		return;
	}

	_strPath = szPath;

	if(FNSEPC != _strPath[_strPath.size() - 1])
		_strPath.push_back(FNSEPC);
}

void StreamDataDumper::setFile(const char* szFile) 
{
	_strFile = szFile;
}
	
bool StreamDataDumper::init()
{
	if (!_bEnableDump)
		return true;

	_strPathFile = _strPath + _strFile;

#ifdef ZQ_OS_MSWIN
	ZQTianShan::ContentProvision::NtfsFileIoFactory::createDirectoryForFile(_strPathFile);
#else
    ZQTianShan::ContentProvision::CStdFileIoFactory::createDirectoryForFile(_strPathFile);
#endif
	_hFile = fopen(_strPathFile.c_str(), "w");
	if (_hFile==0)
	{
		return false;
	}

	return true;
}

void StreamDataDumper::close(bool bSucc)
{
	if (_hFile!=0)
	{
		fclose(_hFile);
		_hFile = 0;

		if (bSucc && _bDeleteOnSuccess)
		{
			remove(_strPathFile.c_str());
		}
	}	
}

bool StreamDataDumper::dump(char* buf, int len)
{
	if (!_bEnableDump || _hFile == 0)
		return false;

	size_t dwWrite = fwrite(buf, 1,  len, _hFile);
	return (dwWrite == static_cast<size_t>(len));
}

void StreamDataDumper::enable(bool bEnable)
{
	_bEnableDump = bEnable;
}

void StreamDataDumper::deleteOnSuccess(bool bDelOnSucc)
{
	_bDeleteOnSuccess = bDelOnSucc;
}

	
