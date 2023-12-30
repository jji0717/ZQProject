// MuxItemImpl.cpp: implementation of the MuxItemImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDef.h"
#include "DataStreamImpl.h"
#include "MuxItemImpl.h"
#include "DataSource.h"
#include "DataReader.h"
namespace TianShanIce  {
namespace Streamer     {
namespace DataOnDemand {

MuxItemImpl::MuxItemImpl(ZQADAPTER_DECLTYPE& adapter):
	_adapter(adapter)
{
	_dataSource = NULL;
	_destroy = false;
}

MuxItemImpl::~MuxItemImpl()
{

}

bool MuxItemImpl::init(const MuxItemInfo& info)
{
	_info = info;
	return true;
}

::std::string
MuxItemImpl::getName(const Ice::Current& current) const
{
    return _info.name;
}

void
MuxItemImpl::notifyFullUpdate(const ::std::string& fileName, 
							  const Ice::Current& current)
{
	Lock sync(*this);
	
	std::string spaceName = _parent->getSpaceName();
	std::string strmName = _parent->getName();

	glog(ZQLIB::Log::L_DEBUG,  
		"MuxItemImpl::notifyFullUpdate(%s): muxItemName = %s::%s::%s" 
		LOG_FMT, fileName.c_str(), spaceName.c_str(), strmName.c_str(), 
		_info.name.c_str(), LOG_ARG);
	
	if (!fetchFile(fileName)) {

		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::notifyFullUpdate(): muxItemName = %s::%s::%s " 
			"cannot fetch the file(%s)." LOG_FMT, spaceName.c_str(), 
			strmName.c_str(), _info.name.c_str(), fileName.c_str(), 
			LOG_ARG);

		return;
	}

	_fileList.clear();
	_fileList.push_back(fileName);

	// for local folder
	if (_dataSource) {

		std::string filePart = ::DataStream::getFileNamePart(fileName);
		::DataStream::DataReader* reader = rebuildReader(filePart, 
			UpdateFull);
		_dataSource->switchReader(reader);
	}
}

void
MuxItemImpl::notifyFileAdded(const ::std::string& fileName,
							 const Ice::Current& current)
{
	Lock sync(*this);

	std::string spaceName = _parent->getSpaceName();
	std::string strmName = _parent->getName();

	glog(ZQLIB::Log::L_DEBUG,  
		"MuxItemImpl::notifyFileAdded(%s): muxItemName = %s::%s::%s" 
		LOG_FMT, fileName.c_str(), spaceName.c_str(), strmName.c_str(), 
		_info.name.c_str(), LOG_ARG);

	assert(_dataSource);

	if (!fetchFile(fileName)) {

		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::notifyFileAdded(): muxItemName = %s::%s::%s " 
			"cannot fetch the file(%s)." LOG_FMT, spaceName.c_str(), 
			strmName.c_str(), _info.name.c_str(), fileName.c_str(), 
			LOG_ARG);

		return;
	}

	std::string filePart = ::DataStream::getFileNamePart(fileName);
	_fileList.push_back(fileName);	

	if (_dataSource) {
		::DataStream::DataReader* reader = rebuildReader(filePart, 
			UpdateAdded);
		_dataSource->switchReader(reader);
	} else {		

		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::notifyFullUpdate(): muxItemName = %s::%s::%s " 
			"_dataSource == NULL " LOG_FMT, spaceName.c_str(), 
			strmName.c_str(), _info.name.c_str(), LOG_ARG);

		assert(false);
	}
}

void
MuxItemImpl::notifyFileDeleted(const ::std::string& fileName,
							   const Ice::Current& current)
{
	Lock sync(*this);

	std::string spaceName = _parent->getSpaceName();
	std::string strmName = _parent->getName();

	glog(ZQLIB::Log::L_DEBUG,  
		"MuxItemImpl::notifyFileDeleted(%s): muxItemName = %s::%s::%s" 
		LOG_FMT, fileName.c_str(), spaceName.c_str(), strmName.c_str(), 
		_info.name.c_str(), LOG_ARG);

	assert(_dataSource);

	FileList::iterator it = std::find(_fileList.begin(), _fileList.end(), 
		fileName);

	if (it == _fileList.end()) {		
		
		glog(ZQLIB::Log::L_WARNING,  
			"MuxItemImpl::notifyFileDeleted(): muxItemName = %s::%s::%s " 
			"file not found(%s)" LOG_FMT, spaceName.c_str(), 
			strmName.c_str(), _info.name.c_str(), fileName.c_str(), LOG_ARG);

		return;
	}

	deleteFile(fileName);
	_fileList.erase(it);

	std::string filePart = ::DataStream::getFileNamePart(fileName);
	if (_dataSource) {

		::DataStream::DataReader* reader = rebuildReader(filePart, 
			UpdateDeleted);	
		_dataSource->switchReader(reader);

	} else {

		std::string spaceName = _parent->getSpaceName();
		std::string strmName = _parent->getName();

		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::notifyFullUpdate(): muxItemName = %s::%s::%s " 
			"_dataSource == NULL " LOG_FMT, spaceName.c_str(), 
			strmName.c_str(), _info.name.c_str(), LOG_ARG);
		assert(false);
	}
}

MuxItemInfo
MuxItemImpl::getInfo(const Ice::Current& current) const
{
    return _info;
}

void
MuxItemImpl::destroy(const Ice::Current& current)
{
	Lock sync(*this);
	
	if (_parent->removeMuxItem(_info.name))
		_destroy = true;
	else
		throw DataStreamError("DataStreamImpl", 0, "cannot destory mux item");
}

void 
MuxItemImpl::setProperties(const ::TianShanIce::Properties& props, 
						  const ::Ice::Current& current)
{
	_props = props;
}

::TianShanIce::Properties 
MuxItemImpl::getProperties(const ::Ice::Current& current) const
{
	return _props;
}

::DataStream::DataSource* 
MuxItemImpl::getDataSource()
{
	return _dataSource;
}

void 
MuxItemImpl::setDataSource(::DataStream::DataSource* dataSource)
{
	Lock sync(*this);

	_dataSource = dataSource;

	// for local folder
	if (_fileList.size()) {
		assert(_fileList.size() == 1);
		std::string& fileName = _fileList[0];
		std::string filePart = ::DataStream::getFileNamePart(fileName);
		::DataStream::DataReader* reader = rebuildReader(filePart, 
			UpdateFull);
		_dataSource->switchReader(reader);		
	}
}

bool MuxItemImpl::fetchFile(const std::string& fileName)
{
	std::string cacheFile = _parent->getCacheDir();
	cacheFile += "\\";
	cacheFile += ::DataStream::getFileNamePart(fileName);

	if (!CopyFileA(fileName.c_str(), cacheFile.c_str(), FALSE))
		return GetLastError() == ERROR_ALREADY_EXISTS;

	return true;
}

bool MuxItemImpl::deleteFile(const std::string& fileName)
{
	std::string cacheFile = _parent->getCacheDir();
	cacheFile += "\\";
	cacheFile += ::DataStream::getFileNamePart(fileName);
	return DeleteFileA(cacheFile.c_str());
}

bool MuxItemImpl::appendFile(FILE* dest, FILE* src)
{
	char buf[0x1000];
	while (!feof(src)) {
		size_t readLen = fread(buf, 1, sizeof(buf), src);
		fwrite(buf, 1, readLen, dest);
	}

	return true;
}

::DataStream::DataReader* MuxItemImpl::rebuildReader(
	const std::string& fileName, int type)
{
	if (_fileList.size() == 0)
		return NULL;

	char linkedName[MAX_PATH];
	std::string cacheDir = _parent->getCacheDir();
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	
	sprintf(linkedName, "%s\\%02d%02d%02d%02d%02d%03d%u%u.dod", 
		cacheDir.c_str(), sysTime.wMonth, sysTime.wDay, sysTime.wHour, 
		sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, 
		rand(), rand());

	FILE* lnfp = fopen(linkedName, "wb");
	if (lnfp == NULL) {
		
		std::string spaceName = _parent->getSpaceName();
		std::string strmName = _parent->getName();

		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::rebuildReader(): muxItemName = %s::%s::%s " 
			"cannot create file(%s). lastErr = %d, errno = %d" LOG_FMT, 
			spaceName.c_str(), strmName.c_str(), _info.name.c_str(), 
			linkedName, GetLastError(), errno, LOG_ARG);
		
		return NULL;
	}

	FileList::iterator it;
	for (it = _fileList.begin(); it != _fileList.end(); it ++) {

		// local filename
		std::string itemName = cacheDir + "\\" + 
			::DataStream::getFileNamePart(*it);

		FILE* ifp = fopen(itemName.c_str(), "rb");

		if (ifp == NULL) {

			DWORD lastErr = GetLastError();
		
			std::string spaceName = _parent->getSpaceName();
			std::string strmName = _parent->getName();

			glog(ZQLIB::Log::L_WARNING,  
				"MuxItemImpl::rebuildReader(): muxItemName = %s::%s::%s " 
				"cannot open file(%s). lastErr = %d, errno = %d" LOG_FMT, 
				spaceName.c_str(), strmName.c_str(), _info.name.c_str(), 
				itemName.c_str(), lastErr, errno, LOG_ARG);

		} else {
			appendFile(lnfp, ifp);
			fclose(ifp);
		}
	}

	fclose(lnfp);
	
	::DataStream::FsDataReader* reader = 
		new ::DataStream::FsDataReader();
	if (!reader->open(linkedName)) {
		
		DWORD lastErr = GetLastError();

		delete reader;

		std::string spaceName = _parent->getSpaceName();
		std::string strmName = _parent->getName();
		
		glog(ZQLIB::Log::L_ERROR,  
			"MuxItemImpl::rebuildReader(): muxItemName = %s::%s::%s " 
			"cannot open linkage file(%s). lastErr = %d, errno = %d" LOG_FMT, 
			spaceName.c_str(), strmName.c_str(), _info.name.c_str(), 
			linkedName, lastErr, errno, LOG_ARG);

		return NULL;
	}
		
	return reader;
}

} // namespace DataOnDemand {
}
}
