#ifndef __ASSET_POOL__
#define __ASSET_POOL__

#include "ZQ_common_conf.h"
#include <set>
#include <vector>
#include <string>
#include <map>

class AssetPool {

	friend class ScriptParser;

public:

	typedef std::set<std::string>::const_iterator RecordCursor;
	typedef std::map<std::string, std::string> AssetAttrs;

	enum AllocMethod {RANDOM, SEQUENTIAL};

	typedef struct {
		std::string name;
		AssetAttrs params;	
	} Asset;

public:

	AssetPool(const std::string& name, AllocMethod method=RANDOM, bool spin=0);
	~AssetPool(); 
	
public:

	void addAsset(const Asset& asset);
	Asset allocate();
	std::string getAssetParam(const std::string&, const std::string&) const;

	std::string name() const;
	AllocMethod allocMethod() const;
	bool spin() const;
	
private:

	std::string _name;	
	AllocMethod _method;
	bool _spin;

private:

	uint _last_idx;
	std::set<std::string> _alloc_record;

	std::vector<Asset> _assets;
	std::map<std::string, Asset> _assetMap;
};

#endif

// vim: ts=4 sw=4 nu bg=dark

