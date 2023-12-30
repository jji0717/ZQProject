// TsWriterWorker.h: interface for the TsWriterWorker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSWRITERWORKER_H__979B741F_1873_4346_96E3_8FBBABD28CBB__INCLUDED_)
#define AFX_TSWRITERWORKER_H__979B741F_1873_4346_96E3_8FBBABD28CBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TsWriterWorker : public TSWorker
{
public:
	TsWriterWorker(const char* fileName);
	virtual ~TsWriterWorker();

	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();

protected:
	FILE*		_fp;
	LPCTSTR		_fileName;
};

#endif // !defined(AFX_TSWRITERWORKER_H__979B741F_1873_4346_96E3_8FBBABD28CBB__INCLUDED_)
