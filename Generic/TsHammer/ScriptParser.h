#ifndef __SCRIPT_PARSER__
#define __SCRIPT_PARSER__

#include <sstream>
#include "HammerSession.h"

#include "expatxx.h"
#include "AssetPool.h"
#include "RuleEngine.h"

class ScriptParser : public ZQ::common::ExpatBase {

	static const std::string VAR_START; 
	static const std::string VAR_END; 
	static const std::string FUNC_INC; 
	static const std::string FUNC_POOL_ALLOC; 
	static const std::string FUNC_POOL_FIND;
	static const std::string FUNC_UUID;
	static const std::string FUNC_END;

public:
	
	ScriptParser(const std::string& script);
	virtual ~ScriptParser();

public:

	enum ClientType {RTSP, HTTP};

	ClientType clientType;		
	uint clientCount;
	uint sessionPerClient;
	uint loopCount;
	uint sessionInterval;
	uint messageTimeout;
	uint clientIdleTimeout;
	uint hammerThreads;
	uint clientThreads;

	typedef enum _HammerLogic
	{
		IMMEDIATELY_ADD,
		NEXT_LOOP_ADD,
		NOT_SUPPORT
    } HammerLogic;

	HammerLogic hammerLogic;

	std::string serverUrl;

	typedef struct {
		std::string name;
		ZQ::common::Log::loglevel_t level;
		int size;
		uint count;
		uint16 flags;
	} Logger;

	Logger logger;
	ZQ::common::RuleEngine::Rule hammerRule;

public:

	typedef struct {
		std::string key;
		std::string val;
		bool global;
	} SessContext;

	typedef struct {
		std::string name;
		std::string syntax;
		SessContext ctx;
	} RespHdr;

	typedef std::vector<SessContext> SessionContexts;
	typedef std::map<std::string, std::string> Variables;
	typedef std::map<std::string, AssetPool*> AssetPools; 
	typedef std::map<std::string, RespHdr> RespHeaders;

public:
	
	virtual void OnStartElement(const XML_Char* name, const XML_Char** atts);
	virtual void OnEndElement(const XML_Char* name);

public:

	void init();
	bool hasFunction(const std::string&);

	std::string getVar(const std::string& key) {
		ZQ::common::MutexGuard guard(_var_lock);
		return (_vars.find(key) == _vars.end()) ? std::string() : _vars[key];
	}
	void setVar(const std::string& key, const std::string& val) {
		ZQ::common::MutexGuard guard(_var_lock);
		_vars[key] = val;
	}

	const RespHdr* getRespHdr(const std::string& name) const {
		RespHeaders::const_iterator iter = _respHdrs.find(name);
		if(iter != _respHdrs.end()) {
			return (&iter->second);
		}
		return 0;
	}
	
	/* try to locate var from the specified table in addition to global vars */
	void expandVariables(std::string& var, const Variables& table);
	void expandFunctions(std::string& src, const Variables& table);

	void parseSessionContext(HammerSession::SessionCtx&);

public:

	static uint to_uint (const std::string& src); 
	static int64 to_int64(const std::string& src); 
	static std::string to_string(size_t);

private:

	uint _increment(uint value);
	std::string _allocAsset(const std::string& poolName);
	std::string _getAssetParam(const std::string& poolName, const std::string& assetName, const std::string& paramName);

private:

	Variables _vars;
	ZQ::common::Mutex _var_lock;
	SessionContexts _sess_ctxs;

private:

	AssetPools _pools;
	AssetPool::Asset _currentAsset;
	AssetPool* _currentAssetPool;
	ZQ::common::Mutex _pool_lock;

	ZQ::common::RuleEngine::RuleItem _currentAction;

	RespHeaders _respHdrs;
	RespHdr _currentRespHdr;
};

#endif

// vim: ts=4 sw=4 nu bg=dark

