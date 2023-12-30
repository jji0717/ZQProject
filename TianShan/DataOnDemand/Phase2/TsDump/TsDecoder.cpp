// TsDecoder.cpp: implementation of the TsDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsDecoder.h"

extern bool dod_special;

//////////////////////////////////////////////////////////////////////////

TablePicker::TablePicker(PackageDecoder& pkgDecoder, WORD tid, const char* fileName) :
	_pkgDecoder(pkgDecoder), _table_id(tid)
{
	if (fileName)
		strcpy(_cacheFileName, fileName);
	else
		strcpy(_cacheFileName, "");

	_fp = NULL;
	_cache_ptr = 0;
	_version_number = 0;

	reset();
}

TablePicker::~TablePicker()
{

}

WORD TablePicker::getTableId()
{
	return _table_id;
}

const char* TablePicker::getCacheFileName()
{
	return _cacheFileName;
}

bool TablePicker::init()
{
	if (strlen(_cacheFileName) > 0) {
		_fp = fopen(_cacheFileName, "w+");
		if (!_fp)
			return false;
	}
	return true;
}

int TablePicker::process(ULONG index, char* package, size_t len, 
						 bool table_start)
{
	int result = 1;
	size_t data_len;

	if (table_start) {
		PSHeader* psHdr = (PSHeader* )package;
		if (_version_number != psHdr->version_number) {			
			reset();
			_version_number = psHdr->version_number;
		}

		WORD tab_id_ext = MAKEWORD(psHdr->table_id_extension_lo, 
				psHdr->table_id_extension_hi);

		/*
		if (_table_id == 16) {
			printf("pid %d, tab_id %d, section_num %d, table_id_ext %d", 
				_pkgDecoder.getPid(), _table_id, psHdr->section_number, 
				tab_id_ext);

			printf(", section_len %d, last_section %d, _section_num %d\n", 
				_section_len, psHdr->last_section_number, _section_num);
		}
		*/
		
		int st = find_table_id_ext(tab_id_ext);
		if (st == -1 && _subtable_finished && 
			psHdr->section_number == 0) {

			// 新的子表开始了
			insert_table_id_ext(tab_id_ext);
			_section_num = -1;
			_table_id_ext = tab_id_ext;
			_subtable_finished = false;
			
		} else if (tab_id_ext != _table_id_ext) {
			return result;
		}

		// subtable 是否已经完成, 但又接收到这个子表的数据
		if (_subtable_finished) {
			return result;
		}

		if (psHdr->section_number != _section_num) {
			if (psHdr->section_number == _section_num + 1) {

				// 新的节开始了
				_section_num ++;
				_section_pos = 0;
				_section_finished = false;
				_last_section_num = psHdr->last_section_number;

				// section_length_lowbits 以下占5个字节, 还有4个字节是crc
				_section_len = MAKEWORD(psHdr->section_length_lowbits,		
					psHdr->section_length_highbits) - 5 - 4;
				
			} else {
				return result;
			}
		}
		
		data_len = len - sizeof(PSHeader);
		if (_section_len <= _section_pos + data_len) {
			data_len = _section_len;
			_section_finished = true;

			if (_section_num == _last_section_num)
				_subtable_finished = true;
		}

		writeData(&package[sizeof(PSHeader)], data_len);
		if (_subtable_finished)
			flush();

		_section_pos += data_len;
	} else {
		if (_section_finished)
			return 1;

		if (_section_pos + len >= _section_len) {
			data_len = _section_len - _section_pos;
			_section_finished = true;

			if (_section_num == _last_section_num)
				_subtable_finished = true;

		} else
			data_len = len;
		
		writeData(package, data_len);
		if (_subtable_finished)
			flush();

		_section_pos += data_len;
	}

	return result;
}

void TablePicker::final()
{
	if (_fp) {
		fclose(_fp);
	}
}

size_t TablePicker::writeData(const void* buf, size_t len)
{
	if (_fp) {
		size_t r = fwrite(buf, 1, len, _fp);
		return r;
	}
	else {
		assert(_cache);
		if (_cache_ptr + len > MAX_CACHE_LEN) {
			assert(false);
			return -1;
		}

		memcpy(_cache + _cache_ptr, buf, len);
		_cache_ptr += len;
		return len;
	}
}

bool TablePicker::flush()
{
	if (_fp)
		fflush(_fp);

	return true;
}

bool TablePicker::isFinished()
{
	return _subtable_finished;
}

void TablePicker::reset()
{
	_subtable_finished = true;
	_section_finished = true;
	_section_pos = 0;
	_section_len = 0;
	_table_id_ext_count = 0;
	// _sub_table.clear();
	_section_num = -1;
	_table_id_ext = -1;
	if (_fp)
		fseek(_fp, 0, SEEK_SET);
}

size_t TablePicker::getSize()
{
	if (_fp) {
		return ftell(_fp);
	} else
		return _cache_ptr;
}

size_t TablePicker::readData(void* buf, size_t pos, size_t len)
{
	size_t size = getSize();
	if (pos + len > size)
		len = size - pos;
	if (len == 0)
		return 0;
	if (_fp) {
		fseek(_fp, pos, SEEK_SET);
		fread(buf, 1, len, _fp);
		fseek(_fp, 0, SEEK_END);
	} else {
		memcpy(buf, _cache + pos, len);
	}

	return len;
}

//////////////////////////////////////////////////////////////////////////

PackageDecoder::PackageDecoder(WORD pid, const char* cachePath)
{
	_pid = pid;
	_cachePath = cachePath;
	_continuity_counter = 0;
	_got_counter = false;
	_pickerCount = 0;
	_currentTablePicker = NULL;
}

PackageDecoder::~PackageDecoder()
{
	for (int i = 0; i < _pickerCount; i ++) {
		delete _tablePickers[i];	
	}
}

bool PackageDecoder::init()
{
	return true;	
}

int PackageDecoder::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	TsHeader* tsHdr = (TsHeader* )package;
	if (!_got_counter) {
		_continuity_counter = tsHdr->continuity_counter;
		_got_counter = true;
	} else {
		BYTE counter = nextContinuityCounter(_continuity_counter);
		if (counter != tsHdr->continuity_counter)
			return 1;

		_continuity_counter = counter;
	}


	int pos = 4;
	if (tsHdr->payload_unit_start_indicator) {
		
		PSHeader* psHdr = (PSHeader* )&package[5];
		if (_currentTablePicker == NULL || 
			psHdr->table_id != _currentTablePicker->getTableId()) {

			TablePicker* tablePicker = findTablePicker(psHdr->table_id);
			if (tablePicker == NULL) {
				tablePicker = createTablePicker(psHdr->table_id);
			}

			if (tablePicker == NULL)
				return 0;

			_currentTablePicker = tablePicker;
		}

		pos = 5;
	}

	if (_currentTablePicker)
		return _currentTablePicker->process(index, &package[pos], 
			TS_PACKAGE_SIZE - pos, pos == 5);

	// 没有找到表开头的包
	return 1;
}

void PackageDecoder::final()
{

}

WORD PackageDecoder::getPid()
{
	return _pid;
}

bool PackageDecoder::flush()
{
	return true;
}

BYTE PackageDecoder::nextContinuityCounter(BYTE counter)
{
	counter = (counter + 1) % 16;
	return counter;
}

TablePicker* PackageDecoder::findTablePicker(WORD tid)
{
	for (int i = 0; i < _pickerCount; i ++) {
		if (_tablePickers[i]->getTableId() == tid)
			return _tablePickers[i];
	}
		
	return NULL;
}

TablePicker* PackageDecoder::createTablePicker(WORD tid)
{
	char fileName[MAX_PATH];
	char* cacheFile = NULL;
	if (_cachePath) {
		sprintf(fileName, "%s\\pid_%d_tid_%d.tab", 
			_cachePath, _pid, tid);
		cacheFile = fileName;
	}

	TablePicker* picker = new TablePicker(*this, tid, cacheFile);
	_tablePickers[_pickerCount ++] = picker;
	if (!picker->init()) {
		delete picker;
		picker = NULL;
	}

	return picker;
}

//////////////////////////////////////////////////////////////////////////

PatDecoder::PatDecoder(): PackageDecoder(PAT_PID, NULL)
{
	_analysed = false;
	_pmtPidCount = 0;
}

PatDecoder::~PatDecoder()
{

}

bool PatDecoder::init()
{
	if (!PackageDecoder::init())
		return false;

	return true;
}

int PatDecoder::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	// printf("******* %d: PAT found! *******\n", GetTickCount());
	if (PackageDecoder::process(index, package) == 0)
		return 0;

	return 1;
}

void PatDecoder::final()
{
	PackageDecoder::final();
}

bool PatDecoder::flush()
{
	if (!PackageDecoder::flush())
		return false;

	return true;
}

bool PatDecoder::isFinished()
{
	TablePicker* picker = findTablePicker(PAT_TABLE_ID);
	if (!picker)
		return false;

	return picker->isFinished();
}

int PatDecoder::findPmt(WORD pid)
{
	TablePicker* picker = findTablePicker(PAT_TABLE_ID);
	if (!picker)
		return -2;
	if (!picker->isFinished())
		return -2;
	
	if (!_analysed) {
		_analysed = analysePat();
	}

	if (_analysed) {
		for (int i = 0; i < _pmtPidCount; i ++) {
			if (_pmtPids[i] == pid)
				return i;
		}
	}

	return -1;
}

size_t PatDecoder::getPmtCount()
{
	if (!_analysed)
		return 0;

	return _pmtPidCount;
}

WORD PatDecoder::getPmtPid(size_t index)
{
	if (!_analysed)
		return 0;
	
	if (index < _pmtPidCount)
		return _pmtPids[index];

	return 0;
}

bool PatDecoder::analysePat()
{
	TablePicker* picker = findTablePicker(PAT_TABLE_ID);
	assert(picker);
	char package[188];
	size_t map_len = picker->readData(package, 0, sizeof(package));
	int prog_count = map_len / sizeof(PatSectionProgramMap);
	PatSectionProgramMap* prog_map = (PatSectionProgramMap* )package;

	for (int i = 0; i < prog_count; i ++) {
		WORD prog_num = MAKEWORD(prog_map[i].program_number_lo, 
			prog_map[i].program_number_hi);
		WORD prog_pid = MAKEWORD(prog_map[i].program_map_PID_lo, 
			prog_map[i].program_map_PID_highbits);
		_progNums[_pmtPidCount] = prog_num;
		_pmtPids[_pmtPidCount] = prog_pid;
		_pmtPidCount ++;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

PmtDecoder::PmtDecoder(WORD pid) : PackageDecoder(pid, NULL)
{
	_pmtElemCount = 0;
	_analysed = false;
}

PmtDecoder::~PmtDecoder()
{

}

bool PmtDecoder::init()
{
	if (!PackageDecoder::init())
		return false;

	return true;
}

int PmtDecoder::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	// printf("******* %d: PMT found! *******\n", GetTickCount());

	if (PackageDecoder::process(index, package) == 0)
		return 0;

	return 1;
}

void PmtDecoder::final()
{
	PackageDecoder::final();
}

bool PmtDecoder::flush()
{
	if (!PackageDecoder::flush())
		return false;

	return true;
}

bool PmtDecoder::isFinished()
{
	TablePicker* picker = findTablePicker(PMT_TABLE_ID);
	if (picker)
		return false;

	return picker->isFinished();
}

size_t PmtDecoder::getElementCount()
{
	if (!_analysed)
		return 0;

	return _pmtElemCount;
}

WORD PmtDecoder::getElementPid(size_t index)
{
	if (!_analysed)
		return 0;

	if (index >= _pmtElemCount)
		return 0;
	
	return _pmtElemPids[index];
}

bool PmtDecoder::analysePmt()
{
	
	return false;
}

//////////////////////////////////////////////////////////////////////////

DODDecoder::DODDecoder(WORD pid, const char* cachePath):
	PackageDecoder(pid, cachePath)
{

}

DODDecoder::~DODDecoder()
{

}

bool DODDecoder::init()
{
	return true;
}

int DODDecoder::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	return 0;
}

void DODDecoder::final()
{

}

bool DODDecoder::flush()
{
	const char* indexFile;
	const char* dataFile;

	if (dod_special) {
		for (int i = 0; i < _pickerCount; i ++) {
			if (_tablePickers[i]->getTableId() == 0x80) {
				indexFile = _tablePickers[i]->getCacheFileName();
			}

			if (_tablePickers[i]->getTableId() == 0x10) {
				dataFile = _tablePickers[i]->getCacheFileName();
			}

			return saveObj(indexFile, dataFile, _cachePath) != 0;
		}
	}

	return true;
}

bool DODDecoder::saveObj(const char* indexFile, const char* dataFile, 
		const char* cachePath)
{
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

TsDecoder::TsDecoder(const char* cachePath)
{
	strcpy(_cachePath, cachePath);
	_phase = RECV_PATPMT;
	_packageCount = 0;
}

TsDecoder::~TsDecoder()
{

}

bool TsDecoder::init()
{
	return true;
}

int TsDecoder::process(ULONG index, char package[TS_PACKAGE_SIZE])
{
	TsHeader* tsHdr = (TsHeader* )package;
	WORD pid = MAKEWORD(tsHdr->pid_low, tsHdr->pid_high);

	PackageDecoder* PackageDecoder = findPackageDecoder(pid);
	if (PackageDecoder == NULL)
		PackageDecoder = createPackageDecoder(pid);
	assert(PackageDecoder);
	return PackageDecoder->process(index, package);
}

void TsDecoder::final()
{

}

PackageDecoder* TsDecoder::findPackageDecoder(WORD pid)
{
	for (int i = 0; i < _packageCount; i ++) {
		if (_packageDecoders[i]->getPid() == pid)
			return _packageDecoders[i];
	}
		
	return NULL;
}

PackageDecoder* TsDecoder::createPackageDecoder(WORD pid)
{
	PackageDecoder* pkgDecoder;
	if (pid == PAT_PID)
		pkgDecoder = new PatDecoder();
	else {
		PatDecoder* patDeco = (PatDecoder* )findPackageDecoder(PAT_PID);
		if (patDeco && patDeco->isFinished() && patDeco->findPmt(pid) >= 0) {
			pkgDecoder = new PmtDecoder(pid);
		} else
			pkgDecoder = new PackageDecoder(pid, _cachePath);
	}

	_packageDecoders[_packageCount ++] = pkgDecoder;
	return pkgDecoder;
}
