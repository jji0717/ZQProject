
//////////////////////////////////////////////////////////////////////////
// EventLog.h: interface for the FileLog class.
//////////////////////////////////////////////////////////////////////

#ifndef __ZQ_COMMON_EVENTLOG_H__
#define __ZQ_COMMON_EVENTLOG_H__

#include <NativeThread.h>
#include <Locks.h>

#define		ZQLOG_DEFAULT_FILENUM		1000					
#define		ZQLOG_DEFAULT_FILESIZE		1024*1024*10		//默认log文件大小，单位字节
#define		ZQLOG_DEFAULT_BUFFSIZE		8*1024				//默认缓冲区大小，单位字节
#define		ZQLOG_DEFAULT_MAXLINESIZE	2*1024				//默认每行最多字符数
#define		ZQLOG_DEFAULT_FLUSHINTERVAL 2					//单位秒，每隔多长时间将缓冲区中的数据写入文件，不管缓冲区有没有满

#define		ZQLOG_DEFAULT_LINE_RESERVERD 48					//给每行中 信息之前的时间和信息结尾的回车换行预留的空间
															//所以实际填写信息的大小应该为ZQLOG_DEFAULT_MAXLINESIZE - ZQLOG_DEFAULT_FLUSHINTERVAL = 2000

class CFileLogException : public ZQ::common::IOException
{
public:
	CFileLogException(const std::string &what_arg) throw();
	virtual ~CFileLogException() throw();
};
	
class CEventFileLog : public ZQ::common::Log , public ZQ::common::NativeThread
{
	friend class CFileLogNest;

public:
	///如果文件创建不了，会抛出一个FileLogException异常
	CEventFileLog(const char* filename, const int verbosity=L_ERROR, int logFileNum = ZQLOG_DEFAULT_FILENUM, int fileSize =ZQLOG_DEFAULT_FILESIZE, int buffersize =ZQLOG_DEFAULT_BUFFSIZE, int flushInterval =ZQLOG_DEFAULT_FLUSHINTERVAL);

	virtual ~CEventFileLog();
	void flush();
protected:
	
	//向缓冲区写入数据
	virtual void writeMessage(const char *msg, int level=-1);
	virtual void writeMessage(const wchar_t *msg, int level=-1);
	
	//强制将缓冲区数据写入log文件
	virtual void flushData();

	//与log文件相关的变量
protected:
	ZQ::common::SysLog*		m_pSysLog;					//系统事件log
	int			m_nMaxLogfileNum;			//最多log文件数目
	int			m_nCurrentFileSize;			//当前文件size
	int			m_nMaxFileSize;				//文件最大size
	char		m_FileName[MAX_PATH];		//log文件名
	CFileLogNest* _pNest;

	//与log缓冲区相关的变量
protected:
	void	RenameAndCreateFile();
	bool	IsFileExsit(const char* filename,int& retFileSize);
	char*	m_Buff;						//缓冲区指针
	int		m_nMaxBuffSize;				//缓冲区最大size
	int		m_nCurrentBuffSize;			//当前缓冲区size
	ZQ::common::Mutex	m_buffMtx;					//缓冲区保护锁
	int		m_nFlushInterval;			//定时将缓冲区中的数据写入log文件
	WORD	m_currentDay;				//当月的日期

	//与线程相关的函数
public:
	void stop();
protected:
	int run();
	void final(void);
	bool init(void);
	HANDLE _event;

};


#endif  // __ZQ_COMMON_FileLog_H__
