#ifndef __SEACHANGE_PHO_PUBLC_H__
#define __SEACHANGE_PHO_PUBLC_H__
#include <string>
#include <TianShanIce.h>
#include <TsTransport.h>
#include <TsSRM.h>

typedef struct
{
	::std::string		_strEdgeRMEndpoint;
	::Ice::Int			_modulationFormat;
	::std::string		_streamLinkID;
	//RoutingMode			_routingMode;
	std::string			_strQAMIDs;
	std::string         _strQAMZONE;
	std::vector<std::string>	_vecQAMID;
	std::string         _strSopName;
}EdgeRMLinkAttr;

typedef std::map<std::string, EdgeRMLinkAttr>		EdgeRMLinkAttrMap;	//<liknId, LinAttr>

typedef struct
{
	::std::string       _streamLinkID;
	::std::string		_rtspIp;
	::Ice::Int			_rtspPort;
	::Ice::Int			_modulationFormat;
	::Ice::Long         _totalBandWidth;
	std::string			_strQAMIDs;
	std::vector<std::string>	_vecQAMID;	
	Ice::Long			_availableBandwidth;
	Ice::Long			_usedBandwidth;
	std::string         _strSopName;
	::Ice::Int          _symbolRate;
	Ice::Int            _svcGrpId;
	::Ice::Int          _qamType;
}S6EdgeRMLinkAttr;

typedef std::map<std::string, S6EdgeRMLinkAttr>		S6EdgeRMLinkAttrMap;	//<liknId, LinAttr>

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
											   const std::string& sessId="",
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

