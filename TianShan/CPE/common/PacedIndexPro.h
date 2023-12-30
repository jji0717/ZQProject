

#ifndef _PROCESS_PACED_INDEX_
#define _PROCESS_PACED_INDEX_

#include "BaseClass.h"

#define PROCESS_TYPE_PACEDINDEX	"PacedIndex"


struct SubfileCtx
{
	PacedIndexProc	*	context;
	ULONG				index;				// create table index
	LARGE_INTEGER		fileOffset;
	ULONGLONG			runningByteOffset;
	const void*			pacingIndexCtx;
	std::string			filename;
};

class PacedIndexProc : public BaseProcess
{	
public:
	PacedIndexProc();
	~PacedIndexProc();
	
	virtual bool Init();
	virtual bool Start();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName() {return PROCESS_TYPE_PACEDINDEX;}
	
	virtual bool Receive(MediaSample* pSample, int nInputIndex = 0);
	
	virtual LONGLONG getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

	static void pacingAppLogCbk(const char * pMsg);
	static int pacingAppWrite(const void * const pCbParam, const int len, const char buf[]);
	static int pacingAppSeek(const void * const pCbParam, const LONGLONG offset);
    static int pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset);
	static void pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2);
	static char *DecodePacedIndexError(const unsigned long err);

protected:
	std::string		_pacingType;
	SubfileCtx		_subFiles[4];
}


#endif