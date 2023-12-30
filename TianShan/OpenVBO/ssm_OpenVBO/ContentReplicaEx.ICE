#ifndef __ContentReplicaEx_ICE__
#define __ContentReplicaEx_ICE__
#include <../common/TianShanUtils.ICE>
#include <../Ice/TsRepository.ICE>
#include <MetaLib.ICE>
module TianShanIce
{
module Repository
{

class ContentReplicaEx extends LibMetaObject implements ContentReplica, ::TianShanUtils::TimeoutObj
{
    string    netId; 
    string    volumeName;
    string    contentName;
    string    lastModified;
    ::TianShanIce::Storage::Content* content; // access to the content object on the ContentStore

    ::TianShanIce::Storage::ContentState state;
    
    ["freeze:write"] ::TianShanIce::Properties getUpdateMetaData();
    ["freeze:write"] ::TianShanIce::Storage::ContentState getUpdateState();
    ["freeze:write"] void setLastModified(string timeStamp);
    ["cpp:const", "freeze:read"] idempotent
    string getLastModified();
	["freeze:write"] void setMetaDataMap(TianShanIce::Repository::MetaDataMap valueMap);
};

class ContentStoreReplicaEx extends LibMetaObject implements ContentStoreReplica, ::TianShanUtils::TimeoutObj
{
		string    netId; // search value, same as ident.name
		string    endpoint; // use it to connect ContentStore
		long    	lastSync;
		
		::TianShanIce::Storage::ContentStore* contentstore; // access to the ContentStore
	  ["freeze:write"] void setLastSync(long time);
    ["cpp:const", "freeze:read"] idempotent
    long getLastSync();	
    ["cpp:const", "freeze:read"] idempotent
    MetaObjectInfos listVolume();
};

class MetaVolumeEx extends LibMetaObject implements MetaVolume, ::TianShanUtils::TimeoutObj
{
		string    netId;
		string    volumeName; // search value, same as ident.name
		string    lastModified;
		
		::TianShanIce::Storage::Volume* volume; // access to the volume object on the ContentStore
		
		["freeze:write"] void setLastModified(string timeStamp);
    ["cpp:const", "freeze:read"] idempotent
    string getLastModified();
};

};
};
#endif // __ContentReplicaEx_ICE__
