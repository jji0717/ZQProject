#include "AssetPool.h"
#include <algorithm>

AssetPool::AssetPool(const std::string& name, AllocMethod method, bool spin)
:_name(name), _method(method), _spin(spin), _last_idx(0) {
}

AssetPool::~AssetPool() {
}

void AssetPool::addAsset(const AssetPool::Asset& asset) {
	_assets.push_back(asset);
	_assetMap[asset.name] = asset;
}

AssetPool::Asset AssetPool::allocate() {
	if(_method == RANDOM) {
		_last_idx = 0;
rand:
		std::random_shuffle(_assets.begin(), _assets.end());		
		if(_spin) {
			std::string name = _assets.at(_last_idx).name;
			RecordCursor cursor = _alloc_record.find(name);
			/* this asset already been allocated in current round */
			if(cursor != _alloc_record.end()) {
				goto rand;
			}
			/* not allocated before, keep a record */
			else {
				_alloc_record.insert(name);
				/* already gone through the whole sequence */
				if(_alloc_record.size() == _assets.size()) {
					_alloc_record.clear();
				}
			}
		}
	}
	/* sequential */
	else {
		_last_idx = ((_last_idx+1) == _assets.size()) ? 0 : (_last_idx+1);
	}

	return _assets.at(_last_idx);
}

std::string AssetPool::getAssetParam(const std::string& assetName, const std::string& paramName) const {
	std::map<std::string, Asset>::const_iterator iter  = _assetMap.find(assetName);
	if(iter != _assetMap.end()) {
		AssetAttrs::const_iterator iter2 = iter->second.params.find(paramName);	
		if(iter2 != iter->second.params.end()) {
			return iter2->second;
		}
	}
	return std::string();
}

std::string AssetPool::name() const {
	return _name;
}

AssetPool::AllocMethod AssetPool::allocMethod() const {
	return _method;
}

bool AssetPool::spin() const {
	return _spin;
}

