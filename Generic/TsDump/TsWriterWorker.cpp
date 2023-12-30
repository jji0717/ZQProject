// TsWriterWorker.cpp: implementation of the TsWriterWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsdump.h"
#include "TsWriterWorker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TsWriterWorker::TsWriterWorker(const char* fileName):
	_fileName(fileName)
{
	
}

TsWriterWorker::~TsWriterWorker()
{

}

bool TsWriterWorker::init()
{
	_fp = fopen(_fileName, "wb");
	if (_fp == NULL)
		return false;

	printf("Writer processing...\n");
	return true;
}

int TsWriterWorker::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	if (!fwrite(package, 1, TS_PACKAGE_SIZE, _fp))
		return 0;

	return 1;
}

void TsWriterWorker::final()
{
	fclose(_fp);
}
