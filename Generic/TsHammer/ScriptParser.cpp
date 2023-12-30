#include "ScriptParser.h"
#include "strHelper.h"
#include "Guid.h"
#include "Hammer.h"
#include <algorithm>

const std::string ScriptParser::VAR_START = "${";
const std::string ScriptParser::VAR_END = "}";
const std::string ScriptParser::FUNC_INC = "$INC(";
const std::string ScriptParser::FUNC_POOL_ALLOC = "$POOLALLOC(";
const std::string ScriptParser::FUNC_POOL_FIND = "$POOLFIND(";
const std::string ScriptParser::FUNC_UUID = "$UUID(";
const std::string ScriptParser::FUNC_END = ")";

ScriptParser::ScriptParser(const std::string& script) {

	init();
	parse(script.c_str());
}

ScriptParser::~ScriptParser() {
	AssetPools::iterator iter = _pools.begin();
	for(; iter != _pools.end(); ++iter) {
		delete (iter->second);
	}
	_pools.clear();
}

void ScriptParser::init() {
	logger.level = ZQ::common::Log::L_INFO;
	logger.size = 10000000;
	logger.count = 5;
	logger.flags = 0;

	ZQ::common::RuleEngine::RuleItem act;
	act.actionName = Hammer::ACT_OPEN;
	act.isASubRule = false;
	hammerRule.push_back(act);
	
	act.actionName = Hammer::ACT_NEXT;
	act.isASubRule = false;
	hammerRule.push_back(act);

	_vars["CLS"] = "\r\n";
	hammerLogic = NOT_SUPPORT;
}
	
void ScriptParser::OnStartElement(const XML_Char* name, const XML_Char** attr) {
	ZQ::common::Action::Properties attrs;

	/* keep all attributes for current node */
	std::string hierarchyName = getHiberarchyName();
	for(short i = 0; attr[i]; i+=2) {
		attrs[attr[i]] = attr[i+1];
	}

	/* Global */
	if(hierarchyName == "/HammerTest") {
		clientCount = to_uint(attrs["client"]);

		if(attrs["clientType"]=="RTSPClient") {
			clientType = RTSP;
		}
		if(attrs["clientType"]=="HTTPClient") {
			clientType = HTTP;
		}
		if ("immediately" == attrs["hammerLogic"]){
			hammerLogic = IMMEDIATELY_ADD;
		}else if ("nextLoop" == attrs["hammerLogic"]){
			hammerLogic = NEXT_LOOP_ADD;
		}else{
			hammerLogic = NEXT_LOOP_ADD;
		}

		sessionPerClient = to_uint(attrs["iterationPerClient"]);
		loopCount = to_uint(attrs["loop"]);
		sessionInterval = to_uint(attrs["interval"]);
		messageTimeout = to_uint(attrs["messageTimeout"]);
		clientIdleTimeout = to_uint(attrs["clientIdleTimeout"]);
		hammerThreads = to_uint(attrs["hammerThreads"]);
		clientThreads = to_uint(attrs["clientThreads"]);
		return;
	}

	if(hierarchyName == "/HammerTest/Log") {
		logger.level = (ZQ::common::Log::loglevel_t)to_uint(attrs["level"]);
		logger.name = attrs["filename"];
		logger.size = to_uint(attrs["size"]);
		logger.count = to_uint(attrs["count"]);

		if(attrs["hexdumpSentMsg"] == "1") {
			logger.flags |= RTSP_VERBOSEFLG_SEND_HEX;
		}
		if(attrs["hexDumpRcvdMsg"] == "1") {
			logger.flags |= RTSP_VERBOSEFLG_RECV_HEX;
		}
		if(attrs["traceTcpThreadPool"] == "1") {
			logger.flags |= RTSP_VERBOSEFLG_TCPTHREADPOOL;
		}
		if(attrs["traceUserThreadPool"] == "1") {
			logger.flags |= RTSP_VERBOSEFLG_THREADPOOL;
		}
	}

	if(hierarchyName == "/HammerTest/Server") {
		serverUrl = attrs["url"];
		return;
	}

	if(hierarchyName == "/HammerTest/SessCtx") {
		_vars[attrs["name"]] = attrs["value"];
	}

	/* Pool */
	if(hierarchyName == "/HammerTest/Pool") {
		AssetPool::AllocMethod method;
		if(attrs["sequencePrepare"] == "random") {
			method = AssetPool::RANDOM;
		}
		else {
			method = AssetPool::SEQUENTIAL;
		}
		_currentAssetPool = new AssetPool(attrs["name"], method, to_uint(attrs["spin"]));
		_pools[attrs["name"]] = _currentAssetPool;
		return;
	}

	if(hierarchyName == "/HammerTest/Pool/Item") {
		_currentAsset.name = attrs["name"];	
		return;
	}

	if(hierarchyName == "/HammerTest/Pool/Item/param") {
		_currentAsset.params[attrs["key"]] = attrs["value"];
		return;
	}

	/* Session */
	if(hierarchyName == "/HammerTest/Session/SessCtx") {
		if(attrs["name"] == "dest") {
			_vars["dest"] = attrs["value"];
		}

		SessContext ctx;
		ctx.key = attrs["name"];	
		ctx.val = attrs["value"];
		ctx.global = to_uint(attrs["global"]);
		_sess_ctxs.push_back(ctx);
		
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Sleep") {
		_currentAction.actionName = Hammer::ACT_SLEEP;
		_currentAction.isASubRule = false;
		_currentAction.inputArgs = attrs;
	}

	/* SessionRequest */
	if(hierarchyName == "/HammerTest/Session/Request") {
		_currentAction.actionName = Hammer::ACT_REQ;
		_currentAction.isASubRule = false;
		_currentAction.inputArgs = attrs;
		_vars[attrs["operation"]] = attrs["continueOnFailures"];
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Request/Header") {
		_currentAction.inputArgs[std::string("hdr:")+attrs["name"]] = attrs["value"]; 
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Request/Body") {
		std::string body = attrs["value"];
		expandVariables(body, _vars); 
		_currentAction.inputArgs["body"] = body;
		return;
	}

	/* Session/Response */
	if(hierarchyName == "/HammerTest/Session/Response") {
		_currentAction.actionName = Hammer::ACT_RESP;
		_currentAction.isASubRule = false;
		_currentAction.inputArgs = attrs;
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Response/Header") {
		_currentAction.inputArgs[attrs["name"]] = attrs["syntax"];
		_currentRespHdr.name = attrs["name"];
		_currentRespHdr.syntax = attrs["syntax"];
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Response/Header/SessCtx") {
		SessContext ctx;
		ctx.key = attrs["key"];
		ctx.val = attrs["value"];
		ctx.global = to_uint(attrs["global"]);
		_currentRespHdr.ctx = ctx;
		return;
	}
}

void ScriptParser::OnEndElement(const XML_Char* name) {
	std::string hierarchyName = getHiberarchyName();

	/* SessionRequest */
	if(hierarchyName == "/HammerTest/Session/Request" ||
	   hierarchyName == "/HammerTest/Session/Response"||
	   hierarchyName == "/HammerTest/Session/Sleep") {
		hammerRule.push_back(_currentAction);
		_currentAction.actionName = "";
		_currentAction.inputArgs.clear();
		return;
	}

	if(hierarchyName == "/HammerTest/Pool") {
		_currentAssetPool = 0;
	}

	if(hierarchyName == "/HammerTest/Pool/Item") {
		_currentAssetPool->addAsset(_currentAsset);
		_currentAsset.name = "";
		_currentAsset.params.clear();
		return;
	}

	if(hierarchyName == "/HammerTest/Session/Response/Header") {
		_respHdrs[_currentRespHdr.name] = _currentRespHdr;
		_currentRespHdr.name.clear();
		_currentRespHdr.syntax.clear();
		_currentRespHdr.ctx.key.clear();
		_currentRespHdr.ctx.val.clear();
		_currentRespHdr.ctx.global = 0;
	}

	if(hierarchyName == "/HammerTest") {
		ZQ::common::RuleEngine::RuleItem act;
		act.actionName = Hammer::ACT_CLOSE;
		act.isASubRule = false;
		hammerRule.push_back(act);
	}
}

void ScriptParser::expandVariables(std::string& src, const Variables& table) {	
	std::string::size_type var_start = 0;
	while((var_start = src.find(VAR_START, var_start)) != std::string::npos) {
		std::string::size_type var_end = src.find_first_of(VAR_END, var_start);
		if(var_end == std::string::npos) {
			return;
		}
		std::string var(src, var_start+VAR_START.length(), var_end-var_start-VAR_START.length());
		Variables::const_iterator iter_var;

		ZQ::common::MutexGuard guard(_var_lock);
		if(((iter_var = table.find(var)) == table.end()) && ((iter_var = _vars.find(var)) == _vars.end())) {
			var_start += VAR_END.length();
			continue;
		}
		else {
			size_t extra_len = VAR_START.length()+VAR_END.length();
			src.replace(var_start, var.length()+extra_len, iter_var->second);
		}
	}
}

void ScriptParser::expandFunctions(std::string& src, const Variables& table) {
	std::string::size_type curr_pos = 0;
	std::string result;
	size_t extra_len, param_len = 0;
	while(curr_pos < src.length()) {
		if((curr_pos = src.find(FUNC_INC)) != std::string::npos) {
			std::string::size_type func_end = src.find_first_of(FUNC_END, curr_pos);
			if(func_end == std::string::npos) {
				curr_pos += curr_pos+FUNC_INC.length();
				continue;
			}
			std::string param(src, curr_pos+FUNC_INC.length(), func_end-curr_pos-FUNC_INC.length());
			param_len = param.length();
			expandVariables(param, table);
			result = to_string(_increment(to_uint(param)));
			extra_len = FUNC_INC.length()+FUNC_END.length();
		}
		else if((curr_pos = src.find(FUNC_POOL_ALLOC)) != std::string::npos) {
			std::string::size_type func_end = src.find_first_of(FUNC_END, curr_pos);
			if(func_end == std::string::npos) {
				curr_pos += curr_pos+FUNC_POOL_ALLOC.length();
				continue;
			}
			std::string param(src, curr_pos+FUNC_POOL_ALLOC.length(), func_end-curr_pos-FUNC_POOL_ALLOC.length());
			param_len = param.length();
			expandVariables(param, table);
			result = _allocAsset(param);
			extra_len = FUNC_POOL_ALLOC.length()+FUNC_END.length();
		}
		else if((curr_pos = src.find(FUNC_POOL_FIND)) != std::string::npos) {
			std::string::size_type func_end = src.find_first_of(FUNC_END, curr_pos);
			if(func_end == std::string::npos) {
				curr_pos += curr_pos+FUNC_POOL_FIND.length();
				continue;
			}
			std::string param(src, curr_pos+FUNC_POOL_FIND.length(), func_end-curr_pos-FUNC_POOL_FIND.length());
			param_len = param.length();
			typedef std::vector<std::string> Params;
			param.erase(std::remove(param.begin(), param.end(), ' '), param.end());
			Params params = ZQ::common::stringHelper::split(param, ',');
			if(params.size() != 3) {
				/* invalid params */
				curr_pos += param.length();
				continue;
			}
			expandVariables(params.at(0), table);
			expandVariables(params.at(1), table);
			expandVariables(params.at(2), table);

			result = _getAssetParam(params.at(0), params.at(1), params.at(2));
			extra_len = FUNC_POOL_FIND.length()+FUNC_END.length();
		}
		else if((curr_pos = src.find(FUNC_UUID)) != std::string::npos) {
			std::string::size_type func_end = src.find_first_of(FUNC_END, curr_pos);
			if(func_end == std::string::npos) {
				curr_pos += curr_pos+FUNC_UUID.length();
				continue;
			}
			ZQ::common::Guid uuid;
			uuid.create();
			char buf[50] = {0};
			int len = uuid.toString(buf, sizeof buf);
			result.assign(buf, len);
			extra_len = FUNC_UUID.length()+FUNC_END.length();
		}
		else {
			continue;
		}

		src.replace(curr_pos, param_len+extra_len, result);

		curr_pos += result.length();
	}
}

void ScriptParser::parseSessionContext(Variables& table) {
	std::string value;
	SessionContexts::const_iterator iter = _sess_ctxs.begin();
	for(; iter != _sess_ctxs.end(); ++iter) {
		value = iter->val;
		if(hasFunction(iter->val)) {
			expandFunctions(value, table);
		}				
		expandVariables(value, table);
		ZQ::common::MutexGuard guard(_var_lock);
		Variables::iterator var = _vars.find(iter->key);
		if(var != _vars.end()) {
			_vars[iter->key] = value;
		}
		table[iter->key] = value;
	}
}

uint ScriptParser::_increment(uint value){
	return (value+1);
}

std::string ScriptParser::_allocAsset(const std::string& poolName){
	ZQ::common::MutexGuard guard(_pool_lock);
	AssetPool* pool = _pools.find(poolName)->second;	
	return pool->allocate().name;
}

std::string ScriptParser::_getAssetParam(const std::string& poolName, const std::string& assetName, const std::string& paramName){
	ZQ::common::MutexGuard guard(_pool_lock);
	AssetPools::const_iterator iter = _pools.find(poolName);
	if(iter != _pools.end()) {
		AssetPool* pool = iter->second;	
		return pool->getAssetParam(assetName, paramName);
	}
	return std::string();
};	

bool ScriptParser::hasFunction(const std::string& value) {
	std::string::size_type pos = 0;
	if ((pos = value.find(FUNC_INC)) != std::string::npos        ||
		(pos = value.find(FUNC_POOL_ALLOC)) != std::string::npos ||
		(pos = value.find(FUNC_POOL_FIND)) != std::string::npos  ||
		(pos = value.find(FUNC_UUID)) != std::string::npos) {
		return true;
	} 
	return false;
}

uint ScriptParser::to_uint(const std::string& value) {
	uint tmp;
	std::istringstream iss(value);
	iss >> tmp;
	return tmp;
}

int64 ScriptParser::to_int64(const std::string& value) {
	int64 tmp;
	std::istringstream iss(value);
	iss >> tmp;
	return tmp;
}

std::string ScriptParser::to_string(size_t value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();	
}

// vim: ts=4 sw=4 nu bg=dark
