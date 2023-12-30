

#ifndef _BASE_IO_H_ 
#define _BASE_IO_H_

#include <string>
#include "utils.h"
#include "Locks.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
}
#endif
class BaseIOI
{
public:
	enum
	{
		FP_BEGIN,
		FP_CURRENT,
		FP_END
	};

	enum
	{
		DEF_IORW_SIZE = 64*1024
	};

	enum
	{
		BF_READ = 0x01,
		BF_WRITE = 0x02,
		BF_READWRITE = 0x03
	};

	enum
	{
		IO_NTFS,
		IO_VSTRM
	};
	
	static bool Init();
	static void Uninit();

	static bool setDefaultIO(int nType);

	static BaseIOI* CreateInstance();
	static BaseIOI* CreateInstance(int nType);
	

	virtual bool Open(const char* szFile, int nOpenFlag) = 0;
	virtual int64 GetFileSize(const char* szFile) = 0;
	virtual int Read(char* pPtr, int nReadLen) = 0;
	virtual bool Write(char* pPtr, int nWriteLen) = 0;
	virtual bool Seek(int64 lOffset, int nPosFlag = FP_BEGIN) = 0;
	virtual int GetRecommendedIOSize();
	virtual void Close() = 0;
	virtual void Release();

	virtual bool ReserveBandwidth(int nbps) = 0;
	virtual void ReleaseBandwidth() = 0;
	
#ifdef ZQ_OS_MSWIN
	virtual HANDLE FindFirstFile(char* name, WIN32_FIND_DATAA& w) = 0;
	virtual bool FindNextFile(HANDLE hHandle, WIN32_FIND_DATAA& w) = 0;
	virtual void FindClose(HANDLE hHandle) = 0;
#else
	virtual DIR* openDirectory(const char* chName, DIR* dirStream) = 0;
	virtual struct dirent* readDirectory(DIR* dirStream, struct dirent* dirNode) = 0;
	virtual bool closeDirectory(DIR* dirStream) = 0;
#endif
	virtual int getFileStats(char* name, FSUtils::fileInfo_t* info) = 0;

	virtual ~BaseIOI();
protected:
	BaseIOI();

	bool _bOpened;
	std::string _strFile;

private:
	typedef bool (* PFInit)();
	typedef void (* PFUninit)();
	typedef BaseIOI* (* PFCreate)();

	struct IOINFO
	{
		PFInit		pfInit;
		PFUninit	pfUninit;
		PFCreate	pfCreate;
		char		szIOName[12];
		int			nIOType;
		bool		bInited;
	};

	static struct IOINFO  _Infos[];
	static int			  _nInfo;	
	static int					_nDefaultIOType;
	static ZQ::common::Mutex	_lock;
};




#endif
