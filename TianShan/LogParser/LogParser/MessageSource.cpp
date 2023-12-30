#include "MessageSource.h"
#include <Exception.h>
#include <vector>
#include <time.h>
#include <TimeUtil.h>
#include <algorithm>
#include "TimeConv.h"
#include "FileSystemOp.h"
#include <strHelper.h>

#pragma message(__MSGLOC__"NOTICE: Define the MACRO EventSink_Log_Trivial to enable the trivial log")

// define the macro EventSink_Log_Trivial for the trivial log
// #define EventSink_Log_Trivial 1
/////////////////////////////////////////////////////////////
/// FileLogChopper
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/stat.h>
}
	#ifndef stricmp
		#define stricmp strcasecmp
	#endif
#else
#include <io.h>
#endif

// check the validity of a stream postion
// call this before an uncertain seek
static bool validPosition(std::istream& f, int pos)
{
    if(pos < 0)
        return false;

    f.clear();
    f.seekg(0, std::ios::end);
    return (pos <= f.tellg());
}

// take some data from a file
static bool take(std::string& data, std::istream& f, int pos, size_t count)
{
    data.clear();

    if(validPosition(f, pos))
    {
        if(count == 0) // just test the position
            return true;

        if(validPosition(f, pos + count))
        {
            f.clear();
            f.seekg(pos);

            std::vector<char> buf;
            buf.resize(count + 1);
            if(f.read(&buf[0], count))
            {
                data.reserve(count);
                data.assign(&buf[0], count);
                return true;
            }
            else // io error
            {
                return false;
            }
        }
        else // bad end position
        {
            return false;
        }
    }
    else // bad begin position
    {
        return false;
    }
}
// take one line from a file
static bool takeLine(std::string& data, std::istream& f, int pos)
{
    data.clear();
    data.reserve(256); // just a guess
    if(validPosition(f, pos))
    {
        f.clear();
        f.seekg(pos);

        char ch = '\0';
        while(f.get(ch) && ch != '\r' && ch != '\n') {
            data.push_back(ch);
        }
        if(f || f.eof()) {
            return true;
        } else { // io error
            return false;
        }
    }
    else // bad begin position
    {
        return false;
    }
}

ArchivedLogChopper::ArchivedLogChopper(ZQ::common::Log& log)
    :_log(log), _pos(0), _nSkipLines(0)
{
    _linesBuf.clear();
}

// EventSink_Log_Trivial

// open the correct log file
bool ArchivedLogChopper::open(const MessageIdentity& recoverPoint)
{
    using namespace ZQ::common;
    ArchiveCursor& ac = getArchiveCursor();
    std::string logname = ac.getMainFile();
    std::string recoverStamp;
    if(recoverPoint.stamp > 0)
        recoverStamp = time2utc(recoverPoint.stamp);

    if(_logfile.is_open())
    {
        _logfile.close();
    }
    _pos = recoverPoint.position;
    _nSkipLines = 0;
    if(recoverStamp.empty()) {
        _log(Log::L_INFO, CLOGFMT(ArchivedLogChopper, "open() [%s] Invalid stamp:%lld. Recover from main log."), logname.c_str(), recoverPoint.stamp);
        ac.reset(); 
        _pos = 0;
    }

    // search for the correct file
    std::string failReason; // record the reason of search failure
    _logfile.open(ac.getCurrentFile().c_str(), std::ios::in | std::ios::binary);
    while(_logfile.is_open())
    {
        if(recoverStamp.empty()) { // must be the main file
#ifdef EventSink_Log_Trivial
            _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "open() [%s] got EMPTY recover record, recover from the main log."), logname.c_str());
#endif
            break;
        }
        std::string l;
        // check the stamp at the recover point
        if(takeLine(l, _logfile, _pos) && !l.empty()) {
            int64 targetTime = getStampOfLine(l.c_str());
#ifdef EventSink_Log_Trivial
            std::string stampStr = time2utc(targetTime);
            _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "open() [%s] check the recover position [%lld] of [%s], got line[%s], stamp [%s], recover point[%s]."), logname.c_str(), recoverPoint.position, ac.getCurrentFile().c_str(), l.c_str(), stampStr.c_str(), recoverStamp.c_str());
#endif
            if(targetTime == recoverPoint.stamp) { // got the correct file
                // we should recover from the line after this point
                // don't calculate the position here but skip one line in the fetchNext()
                _nSkipLines = 1;
#ifdef EventSink_Log_Trivial
                _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "open() [%s] Get the recover point at file [%s]."), logname.c_str(), ac.getCurrentFile().c_str());
#endif
                break;
            } else {
#ifdef EventSink_Log_Trivial
                _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "open() [%s] Didn't find the recover point at [%s] due to stamp mismatch."), logname.c_str(), ac.getCurrentFile().c_str());
#endif
            }
        } else {
#ifdef EventSink_Log_Trivial
            _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "open() [%s] Didn't find the recover point at [%s] due to bad position."), logname.c_str(), ac.getCurrentFile().c_str());
#endif
        }
        { // skip this file
            // try the next history file
            _logfile.close();
            ac.movePrevious();
            _logfile.open(ac.getCurrentFile().c_str(), std::ios::in | std::ios::binary);
            continue;
        }
    }
    // all file tested

    if(_logfile.is_open())
    { // get the file
        return true;
    }
    else
    { // we can't start from the last record position
        if(!failReason.empty())
        {
            _log(Log::L_INFO, CLOGFMT(ArchivedLogChopper, "open() Not open log file %s due to %s"), ac.getCurrentFile().c_str(), failReason.c_str());
        }
        else // we need check the open right
        {
            // check the accessibility
            if(0 != access(ac.getCurrentFile().c_str(), 0))
            { // not exist
                _log(Log::L_INFO, CLOGFMT(ArchivedLogChopper, "open() Can't access file %s due to %s"), ac.getCurrentFile().c_str(), strerror(errno));
            }
            else if(0 != access(ac.getCurrentFile().c_str(), 4))
            { // not readable
                _log(Log::L_INFO, CLOGFMT(ArchivedLogChopper, "open() Can't read file %s due to %s"), ac.getCurrentFile().c_str(), strerror(errno));
            }
            else
            {
                _log(Log::L_WARNING, CLOGFMT(ArchivedLogChopper, "open() File %s is readable but can't be opened"), ac.getCurrentFile().c_str());
            }
        }

        if(ac.isMainFile())
        { // can't open the main file
            _log(Log::L_ERROR, CLOGFMT(ArchivedLogChopper, "open() Can't open main log file %s"), ac.getCurrentFile().c_str());
            return false;
        }
        else
        {
            // all history log need to be processed
            ac.moveNext(); // the earliest log
            _log(Log::L_INFO, CLOGFMT(ArchivedLogChopper, "open() All files checked but can't find the recover point(position:%lld, stamp:%s). Start parsing from file %s"), _pos, recoverStamp.c_str(), ac.getCurrentFile().c_str());
            return switchFile(ac.getCurrentFile());
        }
    }
}

void ArchivedLogChopper::close()
{
    _pos = 0;
    _linesBuf.clear();
    if(_logfile.is_open())
    {
        _logfile.close();
    }
}

int ArchivedLogChopper::fetchNext(char* buf, int* len, MessageIdentity& mid)
{
    mid.source.clear();
    mid.position = 0;
    mid.stamp = 0;
    int nSkipped = 0; // the count skipped lines
    int nSkippedWarningThreshold = 64;
    ArchiveCursor& ac = getArchiveCursor();
    while(true) { // may switch files
        if(!_logfile.is_open())
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(ArchivedLogChopper, "fetchNext() need open the file %s first"), ac.getCurrentFile().c_str());
            return -1;
        }

        if(*len > 0)
        {
            *buf = '\0';
        }
        while(true) { // may skip some lines
            if(nSkipped == nSkippedWarningThreshold) {
                if(nSkippedWarningThreshold < 1024)
                    nSkippedWarningThreshold *= 2;
                else
                    nSkippedWarningThreshold += 1024;
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(ArchivedLogChopper, "fetchNext() %d lines had been skipped. The content of file(%s) may be corrupt."), nSkipped, ac.getCurrentFile().c_str());
            }
            if(!_linesBuf.hasLines())
            { // no cached data
                if(!_linesBuf.feed(_logfile, _pos))
                { // bad position?
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(ArchivedLogChopper, "fetchNext() can't read file %s at position %d. Continue from the beginning of file."), ac.getCurrentFile().c_str(), _pos);

                    _pos = 0;
                    *len = 0;
                    return 0;
                }
            }

            const char* curLine = _linesBuf.getLine();
            if(curLine)
            {
                if(_nSkipLines > 0) { // skip this line as need
                    --_nSkipLines;
                    _pos += _linesBuf.getLineSize();
                    _linesBuf.next();
                    ++nSkipped;
                    continue;
                }

                int64 stamp = getStampOfLine(curLine);
                if(stamp <= 0) { // bad content, skip this line
                    _pos += _linesBuf.getLineSize();
                    _linesBuf.next();
                    ++nSkipped;
                    continue;
                }
                // copy the line
                int bufLen = *len;
                int dataLen = strlen(curLine);
                *len = dataLen + 1;
                if(bufLen >= *len) // buffer enough
                { // just copy the data
                    strcpy(buf, curLine);
                    mid.source = ac.getMainFile();
                    mid.position = _pos;
                    mid.stamp = stamp;

                    _pos += _linesBuf.getLineSize();
                    _linesBuf.next();
                    return dataLen;
                }
                else
                { // need more buffer
                    return 0;
                }
            }
            else // no data
            { // reach the end of file
                *len = 0;
                if(ac.isMainFile()) // main file
                {
                    return 0;
                }
                else // history file
                {
                    ac.moveNext();
                    _log(ZQ::common::Log::L_INFO, CLOGFMT(ArchivedLogChopper, "fetchNext() Reach the end of file. Start parsing file %s"), ac.getCurrentFile().c_str());
                    if(!switchFile(ac.getCurrentFile()))
                        return -1;
                    // else: try the first line of the next file
                }
            }
        } // skip lines
    } // switch files
}
bool ArchivedLogChopper::switchFile(const std::string& fileName)
{
    using namespace ZQ::common;
    if(_logfile.is_open())
    {
        _logfile.close();
    }

    _log(Log::L_DEBUG, CLOGFMT(ArchivedLogChopper, "switchFile() Change log file to %s"), fileName.c_str());
    _logfile.open(fileName.c_str(), std::ios::in | std::ios::binary);
    if(_logfile.is_open())
    {
        _pos = 0;
        return true;
    }
    else
    { // can't open file
        _log(Log::L_ERROR, CLOGFMT(ArchivedLogChopper, "switchFile() Can't open log file %s"), fileName.c_str());
        return false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// NewFileLog
bool NewFileLogChopper::open(const MessageIdentity& recoverPoint)
{
	using namespace ZQ::common;
	_pos  = recoverPoint.position;
    _time = recoverPoint.stamp;

    if(_logfile.is_open())
	{
        _logfile.close();
	}
    if(0 == _time)
	{
        _log(Log::L_INFO, CLOGFMT(NewFileLogChopper, "open() [%s] Invalid stamp:%lld. Recover from main log."),  _cursor.getCurrentFile().c_str(), recoverPoint.stamp);
        _cursor.reset(); 
        _pos = 0;
    }

    // search for the correct file
    _logfile.open(_cursor.getCurrentFile().c_str(), std::ios::in | std::ios::binary);
    while(_logfile.is_open())
    {
		if(0 == _time)
		{
			return true;
		}
        std::string l;
        // check the stamp at the recover point
        if(takeLine(l, _logfile, _pos) && !l.empty())
		{
            int64 targetTime = getStampOfLine(l.c_str());
//			_log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "open() [%s] check the recover position [%lld] of [%s], got line[%s], stamp [%s], recover point[%s]."), _cursor.getShortName().c_str(), recoverPoint.position, _cursor.getCurrentFile().c_str(), l.c_str(), (time2utc(targetTime)).c_str(), (time2utc(_time)).c_str());
            if(targetTime == recoverPoint.stamp) // got the correct file
			{ 
                // we should recover from the line after this point
                // don't calculate the position here but skip one line in the fetchNext()
                _nSkipLines = 1;
                return true;
            }
        }
		if(_cursor.isOldestFile())
		{	
			_cursor.reset(); 
			
			if(switchFile(_cursor.getCurrentFile()))
			{
				_log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "open() All files checked but can't find the recover point(position:%lld, stamp:%s). Start parsing from file %s"), recoverPoint.position, time2utc(_time).c_str(), _cursor.getCurrentFile().c_str());
				return true;
			}
			else
				return false;
		}
		else
		{	// skip this file
            // try the next history file
            _logfile.close();

			_log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "skip the history file [%s]"),_cursor.getCurrentFile().c_str());
            _cursor.movePrevious();
            _logfile.open(_cursor.getCurrentFile().c_str(), std::ios::in | std::ios::binary);
			if(_logfile.is_open())
				_log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "try the history file [%s]"),_cursor.getCurrentFile().c_str());
            continue;
        }
	}

	if(!_logfile.is_open())
    {
		// check the accessibility
        if(0 != access(_cursor.getCurrentFile().c_str(), 0))
        { // not exist
			_cursor.reset();	//recoverPoint has been invalid, go to reset()
			_log(Log::L_INFO, CLOGFMT(NewFileLogChopper, "open() Can't access file %s due to %s"), _cursor.getCurrentFile().c_str(), strerror(errno));
		}
		else if(0 != access(_cursor.getCurrentFile().c_str(), 4))
		{ // not readable
			_log(Log::L_INFO, CLOGFMT(NewFileLogChopper, "open() Can't read file %s due to %s"), _cursor.getCurrentFile().c_str(), strerror(errno));
		}
		else
		{
			_log(Log::L_WARNING, CLOGFMT(NewFileLogChopper, "open() File %s is readable but can't be opened"), _cursor.getCurrentFile().c_str());
		}

		return false;
    }
}
void NewFileLogChopper::close()
{
	_pos = 0;
	_linesBuf.clear();
	if(_logfile.is_open())
	{
		_logfile.close();
	}
}
int  NewFileLogChopper::fetchNext(char* buf, int* len, MessageIdentity& mid)
{
	mid.source.clear();
	mid.position = 0;
	mid.stamp = 0;
	int nSkipped = 0;	// the count skipped lines
	int nSkippedWarningThreshold = 64;
	while(true)			// may switch files
	{	
		if(!_logfile.is_open())
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(NewFileLogChopper, "fetchNext() need open the file %s first"), _cursor.getCurrentFile().c_str());
            return -1;
        }
		if(*len > 0)
		{
			*buf = '\0';
		}
		while(true)		// may skip some lines
		{ 
			if(nSkipped == nSkippedWarningThreshold)
			{
				if(nSkippedWarningThreshold < 1024)
					nSkippedWarningThreshold *= 2;
				else
					nSkippedWarningThreshold += 1024;
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(NewFileLogChopper, "fetchNext() %d lines had been skipped. The content of file(%s) may be corrupt."), nSkipped, _cursor.getCurrentFile().c_str());
            }
			if(!_linesBuf.hasLines())	// no cached data
			{ 
				if(!_linesBuf.feed(_logfile, _pos))	// bad position?
				{ 
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(NewFileLogChopper, "fetchNext() can't read file %s at position %d. Continue from the beginning of file."), _cursor.getCurrentFile().c_str(), _pos);

					_pos = 0;
					*len = 0;
					return 0;
				}
			}

			const char* curLine = _linesBuf.getLine();
			if(curLine)
			{
				if(_nSkipLines > 0)		// skip this line as need
				{
					--_nSkipLines;
					_pos += _linesBuf.getLineSize();
					_linesBuf.next();
					++nSkipped;
					continue;
				}
				
				int64 stamp = getStampOfLine(curLine);
				if(stamp <= 0)			// bad content, skip this line
				{ 
					_pos += _linesBuf.getLineSize();
					_linesBuf.next();
					++nSkipped;
					continue;
				}
                // copy the line
                int bufLen = *len;
                int dataLen = strlen(curLine);
                *len = dataLen + 1;
                if(bufLen >= *len)		// buffer enough,just copy the data
                {
                    strcpy(buf, curLine);
                    mid.source = _cursor.getMainFile();
                    mid.position = _pos;
                    mid.stamp = stamp;

                    _pos += _linesBuf.getLineSize();
                    _linesBuf.next();
                    return dataLen;
                }
                else	// need more buffer
                { 
                    return 0;
                }
            }
            else		// no data
            { 
				if(_cursor.isLatestFile())
				{
					*len = 0;
					return 0;
				}
				else
                {
                    _cursor.moveNext();
                    _log(ZQ::common::Log::L_INFO, CLOGFMT(NewFileLogChopper, "fetchNext() Reach the end of file. Start parsing file %s"), _cursor.getCurrentFile().c_str());
                    if(!switchFile(_cursor.getCurrentFile()))
                        return -1;
                    // else: try the first line of the next file
                }
            }
        } // skip lines
    } // switch files
}
bool NewFileLogChopper::switchFile(const std::string& fileName)
{
	using namespace ZQ::common;
    if(_logfile.is_open())
    {
        _logfile.close();
    }

    _log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "switchFile() Change log file to %s"), fileName.c_str());
    _logfile.open(fileName.c_str(), std::ios::in | std::ios::binary);
    if(_logfile.is_open())
    {
        _pos = 0;
        return true;
    }
    else
    { // can't open file
        _log(Log::L_ERROR, CLOGFMT(NewFileLogChopper, "switchFile() Can't open log file %s"), fileName.c_str());
        return false;
    }
}
NewFileLogChopper::NewFileLogsCursor::NewFileLogsCursor()
{
}
void NewFileLogChopper::NewFileLogsCursor::getAllFile()
{
	std::vector<std::string> filesOfLastYear; 

	char searchFor[260];
	snprintf(searchFor, MAX_PATH-2, "%s" FNSEPS "%s.*.%s", dirName.c_str(), shortName.c_str(), "log");
#ifdef ZQ_OS_MSWIN

	SYSTEMTIME filetime;
	SYSTEMTIME currenttime;
	GetLocalTime(&currenttime);

	WIN32_FIND_DATAA	finddata;
	HANDLE hFind;
	bool done = false;
	for (hFind = ::FindFirstFileA(searchFor, &finddata); INVALID_HANDLE_VALUE != hFind && !done;
		done = (0 ==::FindNextFileA(hFind, &finddata)))
	{
		__int64 timestamp =0;
		char extname[20]="\0";
		if (sscanf(finddata.cFileName +shortName.length(), ".%lld%s", &timestamp, extname) <=1 || 0 != stricmp(extname, ".log"))
			continue; // skip those files that are not in the pattern of rolling filenames

		if(timestamp < 1010000000 || timestamp > 12312359599) // 01010000000 is the minimum timestamp ,12312359599 is the maximum timestamp
			continue;

		FileTimeToSystemTime(&finddata.ftCreationTime, &filetime); 

		if (filetime.wYear < currenttime.wYear || (timestamp/1000000000) > currenttime.wMonth)
			filesOfLastYear.push_back(finddata.cFileName);
		else files.push_back(finddata.cFileName);
	}

	::FindClose(hFind);

#else

	//Wei-TODO:
	struct timeval tv;
	struct tm* filetime;
	struct tm* currenttime;
	gettimeofday(&tv, NULL);
	currenttime = localtime(&tv.tv_sec);

	glob_t gl;
	struct stat fileinfo;

	if(!glob(searchFor, GLOB_PERIOD|GLOB_TILDE, 0, &gl))
	{
		for(size_t i = 0 ; i < gl.gl_pathc ; ++i)
		{
			int64 timestamp = 0;
			char extname[20] = "\0";
			char filename[MAX_PATH] = "\0";
			if(sscanf(gl.gl_pathv[i]+ dirName.length() + 1 + shortName.length(),".%lld%s" ,&timestamp, &extname) <= 1 && 0 != strcasecmp(extname, ".log") )
				continue; // skip those files that are not in the pattern of rolling filenames

			if(timestamp < 1010000000 || timestamp > 12312359599) // 01010000000 is the minimum timestamp ,12312359599 is the maximum timestamp
				continue;

			stat(gl.gl_pathv[i], &fileinfo);
			filetime = localtime(&fileinfo.st_mtime);
			snprintf(filename, MAX_PATH-2, "%s.%011lld.log", shortName.c_str(), timestamp);
			if(filetime->tm_year < currenttime->tm_year || (timestamp/1000000000) > currenttime->tm_mon+1)
				filesOfLastYear.push_back(filename);
			else files.push_back(filename);

		}
	}
	globfree(&gl);
#endif

	sort(filesOfLastYear.begin(), filesOfLastYear.end());
	sort(files.begin(), files.end());
	files.insert(files.begin(), filesOfLastYear.begin(), filesOfLastYear.end());
	files.push_back(shortName + ".log");
}

bool NewFileLogChopper::NewFileLogsCursor::isLatestFile()
{
	refresh();
	return (*itFileName == *files.rbegin());
}
bool NewFileLogChopper::NewFileLogsCursor::isOldestFile()
{
	refresh();
	return (*itFileName == *files.begin());
}
bool NewFileLogChopper::NewFileLogsCursor::init(const std::string& filePath)
{
    using namespace ZQ::common;
    if(filePath.empty()) {
        return false;
    }
	
	// parse the file name
	std::string extName;
	char searchFor[260];
    const char* fileName = filePath.c_str();
    const char* p = ::strrchr(fileName, FNSEPC);
	if (NULL != p)
	{
		dirName = ::std::string(fileName, p-fileName);
		shortName = p+1;
	}
	else
		shortName = fileName;

	size_t pos = shortName.find_last_of(".");
	if (std::string::npos != pos && pos >1)
	{
		extName = shortName.substr(pos+1);
		shortName = shortName.substr(0, pos);
	}
	
	// get all the file name
	getAllFile();

	// get the latest filename
	if(files.size() != 0)
	{
		itFileName = files.end()-1; 

		return true;
    }
	else// not fit the log file name scheme
	{
		return false;
    }
}
// get the module name
const std::string NewFileLogChopper::NewFileLogsCursor::getShortName() const
{
	return shortName;
}
// get the main file name
const std::string NewFileLogChopper::NewFileLogsCursor::getMainFile() const
{
    return (dirName + FNSEPS + shortName + ".log");
}
// get the current file name
const std::string NewFileLogChopper::NewFileLogsCursor::getCurrentFile() const
{
    return (dirName + FNSEPS + *itFileName);
}
// move the cursor to the main file
void NewFileLogChopper::NewFileLogsCursor::reset()
{
	files.erase(files.begin(),files.end());
	getAllFile();
	itFileName = files.end() - 1;
}
// move the cursor to the newer archive file;
void NewFileLogChopper::NewFileLogsCursor::moveNext()
{
	refresh();
	itFileName++;
}
// move the cursor to the older archive file
void NewFileLogChopper::NewFileLogsCursor::movePrevious()
{
	refresh();
	itFileName--;
}
void NewFileLogChopper::NewFileLogsCursor::refresh()
{
	std::string currentFileName = *itFileName;
	// 实时获取当前所有匹配的FileLog
	files.erase(files.begin(),files.end());
	getAllFile();
	
	// 给itFileName定位，使其指向current filename
	itFileName = find(files.begin(), files.end(), currentFileName);
}
NewFileLogChopper::NewFileLogChopper(ZQ::common::Log& log, const std::string& filePath)
:_log(log), _nSkipLines(0)
{
    using namespace ZQ::common;
    log(Log::L_DEBUG, CLOGFMT(NewFileLogChopper, "Init NewFileLogChopper with file [%s]"), filePath.c_str());
    if(!_cursor.init(filePath))
    {
        throw ZQ::common::Exception(std::string("Bad file name: ") + filePath);
    }
}
// get the time stamp of the log data
int64 NewFileLogChopper::getStampOfLine(const char* line)
{
    return getFilelogTime(line);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FileLog
FileLogChopper::FileLogsCursor::FileLogsCursor()
    :instanceId_(-1) {
}
bool FileLogChopper::FileLogsCursor::init(const std::string& filePath) {
    using namespace ZQ::common;
    if(filePath.empty()) {
        return false;
    }
    const char* fileName = filePath.c_str();
    // parse the file name
    const char* ext = strrchr(fileName, '.');
    if(ext && 0 == stricmp(ext, ".log")) {
        baseFileName_.assign(fileName, (ext - fileName));
        mainFileName_ = baseFileName_ + ".log";
        currentFileName_ = mainFileName_;
        instanceId_ = -1;
        return true;
    } else { // not fit the log file name scheme
        return false;
    }
}
// get the main file name
const std::string& FileLogChopper::FileLogsCursor::getMainFile() const {
    return mainFileName_;
}
// get the current file name
const std::string& FileLogChopper::FileLogsCursor::getCurrentFile() const {
    return currentFileName_;
}
// check if the current file is the main file
bool FileLogChopper::FileLogsCursor::isMainFile() const {
    return (instanceId_ == -1);
}
// move the cursor to the main file
void FileLogChopper::FileLogsCursor::reset() {
    instanceId_ = -1;
    currentFileName_ = mainFileName_;
}
// move the cursor to the newer archive file;
void FileLogChopper::FileLogsCursor::moveNext() {
    if(instanceId_ >= 0) {
        --instanceId_;
        currentFileName_ = logFileName(instanceId_);
    }
}
// move the cursor to the older archive file
void FileLogChopper::FileLogsCursor::movePrevious() {
    ++instanceId_;
    currentFileName_ = logFileName(instanceId_);
}

std::string FileLogChopper::FileLogsCursor::logFileName(int inst) const {
    if(inst < 0) {
        return (baseFileName_ + ".log");
    } else {
        char buf[12] = {0};
        return (baseFileName_ + "." + itoa(inst, buf, 10) + ".log");
    }
}

FileLogChopper::FileLogChopper(ZQ::common::Log& log, const std::string& filePath)
    :ArchivedLogChopper(log) {
    using namespace ZQ::common;
    log(Log::L_DEBUG, CLOGFMT(FileLogChopper, "Init FileLogChopper with file [%s]"), filePath.c_str());
    if(!_cursor.init(filePath))
    {
        throw ZQ::common::Exception(std::string("Bad file name: ") + filePath);
    }
}
// get the archive cursor
ArchiveCursor& FileLogChopper::getArchiveCursor() {
    return _cursor;
}

// get the time stamp of the log data
int64 FileLogChopper::getStampOfLine(const char* line) {
    return getFilelogTime(line);
}


///////////////////////////////////////////////////////////////
/// SCLogChopper
SCLogChopper::SCLogChopper(ZQ::common::Log& log, const std::string& filePath)
    :_log(log), _filePath(filePath), _pos(0)
{
    _nSkipLines = 0;
    using namespace ZQ::common;
    _log(Log::L_DEBUG, CLOGFMT(SCLogChopper, "Init FileLogChopper with file [%s]"), filePath.c_str());
    if(!init(filePath))
    {
        throw ZQ::common::Exception(std::string("Bad file name: ") + filePath);
    }
}

#define SCLogStampLength 18
// open the correct log file
bool SCLogChopper::open(const MessageIdentity& recoverPoint)
{
    using namespace ZQ::common;
    std::string recoverStamp;
    if(recoverPoint.stamp > 0)
        recoverStamp = time2utc(recoverPoint.stamp);

    if(_logfile.is_open())
    {
        _logfile.close();
    }
    _pos = recoverPoint.position;
    _nSkipLines = 0;
    // search for the correct file
    _logfile.open(_filePath.c_str(), std::ios::in | std::ios::binary);
    if(_logfile.is_open())
    {
        if(!recoverStamp.empty()) {
            std::string timestr;
            if(take(timestr, _logfile, _pos, SCLogStampLength)) {
                std::string stamp = time2utc(getSclogTime(timestr.c_str()));
                if(stamp == recoverStamp) { // got the recover point
                    // we should recover from the line after this point
                    // don't calculate the position here but skip one line in the fetchNext()
                    _nSkipLines = 1;
                    return true;
                } else { // rolled
                    _log(Log::L_DEBUG, CLOGFMT(SCLogChopper, "open() [%s] recover point(position:%lld, stamp:%s) already be rolled"), _filePath.c_str(), recoverPoint.position, recoverStamp.c_str());
                }
            } else { // can't get the stamp string
                _log(Log::L_DEBUG, CLOGFMT(SCLogChopper, "open() [%s] can't get stamp string at recover point(position:%lld, stamp:%s)"), _filePath.c_str(), recoverPoint.position, recoverStamp.c_str());
            }
        } else { // bad recover point
            _log(Log::L_DEBUG, CLOGFMT(SCLogChopper, "open() [%s] invalid recover point(position:%lld, stamp:%s)"), _filePath.c_str(), recoverPoint.position, recoverStamp.c_str());
        }
        _pos = beginningPosition();
        _log(Log::L_INFO, CLOGFMT(SCLogChopper, "open() [%s] Can't find the recover point(position:%lld, stamp:%s). Recover at position %d"), _filePath.c_str(), recoverPoint.position, recoverStamp.c_str(), _pos);
        return true;
    }
    else
    { // can't open
        _log(Log::L_DEBUG, CLOGFMT(SCLogChopper, "open() Can't open file %s"), _filePath.c_str());
        return false;
    }
}

void SCLogChopper::close()
{
    if(_logfile.is_open())
    {
        _logfile.close();
    }
    _linesBuf.clear();
}

int SCLogChopper::fetchNext(char* buf, int* len, MessageIdentity& mid)
{
    mid.source.clear();
    mid.position = 0;
    mid.stamp = 0;

    if(*len > 0)
    {
        *buf = '\0';
    }
    while(true) {
        if(!_linesBuf.hasLines())
        { // no cached data
            if(!_linesBuf.feed(_logfile, _pos, writingPosition()))
            { // end of log?
                _pos = writingPosition();
                *len = 0;
                return 0;
            }
        }

        const char* curLine = _linesBuf.getLine();
        if(curLine)
        {
            if(_nSkipLines > 0) { // skip this line as need
                --_nSkipLines;
                _pos += _linesBuf.getLineSize();
                _linesBuf.next();
                continue;
            }

            int64 stamp = getSclogTime(curLine);
            if(stamp <= 0) { // bad content, skip this line
                _pos += _linesBuf.getLineSize();
                _linesBuf.next();
                continue;
            }

            // copy the line
            int bufLen = *len;
            int dataLen = strlen(curLine);
            *len = dataLen + 1;
            if(bufLen >= *len) // buffer enough
            { // just copy the data
                strcpy(buf, curLine);
                mid.source = _filePath;
                mid.position = _pos;
                mid.stamp = stamp;

                _pos += _linesBuf.getLineSize();
                _linesBuf.next();
                return dataLen;
            }
            else
            { // need more buffer
                return 0;
            }
        }
        else // no data
        {
            *len = 0;
            if(_pos != writingPosition() && _pos >= endPosition())
            { // cycle to the beginning of the file
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(SCLogChopper, "[%s] fetchNext() Reach the end of file. Cycle to the beginning."), _filePath.c_str());
                _pos = beginningPosition();
            }
            return 0;
        }
    }
}

bool SCLogChopper::init(const std::string& filePath)
{
    if(filePath.empty())
    {
        return false;
    }
    _pos = 0;
    _nSkipLines = 0;
    _linesBuf.clear();
    return true;
}
int SCLogChopper::beginningPosition()
{ // find the second line
    _logfile.clear();
    _logfile.seekg(0);
    char buf[32] = {0};
    if(_logfile.getline(buf, 32))
    {
        return _logfile.tellg();
    }
    else
    {
        if(_logfile.eof())
        {
            return endPosition();
        }
        else
        {
            return 0;
        }
    }
}
int SCLogChopper::endPosition()
{
    _logfile.clear();
    _logfile.seekg(0, std::ios::end);
    return _logfile.tellg();
}

static std::string trimString(const std::string &s, const std::string &chs = " ")
{
    std::string::size_type pos_beg = std::string::npos;
    std::string::size_type pos_end = std::string::npos;
    pos_beg = s.find_first_not_of(chs);
    if(std::string::npos == pos_beg)
        return "";
    pos_end = s.find_last_not_of(chs);
    return s.substr(pos_beg, pos_end - pos_beg + 1);
}
int SCLogChopper::writingPosition()
{
    _logfile.clear();
    _logfile.seekg(0);
    char buf[32] = {0};
    _logfile.getline(buf, 32);
    return atoi(trimString(buf).c_str());
}
///////////////////////////////////////////////////////////////
/// MessageSourceFactory
MessageSourceFactory::MessageSourceFactory(ZQ::common::Log& log)
    :_log(log)
{
    /*
	if(!_posRecorder.initDataBase())
        ZQ::common::_throw<ZQ::common::Exception>(log, "Failed to init database at (%s) for writing log position info", positionFilePath.c_str());
    */
}

IRawMessageSource* MessageSourceFactory::create(const std::string& logFile, const std::string& typestr)
{
    std::pair<std::string, std::string> typeinfo = parseType(typestr);
    const std::string& type = typeinfo.first;
    const std::string& subtype = typeinfo.second;
    using namespace ZQ::common;
	if(0 == stricmp("FileLog", type.c_str()))
	{
        try
        {
            IRawMessageSource* src = new NewFileLogChopper(_log, logFile);
            return src;
        }
        catch(const ZQ::common::Exception& e)
        {
            _log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Caught [%s] during creating FileLogChopper."), e.getString());
            return NULL;
        }
	}
	else 
	if(0 == stricmp("FileLogV1", type.c_str()))
    {
        try
        {
            IRawMessageSource* src = new FileLogChopper(_log, logFile);
            return src;
        }
        catch(const ZQ::common::Exception& e)
        {
            _log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Caught [%s] during creating FileLogChopper."), e.getString());
            return NULL;
        }
    }
    else if(0 == stricmp("SCLog", type.c_str()))
    {
        try
        {
            IRawMessageSource* src = new SCLogChopper(_log, logFile);
            return src;
        }
        catch(const ZQ::common::Exception& e)
        {
            _log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Caught [%s] during creating SCLogChopper."), e.getString());
            return NULL;
        }
    }
#ifdef ZQ_OS_MSWIN
	else if(0 == stricmp(WINEVENTLOG, type.c_str()))
	{
		try
		{
			IRawMessageSource* src = new WinEventLogChopper(_log, logFile);
			return src;
		}
		catch(const ZQ::common::Exception& e)
		{
			_log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Caught [%s] during creating WinEventLogChopper with filename [%s]"),
				e.getString(), logFile.c_str());
			return NULL;
		}
	}
#else
	else if(0 == stricmp("SysLog", type.c_str()))
	{ // we need the subtype to configure the log style
		try
		{
			IRawMessageSource* src = new LinuxSysLogChopper(_log, logFile, subtype);
			return src;
		}
		catch(const ZQ::common::Exception& e)
		{
			_log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Caught [%s] during creating LinuxSysLogChopper with filename [%s]"),
					e.getString(), logFile.c_str());
			return NULL;
		}
	}
#endif
	else
		_log(Log::L_ERROR, CLOGFMT(MessageSourceFactory, "Unknown the log type[%s] of logfile[%s]"), 
			type.c_str(), logFile.c_str());
		
    return NULL;
}

void MessageSourceFactory::destroy(IRawMessageSource* src)
{
    if(src)
    {
        delete src;
    }
}

bool MessageSourceFactory::checkType(const std::string& typestr)
{
    std::string type = parseType(typestr).first;
	if(0 == stricmp("FileLog", type.c_str()))
    {
        return true;
    }else
	if(0 == stricmp("FileLogV1", type.c_str()))
    {
        return true;
    }
    else if(0 == stricmp("SCLog", type.c_str()))
    {
        return true;
    }
	else if(0 == stricmp(WINEVENTLOG, type.c_str()))
	{
		return true;
	}
	else if(0 == stricmp("SysLog", type.c_str()))
	{
		return true;
	}
    else
    {
        return false;
    }
}
std::pair<std::string, std::string> MessageSourceFactory::parseType(const std::string& typestr)
{
    std::string type, subtype;
    std::string::size_type pos = typestr.find(':');
    if(pos != std::string::npos) {
        type = typestr.substr(0, pos);
        subtype = typestr.substr(pos + 1);
    } else { // no subtype
        type = typestr;
    }
    // icase
    std::transform(type.begin(), type.end(), type.begin(), tolower);
    return std::make_pair(type, subtype);
}
#ifdef ZQ_OS_MSWIN
///////////////////////////////////////////////////////////////
/// WinEventLogChopper
///////////////////////////////////////////////////////////////
#define  BUFFER_SIZE       32*1024
#define  NUM_OF_ASTRINGS   20
#define  RETRY_WAIT        60*1000

typedef std::vector<HINSTANCE> HMSGLIBS; 

static time_t time64To32(int64 t) {
    if(t <= 0) {
        return 0;
    }
    using namespace ZQ::common;
    char buf[64] = {0};
    if(TimeUtil::TimeToUTC(t, buf, 64)) {
        time_t t32 = 0;
        if(TimeUtil::Iso2TimeEx(buf, t32)) {
            return t32;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

static int64 time32To64(time_t t) {
    using namespace ZQ::common;
    char buf[64] = {0};
    if(TimeUtil::Time2Iso(t, buf, 64)) {
        return TimeUtil::ISO8601ToTime(buf);
    } else {
        return 0;
    }
}
WinEventLogChopper::WinEventLogChopper(ZQ::common::Log& log, const std::string& filePath)
:_log(log), _filePath(filePath), _logfile(NULL), _host("")
{
    _fullPath = filePath; // save the path for message identity generating
	using namespace ZQ::common;
	_log(Log::L_DEBUG, CLOGFMT(WinEventLogChopper, "Init WinEventLogChopper with file [%s]"), filePath.c_str());
  
	_filePathKey = filePath;
	if(!init(filePath))
	{
		throw ZQ::common::Exception(std::string("Bad file name: ") + filePath);
	}
}

// open the correct log file
bool WinEventLogChopper::open(const MessageIdentity& recoverPoint)
{
    _pos = recoverPoint.position;
    _time = time64To32(recoverPoint.stamp);
	using namespace ZQ::common;

	if(_logfile)
	{
		CloseEventLog(_logfile);
		_logfile = NULL;
	}
	if(_host.size() == 0)
	{
		_logfile = OpenEventLog( NULL,               // Use the local computer.
			_filePath.c_str());
	}
	else
	{
		_logfile = OpenEventLog( _host.c_str(),               // Use the local computer.
			_filePath.c_str());
	}

	if(_logfile)
	{
		//chack the log is change, for example: delete the log;
		DWORD dwThisRecord =0, dwCount = 0;

		GetOldestEventLogRecord(_logfile, &dwThisRecord);
		GetNumberOfEventLogRecords(_logfile, &dwCount);
		if(dwCount == 0 || _pos > dwThisRecord + dwCount - 1) // no event log  or event log changed
		{
			_pos = 0;
		}
		//first read eventlog
       if(_pos == 0 && _time == 0) 
	   {
		   DWORD dwThisRecord;
		   // Get the record number of the oldest event log record. -----------
		   GetOldestEventLogRecord(_logfile, &dwThisRecord);
		   _pos = dwThisRecord;
           time((time_t*)&_time);
	   }
		return true;
	}
	else
	{ // can't open
		_log(Log::L_DEBUG, CLOGFMT(WinEventLogChopper, "open() Can't open file %s"), _filePathKey.c_str());
		return false;
	}
}

void WinEventLogChopper::close()
{
	try
	{
		if(_logfile)
		{
			CloseEventLog(_logfile);
			_logfile = NULL;
		}
	}
	catch (...)
	{
		
	}
}

int WinEventLogChopper::fetchNext(char* buf, int* len, MessageIdentity& mid)
{
    mid.source.clear();
    mid.position = 0;
    mid.stamp = 0;

	if (*len > 0)
		*buf = '\0';

	EVENTLOGRECORD *logRecord;
	BYTE bBuffer[BUFFER_SIZE];
	DWORD dwRead, dwCount;

	//event parameter
	char* lpSourceName;
	DWORD   eventID;
	std::string eventType;
	DWORD eventCategory;
	DWORD  recordNumber;
	time_t ntimeWritten;
	std::string timeWritter;

	// Initialize the event record buffer. -----------------------------
	logRecord = (EVENTLOGRECORD *) &bBuffer;

	// Retry if soft errors
	DWORD dwEventLogRetryTMO = 10*60;  
	bool bRSts;
	time_t tT0, tT1;

	for (tT0=time(&tT1); (tT1 < (time_t)(tT0+dwEventLogRetryTMO)); time(&tT1)) 
	{
		bRSts = ReadEventLog(_logfile,               // Event log handle
			EVENTLOG_FORWARDS_READ |          // Reads forward
			EVENTLOG_SEEK_READ ,              // Sequential read EVENTLOG_SEEK_READ
			_pos + 1,                  // Ignored for sequential read
			logRecord,                        // Pointer to buffer
			BUFFER_SIZE,                      // Size of buffer
			&dwRead,                          // Number of bytes read
			&dwCount);                       // Bytes in the next record
		if(bRSts)
			break;
		int status = GetLastError();
		if (status == ERROR_HANDLE_EOF) break;

		TCHAR *pszLog = NULL;

		if (ERROR_EVENTLOG_FILE_CHANGED != status) 
		{
			Sleep(RETRY_WAIT);
			continue;
		}

		// If EventLog was cleared, then close EventLog, reset state, and try again
		_pos = 0;
		time((time_t*)&_time);
		close();
		MessageIdentity mid;
		mid.source = _fullPath;
		mid.position = _pos;
		mid.stamp = time32To64(_time);
		if (!open(mid))
			return -1;
		return 0;
	}

	if (!bRSts)// no data or error
	{
		*len = 0; 
		return 0;
	}

	time_t currtime;
	time(&currtime);

//	while (dwRead > 0)
	for (; dwRead > 0; dwRead -= logRecord->Length, logRecord = (EVENTLOGRECORD *) ((LPBYTE) logRecord + logRecord->Length))
	{
		if ((((time_t)logRecord->TimeWritten > _time) || (((time_t)logRecord->TimeWritten == _time) && (logRecord->RecordNumber > _pos)))) 
		{
			// Get the event source name.
			lpSourceName   = (char*) ((LPBYTE) logRecord + sizeof(EVENTLOGRECORD)); 
			eventID        = logRecord->EventID & 0XFFFF;
			recordNumber   = logRecord->RecordNumber;
			eventCategory  = logRecord->EventCategory;
			ntimeWritten = (time_t)logRecord->TimeWritten;
			timeWritter = ctime(&ntimeWritten);

			switch(logRecord->EventType)
			{
			case EVENTLOG_ERROR_TYPE:
				eventType = "EVENTLOG_ERROR";
				break;
			case EVENTLOG_WARNING_TYPE:
				eventType = "EVENTLOG_WARNING";
				break;
			case EVENTLOG_INFORMATION_TYPE:
				eventType = "EVENTLOG_INFORMATION";
				break;
			case EVENTLOG_AUDIT_SUCCESS:
				eventType = "EVENTLOG_AUDIT_SUCCESS";
				break;
			case EVENTLOG_AUDIT_FAILURE:
				eventType = "EVENTLOG_AUDIT_FAILURE";
				break;
			default:
				std::cout << "Unknown  " << std::endl;
				eventType = "Unknown ";
				break;
			} 

			char    szMsgBuf[4096]="";
			char   *NullTerminator = "(Not Supplied)";
			char   *aStrings[NUM_OF_ASTRINGS];
			int i = 0;

			for(i=0; i<NUM_OF_ASTRINGS; i++)
			{
				aStrings[i] = NullTerminator;
			}

			char *pStr =(char*)((LPBYTE)logRecord + logRecord->StringOffset);
			for (i=0; i<logRecord->NumStrings; i++) 
			{
				aStrings[i] = pStr;
				pStr = pStr+ strlen(pStr) + 1;
			}

			if (!GetEventMessage(lpSourceName, logRecord->EventID, szMsgBuf, sizeof(szMsgBuf)/sizeof(char), i, aStrings))
				continue; // failed to read the registry key, continue to next winevent

			std::string writterTime = writtenTimeToUTC(logRecord->TimeWritten);
			char eventMsg[4096]="";
			memset(eventMsg, 0, sizeof(eventMsg));
			sprintf(eventMsg, "%s|%s|%s|%d|%s|desc=%s\0",
				_filePath.c_str(), eventType.c_str(), lpSourceName, eventID, writterTime.c_str(), szMsgBuf);
			std::string message = eventMsg;

			std::replace(message.begin(), message.end(), '\n', ' ');
			std::replace(message.begin(), message.end(), '\r', ' ');

		//	_log(ZQ::common::Log::L_INFO, CLOGFMT(WinEventLogChopper, "%s"), message.c_str());

			int bufLen = *len;
			int dataLen = message.size();
			*len = dataLen + 1;
			if (bufLen < *len)
				return 0; // need more buffer

			_pos = logRecord->RecordNumber; 
			_time = logRecord->TimeWritten;
			strcpy(buf, message.c_str());

			mid.source = _fullPath;
			mid.position = _pos;
			mid.stamp = time32To64(_time);
			return dataLen;
		}	
		else
		{
			_pos = logRecord->RecordNumber;
		}
//		dwRead -= logRecord->Length;
//		logRecord = (EVENTLOGRECORD *) ((LPBYTE) logRecord + logRecord->Length);
	}
	return 0;
}

bool WinEventLogChopper::init(const std::string& filePath)
{
	int npos = _filePath.rfind('\\');
	if(npos > 0)
	{
		_host = _filePath.substr(0, npos);
		_filePath = _filePath.substr(npos+1, _filePath.size() - npos);
	}
	else
		if(npos == 0) //indicate the first letter is '\'
			return false;
		else 
			_host = "";

	if(0 == stricmp(_filePath.c_str(), WINEVENT_APPLICATION))
	{
		_eventtype = APP_EVTLOG;
		_logfilename = WINEVENT_APP_FILENAME;
	}
	else 
		if(0 == stricmp(_filePath.c_str(), WINEVENT_SYSTEM))
		{
			_eventtype = SYS_EVTLOG;
			_logfilename = WINEVENT_SYSTEM_FILENAME;
		}
		else 
			if(0 == stricmp(_filePath.c_str(), WINEVENT_SECURIT))
			{
				_eventtype = SEC_EVTLOG;
				_logfilename = WINEVENT_SECURIT_FILENAME;
			}
			else 
				return false;

   //compare the filepath
	if(_filePath.empty())
	{
		return false;
	}

	char strHostName[MAX_PATH]="";
	memset(strHostName, 0, MAX_PATH);
	gethostname(strHostName, MAX_PATH * sizeof(char));

	return true;
}

bool 
WinEventLogChopper::GetEventMessage(char *pszSourceName, DWORD iEventId, char *pszMsgBuf, int iSizeofMsgBuf, int iNStrings, char ** aStrings)
{
	int i, iSts;
	bool bRetSts=FALSE;

#define MAX_SRC_TB_SIZE 500

	static struct SrcTb_s {
		char szSourceN[MAX_PATH];
		char szMessageFile[MAX_PATH];
		char szParmMsgFile[MAX_PATH];
		HMSGLIBS hMsgLib;
		HINSTANCE hParmMsgLib;
		int iStatus;
	} SourceTable[MAX_SRC_TB_SIZE];
	static int iCntSrcTb = 0;

	const char *EventLogPath = "System\\CurrentControlSet\\Services\\EventLog";
	char szPathBuf[MAX_PATH];
	DWORD	iSizeofValue;
	DWORD	iValueType;
	HKEY	hResultKey = NULL;

	try {

		// Map Source name to handle of MessageLibrary
		for (i=0; i<iCntSrcTb; i++)
		{
			if ((stricmp(pszSourceName, (char *)(SourceTable[i].szSourceN))) == 0)
				break;
		}

		// if table entry is found
		if (i<iCntSrcTb)
		{
			if (SourceTable[i].hMsgLib.size() < 1)
			{
				iSts = SourceTable[i].iStatus;        
				sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
				return false;
			}
		} 
		else
		{
			if (i >= MAX_SRC_TB_SIZE)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(WinEventLogChopper, "GetEventMessage\\SourceTable is full (%d)"),i);
				iSts = -1;        
				sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
				return false;
			}
			iCntSrcTb += 1;

			// Update table entry
			strcpy(SourceTable[i].szSourceN, pszSourceName);
			SourceTable[i].szMessageFile[0] = '\0';	 
			SourceTable[i].szParmMsgFile[0] = '\0';	
			SourceTable[i].hMsgLib.clear();
			SourceTable[i].hParmMsgLib = NULL;

			// Find EventLog Entry in Registry
			sprintf(szPathBuf, "%s\\%s\\%s", EventLogPath, "Application", pszSourceName);
			if ((iSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPathBuf,	0, KEY_READ, &hResultKey)) != ERROR_SUCCESS)
			{
					sprintf(szPathBuf, "%s\\%s\\%s", EventLogPath, "System", pszSourceName);
					if ((iSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPathBuf,	0, KEY_READ, &hResultKey)) != ERROR_SUCCESS)
					{
							sprintf(szPathBuf, "%s\\%s\\%s", EventLogPath, "Security", pszSourceName);
							iSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPathBuf, 0, KEY_READ, &hResultKey);
					}
			}

			if (iSts != ERROR_SUCCESS)
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(WinEventLogChopper, "EventSource key (%s\\...\\%s) is not accessible. GLE=%d"),
					EventLogPath, pszSourceName, iSts);

				SourceTable[i].iStatus = iSts;
				sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
				return false;
			}

			iSizeofValue = sizeof(SourceTable[i].szMessageFile);
			if (ERROR_SUCCESS != (iSts = RegQueryValueEx(hResultKey, "EventMessageFile",
				NULL, &iValueType, (LPBYTE)SourceTable[i].szMessageFile, &iSizeofValue)))
			{
				RegCloseKey(hResultKey);
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(WinEventLogChopper, "failed to retrieve EventMessageFile from registry for %s. GLE=%d"), 
					szPathBuf, iSts);
				SourceTable[i].iStatus = iSts;
				sprintf(pszMsgBuf, "unable to format message. GLE=(%d)", iSts);
				return false;
			}

			if (iValueType == REG_EXPAND_SZ)
			{
				char szTmp[MAX_PATH];
				ExpandEnvironmentStrings(SourceTable[i].szMessageFile, szTmp, sizeof(szTmp));
				strcpy(SourceTable[i].szMessageFile, szTmp);
			}

			iSizeofValue = sizeof(SourceTable[i].szParmMsgFile);
			if (ERROR_SUCCESS == (iSts = RegQueryValueEx(hResultKey, "ParameterMessageFile", 
				NULL, &iValueType, (LPBYTE)SourceTable[i].szParmMsgFile, &iSizeofValue))) 
			{
				if (iValueType == REG_EXPAND_SZ)
				{
					char szTmp[MAX_PATH];
					ExpandEnvironmentStrings(SourceTable[i].szParmMsgFile, szTmp, sizeof(szTmp));
					strcpy(SourceTable[i].szParmMsgFile, szTmp);
				}
			}
			RegCloseKey(hResultKey);

			TianShanIce::StrValues messagefiles;
			compartString(SourceTable[i].szMessageFile, ';', messagefiles);

			if(messagefiles.size() < 1)
			{
				iSts = GetLastError();
				iSts = SourceTable[i].iStatus;        
				sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
				return false;
			}

#ifndef LOAD_LIBRARY_AS_IMAGE_RESOURCE
#define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0
#endif//LOAD_LIBRARY_AS_IMAGE_RESOURCE
			for(int k = 0; k < messagefiles.size(); k++)
			{
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(WinEventLogChopper, "(%s)LoadLibrary (%s)"),
					SourceTable[i].szSourceN, messagefiles[k].c_str());

				//SourceTable[i].hMsgLib = LoadLibraryEx(
				//	ptr , NULL , LOAD_LIBRARY_AS_DATAFILE);	
				HINSTANCE hMsgLib = NULL;
				hMsgLib = ::LoadLibraryEx(messagefiles[k].c_str(), NULL,
					LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_IGNORE_CODE_AUTHZ_LEVEL);	

				if (hMsgLib == NULL)
				{
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(WinEventLogChopper, "failed to LoadLibrary (%s) for %s"), 
						messagefiles[k].c_str(), szPathBuf);
					iSts = GetLastError();
					SourceTable[i].iStatus = iSts;
					SourceTable[i].hMsgLib.clear();
					sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
					return false;
				}

				SourceTable[i].hMsgLib.push_back(hMsgLib);
				hMsgLib = NULL;
			}

			if (strlen(SourceTable[i].szParmMsgFile))
				SourceTable[i].hParmMsgLib = ::LoadLibraryEx(SourceTable[i].szParmMsgFile, NULL,
				LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_IGNORE_CODE_AUTHZ_LEVEL);											
		}

		// FormatMessage
		if (SourceTable[i].hParmMsgLib)
		{
			int j;
			char * aTStrings[10];
			char szTStrings[10][256];
			int iParm;

			for (j=0; j<iNStrings; j++)
			{
				if (NULL == strrchr(aStrings[j], '%'))
				{
					aTStrings[j] = aStrings[j];
					continue;
				}

				iParm = atoi(aStrings[j] + strcspn(aStrings[j], "0123456789"));
				iSts = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY | 255,
					SourceTable[i].hParmMsgLib, iParm,
					0, //MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
					&szTStrings[j][0], iSizeofMsgBuf, (va_list *)aStrings);

				if (iSts == 0)
					szTStrings[j][0] = '\0';
				aTStrings[j] = &szTStrings[j][0];
			}

			for(int k = 0; k < SourceTable[i].hMsgLib.size(); k++)
			{
				iSts = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_MAX_WIDTH_MASK | 80,
					SourceTable[i].hMsgLib[k], iEventId,
					0, //MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
					pszMsgBuf, iSizeofMsgBuf, (va_list *)aTStrings);
				
				if (iSts != 0)
					break;

				iSts = GetLastError();
				SourceTable[i].iStatus = iSts;
				if (k == SourceTable[i].hMsgLib.size() -1)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(WinEventLogChopper, "failed to format message with library[%s] for event(%d). GLE=%d"), 
						SourceTable[i].szMessageFile, iEventId & 0XFFFF, iSts);
					sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
					return false;
				}
			}
		}
		else
		{
			for(int k = 0; k < SourceTable[i].hMsgLib.size(); k++)
			{	
				iSts = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_MAX_WIDTH_MASK | 80,
					SourceTable[i].hMsgLib[k], iEventId,
					0, //MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
					pszMsgBuf, iSizeofMsgBuf, (va_list *)aStrings);

				if (iSts != 0)
					break;

				iSts = GetLastError();
				SourceTable[i].iStatus = iSts;

				if (k == SourceTable[i].hMsgLib.size() -1)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(WinEventLogChopper, "failed to format message with library[%s] for event(%d). GLE=%d"), 
						SourceTable[i].szMessageFile, iEventId & 0XFFFF, iSts);

					sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", iSts);
					return false;
				}
			}
		}

		bRetSts = TRUE;
	} 
	catch(...) 
	{
		if (!bRetSts)
			sprintf(pszMsgBuf, "Unable to format message. GLE=(%d)", GetLastError());
	}

	return (bRetSts);
}

std::string 
WinEventLogChopper::writtenTimeToUTC(DWORD writtertime)
{
	time_t  writtenTM = writtertime;
	char buf[32];
	return (ZQ::common::TimeUtil::Time2Iso(writtenTM, buf, 32) ? buf : "");
}
void 
WinEventLogChopper::compartString( const std::string& src, char delimiter, TianShanIce::StrValues& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;
			result.push_back(str);
		}
	}
	if(beginPos != src.end())
	{
		std::string str(beginPos, it);
		result.push_back(str);
	}
}

#else
/////////////////////////// linux syslog ////////////////////

LinuxSysLogChopper::LinuxSysLogChopper(ZQ::common::Log& log, const std::string& filePath, const std::string& subtype)
    :ArchivedLogChopper(log), _lineStyle(lsUnknown), _stampStyle(ssUnknown) {
    // parse the subtype.
    std::vector<std::string> parameters;
    ZQ::common::stringHelper::SplitString(subtype, parameters);
#define ELEM(el, cont) (std::find(cont.begin(), cont.end(), el) != cont.end())
    SyslogsCursor::NamingStyle ns = ELEM("*.1", parameters) ? SyslogsCursor::nsDotN : (ELEM("*1.*", parameters) ? SyslogsCursor::nsNDotExt : SyslogsCursor::nsUnknown);
    _lineStyle = ELEM("stamp+", parameters) ? lsStamp : (ELEM("host+stamp", parameters) ? lsHostStamp : lsUnknown);
    _stampStyle = ELEM("mmm", parameters) ? ssMMM : (ELEM("yyyy", parameters) ? ssYYYY : ssUnknown);

    log(ZQ::common::Log::L_INFO, CLOGFMT(FileLogChopper, "Init LinuxSysLogChopper with file [%s], subtype [%s]"), filePath.c_str(), subtype.c_str());
    if(!_cursor.init(filePath, ns)) {
        throw ZQ::common::Exception(std::string("Bad file name: ") + filePath);
    }
}
// get the archive cursor
ArchiveCursor& LinuxSysLogChopper::getArchiveCursor() {
    return _cursor;
}
static const char* seekToStampInHostStampStyle(const char* l) {
    const char* p = strchr(l, ':');
    if(p) {
        ++p; // skip the ':'
        for(; *p == ' '; ++p) {} // skip the leading SPACE
        return p;
    } else {
        return NULL;
    }
}
// get the time stamp of the log data
int64 LinuxSysLogChopper::getStampOfLine(const char* l) {
    switch(_lineStyle) {
    case lsStamp:
        return tryGetTime(l);
    case lsHostStamp:
        return tryGetTime(seekToStampInHostStampStyle(l));
    default:
        int64 stamp = tryGetTime(l);
        if(stamp != 0) {
            _lineStyle = lsStamp;
            return stamp;
        }
        stamp = tryGetTime(seekToStampInHostStampStyle(l));
        if(stamp != 0) {
            _lineStyle = lsHostStamp;
            return stamp;
        }
        return 0;
    }
}

int64 LinuxSysLogChopper::tryGetTime(const char* timestr) {
    if(NULL == timestr) {
        return 0;
    }

    switch(_stampStyle) {
    case ssMMM:
        return getSyslogTime1(timestr);
    case ssYYYY:
        return getSyslogTime2(timestr);
    default:
        int64 t = 0;
        t = getSyslogTime1(timestr);
        if(t != 0) {
            _stampStyle = ssMMM;
            return t;
        }
        t = getSyslogTime2(timestr);
        if(t != 0) {
            _stampStyle = ssYYYY;
            return t;
        }
        return 0;
    }
}
// style 1: abc, abc.1, ..., abc.n
// style 2: abc.log, abc1.log, ..., abcn.log
LinuxSysLogChopper::SyslogsCursor::SyslogsCursor()
    :instanceId_(0), namingStyle_(nsUnknown) {
}
bool LinuxSysLogChopper::SyslogsCursor::init(const std::string& name, NamingStyle ns) {
    if(name.empty()) {
        return false;
    }
    mainFileName_ = name;
    currentFileName_ = mainFileName_;
    instanceId_ = 0;
    namingStyle_ = ns;
    // parse the file path
    std::string::size_type dotPos = name.rfind(".");
    if(dotPos == std::string::npos) { // no extension in the file name
        fileNameBasePart_ = name;
        fileNameExtPart_.clear();
    } else {
        if(name.find("/", dotPos) == std::string::npos) {
            fileNameBasePart_ = name.substr(0, dotPos);
            fileNameExtPart_ = name.substr(dotPos);
        } else { // "aaa.bbb/ccc", not a valid extension
            fileNameBasePart_ = name;
            fileNameExtPart_.clear();
        }
    }

    // override the naming style
    if(fileNameExtPart_.empty()) {
        namingStyle_ = nsDotN;
    }
    // guess the naming style if need
    if(namingStyle_ == nsUnknown) {
        namingStyle_ = guessNamingStyle();
    }
    return true;
}
// get the main file name
const std::string& LinuxSysLogChopper::SyslogsCursor::getMainFile() const {
    return mainFileName_;
}
// get the current file name
const std::string& LinuxSysLogChopper::SyslogsCursor::getCurrentFile() const {
    return currentFileName_;
}
// check if the current file is the main file
bool LinuxSysLogChopper::SyslogsCursor::isMainFile() const {
    return (instanceId_ == 0);
}
// move the cursor to the main file
void LinuxSysLogChopper::SyslogsCursor::reset() {
    instanceId_ = 0;
    currentFileName_ = mainFileName_;
}
// move the cursor to the newer archive file;
void LinuxSysLogChopper::SyslogsCursor::moveNext() {
    if(instanceId_ > 0) {
        --instanceId_;
        currentFileName_ = logFileName(instanceId_);
    }
}
// move the cursor to the older archive file
void LinuxSysLogChopper::SyslogsCursor::movePrevious() {
    ++instanceId_;
    currentFileName_ = logFileName(instanceId_);
}

// construct the log file name with instance number
std::string LinuxSysLogChopper::SyslogsCursor::logFileName(int inst) {
    if(inst <= 0) {
        return mainFileName_;
    }
    if(namingStyle_ == nsUnknown) {
        namingStyle_ = guessNamingStyle();
    }

    char buf[12] = {0};
    sprintf(buf, "%d", inst);
    switch(namingStyle_) {
        case nsNDotExt:
            return (fileNameBasePart_ + buf + fileNameExtPart_);
        case nsDotN:
        default: // treat the unknown style as default style
            return (mainFileName_ + "." + buf);
    }
}
// guess the naming style base on the files in the fs
LinuxSysLogChopper::SyslogsCursor::NamingStyle LinuxSysLogChopper::SyslogsCursor::guessNamingStyle() const {
    // try the "abc.n" style
    std::string tryName = mainFileName_ + ".1";
    if(0 == access(tryName.c_str(), 0)) {
        return nsDotN;
    }
    // try the "abcn.log" style
    tryName = fileNameBasePart_ + "1" + fileNameExtPart_;
    if(0 == access(tryName.c_str(), 0)) {
        return nsNDotExt;
    }
    return nsUnknown;
}
#endif

///////////////////////////////////////////////////////////////////////////////////
/// auxiliary class LinesBuffer
LinesBuffer::LinesBuffer()
{
    _buf.resize(4096);
    clear();
}
// feed data in the range [from, to) of the stream
bool LinesBuffer::feed(std::istream& f, int from, int to)
{
    clear();
    if(to == from)
    {
        return true;
    }

    int nMaxRead = _buf.size() - 1;
    if(to > from)
    {
        nMaxRead = (to - from) < nMaxRead ? (to - from) : nMaxRead;
    }

    if(!validPosition(f, from))
    { // the postion must be wrong
        f.clear();
        return false;
    }
    f.clear();
    f.seekg(from);

    char* buf = &_buf[0];
    if(f.read(buf, nMaxRead))
    { // discard the last line data
        char* p = buf + f.gcount();
        while((--p) >= buf && *p != '\r' && *p != '\n'){}
        _lastLineEnd = p + 1;
        *_lastLineEnd = 0;
    }
    else
    {
        if(f.eof())
        { // the last line data should be keeped
            _lastLineEnd = &(_buf[0]) + f.gcount();
            *_lastLineEnd = 0;
        }
        else
        {
            return false;
        }
    }

    // replace the LF CR to null
    char* p = buf;
    while(p < _lastLineEnd)
    {
        if(*p == '\r' || *p == '\n')
            *p = '\0';
        ++p;
    }
    // init the current line
    _curLine = buf;
    _curLineEnd = _curLine;
    next();
    return true;
}
// get the current line
const char* LinesBuffer::getLine()
{
    if(hasLines())
    {
        return _curLine;
    }
    else
    {
        return NULL;
    }
}
// get the current line size
size_t LinesBuffer::getLineSize()
{
    return (_curLineEnd - _curLine);
}
// move to the next line
void LinesBuffer::next()
{
    if(hasLines())
    {
        _curLine = _curLineEnd;
        char* p = _curLine + strlen(_curLine);
        while(*p == '\0' && p < _lastLineEnd)
            ++p;
        _curLineEnd = p;
    }
}
// has data in the buffer
bool LinesBuffer::hasLines()
{
    return (_curLine < _lastLineEnd);
}

void LinesBuffer::clear()
{
    _curLine = &(_buf[0]);
    _curLineEnd = _curLine;
    _lastLineEnd = _curLine;
    *_lastLineEnd = '\0';
}
