
#include "TianShanDefines.h"
#include <TianShanIce.h>
#include <TsSRM.h>
#include "public.h"
#include "Log.h"

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
void	PutResourceMapData(TianShanIce::SRM::ResourceMap& rcMap,
						   const TianShanIce::SRM::ResourceType& type,
						   const std::string& strKey,
						   const TianShanIce::Variant& value,
						   const std::string& sessId,
						   const TianShanIce::SRM::ResourceStatus& status, 						   
						   bool bNewResource)
{
	TianShanIce::SRM::ResourceMap::iterator it=rcMap.find(type);
	
	glog(ZQ::common::Log::L_INFO,"Session[%s] add or update Resource with type[%s] key[%s]",
				sessId.c_str(),ConvertWeiwooResourceType(type),strKey.c_str());
	if(it==rcMap.end()||bNewResource)
	{
		//insert the resource
		TianShanIce::SRM::Resource res;
		res.attr	=	TianShanIce::SRM::raMandatoryNegotiable;
		res.status	=	status;
		res.resourceData[strKey]=value;
		res.resourceData[strKey].bRange = false;
		rcMap[type]=res;
	}
	else
	{
		it->second.attr		= TianShanIce::SRM::raMandatoryNegotiable;
		it->second.status	= status;
		it->second.resourceData[strKey]=value;
		it->second.resourceData[strKey].bRange=false;
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
		
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter>(EXPFMT("IpEdgePHO",1001,"%s"),(szBuf));
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter>(EXPFMT("IpEdgePHO",1002,"%s"),(szBuf));
	}
	return it->second;	
}
