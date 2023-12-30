#ifndef __MetaVolumeImpl_H__
#define __MetaVolumeImpl_H__

#define _WINSOCK2API_
#include <ContentReplicaEx.h>
#include "MetaLibImpl.h"

#define METAVOLUME "MetaVolume"
#define FREESPACE  "FreeSpace"
#define TOTALSPACE "TotalSpace"

//class MetaVolumeImpl : public ::TianShanIce::Repository::MetaVolumeEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class MetaVolumeImpl : public ::TianShanIce::Repository::MetaVolumeEx, public ICEAbstractMutexRLock
{
public:
	MetaVolumeImpl(::ZQTianShan::MetaLib::MetaLibImpl& _lib);
	~MetaVolumeImpl();

public:
	virtual Ice::Identity getObjId(const ::Ice::Current& c) const;

	virtual std::string getType(const ::Ice::Current& c) const;

	virtual Ice::Identity getNetId(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::VolumePrx theVolume(const ::Ice::Current& c) const;

	virtual ::TianShanIce::StatedObjInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;

	virtual void OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c); // process the content event

	virtual void OnTimer(const ::Ice::Current& c);

	virtual std::string getCreatedTime(const Ice::Current &) const;

	virtual TianShanIce::Repository::MetaDataValue get(const std::string &,const Ice::Current &) const;

	virtual void set(const std::string &,const TianShanIce::Repository::MetaDataValue &,const Ice::Current &);

	virtual void setLastModified(const ::std::string&, const Ice::Current &);

	virtual ::std::string getLastModified(const Ice::Current &) const;

	virtual ::TianShanIce::Repository::MetaDataMap getMetaDataMap(const ::Ice::Current& c) const;

	virtual void setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &);

private:
	::ZQTianShan::MetaLib::MetaLibImpl& _lib;
};

typedef ::IceInternal::Handle< MetaVolumeImpl> MetaVolumeImplPtr;

#endif //__MetaVolumeImpl_H__

