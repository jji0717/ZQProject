#pragma once

// ------------------------------------------------------ Modified by zhenan_ji at 2006��6��23�� 11:49:08
class CArrangeBuf
{
public:
	CArrangeBuf(void);
	~CArrangeBuf(void);

	int ReNewOpenFile();
	int m_nTotalPacketNumber;
	int m_nUsedPacketNumber;
	int m_nPID;
	char m_strFilename[MAX_PATH*2];	
	HANDLE m_hFile;	

	int m_nFirstInsetPad;
	BOOL m_bFileOver;
	BOOL m_bIsFirstPadding;

};
typedef CArrangeBuf* PCARRANGEBUF;