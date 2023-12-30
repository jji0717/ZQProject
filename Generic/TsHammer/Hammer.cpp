#include "Hammer.h"
#include "ScriptParser.h"
#include "HammerMonitor.h"
#include <boost/regex.hpp>
#include <algorithm>
#ifdef ZQ_OS_LINUX
#include <term.h>
#endif

const char* Hammer::RULE_NAME     = "HammerRule";
const char* Hammer::RULE_CATEGORY = "TsHammer";
const char* Hammer::EVT_RESP      = "ResponseEvent";
const char* Hammer::EVT_TIMER     = "TimerEvent";
const char* Hammer::ACT_OPEN      = "OpenSessionAction";
const char* Hammer::ACT_NEXT      = ".ToNextAction";
const char* Hammer::ACT_REQ       = "RequestAction";
const char* Hammer::ACT_RESP      = "ResponseAction";
const char* Hammer::ACT_SLEEP     = "SleepAction";
const char* Hammer::ACT_CLOSE     = "CloseSessionAction";

Hammer::Hammer(ScriptParser& parser, ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool, bool print)
:RuleEngine(log, pool, 1), _parser(parser), _writer(0), _monitorThreads(30), _monitor(0), _print(print)
{

	ZQ::common::RuleEngine::Rule::iterator iter = _parser.hammerRule.begin();
	for(; iter != _parser.hammerRule.end(); ++iter) {
		if(iter->actionName == Hammer::ACT_OPEN) {
			_acts.push_back(new OpenSessionAction(*this));
		}
		else if(iter->actionName == Hammer::ACT_REQ) {
			_acts.push_back(new RequestAction(*this));
		}
		else if(iter->actionName == Hammer::ACT_RESP) {
			_acts.push_back(new ResponseAction(*this));
		}
		else if(iter->actionName == Hammer::ACT_SLEEP) {
			_acts.push_back(new SleepAction(*this));
		}
		else if(iter->actionName == Hammer::ACT_CLOSE) {
			_acts.push_back(new CloseSessionAction(*this));
		}
		else {
		}
	}
	registerRule(Hammer::RULE_NAME, _parser.hammerRule);

	std::vector<std::string> names(1, Hammer::RULE_NAME);
	applyRulesToEvent(Hammer::RULE_CATEGORY, Hammer::EVT_RESP, names);
	applyRulesToEvent(Hammer::RULE_CATEGORY, Hammer::EVT_TIMER,names);

	_thpool.resize(_parser.hammerThreads);
	ZQ::common::TCPSocket::resizeThreadPool(_parser.clientThreads);
	_seqs.resize(_parser.clientCount*_parser.sessionPerClient);
}

Hammer::~Hammer() {
	_monitorThreads.stop();
	_thpool.stop();
	
	if(_monitor) {
		delete _monitor;
	}

//by wei	RTSPClients::iterator client = _clients.begin();
	HammerClients::iterator client = _clients.begin();
	for(; client != _clients.end(); ++client) {
		delete (*client);
		*client = 0;
	}

//	ZQ::common::RTSPSession::stopWatchdog();

	Actions::iterator action = _acts.begin();
	for(; action != _acts.end(); ++action) {
		delete (*action);
		*action = 0;
	}

	if(_writer) {
		delete _writer;
		_writer = 0;
	}
}

void Hammer::countResponse(const char* cmd, const int retCode)
{
	if (NULL == cmd || 0x00 == cmd[0] || retCode<0)
		return;

	char counterName[32];
	snprintf(counterName, sizeof(counterName)-2, "RESP%d-%s", retCode, cmd);

	ZQ::common::MutexGuard g(_lkResponseCounterMap);
	CounterMap::iterator it = _respCounterMap.find(counterName);
	if (_respCounterMap.end() != it) {
		++(it->second);
	}
	else  {
		_respCounterMap.insert(CounterMap::value_type(counterName, 1));
	}
}

int Hammer::formatRespCounters(std::string& text, const char* prefixLine)
{
	std::string lastRet;
	int lines_ = 0;
	char buf[100];
	ZQ::common::MutexGuard g(_lkResponseCounterMap);
	for (CounterMap::iterator it = _respCounterMap.begin(); it!=_respCounterMap.end(); it++)
	{
		size_t pos = it->first.find('-');
		if (std::string::npos == pos)
			continue;

		std::string Ret = it->first.substr(0, pos);
		if (0 != Ret.substr(0, 4).compare("RESP"))
			continue;
		Ret = Ret.substr(4);

		if (0 != Ret.compare(lastRet))
		{
			if (!lastRet.empty()) {
				text +='\n';
				_writer->incLines();
			}
			snprintf(buf, sizeof(buf) -2, "%s%s: ", prefixLine ? prefixLine :"", Ret.c_str());
			text +=  buf;
			++lines_;
			lastRet = Ret;
		}

		snprintf(buf, sizeof(buf) -2, "%s[%ld] ", it->first.substr(pos+1).c_str(), it->second);
		text +=buf;
	}

	return lines_;
}

void Hammer::start() {
	_monitor = new HammerMonitor(_log, _monitorThreads, *this);
	_monitor->start();

	_writer = new ConsoleWriter(*this, _parser.clientCount, _parser.sessionPerClient, _parser.loopCount, _print);
	_writer->start();

	_log(ZQ::common::Log::L_INFO, CLOGFMT(Hammer, 
			"========== Clients[%d] Iterations[%d] Loops[%d] =========="),
			_parser.clientCount, _parser.sessionPerClient, _parser.loopCount);	

	ZQ::common::InetHostAddress addr("127.0.0.1");
	for(uint i = 0; i < _parser.clientCount; ++i) {
		HammerClient* client = new HammerClient(*this,
//by wei		ZQ::common::RTSPClient* client = new ZQ::common::RTSPClient(
					_log, _thpool, addr, _parser.serverUrl, 0, ZQ::common::Log::L_DEBUG);
		
		client->setClientTimeout(_parser.messageTimeout*2, _parser.messageTimeout);
//		client->setTimeout(_parser.clientIdleTimeout);
		
		_clients.push_back(client);
	}
//by wei	ZQ::common::RTSPClient::setVerboseFlags(_parser.logger.flags);
	HammerClient::setVerboseFlags(_parser.logger.flags);

//	ZQ::common::RTSPSession::startWatchdog();

	size_t totalSeq = _parser.sessionPerClient * _parser.clientCount;
	int64 stamp = ZQ::common::now();
	for(size_t seqIdx = 0; seqIdx < totalSeq; ++seqIdx)
	{
		_seqs[seqIdx].id = seqIdx;		
		_seqs[seqIdx].session = 0;
		_seqs[seqIdx].numLoop = 1;
		_seqs[seqIdx].clientIdx = seqIdx % _parser.clientCount;

		_monitor->watch(seqIdx, stamp);
		stamp += _parser.sessionInterval;
	}

wait:

	SYS::SingleObject::STATE st = _handle.wait();
	if(st == SYS::SingleObject::SIGNALED) {
		return;
	}
	goto wait;
}

void Hammer::OnSessionEvent(ConsoleWriter::EVT evt, ConsoleWriter::CMD cmd, size_t seqIdx) {

	if(cmd == ConsoleWriter::SESS && (evt == ConsoleWriter::SUCCEEDED || evt == ConsoleWriter::FAILED)) {
		SessSeq& seq = getSessSeq(seqIdx);

		seq.session->destroy();
		seq.session = 0;	
		rmActIdx(seq.id);
		_monitor->rmWatch(seqIdx);
	
		if((seq.numLoop+1) > _parser.loopCount) 
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "[%d-%d-%d] finished"), seqIdx, seq.clientIdx, seq.numLoop);	
		else if (ScriptParser::IMMEDIATELY_ADD == _parser.hammerLogic)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "[%d-%d-%d] start next session loop immediately"), seqIdx, seq.clientIdx, seq.numLoop);	
			seq.numLoop += 1;
			seq.id += numSessSeq();		
			watchSessSeq(seqIdx, 0);
		}
		else if (ScriptParser::NEXT_LOOP_ADD == _parser.hammerLogic)
		{	
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "[%d-%d-%d] start next session loop after current processing loop finished"), seqIdx, seq.clientIdx, seq.numLoop);	
			seq.numLoop += 1;
			seq.id += numSessSeq();		
			watchNextLoopSessSeq(seqIdx, seq.numLoop);
		}
		else
		{
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "[%d-%d-%d] hammer logic failed"), seqIdx, seq.clientIdx, seq.numLoop);	
		}
	}

	_writer->onEvent(evt, cmd);
}

void Hammer::quit() {
	_handle.signal();
}

void Hammer::watchSessSeq(size_t seqIdx, int64 time) {
	_monitor->watch(seqIdx, time);
}

//not merge with watchSessSeq because using only session finished
void Hammer::watchNextLoopSessSeq(size_t seqIdx, uint nextLoop)
{
	if (ScriptParser::NEXT_LOOP_ADD != _parser.hammerLogic)
		return;

	ZQ::common::MutexGuard gd(_watchNextLoopLock);
	_nextLoopSessSeqs[seqIdx] = nextLoop;

	size_t totalSeq = _parser.sessionPerClient * _parser.clientCount;
	if (totalSeq <= _nextLoopSessSeqs.size())
	{
		WatchNextLoopSessSeqs::const_iterator iter = _nextLoopSessSeqs.begin();
		for(; iter != _nextLoopSessSeqs.end(); ++iter) 
			_monitor->watch(iter->first, 0);

		_nextLoopSessSeqs.erase(_nextLoopSessSeqs.begin(), _nextLoopSessSeqs.end());
	}
}

void Hammer::removeWatch(size_t seqIdx) {
	_monitor->rmWatch(seqIdx);
}

void Hammer::triggerEvent(const std::string& evt, ZQ::common::Action::Properties& md) {
	char buf[128];
	ZQ::common::TimeUtil::TimeToUTC(ZQ::common::TimeUtil::now(), buf, 128, true);
	OnEvent(Hammer::RULE_CATEGORY, evt, buf, "localhost", md);
}

size_t Hammer::getActIdx(size_t idx) {
	ZQ::common::MutexGuard guard(_backup_lock);
	ActIdxMap::const_iterator iter = _act_idx.find(idx);
	if(iter != _act_idx.end()) {
		return iter->second;
	}
	return maxActIdx();
}

std::string Hammer::getOpr(size_t idx) {
	if(idx > maxActIdx()) {
		return "";
	}
	ZQ::common::RuleEngine::RuleItem item = _ruleMap.find(Hammer::RULE_NAME)->second.at(idx);
	ZQ::common::Action::Properties::iterator it = item.inputArgs.find("operation");
	if(it != item.inputArgs.end()) {
		return it->second;
	}
	return "";
}

//by wei	ZQ::common::RTSPClient* Hammer::getClient(uint idx) const {
	HammerClient* Hammer::getClient(uint idx) const {
	if(idx < _clients.size()) {
		return _clients.at(idx);
	}
	return 0;
}

void Hammer::OnTimer(size_t seqIdx) {
	SessSeq& seq = getSessSeq(seqIdx);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] OnTimer"), 
			seqIdx, seq.clientIdx, seq.numLoop);		

	ZQ::common::Action::Context ctx; 
	ctx.metaData[SEQ_IDX] = ScriptParser::to_string(seqIdx);

	triggerEvent(Hammer::EVT_TIMER, ctx.metaData);
}

void Hammer::OnRuleExecuted(
	const std::string& ruleName, uint64 execId, 
	const ZQ::common::Action::Context& outputCtx, const std::string& userTxnId) {

	const std::string data = outputCtx.metaData.find(PROP(SEQ_IDX))->second;
	uint seqIdx = ScriptParser::to_uint(data);
	if(seqIdx < 0 || seqIdx >= numSessSeq()) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, "rule: invalid seq idx [%d]"), seqIdx);	
		return;
	}

	Hammer::SessSeq& seq = getSessSeq(seqIdx);

	/* check LastActionIdx */
	ZQ::common::Action::Properties::const_iterator iter = outputCtx.metaData.find(SYS_PROP(LastActionIdx));
	if(iter != outputCtx.metaData.end()) {
		/* already reached the last action, no need to restore 
		   max index: actions+.ToNextAction-1 == actions.size */
		uint act_idx = ScriptParser::to_uint(iter->second); 
		if(act_idx < maxActIdx()) {
			putActIdx(seq.id, act_idx);
		}
		else {
			rmActIdx(seq.id);
		}
	}
}

ZQ::common::Action& OpenSessionAction::operator()(ZQ::common::Action::Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output) {
	
	Hammer* engine = dynamic_cast<Hammer*>(&_engine);

	uint seqIdx = ScriptParser::to_uint(ctx.metaData[PROP(SEQ_IDX)]);
	if(seqIdx < 0 || seqIdx >= engine->numSessSeq()) {
		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, "open: invalid seq idx [%d]"), seqIdx);	
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}
	
	Hammer::SessSeq& seq = engine->getSessSeq(seqIdx);
	
	engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] open"), 
			seqIdx, seq.clientIdx, seq.numLoop);		

	if(!seq.session) {
		std::ostringstream url;
		url << "udp://@" << engine->getParser().getVar("dest") << ":" << engine->getParser().getVar("destPort");

		std::ostringstream id;
		id << seqIdx << "-" << seq.clientIdx << "-" << seq.numLoop;
		
		HammerSession::SessionCtx vars;
		engine->getParser().parseSessionContext(vars);

		std::string filePath;
		HammerSession::SessionCtx::iterator iter = vars.find("filePath");
		if(iter != vars.end()) {
			filePath = iter->second;	
		}
		seq.session = new HammerSession(*engine, seqIdx, 
			engine->logger(), engine->threadPool(), url.str().c_str(), 
			filePath.empty()?0:filePath.c_str(), SESS_TIMEOUT, id.str().c_str(), vars);
		
		if(ctx.metaData.find(SKIP_SIZE) != ctx.metaData.end()) {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
					"[%d-%d-%d] new session skip [%s]"), 
					seqIdx, seq.clientIdx, seq.numLoop, ctx.metaData[SKIP_SIZE].c_str());	
		}
	} 
	else {

		size_t act_idx = engine->getActIdx(seq.id);
		if((act_idx) != engine->maxActIdx()) {
			ctx.metaData[SKIP_SIZE] = ScriptParser::to_string(act_idx-1);
		
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
					"[%d-%d-%d] skip [%d]"), 
					seqIdx, seq.clientIdx, seq.numLoop, act_idx);	
		}
	}

	ctx.statusCode = aSucceed;
	return *this;
}

ZQ::common::Action& RequestAction::operator() (ZQ::common::Action::Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output) {

	Hammer* engine = dynamic_cast<Hammer*>(&_engine);

	uint seqIdx = ScriptParser::to_uint(ctx.metaData[PROP(SEQ_IDX)]);
	if(seqIdx < 0 || seqIdx >= engine->numSessSeq()) {
		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, "request: invalid seq idx [%d]"), seqIdx);	
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}

	Hammer::SessSeq& seq = engine->getSessSeq(seqIdx);

	engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] request"), 
			seqIdx, seq.clientIdx, seq.numLoop);	

	HammerSession::SessionCtx& vars = seq.session->ctx();
	engine->getParser().setVar("ClientId", ScriptParser::to_string(seq.clientIdx));
	
	ZQ::common::RTSPMessage::AttrMap headers;
	ZQ::common::Action::Properties::const_iterator iter = input.begin();
	for(; iter != input.end(); ++iter) {
		std::string val = iter->second;
		if(iter->first.substr(0,4) == "hdr:") {
			if(engine->getParser().hasFunction(val)) {
				engine->getParser().expandFunctions(val, vars);
			}
			engine->getParser().expandVariables(val, vars);
			headers[iter->first.substr(4)] = val;
		} 
	}

//by wei	ZQ::common::RTSPClient* client = engine->getClient(seq.clientIdx);
	HammerClient* client = engine->getClient(seq.clientIdx);

	if(!client) {
		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] invalid client idx"), 
				seqIdx, seq.clientIdx, seq.numLoop);
		
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}

	const std::string opr = input.find("operation")->second;
	
	ctx.statusCode = aSucceedQuit;
	int cSeq=0;
	if(opr == "DESCRIBE") {
		cSeq = client->sendDESCRIBE(0, headers);

		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] describe failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);

			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] DESCRIBE(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);
		}
	}
	else if(opr == "OPTIONS") {
		cSeq = client->sendOPTIONS(0, headers);

		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] options failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);

			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] OPTIONS(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);	
		}
	}
	else if(opr == "GET_PARAMETER") {
		std::string body = input.find("body")->second;
		engine->getParser().expandVariables(body, vars);
				
		ZQ::common::RTSPRequest::AttrList params;
		params.push_back(body);
		cSeq = client->sendGET_PARAMETER(*seq.session,params, 0, headers);
		

		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] get_parameter failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);
				
			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] GET_PARAMETER(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);	

			ctx.statusCode = aSucceed;
			engine->OnSessionEvent(ConsoleWriter::SUCCEEDED, ConsoleWriter::REQ);
			return *this;
		}
	}
	else if(opr == "SETUP") {
		std::string body = input.find("body")->second;
		engine->getParser().expandFunctions(body, vars);
		engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "SSSSSSSSSSSSSSSSSSSSSSSSSSSSS[%d]"), GetCurrentThreadId());
		cSeq = client->sendSETUP(*seq.session, body.c_str(), 0, headers);

		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] setup failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);
				
			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->OnSessionEvent(ConsoleWriter::RUNNING, ConsoleWriter::SESS, seqIdx);

			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] SETUP(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);	
		}
	}
	else if(opr == "PLAY") {
		float scale = 0.0;
		ZQ::common::RTSPMessage::AttrMap::iterator iter = headers.find("Scale");
		if(iter != headers.end()) {
			scale = atof(iter->second.c_str());
		}
		cSeq = client->sendPLAY(*seq.session, 0.0, 0.0, scale, 0, headers);
		
		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] play failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);
				
			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] PLAY(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);	
		}

	}
	else if(opr == "TEARDOWN") {
		cSeq = client->sendTEARDOWN(*seq.session, 0, headers);
		
		if(cSeq <= 0 || cSeq > MAX_CLIENT_CSEQ) {
			engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] teardown failed [%d]"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);
				
			ctx.statusCode = aFailedQuit;
		}
		else {
			engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
				"[%d-%d-%d] TEARDOWN(%d) sent"), 
				seqIdx, seq.clientIdx, seq.numLoop, cSeq);	
		}
	}

	if(ctx.statusCode == aSucceedQuit) {
		engine->OnSessionEvent(ConsoleWriter::SUCCEEDED, ConsoleWriter::REQ);
	}
	engine->watchSessSeq(seqIdx, ZQ::common::now()+engine->getParser().messageTimeout*3);

	return *this;
}

ZQ::common::Action& ResponseAction::operator() (ZQ::common::Action::Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output) {
	Hammer* engine = dynamic_cast<Hammer*>(&_engine);

	uint seqIdx = ScriptParser::to_uint(ctx.metaData[PROP(SEQ_IDX)]);
	if(seqIdx < 0 || seqIdx >= engine->numSessSeq()) {
		engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, "response: invalid session idx [%d]"), seqIdx);	
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}

	Hammer::SessSeq& seq = engine->getSessSeq(seqIdx);

	if (ctx.metaData[PROP(name)] != Hammer::EVT_RESP) {
		engine->OnSessionEvent(ConsoleWriter::FAILED, ConsoleWriter::RESP);

		size_t idx = engine->getActIdx(seq.id);
		std::string opr = engine->getOpr(idx);

		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, 
				"[%d-%d-%d] failed to receive response [%s], event [%s]"), 
				seqIdx, seq.clientIdx, seq.numLoop, opr.c_str(), ctx.metaData[PROP(name)].c_str());
		
		if(!opr.empty()) {
			if(engine->skipCommand(opr)) {
				ctx.statusCode = aFailed;
			}
			else {
				engine->OnSessionEvent(ConsoleWriter::FAILED, ConsoleWriter::SESS, seqIdx);
				ctx.statusCode = aFailedQuit;
			}
		}
		else {
			ctx.statusCode = aFailedQuit;	
		}
		return *this;
	}

	engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] response"), 
			seqIdx, seq.clientIdx, seq.numLoop);

	if(input.empty()) {
		engine->logger()(ZQ::common::Log::L_INFO, CLOGFMT(Hammer, 
			"[%d-%d-%d] no response data"),
			seqIdx, seq.clientIdx, seq.numLoop);	

		ctx.statusCode = aSucceed;
		return *this;
	}

	/* locate all the headers to be matched */
	ZQ::common::Action::Properties::const_iterator hdr_to_match = input.begin();
	for(; hdr_to_match != input.end(); ++hdr_to_match) {
		/* to see if the header to be matched exists in header received */
		std::string hdr = std::string("Event.") + hdr_to_match->first; 
		ZQ::common::Action::Properties::const_iterator hdr_recv = ctx.metaData.find(hdr);
		/*  header found in response */
		if(hdr_recv != ctx.metaData.end()) {
			/* get the related session context related */
			const ScriptParser::RespHdr* resp = engine->getParser().getRespHdr(hdr_to_match->first);
			if(resp) {
				/* the regex expr to be matched */
				boost::regex expr(hdr_to_match->second);
				/* actual value of header in response received */
				std::string hdr_val = hdr_recv->second;

				boost::cmatch result;
				if(boost::regex_match(hdr_val.c_str(), result, expr)) {
					/* found a match */
					ScriptParser::SessContext ctx;
					ctx.key = resp->ctx.key;
					ctx.val = std::string(result[0].first, result[0].second);
					ctx.global = resp->ctx.global;
					engine->getParser().setVar(ctx.key, ctx.val);
				}
			}
			else {
				/* unlikely to happen */
				continue;
			}
		}
		else {
			continue;
		}
	}

	ctx.statusCode = aSucceed;
	return *this;
}

ZQ::common::Action& SleepAction::operator() (ZQ::common::Action::Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output) {

	Hammer* engine = dynamic_cast<Hammer*>(&_engine);

	uint seqIdx = ScriptParser::to_uint(ctx.metaData[PROP(SEQ_IDX)]);
	if(seqIdx < 0 || seqIdx >= engine->numSessSeq()) {
		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, "sleep: invalid seq idx [%d]"), seqIdx);
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}

	Hammer::SessSeq& seq = engine->getSessSeq(seqIdx);

	ZQ::common::Action::Properties::const_iterator iter = input.find("wait");
	if(iter == input.end()) {
		ctx.statusCode = aFailed;
		return *this;
	}
	int64 time = ScriptParser::to_int64(iter->second);

	engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] sleep ["FMT64"]ms"), 
			 seqIdx, seq.clientIdx, seq.numLoop, time);	

	engine->watchSessSeq(seqIdx, ZQ::common::now()+time);

	ctx.statusCode = aSucceedQuit; 
	return *this;
}

ZQ::common::Action& CloseSessionAction::operator()(ZQ::common::Action::Context& ctx, const ZQ::common::Action::Properties& input, ZQ::common::Action::Properties& output) {
	
	Hammer* engine = dynamic_cast<Hammer*>(&_engine);

	uint seqIdx = ScriptParser::to_uint(ctx.metaData[PROP(SEQ_IDX)]);
	if(seqIdx < 0 || seqIdx >= engine->numSessSeq()) {
		engine->logger()(ZQ::common::Log::L_ERROR, CLOGFMT(Hammer, "close: invalid seq idx [%d]"), seqIdx);	
		engine->OnSessionEvent(ConsoleWriter::INVALID);
		ctx.statusCode = aFailedQuit;
		return *this;
	}
	
	Hammer::SessSeq& seq = engine->getSessSeq(seqIdx);

	engine->logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"[%d-%d-%d] close"), 
			seqIdx, seq.clientIdx, seq.numLoop);	
	

	engine->OnSessionEvent(ConsoleWriter::SUCCEEDED, ConsoleWriter::SESS, seqIdx);

	ctx.statusCode = aSucceedQuit;
	return *this;
}

ConsoleWriter::ConsoleWriter(Hammer& hammer, uint clients, uint iterations, uint loops, uint interval, bool print)
:_hammer(hammer), _quit(false), _print(print)
#ifdef ZQ_OS_LINUX
,_lines(5)
#endif
{
	_st.req_ok = 0;
	_st.req_err = 0;
	_st.resp = 0;
	_st.sess_err = 0;
	_st.sess_ok = 0;
	_st.sess_running = 0;

	_st.clients = clients;
	_st.iterations = iterations;
	_st.loops = loops;
	_st.interval = interval;

	_st.sess_total = (clients*iterations*loops);

#ifdef ZQ_OS_MSWIN
	_console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_console, &info);
	_cursor.X = 0;
	_cursor.Y = info.dwCursorPosition.Y+1;
#else
	setupterm(0, fileno(stdout), (int*)0);
	_cursor = tigetstr("cuu");
#endif
	
	memset(_progressBar, '\0', PROGRESS_BAR_LEN);
	_progress_len = PROGRESS_BAR_LEN-2-1;
	_dot_len = (double)_st.sess_total/(double)_progress_len;

	printHeader();
}

ConsoleWriter::~ConsoleWriter() {
	_quit = true;
	_handle.signal();
}

void ConsoleWriter::onEvent(EVT evt, CMD cmd) {
	ZQ::common::MutexGuard guard(_data_lock);

	if(evt == SUCCEEDED) {
		if(cmd == REQ) ++_st.req_ok;
		else if(cmd == RESP) ++_st.resp;
		else if(cmd == SESS) { ++_st.sess_ok; --_st.sess_running; }
		else {}
	}
	else if(evt == FAILED) {
		if(cmd == REQ || cmd == RESP) ++_st.req_err;
		else if(cmd == SESS) { ++_st.sess_err; --_st.sess_running; }
		else {}
	}
	else if(evt == RUNNING && cmd == SESS) {
		++_st.sess_running;
	}
	else {
		++_st.sess_err;
	}
}

void ConsoleWriter::printHeader() {
	if(_print) {
//by wei		fprintf(stdout, "TsHammerTest: [%d]client [%d]iteration @[%dms] [%d]loops\n", 
		fprintf(stdout, "TsHammerTest: %dclient %diteration @%dms x%dloops\n",
			_st.clients, _st.iterations, _st.interval, _st.loops);
	}
}

int ConsoleWriter::run() 
{
	onDataChanged();
	while(!_quit)
	{
		SYS::SingleObject::STATE st = _handle.wait(2000);
		if(st == SYS::SingleObject::SIGNALED)
		{
			_quit = true;
			break;
		}

		if(st == SYS::SingleObject::TIMEDOUT)
			onDataChanged();
	}
	onDataChanged();
	if(_print) {
#ifdef ZQ_OS_LINUX
		putp(tparm(tigetstr("cud"), _lines));
#endif
		fprintf(stdout, "TsHammerTest completed\n");
	}

	_hammer.logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(ConsoleWriter, "ConsoleWriter thread quit"));
	return (0);
}

void ConsoleWriter::onDataChanged() {
	Stat st;
	{
		ZQ::common::MutexGuard guard(_data_lock);
		st = _st;
	}
	
	size_t sess_finished = st.sess_ok + st.sess_err;

	if(_print)
	{
#ifdef ZQ_OS_MSWIN
		static int heightY=0;
		COORD crntCursor;
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(_console, &info);
		crntCursor.X = 0;
		crntCursor.Y = info.dwCursorPosition.Y;

		if (heightY >0)
		{
			crntCursor.Y = info.dwCursorPosition.Y - heightY;
			SetConsoleCursorPosition(_console, crntCursor);
		}
#endif

		updateProgressBar(sess_finished);
		fprintf(stdout, "    .Sessions:  Total[%5d]   Finished[%5lu] Executing[%5d]\n", 
			st.sess_total, sess_finished, st.sess_running);

		fprintf(stdout, "    .Requests: Issued[%5d]  ReqFailed[%5d] RespRecvd[%5d]\n", 
			st.req_ok, st.req_err, st.resp);

		std::string responseCounterTxt;
		_hammer.formatRespCounters(responseCounterTxt, "       ");

		fprintf(stdout, "   .Responses:\n%s\n", responseCounterTxt.c_str());
		fflush(stdout);

#ifdef ZQ_OS_MSWIN
		GetConsoleScreenBufferInfo(_console, &info);
		heightY = info.dwCursorPosition.Y - crntCursor.Y;
#else
		putp(tparm(_cursor, _lines));
#endif
	}
	
	if(sess_finished >= st.sess_total) {
		_hammer.logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"========== Total[%d] Finished[%d] Executing[%d] =========="),
			st.sess_total, sess_finished, st.sess_running);	

		_hammer.logger()(ZQ::common::Log::L_DEBUG, CLOGFMT(Hammer, 
			"========== Issued[%d] ReqFailed[%d] RespRecvd[%d] =========="),
			st.req_ok, st.req_err, st.resp);

		_quit = true;
		_handle.signal();

		_hammer.quit();
	}
}

void ConsoleWriter::updateProgressBar(size_t completed) {
	unsigned completed_len = static_cast<unsigned>(floor((double)completed/(double)_dot_len));

	char* p = _progressBar;
	*p++ = '[';
	
	uint i = 0;
	for(; i < completed_len; ++i)
		*p++ = '=';

	if (i < _progress_len-1 && '='==*(p-1))
		*(p-1) = '>';

	for(i=0; i < _progress_len-completed_len; ++i) {
		*p++ = ' ';
	}
	*p = ']';

	unsigned percent = static_cast<unsigned>(floor((double)completed/(double)_st.sess_total*100.0));
#ifdef ZQ_OS_MSWIN
	percent = std::_cpp_min<unsigned>(percent, 100);
#else
	percent = std::min<unsigned>(percent, 100);
#endif
	fprintf(stdout, "%3d%% %s\n", percent, _progressBar);
}

// vim: ts=4 sw=4 bg=dark nu

