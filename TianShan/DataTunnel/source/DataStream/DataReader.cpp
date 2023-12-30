// FsDataReader.cpp: implementation of the FsDataReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataReader.h"
#include "BufferManager.h"

namespace DataStream {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FsDataReader::FsDataReader()
{
	_ptr = NULL;
	_pos = 0;
	_fileSize = 0;
}

FsDataReader::~FsDataReader()
{
	if (!_fileName.empty())
		DeleteFileA(_fileName.c_str());
}

bool FsDataReader::open(const std::string& addr, 
					  const std::string& param)
{
	HANDLE fileHandle = CreateFileA(addr.c_str(), GENERIC_READ, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return false;
	_fileSize = GetFileSize(fileHandle, NULL);
	HANDLE fileMap = CreateFileMappingA(fileHandle, NULL, PAGE_READONLY, 
		0, _fileSize, NULL);
	CloseHandle(fileHandle);
	if (fileMap == NULL)
		return false;

	_ptr = (unsigned char* )MapViewOfFile(fileMap, FILE_MAP_READ, 0, 0, 
		_fileSize);

	CloseHandle(fileMap);

	if (_ptr == NULL)
		return false;

	_fileName = addr;

	return true;
}

size_t FsDataReader::read(unsigned char* buf, size_t len)
{
	ZQLIB::AbsAutoLock sync(*this);

	if (_ptr == NULL)
		return 0;

	size_t remain = _fileSize - _pos;
	size_t readlen = len > remain ? remain : len;
	memcpy(buf, _ptr + _pos, readlen);
	_pos += readlen;
	return readlen;
}

void FsDataReader::close()
{
	ZQLIB::AbsAutoLock sync(*this);

	if (_ptr) {
		UnmapViewOfFile(_ptr);
		_ptr = NULL;
	}
}

void FsDataReader::release()
{
	close();
	DataReader::release();
}

bool FsDataReader::isEnd()
{
	ZQLIB::AbsAutoLock sync(*this);

	return _pos >= _fileSize;
}

void FsDataReader::reset()
{
	ZQLIB::AbsAutoLock sync(*this);

	_pos = 0;
}

} // namespace DataStream {
