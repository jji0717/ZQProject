
//////////////////////////////////////////////////////////////////////////
// EventLog.h: interface for the FileLog class.
//////////////////////////////////////////////////////////////////////

#ifndef __ZQ_COMMON_EVENTLOG_H__
#define __ZQ_COMMON_EVENTLOG_H__

#include <NativeThread.h>
#include <Locks.h>

#define		ZQLOG_DEFAULT_FILENUM		1000					
#define		ZQLOG_DEFAULT_FILESIZE		1024*1024*10		//Ĭ��log�ļ���С����λ�ֽ�
#define		ZQLOG_DEFAULT_BUFFSIZE		8*1024				//Ĭ�ϻ�������С����λ�ֽ�
#define		ZQLOG_DEFAULT_MAXLINESIZE	2*1024				//Ĭ��ÿ������ַ���
#define		ZQLOG_DEFAULT_FLUSHINTERVAL 2					//��λ�룬ÿ���೤ʱ�佫�������е�����д���ļ������ܻ�������û����

#define		ZQLOG_DEFAULT_LINE_RESERVERD 48					//��ÿ���� ��Ϣ֮ǰ��ʱ�����Ϣ��β�Ļس�����Ԥ���Ŀռ�
															//����ʵ����д��Ϣ�Ĵ�СӦ��ΪZQLOG_DEFAULT_MAXLINESIZE - ZQLOG_DEFAULT_FLUSHINTERVAL = 2000

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
	///����ļ��������ˣ����׳�һ��FileLogException�쳣
	CEventFileLog(const char* filename, const int verbosity=L_ERROR, int logFileNum = ZQLOG_DEFAULT_FILENUM, int fileSize =ZQLOG_DEFAULT_FILESIZE, int buffersize =ZQLOG_DEFAULT_BUFFSIZE, int flushInterval =ZQLOG_DEFAULT_FLUSHINTERVAL);

	virtual ~CEventFileLog();
	void flush();
protected:
	
	//�򻺳���д������
	virtual void writeMessage(const char *msg, int level=-1);
	virtual void writeMessage(const wchar_t *msg, int level=-1);
	
	//ǿ�ƽ�����������д��log�ļ�
	virtual void flushData();

	//��log�ļ���صı���
protected:
	ZQ::common::SysLog*		m_pSysLog;					//ϵͳ�¼�log
	int			m_nMaxLogfileNum;			//���log�ļ���Ŀ
	int			m_nCurrentFileSize;			//��ǰ�ļ�size
	int			m_nMaxFileSize;				//�ļ����size
	char		m_FileName[MAX_PATH];		//log�ļ���
	CFileLogNest* _pNest;

	//��log��������صı���
protected:
	void	RenameAndCreateFile();
	bool	IsFileExsit(const char* filename,int& retFileSize);
	char*	m_Buff;						//������ָ��
	int		m_nMaxBuffSize;				//���������size
	int		m_nCurrentBuffSize;			//��ǰ������size
	ZQ::common::Mutex	m_buffMtx;					//������������
	int		m_nFlushInterval;			//��ʱ���������е�����д��log�ļ�
	WORD	m_currentDay;				//���µ�����

	//���߳���صĺ���
public:
	void stop();
protected:
	int run();
	void final(void);
	bool init(void);
	HANDLE _event;

};


#endif  // __ZQ_COMMON_FileLog_H__
