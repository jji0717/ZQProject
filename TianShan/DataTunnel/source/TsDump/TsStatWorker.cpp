// TsStatWorker.cpp: implementation of the TsStatWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsdump.h"
#include "TsStatWorker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TsStatWorker::TsStatWorker(ULONG statTime): 
	_statTime(statTime), _count(0)
{

}

TsStatWorker::~TsStatWorker()
{

}

bool TsStatWorker::init()
{
	printf("Stat processing...\n");
	QueryPerformanceCounter(&_startTime);
	return true;
}

int TsStatWorker::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	TsHeader* hdr = (TsHeader* )package;
	if (hdr->sync_byte != TS_PACKAGE_LEAD)
		return PROCESS_ERROR;
	
	WORD pid = MAKEWORD(hdr->pid_low, hdr->pid_high);
	TsPackageMap::iterator itor;
	itor = _tsPackageMap.find(pid);
	if (itor == _tsPackageMap.end()) {
		_tsPackageMap.insert(TsPackageMap::value_type(pid, 1));
	} else {
		itor->second ++;
	}

	if (++_count >= _statTime) {
		return PROCESS_FINISH;
	} else
		return PROCESS_OK;
}

void TsStatWorker::final()
{
	LARGE_INTEGER finishTime;
	LARGE_INTEGER time;
	LARGE_INTEGER freq;
	QueryPerformanceCounter(&finishTime);
	time.QuadPart = finishTime.QuadPart - _startTime.QuadPart;
	QueryPerformanceFrequency(&freq);
	double t = (double )time.QuadPart / (double )freq.QuadPart;
	TsPackageMap::iterator itor;

	printf("%-10s\t%-10s\t%-10s\n", "PID", "COUNT", "RATE");
	for (itor = _tsPackageMap.begin(); 
		itor != _tsPackageMap.end(); itor ++) {
		
		double r = itor->second * 188 / t;
		printf("%-10d\t%-10d\t%d(b/s), %d(B/s)\n", 
			itor->first, itor->second, (long )(r * 8), (long )r);
	}
	
	double r = _count * 188 / t;
	printf("%-10s\t%-10d\t%d(b/s), %d(B/s)\n", 
			"Total", _count, (long )(r * 8), (long )r);
	printf("\nduration: %f (ms)\n", t * 1000);
}
