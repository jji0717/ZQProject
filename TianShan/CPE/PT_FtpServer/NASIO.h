

#ifndef _NAS_IO_H_
#define _NAS_IO_H_

#include "BaseIO.h"

class NASIO : public BaseIOI
{
public:
	NASIO();
	virtual ~NASIO();

	virtual bool Open(const char* szFile, int nOpenFlag);
	virtual LONGLONG GetFileSize();
	virtual int Read(char* pPtr, int nReadLen);
	virtual bool Write(char* pPtr, int nWriteLen);
	virtual bool Seek(LONGLONG lOffset, int nPosFlag = FP_BEGIN);
	virtual void Close();

	virtual bool ReserveBandwidth(int nbps);
	virtual void ReleaseBandwidth();

protected:
	HANDLE _hFile;
};


#endif