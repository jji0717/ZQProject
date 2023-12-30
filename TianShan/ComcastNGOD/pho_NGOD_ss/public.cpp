
#include <TianShanIce.h>
#include <tssrm.h>
#include "public.h"


void	PutResourceMapData(TianShanIce::SRM::ResourceMap& rcMap,
						   const TianShanIce::SRM::ResourceType& type,
						   const std::string& strKey,
						   const TianShanIce::Variant& value,
						   const TianShanIce::SRM::ResourceStatus& status, 
						   bool bNewResource)
{
	TianShanIce::SRM::ResourceMap::iterator it=rcMap.find(type);
	
	if(it==rcMap.end()||bNewResource)
	{
		//insert the resource
		TianShanIce::SRM::Resource res;
		res.status=status;
		res.resourceData[strKey]=value;
		rcMap[type]=res;
	}
	else
	{
		it->second.status=status;
		it->second.resourceData[strKey]=value;
	}
}

TianShanIce::Variant		GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
											   const TianShanIce::SRM::ResourceType& type,
											   const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() type %d not found",type);
		
		throw TianShanIce::InvalidParameter("GetResoourceMapData",0, std::string(szBuf) );
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		throw TianShanIce::InvalidParameter("GetResoourceMapData",0, std::string(szBuf) );
	}
	return it->second;	
}
char* ConvertWeiwooResourceType(const TianShanIce::SRM::ResourceType& type)
{
	switch(type)
	{
	case TianShanIce::SRM::rtURI:
		return "rtURI";
		break;
	case TianShanIce::SRM::rtStorage:
		return "rtStorage";
		break;
	case TianShanIce::SRM::rtStreamer:
		return "rtStreamer";
		break;
	case TianShanIce::SRM::rtServiceGroup:
		return "rtServiceGroup";
		break;
	case TianShanIce::SRM::rtMpegProgram:
		return "rtMpegProgram";
		break;
	case TianShanIce::SRM::rtTsDownstreamBandwidth:
		return "rtTsDownstreamBandwidth";
		break;
	case TianShanIce::SRM::rtIP:
		return "rtIP";
		break;
	case TianShanIce::SRM::rtEthernetInterface:
		return "rtEthernetInterface";
		break;
	case TianShanIce::SRM::rtPhysicalChannel:
		return "rtPhysicalChannel";
		break;
	case TianShanIce::SRM::rtAtscModulationMode:
		return "rtAtscModulationMode";
		break;
	case TianShanIce::SRM::rtHeadendId:
		return "rtHeadendId";
		break;
	default:
		return "Unknow Type";
	}
}

