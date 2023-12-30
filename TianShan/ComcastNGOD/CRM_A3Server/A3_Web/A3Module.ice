// FileName : A3Module.ice
// Author   : Hui.shao
// Date     : 2009-05
// build steps : 
// $(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir ../ A3Module.ice 
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir ../ --index "A3Module::AssetIdx,A3Module::A3Content,assetKey,case-insensitive" AssetIdx $(InputDir)$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir ../ --index "A3Module::VolumeIdx,A3Module::A3Content,fullVol,case-insensitive" VolumeIdx $(InputDir)$(InputName).ice

#ifndef __A3_Module_H__
#define __A3_Module_H__

#include "TianShanIce.ICE"
#include "TsStorage.ICE"

module A3Module
{

class A3Content
{
    ::Ice::Identity  ident; // the name is formatted in "PAID#PID@NetId$VolName", category = "A3Content"
    string    assetKey; // the index key from the combination of "PAID#PID" to this record
    string    fullVol; // the index key of the netid of ContentStore and the volume on it, formatted in "NetId$VolName"
    string    responseURL; // 
    ::TianShanIce::Storage::Content* content; // access to the content object on the ContentStore

    ::TianShanIce::Properties metaData; // the additional metadata of A3Content
    ::TianShanIce::Storage::ContentState state;
    
    ["cpp:const", "freeze:read"] idempotent void getAssetId(out string providerId, out string providerAssetId);
    ["cpp:const", "freeze:read"] idempotent void getVolumeInfo(out string contentStoreNetId, out string volumeName);
    ["cpp:const", "freeze:read"] idempotent ::TianShanIce::Properties getMetaData();
    ["cpp:const", "freeze:read"] idempotent ::TianShanIce::Storage::ContentState getState();
    ["cpp:const", "freeze:read"] idempotent ::TianShanIce::Storage::Content* theContent();
    ["cpp:const", "freeze:read"] idempotent string getResponseURL();
    ["cpp:const", "freeze:read"] idempotent Ice::Identity getIdentity();
    ["cpp:const", "freeze:read"] idempotent ::TianShanIce::StatedObjInfo getInfo(::TianShanIce::StrValues expectedMetaData);
    
    ["freeze:write"] ::TianShanIce::Properties getUpdateMetaData();
    ["freeze:write"] ::TianShanIce::Storage::ContentState getUpdateState();
    ["freeze:write"] void OnContentEvent(string contentEventName, ::TianShanIce::Properties params); // process the content event
};

sequence <A3Content* > A3Contents;
dictionary<string, int> A3Assets;

interface A3Facede extends TianShanIce::BaseService, TianShanIce::Events::GenericEventSink
{
    A3Content* openA3Content(string providerId, string providerAssetId, string contentStoreNetId, string volumeName);
    A3Contents findContentsByAsset(string providerId, string providerAssetId);
    A3Contents listContentsByVolume(string contentStoreNetId, string volumeName);
    
    /// maxCount <= 0, list all assets
    /// included = true, list from given PAID_PID; included = false, list from next element of PAID_PID
    A3Assets listAssets(string providerId, string providerAssetId, int maxCount, bool included);
    
    bool deleteA3Content(string providerId, string providerAssetId, string contentStoreNetId, string volumeName);
    
};

};

#endif // __A3_Module_H__

