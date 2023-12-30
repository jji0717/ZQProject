// PsiPusher.cpp: implementation of the PsiPusher class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PsiPusher.h"
#include "DataSender.h"
#include "BufferManager.h"
#include "TsEncoder.h"
#include "DataDebug.h"

namespace DataStream {

class PsiDistItem: public DistItem {
public:
	virtual void distribute(void* param)
	{
		PsiPusher* thisPtr = (PsiPusher* )param;
		thisPtr->requestSend();
	}
};

//////////////////////////////////////////////////////////////////////////

class PsiSenderWorkItem: public ZQLIB::ZQWorkItem {
public:
	PsiSenderWorkItem(ZQLIB::ZQThreadPool& threadPool, 
		PsiPusher& pusher): ZQWorkItem(threadPool), _pusher(pusher)
	{

	}

#ifdef _DEBUG
	virtual bool init()
	{
		dataDebug.incCounter(DBG_PSISENDER_COUNTER);
		return true;
	}
#endif

	virtual int run()
	{
		if (_pusher.doSend())
			return 0;

		return -1;
	}

	virtual void final(int )
	{
#ifdef _DEBUG
		dataDebug.decCounter(DBG_PSISENDER_COUNTER);
#endif
		delete this;
	}

protected:
	PsiPusher&		_pusher;
};

//////////////////////////////////////////////////////////////////////////

PsiDest::PsiDest(PsiPusher& pusher, DataSender& sender, Transfer& tran):
	_pusher(pusher), _sender(sender), _tran(tran)
{
	_psiPacket = NULL;
	_lastProgNum = 3;
}

PsiDest::~PsiDest()
{
	if (_psiPacket) {
		assert(false);
		freePsiPacket();
	}
}

bool PsiDest::findProgByPmtPid(int pmtPid)
{
	ProgAsso::iterator it = _progAsso.begin();
	for(;it != _progAsso.end(); it ++) {
		if (it->second.pmtPid == pmtPid)
			return true;
	}

	return false;
}

bool PsiDest::reg(const ProgMapInfo& progInfo)
{
	ZQLIB::AbsAutoLock lock(*this);

	if (findProgByPmtPid(progInfo.pmtPid))
		return false;

	std::pair<ProgAsso::iterator, bool> r;

	r = _progAsso.insert(
		ProgAsso::value_type(progInfo.pmtPid, progInfo));

	if (!r.second) {
		// log warning
		return false;
	}

	return true;
}

bool PsiDest::unreg(unsigned short pmtPid)
{
	ZQLIB::AbsAutoLock lock(*this);
	
	ProgAsso::iterator it = _progAsso.find(pmtPid);
	if (it == _progAsso.end())
		return false;
	_progAsso.erase(it);
	return true;
}

void PsiDest::clear()
{
	_progAsso.clear();
}

unsigned char* PsiDest::buildPat(TsEncoder* tsEncoder, 
								 unsigned char* pos, 
								 size_t len)
{
	size_t progNum = _progAsso.size();

	const size_t bufSize = (progNum + 1) * sizeof(PatEntry);
	PatEntry* patEntries = (PatEntry* )malloc(bufSize);

	initializePatEntry(	patEntries, 0, 16);

	ProgAsso::iterator it = _progAsso.begin();
	size_t i = 1;
	for (; it != _progAsso.end(); it ++) {

		unsigned short progNum = it->second.progNum;
		if (progNum == 0)
			progNum = nextProgNum();

		initializePatEntry(	
			patEntries + i, 
			progNum, 
			it->second.pmtPid);	// pmt pid

		i ++;
	}
	
	TsPatSectionEncoder patSecEnc(0x80);
	TsMemData memData((unsigned char* )patEntries, bufSize);

	tsEncoder->encode(TS_PAT_PID, &patSecEnc, memData);
	size_t patLen;
	unsigned char* patBuf = tsEncoder->nextTable(patLen);
	assert(patLen <= len);
	memcpy(pos, patBuf, patLen);
	free(patEntries);
	return pos + patLen;
}

unsigned char* PsiDest::buildPmt(TsEncoder* tsEncoder, 
								 unsigned char* pos, 
								 size_t len)
{

	ProgAsso::iterator it = _progAsso.begin();

	unsigned char* cur = pos;

	for (; it != _progAsso.end(); it ++) {
		ProgMapInfo::PmeVec& pmes = it->second.progMapEntrys;
		ProgMapInfo::PmeVec::iterator pmeIt;
		
		size_t pmeCount = pmes.size();
		PmtEntry* pmtEntries = (PmtEntry* )malloc(
			pmeCount * sizeof(PmtEntry));

		size_t j = 0;
		for (pmeIt = pmes.begin(); pmeIt != pmes.end(); pmeIt ++) {
			initializePmtEntry(pmtEntries + j, pmeIt->streamType, 
				pmeIt->elementId, 0);
			j ++;
		}

		TsPmtSectionEncoder pmtEncoder(it->second.progNum);
		TsMemData memData((unsigned char* )pmtEntries, 
			pmeCount * sizeof(PmtEntry));
		tsEncoder->encode(it->second.pmtPid, &pmtEncoder, memData);
		size_t pmtLen;
		unsigned char* pmtBuf = tsEncoder->nextTable(pmtLen);
		memcpy(cur, pmtBuf, pmtLen);
		cur += pmtLen;
		free(pmtEntries);
	}
	
	return cur;
}

bool PsiDest::fixPacket(unsigned char* ptr, size_t len)
{
	typedef std::map<unsigned short, unsigned char> PidCountMap;
	PidCountMap pidCntMap;
	
	for(size_t i = 0; i < len / TS_PACKET_SIZE; i ++) {

		TsHeader* tsHdr = (TsHeader* )(ptr + i * TS_PACKET_SIZE);
		unsigned short pid = getTsHeaderPid(tsHdr);

		PidCountMap::iterator it = pidCntMap.find(pid);
		if (it == pidCntMap.end()) {
			std::pair<PidCountMap::iterator, bool> r;
			r = pidCntMap.insert(PidCountMap::value_type(pid, 0));
			
			if (!r.second) {
				// log error
				assert(false);
				return false;
			}

			it = r.first;;
		}

		unsigned char counter = it->second;
		it->second = (it->second + 1) % 0x10;

		setTsHeaderCounter(tsHdr, counter);
	}

	return true;
}

bool PsiDest::buildPsiPacket(TsEncoder* tsEncoder)
{
	ZQLIB::AbsAutoLock lock(*this);

	// size of PAT
	size_t patSize = TS_ADJUST_PACKET_SIZE(
		sizeof(TsHeader) + sizeof(PmtHeader) + 
		_progAsso.size() * sizeof(PatEntry));

	// size of PMT
	size_t pmtSize = 0;
	ProgAsso::iterator it = _progAsso.begin();
	for (; it != _progAsso.end(); it ++) {
		size_t pmtLen = sizeof(TsHeader) + sizeof(PmtHeader);
		pmtLen += it->second.progMapEntrys.size() * sizeof(PmtEntry);
		pmtSize += TS_ADJUST_PACKET_SIZE(pmtLen);
	}

	size_t bufSize = patSize + pmtSize;

	void* buf = malloc(bufSize);

	unsigned char* pos = (unsigned char* )buf;
	TsHeader* tsHdr = (TsHeader* )pos;

	pos = buildPat(tsEncoder, pos, patSize);
	if (pos == NULL) {

		free(buf);
		// log error;
		return false;
	}

	pos = buildPmt(tsEncoder, pos, pmtSize);
	if (pos == NULL) {

		free(buf);
		// log error
		return false;
	}

	if (!fixPacket((unsigned char* )buf, bufSize)) {

		free(buf);
		return false;
	}

	_psiPacket = new GenBufferBlock(buf, bufSize);

	return true;
}

void PsiDest::freePsiPacket()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_psiPacket) {
		void* ptr = _psiPacket->getPtr();
		free(ptr);
		delete _psiPacket;
		_psiPacket = NULL;
	} else {
		assert(false);
		// log error
	}
}

size_t PsiDest::sendPsi()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_psiPacket)
		_sender.directSendData(_tran, *_psiPacket);

	return 0;
}

//////////////////////////////////////////////////////////////////////////

PsiPusher::PsiPusher(ZQLIB::ZQThreadPool& senderThreadPool, 
					 DataSender& sender): 
					 _senderThreadPool(senderThreadPool), 
					 _sender(sender)
{
	_distItem = new PsiDistItem();
	_psiDist = new DataDistributer(33, _distItem);
	_paused = false;
	_tsEncoder = new TsEncoder();
}

PsiPusher::~PsiPusher()
{
	delete _distItem;
	delete _psiDist;
	delete _tsEncoder;
}

bool PsiPusher::buildPsiPackets()
{
	PsiDestMap::iterator it;
	PsiDest* psiDest;

	for (it = _psiDests.begin(); it != _psiDests.end(); it ++) {
		psiDest = it->second;
		if (!psiDest->buildPsiPacket(_tsEncoder))
			return false;
	}

	return true;
}

void PsiPusher::freePsiPackets()
{
	PsiDestMap::iterator it;
	PsiDest* psiDest;

	for (it = _psiDests.begin(); it != _psiDests.end(); it ++) {
		psiDest = it->second;
		psiDest->freePsiPacket();
	}
}

void PsiPusher::requestSend()
{
	PsiSenderWorkItem* item = new PsiSenderWorkItem(
		_senderThreadPool, *this);
	item->start();
}

bool PsiPusher::doSend()
{
	ZQLIB::AbsAutoLock lock(*this);

	// 由线程池执行
	PsiDestMap::iterator it;
	PsiDest* psiDest;

	for (it = _psiDests.begin(); it != _psiDests.end(); it ++) {
		psiDest = it->second;
		psiDest->sendPsi();
	}

	return true;
}

bool PsiPusher::registerProgram(const TransferAddress& addr, 
	const ProgMapInfo& progInfo)
{
	ZQLIB::AbsAutoLock lock(*this);
	
	Transfer* tran = _sender.getTransfer(addr);
	PsiDestMap::iterator it = _psiDests.find(addr);
	PsiDest* psiDest = NULL;

	if (it == _psiDests.end()) {
		psiDest = new PsiDest(*this, _sender, *tran);
		std::pair<PsiDestMap::iterator, bool> r;
		r = _psiDests.insert(PsiDestMap::value_type(addr, psiDest));
		if (!r.second) {
			delete psiDest;
			return false;
		}

	} else 
		psiDest = it->second;

	if (!psiDest->reg(progInfo)) {
		if (psiDest->getProgAssoCount() <= 0) {

			delete psiDest;
		}

		return false;
	}

	return true;
}

bool PsiPusher::unregisterProgram(const TransferAddress& addr, 
	unsigned short pmtPid)
{
	ZQLIB::AbsAutoLock lock(*this);

	PsiDestMap::iterator it = _psiDests.find(addr);
	if (it == _psiDests.end())
		return false;

	PsiDest* psiDest = it->second;
	bool r = psiDest->unreg(pmtPid);
	if (r) {
		if (psiDest->getProgAssoCount() <= 0) {
			_psiDests.erase(it);
			delete psiDest;
		}
	}

	return true;
}

bool PsiPusher::run()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_psiDist->isRunning())
		return false;

	if (this->_psiDests.size() == 0)
		return false;

	if (!_paused)
		buildPsiPackets();
	
	_psiDist->start(this);
	_paused = false;
	
	return true;
}

void PsiPusher::stop()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (!_psiDist->isRunning())
		return;

	_psiDist->stop();
	freePsiPackets();
	_paused = false;
}

void PsiPusher::pause()
{
	ZQLIB::AbsAutoLock lock(*this);

	if (_paused) {
		
		// log waring
		return;
	}

	_psiDist->stop();
	_paused = true;
}

} // namespace DataStream {
