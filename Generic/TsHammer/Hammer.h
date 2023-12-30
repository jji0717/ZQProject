#ifndef __HAMMER__
#define __HAMMER__

#include "RTSPClient.h"
#include "RuleEngine.h"
#include "HammerSession.h"
#include "ScriptParser.h"
#include "SystemUtils.h"

#define PROP(_val) "Event." #_val
#define SEQ_IDX    "SEQ_IDX"
#define SKIP_SIZE  ".ToNextAction.skipSize"
#define SESS_TIMEOUT 1800000

class ConsoleWriter : public ZQ::common::NativeThread {
public:
	ConsoleWriter(Hammer& hammer, uint clients, uint iter, uint loop, uint interval, bool print=true);
	virtual ~ConsoleWriter();

public:
	
	static const int PROGRESS_BAR_LEN = 59;

	typedef struct {
		uint sess_ok, sess_running, sess_err, sess_total;
		uint req_ok, req_err, resp;
		uint clients, iterations, loops, interval;
	} Stat;

	enum CMD {REQ, RESP, SESS};
	enum EVT {SUCCEEDED, FAILED, RUNNING, DESTROY, INVALID};
	
public:

	virtual int run();

public:

	void printHeader();

	void incLines() {
#ifdef ZQ_OS_LINUX
		++_lines;
#endif
	}

	void onEvent(EVT evt, CMD cmd);

private:
	
	void onDataChanged();
	void updateProgressBar(size_t completed);

private:
	ZQ::common::Mutex _data_lock;
	Hammer& _hammer;
	Stat _st;

	SYS::SingleObject _handle;
	bool _quit;

	bool _print;

#ifdef ZQ_OS_MSWIN
	HANDLE _console;
	COORD _cursor;
#else
	char* _cursor;
	short _lines;
#endif

	char _progressBar[PROGRESS_BAR_LEN];
	uint _progress_len;
	double _dot_len; 
};

class HammerClient;
class HammerMonitor;
class Hammer : public ZQ::common::RuleEngine {

public:

	static const char*  RULE_NAME;
	static const char*  RULE_CATEGORY;

	static const char*  EVT_RESP;
	static const char*  EVT_TIMER;

	static const char*  ACT_OPEN;
	static const char*  ACT_NEXT;
	static const char*  ACT_REQ;
	static const char*  ACT_RESP;
	static const char*  ACT_SLEEP;
	static const char*  ACT_CLOSE;

public: 

	typedef struct {
		size_t id;
		uint numLoop;
		uint clientIdx;
		HammerSession::Ptr session;
	} SessSeq;

	typedef std::vector<HammerClient*> HammerClients;
//by wei	typedef std::vector<ZQ::common::RTSPClient*> RTSPClients;
	typedef std::vector<SessSeq> Sequences;
	/* seq.id -> LastActionIndex */
	typedef	std::map<size_t, size_t> ActIdxMap;
	typedef std::map <size_t, uint> WatchNextLoopSessSeqs;

	typedef std::vector<ZQ::common::Action*> Actions;

public:

	Hammer(ScriptParser& parser, ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool, bool print); 
	virtual ~Hammer();

	void start();

private:

	bool init();

public:

	size_t numSessSeq() const {
		return _seqs.size();
	}

	ScriptParser& getParser() {
		return _parser;
	}

	ZQ::common::Log& logger() {
		return _log;
	}
	
	ZQ::common::NativeThreadPool& threadPool() {
		return _thpool;
	}

//by wei	ZQ::common::RTSPClient* getClient(uint idx) const;
	HammerClient* getClient(uint idx) const;
	
	Hammer::SessSeq& getSessSeq(size_t seqIdx) { 
		return _seqs.at(seqIdx);
	}
	
	bool skipCommand(const std::string& cmd) {
		return (_parser.getVar(cmd) == "1"); 
	}

	void OnSessionEvent(ConsoleWriter::EVT, ConsoleWriter::CMD=ConsoleWriter::SESS, size_t seqIdx=(size_t)(-1));

	void quit();

	size_t getActIdx(size_t id);
	void putActIdx(size_t id, size_t skipSize) {
		ZQ::common::MutexGuard guard(_backup_lock);
		_act_idx[id] = skipSize;
	}
	void rmActIdx(size_t id) {
		ZQ::common::MutexGuard guard(_backup_lock);
		_act_idx.erase(id);
	}
	size_t maxActIdx() const {
		return _ruleMap.find(Hammer::RULE_NAME)->second.size()-1;
	}
	size_t maxSkipSize() const {
		return maxActIdx()-2;
	}

	std::string getOpr(size_t idx);

	void watchSessSeq(size_t seqIdx, int64 time); 
	void watchNextLoopSessSeq(size_t seqIdx, uint nextLoop);
	void removeWatch(size_t seqIdx);
	void triggerEvent(const std::string& evt, ZQ::common::Action::Properties& md);

public:
	
	/* callbacks */
	void OnTimer(size_t seq);

	virtual void OnRuleExecuted(
				const std::string& ruleName, uint64 execId, 
				const ZQ::common::Action::Context& outputCtx, const std::string& userTxnId=""); 

	void countResponse(const char* cmd, const int retCode);
	int formatRespCounters(std::string& text, const char* prefixLine="");

//	ZQ::common::Mutex _hammer_lock;		//by wei
private:

	ScriptParser& _parser;
	ConsoleWriter* _writer;

	Actions _acts;
	Sequences _seqs;

//by wei	RTSPClients _clients;
	HammerClients _clients;

	ActIdxMap _act_idx;
	ZQ::common::Mutex _backup_lock;

	WatchNextLoopSessSeqs _nextLoopSessSeqs;
	ZQ::common::Mutex     _watchNextLoopLock;
	
	SYS::SingleObject _handle;

	ZQ::common::NativeThreadPool _monitorThreads;
	HammerMonitor* _monitor;

	typedef std::map <std::string, long> CounterMap;
	CounterMap _respCounterMap;
	ZQ::common::Mutex _lkResponseCounterMap;

	bool _print;
};

class OpenSessionAction : public ZQ::common::Action {
public:
	OpenSessionAction(ZQ::common::RuleEngine& engine):ZQ::common::Action(engine, Hammer::ACT_OPEN) {}
	virtual ~OpenSessionAction() {}

public:
	virtual Action& operator() (Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output);
};

class RequestAction : public ZQ::common::Action {
public:
	RequestAction(ZQ::common::RuleEngine& engine):ZQ::common::Action(engine, Hammer::ACT_REQ) {}
	virtual ~RequestAction() {}

public:
	virtual Action& operator() (Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output);
};

class ResponseAction : public ZQ::common::Action {
public:
	ResponseAction(ZQ::common::RuleEngine& engine):ZQ::common::Action(engine, Hammer::ACT_RESP) {}
	virtual ~ResponseAction() {}

public:
	virtual Action& operator() (Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output);
};

class SleepAction : public ZQ::common::Action {
public:
	SleepAction(ZQ::common::RuleEngine& engine):ZQ::common::Action(engine, Hammer::ACT_SLEEP) {}
	virtual ~SleepAction() {}

public:
	virtual Action& operator() (Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output); 
};

class CloseSessionAction : public ZQ::common::Action {
public:
	CloseSessionAction(ZQ::common::RuleEngine& engine):ZQ::common::Action(engine, Hammer::ACT_CLOSE) {}
	virtual ~CloseSessionAction() {}

public:
	virtual Action& operator() (Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output); 
};


#endif

// vim: ts=4 sw=4 nu bg=dark

