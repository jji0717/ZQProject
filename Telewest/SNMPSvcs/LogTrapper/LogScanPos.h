// LogScanPos.h: interface for the LogScanPos class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGSCANPOS_H__C59C09E8_C44F_49A3_8B0B_5928384AB7B2__INCLUDED_)
#define AFX_LOGSCANPOS_H__C59C09E8_C44F_49A3_8B0B_5928384AB7B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LogScanPos  
{
public:
	LogScanPos();
	virtual ~LogScanPos();

	bool init(const char* szFilename, int nFlashCount = 1000);

	void unInit();

	bool getSafeStoreScanPos(int& nByteOffset, std::string& strLastLine);

	void setScanPos(int nByteOffset, const char* strLine, bool bForceFlash = false);
private:
	char _szLastLine[8192];
	char _szFilename[256];
	int  _nByteOffset;
	int	 _nRecordCount;
	int  _nFlashCount;
	bool _bSafeStoreRead;		// true for success load safestore, false for not
};

#endif // !defined(AFX_LOGSCANPOS_H__C59C09E8_C44F_49A3_8B0B_5928384AB7B2__INCLUDED_)
