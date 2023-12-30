

#ifndef _NTFS_IO_H_
#define _NTFS_IO_H_

#include "BaseIO.h"

class NTFSIO : public BaseIOI
{
public:
	NTFSIO();
	virtual ~NTFSIO();

	static	bool Init();
	static	void Uninit();
	static	BaseIOI* Create();

	virtual bool Open(const char* szFile, int nOpenFlag);
	virtual int64 GetFileSize(const char* szFile);
	virtual int Read(char* pPtr, int nReadLen);
	virtual bool Write(char* pPtr, int nWriteLen);
	virtual bool Seek(int64 lOffset, int nPosFlag = FP_BEGIN);
	virtual void Close();

	virtual bool ReserveBandwidth(int nbps);
	virtual void ReleaseBandwidth();

	virtual int getFileStats(char* name, FSUtils::fileInfo_t* info);
#ifdef ZQ_OS_MSWIN
	virtual HANDLE FindFirstFile(char* name, WIN32_FIND_DATAA& w);
	virtual bool FindNextFile(HANDLE hHandle, WIN32_FIND_DATAA& w);
	virtual void FindClose(HANDLE hHandle);

protected:
	HANDLE _hFile;
#else
	virtual DIR* openDirectory(const char* chName, DIR* dirStream);
	virtual struct dirent* readDirectory(DIR* dirStream, struct dirent* dirNode);
	virtual bool closeDirectory(DIR* dirStream);
protected:
	int _hFile;
#endif


};


#endif
