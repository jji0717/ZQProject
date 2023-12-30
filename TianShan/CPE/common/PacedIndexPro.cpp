
#include "PacedIndex.h"
#include "PacedIndexProc.h"



#pragma comment(lib, "PacedIndex.lib")


#define LOG_MODULE_NAME			"PacedIndex"

using namespace ZQ::common;

#define MOLOG (*_pLog)
//#define SMOLOG (*(This->_pLog))

#define PACING_TYPE_VVX		"VVX"
#define PACING_TYPE_VV2		"VV2"


PacedIndexProc::PacedIndexProc()
{
	_nInputCount = 4;
	_nOutputCount = 4;
	_pacingType = PACING_TYPE_VVX;
}

PacedIndexProc::~PacedIndexProc()
{

}

bool PacedIndexProc::Init()
{
	DWORD pacingGroup = GetCurrentThreadId();
	
	PacedIndexSetLogCbk(2, pacingAppLogCbk);
	DWORD paceresult = 0;

	for(unsigned int i=0;i<4;i++)
	{
		paceresult = PacedIndexAdd((void*) &pacingGroup, _pacingType.c_str(), _subFiles[i].filename.c_str(), 
			pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
			(void*)&_subFiles[i], &_subFiles[i].pacingIndexCtx);

		if(paceresult)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "PacedIndexAdd() failed %s with error"), 
				_subFiles[i].filename.c_str(), DecodePacedIndexError(paceresult));
			return false;	
		}
	}


	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Initialized successful"), 
		_subFiles[i].filename.c_str(), DecodePacedIndexError(paceresult));

	return true;
}

bool PacedIndexProc::Start()
{

	return true;
}

void PacedIndexProc::Stop()
{

}

void PacedIndexProc::Close()
{

}

void PacedIndexProc::endOfStream()
{
	
}

bool PacedIndexProc::Receive(MediaSample* pSample, int nInputIndex)
{

	return true;
}

LONGLONG PacedIndexProc::getProcessBytes()
{

	return 0;
}


#define XX(a,b) {a, b}
const char* PacedIndexProc::DecodePacedIndexError(const unsigned long err)
{
	const char *errString = "unknown error";
	
	static struct
	{
		unsigned long code;
		char *str;
	} 
	errors[] = 
	{
		PACED_INDEX_ERROR_TABLE
	};
	
	for (int i = 0; i < (sizeof(errors) / sizeof(errors[0])); i++)
	{
		if (err == errors[i].code)
		{
			errString = errors[i].str;
		}
	}
	
	return errString;
}

void PacedIndexProc::pacingAppLogCbk(const char * const pMsg)
{
	char				buf[1024];
	int					len;
	unsigned long		written = 0;
	
	//
	// If a msg has arrived without a CRLF, give it one now
	//
	len = strlen(pMsg);
	
	if (len > 1024 - 2)
	{
		len = 1024 - 2;
	}
	
	memcpy(buf, pMsg, len);
	buf[len] = 0;
	
	//output	
}
