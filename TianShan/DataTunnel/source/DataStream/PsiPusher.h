// PsiPusher.h: interface for the PsiPusher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSIPUSHER_H__1F40A5FA_DE85_45A8_AB50_AFC24805F00C__INCLUDED_)
#define AFX_PSIPUSHER_H__1F40A5FA_DE85_45A8_AB50_AFC24805F00C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataDef.h"
#include "ZQThreadPool.h"
#include "DataDistributer.h"

namespace DataStream {

class DataDistributer;
class DataSender;
class NetAddress;
class PsiDistItem;
class PsiSenderWorkItem;
class BufferBlock;
class DataSender;
class Transfer;
class TsEncoder;

//////////////////////////////////////////////////////////////////////////

class PsiPusher;

class PsiDest: public ZQLIB::LightLock {

	friend class PsiPusher;
protected:
	PsiDest(PsiPusher& pusher, DataSender& sender, Transfer& tran);
	virtual ~PsiDest();

	bool buildPsiPacket(TsEncoder* tsEncoder);
	void freePsiPacket();

	bool reg(const ProgMapInfo& progInfo);
	bool unreg(unsigned short pmtPid);
	void clear();

	size_t sendPsi();

	size_t getProgAssoCount() const 
	{
		return _progAsso.size();
	}

	unsigned short nextProgNum()
	{
		return _lastProgNum ++;
	}

protected:
	unsigned char* buildPat(TsEncoder* tsEncoder, unsigned char* pos, 
		size_t len);
	unsigned char* buildPmt(TsEncoder* tsEncoder, unsigned char* pos, 
		size_t len);

	bool findProgByPmtPid(int pmtPid);
	
	bool fixPacket(unsigned char* ptr, size_t len);

protected:
	PsiPusher&		_pusher;
	DataSender&		_sender; 
	Transfer&		_tran;
	BufferBlock*	_psiPacket;
	typedef std::map<unsigned short, ProgMapInfo> ProgAsso;
	ProgAsso		_progAsso;
	unsigned short	_lastProgNum;	
};

//////////////////////////////////////////////////////////////////////////

class PsiPusher: protected ZQLIB::LightLock {
	
	friend class PsiDistItem;
	friend class PsiSenderWorkItem;
	
public:
	PsiPusher(ZQLIB::ZQThreadPool& senderThreadPool, 
		DataSender& sender);

	virtual ~PsiPusher();

	bool registerProgram(const TransferAddress& addr, 
		const ProgMapInfo& progInfo);

	bool unregisterProgram(const TransferAddress& addr, 
		unsigned short pmtPid);
	
	bool run();
	void stop();
	void pause();

protected:
	bool buildPsiPackets();
	void freePsiPackets();

	void requestSend();
	bool doSend();

protected:
	ZQLIB::ZQThreadPool&	_senderThreadPool;
	DataSender&				_sender;
	PsiDistItem*			_distItem;
	bool					_paused;
	DataDistributer*		_psiDist;
	typedef std::map<TransferAddress, PsiDest* > PsiDestMap;
	PsiDestMap				_psiDests;
	TsEncoder*				_tsEncoder;
};

} // namespace DataStream {

#endif // !defined(AFX_PSIPUSHER_H__1F40A5FA_DE85_45A8_AB50_AFC24805F00C__INCLUDED_)
