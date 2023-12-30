#ifndef _C2CONTENT_ATTR_CACHE_H
#define _C2CONTENT_ATTR_CACHE_H

#include "LRUMap.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"
#include "C2Client.h"

#define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL


//#include "c2Define.h"

// -----------------------------
// interface C2ContentQueryBind
// -----------------------------
// the callback interface to receive obtained content attribute
namespace ZQTianShan{
namespace C2SS{

typedef std::map < std::string, std::string > AttrMap;
class C2ContentQueryBind
{
	public:
		typedef std::map < std::string, std::string > AttrMap;
		typedef struct _ContentAttr
		{
			_ContentAttr()
			{
				isPWE = false;
				stampAsOf = -1;
			}
			std::string assetId, providerId;
			std::string mainFileExtName;
			int playTime_msec;
			int muxBitrate;
			bool isPWE;
			AttrMap  attrs;

			int64    stampAsOf;
		} ContentAttr;
	public:
		virtual void OnContentAttributes(const std::string& contentName, const ContentAttr& cattr) =0;
		virtual void OnError(int errCode, const std::string& errMsg) =0;
	};

class C2ContentAttrCache;
// -----------------------------
// interface C2LocateQuery
// -----------------------------
// the callback interface to receive obtained c2Client
class C2LocateQuery : public ZQ::StreamService::C2ClientBind
{
	friend class C2ContentAttrCache;
public:
	typedef ZQ::common::Pointer <C2LocateQuery> Ptr;
public:
	C2LocateQuery(C2ContentAttrCache& owner, ZQ::common::Log& log, ZQ::StreamService::RequestParams params)
		: C2ClientBind(log, params), _owner(owner){}
	virtual ~C2LocateQuery() {}
	bool issueLocateRequest();
	virtual void OnC2LocateResponse(const std::string& contentName, const AttrMap& locRespParamters);
	virtual void OnC2GetResponse(const std::string& contentName, const AttrMap& locRespParamters);
	//virtual void OnC2IndexHeader(std::string& contentName, const uint8* buf, const int len);
	virtual void OnError(int errCode, const std::string& errMsg);

protected:
	C2ContentAttrCache&			_owner;
	//std::string                                 _contentName;
	//ZQ::common::Log&                   _log;
	//std::string                                  _c2Url; 

	C2ContentQueryBind::ContentAttr _ca;
};

// -----------------------------
// class C2ContentResultDispatcher
// -----------------------------
// the command module to dispatch associated content attribute to the subscribers
class C2ContentResultDispatcher : public ZQ::common::ThreadRequest
{
public:
	C2ContentResultDispatcher(C2ContentAttrCache& cache, std::string contentName, ZQ::common::NativeThreadPool& pool);
	~C2ContentResultDispatcher();
protected:
	int run();

	C2ContentAttrCache&			_c2ContentCache;
	std::string						_c2ContentName;
};
// -----------------------------
// class C2ContentAttrCache
// -----------------------------
// the container of C2 content attributes
class C2ContentAttrCache :  public ZQ::common::ThreadRequest
{
	friend class C2LocateQuery;

public:
	C2ContentAttrCache(ZQ::common::Log& logger, ZQ::common::NativeThreadPool& pool, const std::string& url);
	virtual ~C2ContentAttrCache();

	void lookupContent_async(C2ContentQueryBind& cb, const std::string& contentName);

	bool lookupContent_sync(C2ContentQueryBind::ContentAttr& ca, std::string& contentName, int timeout);
	bool getAttr(std::string& contentName, C2ContentQueryBind::ContentAttr& attr);

public:
	typedef std::vector < C2ContentQueryBind* > QueryAwaitList;
	typedef std::map<std::string, QueryAwaitList> QueryAwaitMap; // map of contentName to QueryAwaitList
	QueryAwaitMap _awaitMap;
	ZQ::common::Mutex _lkAwaitMap;

	typedef ZQ::common::LRUMap< std::string, C2ContentQueryBind::ContentAttr > AttrCache; // map of content name to ContentAttr
	AttrCache _attrCache;
	ZQ::common::Mutex _lkAttrCache;
	typedef SYS::SingleObject Event; // for the stupid naming of SingleObject
protected:
	//@return true if hit cache
	bool _lookupCacheAndLocate(const std::string& contentName,  C2ContentQueryBind::ContentAttr& ca, C2ContentQueryBind* pCb=NULL);

protected: // common members
	ZQ::common::Log& _log;
	std::string _url;

	typedef std::map<std::string, C2LocateQuery::Ptr> LocateAwaitMap; // map of contentName to QueryAwaitList
	LocateAwaitMap _locateAwaitMap;
	ZQ::common::Mutex _lkLocateAwaitMap;
	Event _event;

};

	}//namespace C2SS
}//namespace ZQTianShan

#endif