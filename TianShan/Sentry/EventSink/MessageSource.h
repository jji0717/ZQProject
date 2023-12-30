#ifndef __EventSink_MessageSource_H__
#define __EventSink_MessageSource_H__
#include "EventSink.h"
#include "Log.h"
#include "Locks.h"
#include "LogPositionI.h"
#include <fstream>
#include <string>
#include <vector>
#include <TianShanIce.h>

#define  WINEVENTLOG    "WinEventLog"
/*
class FilePositionRecorder
{
public:
    struct Record
    {
        int pos;
        int checkPos;
        std::string checkData;
		int64 chkptTime;
        void clear();
        // verify the file
        bool verify(std::istream& f) const;
    };
    static bool verify(std::istream& f);
    FilePositionRecorder(const std::string& path, ZQ::common::Log& log);
	~FilePositionRecorder();

	bool initDataBase();
	void uninitDataBase();

    // retrive data from file
    bool retrive(const std::string& fileName, Record& rec);

    // save data to file
    bool save(const std::string& fileName, const Record& rec);
private:
    ZQ::common::Log& 		_log;
	ZQ::common::Mutex 		_lockDB;
    std::string 			_path;

	Freeze::EvictorPtr 		_evictor;
	Ice::ObjectAdapterPtr 	_adapter;
	Ice::CommunicatorPtr 	_communicator;
	LogPositionFactoryPtr 	_logfactory;
};
*/
// auxiliary class
class LinesBuffer
{
public:
    LinesBuffer();
    // feed data in the range [from, to) of the stream
    bool feed(std::istream& f, int from, int to = -1);
    // get the current line
    const char* getLine();
    // get the current line size
    size_t getLineSize();
    // move to the next line
    void next();
    // has data in the buffer
    bool hasLines();
    void clear();
private:
    std::vector<char> _buf;
    char* _curLine;
    char* _curLineEnd;
    char* _lastLineEnd;
};

class ArchiveCursor {
public:
	virtual ~ArchiveCursor(){};
    // get the main file name
    virtual const std::string& getMainFile() const = 0;
    // get the current file name
    virtual const std::string& getCurrentFile() const = 0;
    // check if the current file is the main file
    virtual bool isMainFile() const = 0;
    // move the cursor to the main file
    virtual void reset() = 0;
    // move the cursor to the newer archive file;
    virtual void moveNext() = 0;
    // move the cursor to the older archive file
    virtual void movePrevious() = 0;
};
class ArchivedLogChopper: public IRawMessageSource {
public:
    ArchivedLogChopper(ZQ::common::Log& log);
    // open the correct log file
    virtual bool open(const MessageIdentity& recoverPoint);
    virtual void close();
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid);
protected:
    // get the archive cursor
    virtual ArchiveCursor& getArchiveCursor() = 0;
    // get the time stamp of the log data
    virtual int64 getStampOfLine(const char* l) = 0;
private:
    bool switchFile(const std::string& fileName);
private:
    ZQ::common::Log& _log;
    LinesBuffer _linesBuf;
    std::ifstream _logfile;
    int64 _pos; // read position
    int _nSkipLines; // lines need to be skipped in the next fetch
    std::string _curFileName;
};
class NewFileLogChopper : public IRawMessageSource	// defined by weizhaoguang
{
public:
    NewFileLogChopper(ZQ::common::Log& log, const std::string& filePath);
	// open the correct log file
    virtual bool open(const MessageIdentity& recoverPoint);
    virtual void close();
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid);
    bool switchFile(const std::string& fileName);
protected:
    // get the time stamp of the log data
    int64 getStampOfLine(const char*);
private:
	// style:abc.xxx.log	xxx由12位数字组成，年/月/日/时/分/秒/十分之一秒,
    class NewFileLogsCursor {
    public:
        NewFileLogsCursor();
        bool init(const std::string&);
		// get the module name
		const std::string getShortName() const;
		// get the main file name
        const std::string getMainFile() const;
        // get the current file name
        const std::string getCurrentFile() const;
		// check if the current file is the latest file
		bool isLatestFile();
		// check if the current file is the oldest file
		bool isOldestFile();
        // move the cursor to the main file
        void reset();
        // move the cursor to the newer archive file;
        void moveNext();
        // move the cursor to the older archive file
        void movePrevious();
		// get all the file name
		void getAllFile();
		// refresh the allFileName
		void refresh();
    private:
		std::string							dirName;		// keep directory name
		std::string							shortName;		// keep current service or module name
		std::vector<std::string>			files;			// keep all matching filename
		std::vector<std::string>::iterator	itFileName;		// itFileName point to current filename
    };
    NewFileLogsCursor	_cursor;
	ZQ::common::Log&	_log;
    LinesBuffer			_linesBuf;
    std::ifstream		_logfile;
    int64				_pos;			// read position
	int64				_time;
    int					_nSkipLines;	// lines need to be skipped in the next fetch
};

class FileLogChopper : public ArchivedLogChopper
{
public:
    FileLogChopper(ZQ::common::Log& log, const std::string& filePath);
protected:
    // get the archive cursor
    virtual ArchiveCursor& getArchiveCursor();
    // get the time stamp of the log data
    virtual int64 getStampOfLine(const char*);
private:
    // style: abc.log, abc.0.log, ..., abc.n.log
    class FileLogsCursor:public ArchiveCursor {
    public:
        FileLogsCursor();
        bool init(const std::string&);
        // get the main file name
        virtual const std::string& getMainFile() const;
        // get the current file name
        virtual const std::string& getCurrentFile() const;
        // check if the current file is the main file
        virtual bool isMainFile() const;
        // move the cursor to the main file
        virtual void reset();
        // move the cursor to the newer archive file;
        virtual void moveNext();
        // move the cursor to the older archive file
        virtual void movePrevious();
    private:
        // construct the log file name with instance number
        std::string logFileName(int inst) const;
    private:
        std::string baseFileName_;
        std::string mainFileName_;
        std::string currentFileName_;
        int instanceId_; // instance id, -1 for main file
    };
    FileLogsCursor _cursor;
};

/*
class FileLogChopper : public IRawMessageSource
{
public:
    FileLogChopper(ZQ::common::Log& log, const std::string& filePath);

    // open the correct log file
    virtual bool open(const MessageIdentity& recoverPoint);
    virtual void close();
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid);
private:
    bool init(const std::string& filePath);
    bool switchFile(const std::string& fileName);

    // construct the log file name with instance number
    std::string logFileName(int inst = -1) const;
private:
    ZQ::common::Log& _log;
    LinesBuffer _linesBuf;
    std::ifstream _logfile;
    int64 _pos; // read position
    int _inst; // instance number, -1 for main log
    int _nSkipLines; // lines need to be skipped in the next fetch
    std::string _baseFileName;
};
*/
class SCLogChopper : public IRawMessageSource
{
public:
    SCLogChopper(ZQ::common::Log& log, const std::string& filePath);

    // open the correct log file
    virtual bool open(const MessageIdentity& recoverPoint);
    virtual void close();
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid);
private:
    bool init(const std::string& filePath);

    int beginningPosition();
    int writingPosition();
    int endPosition();
private:
    ZQ::common::Log& _log;
    LinesBuffer _linesBuf;
    std::ifstream _logfile;
    std::string _filePath;
    int64 _pos;
    int _nSkipLines;
};

#ifdef ZQ_OS_MSWIN
////////////////////////////////////////////////////
///// class WinEventLogChopper    //////////////////
////////////////////////////////////////////////////

#define WINEVENT_APPLICATION  "Application"
#define WINEVENT_SYSTEM	      "System"
#define WINEVENT_SECURIT      "Security"

#define WINEVENT_APP_FILENAME       "AppEvent.evt"
#define WINEVENT_SYSTEM_FILENAME	"SysEvent.evt"
#define WINEVENT_SECURIT_FILENAME   "SecEvent.evt "

// Event log control information
typedef enum {APP_EVTLOG, SEC_EVTLOG, SYS_EVTLOG, NUMBER_OF_EVTLOGS } EventType;

class WinEventLogChopper : public IRawMessageSource
{
public:
	WinEventLogChopper(ZQ::common::Log& log, const std::string& filePath);

	// open the correct log file
    virtual bool open(const MessageIdentity& recoverPoint);
    virtual void close();
    virtual int fetchNext(char* buf, int* len, MessageIdentity& mid);
private:
	bool init(const std::string& filePath);
	bool GetEventMessage(char *pszSourceName, DWORD iEventId, char *pszMsgBuf, int iSizeofMsgBuf, int iNStrings, char ** aStrings);
	std::string writtenTimeToUTC(DWORD writtertime);
	void compartString( const std::string& src, char delimiter, TianShanIce::StrValues& result);
private:

	ZQ::common::Log& _log;

	HANDLE _logfile;

    //host/filename
	//filename must be  "Application" or "System" or "Security" 
	std::string _filePath;
	std::string _host;
    std::string _fullPath; // save the full path for message identity generating

	//extra parameter
    EventType   _eventtype;
	std::string _logfilename;
	std::string _filePathKey;

    int _pos;
    time_t _time;
};

#else
/////////////////////////// linux syslog ////////////////////
class LinuxSysLogChopper : public ArchivedLogChopper
{
public:
    enum LineStyle {
        lsUnknown,
        lsStamp, // stamp ...
        lsHostStamp // host[pid]: stamp ...
    };
    enum StampStyle {
        ssUnknown,
        ssMMM, // MMM DD HH:MM:SS 
        ssYYYY // YYYY/MM/DD_HH:MM:SS 
    };
    //format of subtype: parameter1[ parameter2 ...]
    // possible of parameters:
    //      NamingStyle: *.1 *1.*
    //      LineStyle: host+stamp stamp+
    //      StampStyle: mmm yyyy
    LinuxSysLogChopper(ZQ::common::Log& log, const std::string& filePath, const std::string& subtype);
protected:
    // get the archive cursor
    virtual ArchiveCursor& getArchiveCursor();
    // get the time stamp of the log data
    virtual int64 getStampOfLine(const char*);
private:
    int64 tryGetTime(const char* timestr);
private:
    class SyslogsCursor: public ArchiveCursor {
    public:
        enum NamingStyle {
            nsUnknown,
            nsDotN, // abc, abc.1, ..., abc.n
            nsNDotExt // abc.log, abc1.log, ..., abcn.log
        };

        SyslogsCursor();
        bool init(const std::string&, NamingStyle);
        // get the main file name
        virtual const std::string& getMainFile() const;
        // get the current file name
        virtual const std::string& getCurrentFile() const;
        // check if the current file is the main file
        virtual bool isMainFile() const;
        // move the cursor to the main file
        virtual void reset();
        // move the cursor to the newer archive file;
        virtual void moveNext();
        // move the cursor to the older archive file
        virtual void movePrevious();
    private:
        // construct the log file name with instance number
        std::string logFileName(int inst);
        // guess the naming style base on the files in the fs
        NamingStyle guessNamingStyle() const;
    private:
        std::string mainFileName_;
        std::string currentFileName_;
        int instanceId_; // instance id, -1 for main file
        NamingStyle namingStyle_;

        // used by NDotExt style
        std::string fileNameBasePart_;
        std::string fileNameExtPart_; // extension part of the file name, with leading '.'
    };
    SyslogsCursor _cursor;
    LineStyle _lineStyle;
    StampStyle _stampStyle;
};
/*
/////////////////////////// linux syslog ////////////////////
class LinuxSysLogChopper : public IRawMessageSource
{
public:
	LinuxSysLogChopper(ZQ::common::Log& log, const std::string& filePath, FilePositionRecorder& posRecorder);

	// open the correct log file
	virtual bool open();
	virtual void close();
	virtual int fetchNext(char* buf, int* len);

private:
	bool init(const std::string& filePath);

	void updateCheckPoint();
	bool switchFile(const std::string& fileName);
	std::string logFileName(int inst = -1) const;

private:
    ZQ::common::Log& _log;
    LinesBuffer _linesBuf;

    FilePositionRecorder& _posRecorder;
    FilePositionRecorder::Record _posRec;

    std::ifstream _logfile;
    int _inst; // instance number, -1 for main log
    std::string _baseFileName;

};
*/
#endif


class MessageSourceFactory
{
public:
    MessageSourceFactory(ZQ::common::Log& log);
    IRawMessageSource* create(const std::string& logFile, const std::string& type);
    void destroy(IRawMessageSource* src);
    bool checkType(const std::string& type);
    // format of type string: TYPE[:SUBTYPE]
    // return pair(type, subtype)
    static std::pair<std::string, std::string> parseType(const std::string& typestr);
private:
    ZQ::common::Log& _log;
};
#endif

