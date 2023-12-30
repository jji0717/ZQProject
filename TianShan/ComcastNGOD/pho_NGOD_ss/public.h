#ifndef __SEACHANGE_PHO_PUBLC_H__
#define __SEACHANGE_PHO_PUBLC_H__


#include <string>

#include <TianShanIce.h>
#include <tsTransport.h>
#include <tsSRM.h>

///put tianshanice variant into resourcemap as a resource 
///@param [in out] rcMap resourceMap to hold new resource
///@param type weiwoo resource type
///@param strKey resource key
///@param value resource value
///@param status resource status
///@param bnewResource if this value is true,the resource of the type will be replace by current resource,
///if it is false,new resource will be insert into the resource map
void						PutResourceMapData(TianShanIce::SRM::ResourceMap& rcMap,
											   const TianShanIce::SRM::ResourceType& type,
											   const std::string& strKey,
											   const TianShanIce::Variant& value,
											   const TianShanIce::SRM::ResourceStatus& status=TianShanIce::SRM::rsAssigned, 
											   bool bNewResource=false);

///get tianshanice variant from resourcemap through a specified key
///@return the tianshanice variant value
///@param rcMap tianshanice weiwoo resource
///@param type tianshanice weiwoo resource type
///@param strKey key to resource value
TianShanIce::Variant		GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
											   const TianShanIce::SRM::ResourceType& type,
											   const std::string& strkey);

void						DumpResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap);

#endif//__SEACHANGE_PHO_PUBLC_H__