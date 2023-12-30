#ifndef __ZQTianShan_StreamLinkParserCfg_H__
#define __ZQTianShan_StreamLinkParserCfg_H__

#include "ConfigHelper.h"
#include <vector>
#include <map>
#include <string>
using namespace ZQ::common;
struct Item
{
	std::string item;
	static void structure(ZQ::common::Config::Holder<Item>&holder)
	{
		holder.addDetail("", "value", &Item::item, NULL, ZQ::common::Config::optReadOnly);
	}; 
};
typedef std::vector< std::string >ITEMS;
struct StreamLinkData{
	std::string key;
	std::string type;
	int32       rang;
    ITEMS       items;
	static void structure(ZQ::common::Config::Holder<StreamLinkData>&holder)
	{
		holder.addDetail("", "key", &StreamLinkData::key, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "type", &StreamLinkData::type, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "range", &StreamLinkData::rang, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("Item",&StreamLinkData::readItems, &StreamLinkData::registerNothing, Config::Range(0,-1));
	}; 
	void readItems(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<Item> nvholder("");
		nvholder.read(node, hPP);
		items.push_back(nvholder.item);
	}
	void registerNothing(const std::string&){}
};
typedef std::vector< Config::Holder<StreamLinkData> >LinkDatas;
struct StreamLinkPrivate{
    
	LinkDatas linkdatas;
	static void structure(ZQ::common::Config::Holder<StreamLinkPrivate>&holder)
	{
		holder.addDetail("Data",&StreamLinkPrivate::readDatas, &StreamLinkPrivate::registerNothing, Config::Range(0,-1));
	}; 
	void readDatas(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<StreamLinkData> nvholder("");
		nvholder.read(node, hPP);
		linkdatas.push_back(nvholder);
	}
    void registerNothing(const std::string&){}
};
struct StreamLinkParserCfg
{
	::std::string linkId;
	::std::string streamerId;
	::std::string servicegroupId;
	::std::string type;
	StreamLinkPrivate privates;
	static void structure(ZQ::common::Config::Holder<StreamLinkParserCfg>&holder)
	{
		holder.addDetail("", "linkId", &StreamLinkParserCfg::linkId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "streamerId", &StreamLinkParserCfg::streamerId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "servicegroupId", &StreamLinkParserCfg::servicegroupId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "type", &StreamLinkParserCfg::type, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("PrivateData",&StreamLinkParserCfg::readPrivate, &StreamLinkParserCfg::registerNothing, Config::Range(0,1));
	};
	void readPrivate(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<StreamLinkPrivate> nvholder("");
		nvholder.read(node, hPP);
		privates = nvholder;
	}
	void registerNothing(const std::string&){}
};
typedef std::vector< Config::Holder<StreamLinkParserCfg> >StreamLinks;
struct StreamLink
{
    StreamLinks streamlinks;
	static void structure(ZQ::common::Config::Holder<StreamLink>&holder)
	{
		holder.addDetail("StreamLinks/Link",&StreamLink::readStreamLink, &StreamLink::registerNothing, Config::Range(0,-1));
	};
	void readStreamLink(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<StreamLinkParserCfg> nvholder("");
		nvholder.read(node, hPP);
		if(nvholder.type == "SeaChange.VSS.NGOD.DVBC")
			streamlinks.push_back(nvholder);
	}
	void registerNothing(const std::string&){}
};
#endif __ZQTianShan_StreamLinkParserCfg_H__