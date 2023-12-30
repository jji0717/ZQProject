// TsDecoder.h: interface for the TsDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDECODER_H__FFC08FFB_4972_47EE_B3F6_B226911781EE__INCLUDED_)
#define AFX_TSDECODER_H__FFC08FFB_4972_47EE_B3F6_B226911781EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tsdump.h"

#define MAX_PROGRAM_NUMBER			10
#define MAX_PMT_ELEMENT				210
#define PAT_PID						0
#define PAT_TABLE_ID				0
#define PMT_TABLE_ID				2
#define MAX_CACHE_LEN				(188 * 1024)

class PackageDecoder;

struct Subtable {
	Subtable()
	{
		_section_num = 0;
	}

	short	_section_num;
};

class TablePicker {
public:
	TablePicker(PackageDecoder& pkgDecoder, WORD tid, 
		const char* fileName);
	virtual ~TablePicker();

	WORD getTableId();
	const char* getCacheFileName();

	bool init();
	int process(ULONG index, char* package, size_t len, 
		bool table_start);
	void final();
	bool isFinished();
	void reset();

	size_t getSize();
	size_t readData(void* buf, size_t pos, size_t len);
	
protected:
	size_t writeData(const void* buf, size_t len);
	bool flush();

	int find_table_id_ext(WORD ext_id)
	{
		for (int i = 0; i < _table_id_ext_count; i ++)
			if (ext_id == _table_id_exts[i])
				return i;

		return -1;

		/*
		SubtableMap::iterator it = _sub_table.find(ext_id);
		if (it == _sub_table.end())
			return NULL;
		return &(it->second);
		*/
	}

	int insert_table_id_ext(WORD ext_id)
	{
		
		if (_table_id_ext_count >= 200)
			return -1;

		_table_id_exts[_table_id_ext_count ++] = ext_id;
		return _table_id_ext_count - 1;	

		/*
		SubtableMap::_Pairib r = _sub_table.insert(
			SubtableMap::value_type(ext_id, Subtable()));
		if (!r.second)
			return false;
		
		return true;
		*/	
	}

protected:
	WORD	_table_id;
	WORD	_version_number;
	FILE*	_fp;
	char	_cacheFileName[MAX_PATH];
	size_t	_section_len;
	size_t	_section_pos;
	bool	_subtable_finished;
	bool	_section_finished;
	char	_cache[MAX_CACHE_LEN];
	size_t	_cache_ptr;
	WORD	_table_id_exts[256];
	WORD	_table_id_ext_count;
	//typedef std::map<WORD, Subtable> SubtableMap;
	//SubtableMap	_sub_table;
	WORD	_table_id_ext;
	
	PackageDecoder&	_pkgDecoder;
	short	_section_num;
	short	_last_section_num;
};

class PackageDecoder {
public:
	PackageDecoder(WORD pid, const char* cachePath);
	virtual ~PackageDecoder();
	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();
	WORD getPid();
	virtual bool flush();
	TablePicker* findTablePicker(WORD tid);

protected:
	BYTE nextContinuityCounter(BYTE counter);
	TablePicker* createTablePicker(WORD tid);	

protected:
	WORD			_pid;
	bool			_got_counter;
	BYTE			_continuity_counter;
	const char*		_cachePath;
	TablePicker*	_tablePickers[200];
	size_t			_pickerCount;
	TablePicker*	_currentTablePicker;
};

class PatDecoder: public PackageDecoder {
public:
	PatDecoder();
	virtual ~PatDecoder();

	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();
	virtual bool flush();

	bool isFinished();
	int findPmt(WORD pid);
	size_t getPmtCount();
	WORD getPmtPid(size_t index);

protected:
	bool analysePat();

protected:
	WORD		_pmtPids[MAX_PROGRAM_NUMBER];
	WORD		_progNums[MAX_PROGRAM_NUMBER];
	size_t		_pmtPidCount;
	bool		_analysed;
};

class PmtDecoder: public PackageDecoder {
public:
	PmtDecoder(WORD pid);
	virtual ~PmtDecoder();
	
	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();
	virtual bool flush();

	bool isFinished();
	size_t getElementCount();
	WORD getElementPid(size_t index);

protected:
	bool analysePmt();
protected:
	WORD		_pmtElemPids[MAX_PROGRAM_NUMBER];
	size_t		_pmtElemCount;
	bool		_analysed;
};

class DODDecoder: public PackageDecoder {
public:
	DODDecoder(WORD pid, const char* cachePath);
	virtual ~DODDecoder();

	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();
	virtual bool flush();

	bool saveObj(const char* indexFile, const char* dataFile, 
		const char* cachePath);

	bool isPmt(WORD pid);
	size_t getPmtCount();
	size_t getPmtId(int index);

protected:

};

/*
class PatPmtDecoder {
public:
	PatPmtDecoder();
	virtual ~PatPmtDecoder();

	bool init();
	int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	void final();

	bool isFinished();

	int getProgramCount();
	WORD getProgramPmtPid(int index);
	int getProgramElementCount(int pmtpid);
	int getProgramElementCount(WORD pmtpid);
	WORD getProgramElement(WORD pmtpid, int index);

	bool isPmtPid(WORD pid);

protected:
	void reset();

protected:

	struct _PatMap {
		WORD		prog_id;
		WORD		pid;
		int			pmt_map_size;
		WORD		pmt_elem_pid[MAX_PMT_ELEMENT];
	};

	_PatMap			_pat_map[MAX_PROGRAM_NUMBER];
	int				_pat_map_size;
	bool			_finished;

	PackageDecoder	_patDecoder;
	PackageDecoder*	_pmtDecoders[MAX_PMT_ELEMENT];
	size_t			_pmt_count;
};
*/

class TsDecoder: public TSWorker
{
public:
	TsDecoder(const char* cachePath);
	virtual ~TsDecoder();

	virtual bool init();
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE]);
	virtual void final();

	const char* getStorePath()
	{
		return _cachePath;
	}
	
	const char* getPatPmtDecoder();
	PackageDecoder* findPackageDecoder(WORD pid);

protected:	
	PackageDecoder* createPackageDecoder(WORD pid);

protected:
	ObjectDescriptor* _objDescList;

	enum Phase {
		RECV_PATPMT, 
		RECV_PES,
	};

	Phase	_phase;

	char 			_cachePath[MAX_PATH];
	PackageDecoder*	_packageDecoders[MAX_PMT_ELEMENT];
	int				_packageCount;
};

#endif // !defined(AFX_TSDECODER_H__FFC08FFB_4972_47EE_B3F6_B226911781EE__INCLUDED_)
