// DataReader.h: interface for the DataReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAREADER_H__B0A630CD_A6EE_4AAB_B06F_722B8BAD1ADD__INCLUDED_)
#define AFX_DATAREADER_H__B0A630CD_A6EE_4AAB_B06F_722B8BAD1ADD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SyncUtil.h"

namespace DataStream {

class BufferBlock;

class DataReader {
public:
	virtual ~DataReader()
	{

	}

	virtual bool open(const std::string& addr, 
		const std::string& param = std::string()) = 0;
	virtual size_t read(unsigned char* buf, size_t len) = 0;
	virtual void close() = 0;
	virtual bool isEnd() = 0;
	virtual void reset() = 0;
	virtual void release()
	{
		delete this;
	}
};

class FsDataReader: public DataReader, protected ZQLIB::LightLock {

public:
	FsDataReader();
	virtual ~FsDataReader();

	virtual bool open(const std::string& addr, 
		const std::string& param = std::string());

	virtual size_t read(unsigned char* buf, size_t len);
	virtual void close();	
	virtual void reset();
	virtual bool isEnd();
	virtual void release();
	
protected:
	std::string		_fileName;
	unsigned char*	_ptr;
	size_t			_fileSize;
	size_t			_pos;
};

} // namespace DataStream {

#endif // !defined(AFX_DATAREADER_H__B0A630CD_A6EE_4AAB_B06F_722B8BAD1ADD__INCLUDED_)
