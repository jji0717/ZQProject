// TsStatWorker.h: interface for the TsStatWorker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSSTATWORKER_H__620374A0_4AF8_4B86_8436_6E048E97F92F__INCLUDED_)
#define AFX_TSSTATWORKER_H__620374A0_4AF8_4B86_8436_6E048E97F92F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef std::map<WORD, ULONG> TsPackageMap;

/*
class TsStatPackage {
public:
	TsStatPackage(WORD pid) : _pid(pid), _count(0)
	{

	}

	virtual ~TsStatPackage()
	{

	}

	int process(ULONG index, char package[TS_PACKAGE_SIZE])
	{
		_count ++;
	}

protected:
	WORD		_pid;
	ULONG		_count;
};
*/

class TsStatWorker: public TSWorker
{
public:
	TsStatWorker(ULONG statTime);
	virtual ~TsStatWorker();

	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();

protected:
	TsPackageMap	_tsPackageMap;
	ULONG			_count;
	LARGE_INTEGER	_startTime;
	ULONG			_statTime;
};

#endif // !defined(AFX_TSSTATWORKER_H__620374A0_4AF8_4B86_8436_6E048E97F92F__INCLUDED_)
