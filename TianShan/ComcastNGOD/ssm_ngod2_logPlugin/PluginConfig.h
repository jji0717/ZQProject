// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//

#ifndef __ZQTianShan_PLUGINConfig_H__
#define __ZQTianShan_PLUGINConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

static ::std::string NGOD2_PluginLayer = "ngod2_plugin";
static ::std::string Slash ="/";
static ::std::string LogLayer = "log";

struct myLog
{
	::std::string	path;
	int32			level;
	int32			logNum;
	int32			size;
	int32			buffer;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< myLog > &holder)
	{
		static ::std::string Layer = NGOD2_PluginLayer + Slash + LogLayer;
		holder.addDetail(Layer.c_str(), "path", &myLog::path, NULL);
		holder.addDetail(Layer.c_str(), "level", &myLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "logNum", &myLog::logNum, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &myLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &myLog::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &myLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string DBPathLayer = "DBPath";
struct DBPath
{
	::std::string	type;
	::std::string	templatPath;
	::std::string	path;
	::std::string	dsn;
	::std::string	user;
	::std::string	auth;
	static void structure(ZQ::common::Config::Holder< DBPath > &holder)
	{
		static ::std::string Layer = NGOD2_PluginLayer + Slash + DBPathLayer;
		holder.addDetail(Layer.c_str(), "type", &DBPath::type, NULL);
		holder.addDetail(Layer.c_str(), "templatPath", &DBPath::templatPath, NULL);
		holder.addDetail(Layer.c_str(), "path", &DBPath::path, NULL);
		holder.addDetail(Layer.c_str(), "dsn", &DBPath::dsn, NULL);
		holder.addDetail(Layer.c_str(), "user", &DBPath::user, NULL);
		holder.addDetail(Layer.c_str(), "auth", &DBPath::auth, NULL);
	}
};

static ::std::string TimeOutLayer = "Timeout";
struct TimeOut
{
	int32	iTime;
	static void structure(ZQ::common::Config::Holder< TimeOut > &holder)
	{
		static ::std::string Layer = NGOD2_PluginLayer + Slash + TimeOutLayer;
		holder.addDetail(Layer.c_str(), "time", &TimeOut::iTime, "", ZQ::common::Config::optReadOnly);
	}
};

class PluginConfig
{
public:
	PluginConfig(const char *filepath);
	virtual ~PluginConfig();

	void ConfigLoader();

	//default config
	ZQ::common::Config::Loader< myLog >		_myLog;
	ZQ::common::Config::Loader< DBPath >	_dbPath;
	ZQ::common::Config::Loader< TimeOut >	_timeOut;
private:
	::std::string	_strFilePath;
};

struct NGOD2PlugInCfg
{
	//Default config
	myLog	_myLog;
	DBPath	_dbPath;
	TimeOut	_timeOut;

	static void structure(::ZQ::common::Config::Holder<NGOD2PlugInCfg> &holder)
	{
		//read default config
		holder.addDetail("",&NGOD2PlugInCfg::readLog, &NGOD2PlugInCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NGOD2PlugInCfg::readDBPath, &NGOD2PlugInCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NGOD2PlugInCfg::readTimeOut, &NGOD2PlugInCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
	}

	void readLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<myLog> nvholder("");
		nvholder.read(node, hPP);
		_myLog = nvholder;
	}

	void readDBPath(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<DBPath> nvholder("");
		nvholder.read(node, hPP);
		_dbPath = nvholder;
	}
	void readTimeOut(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<TimeOut> nvholder("");
		nvholder.read(node, hPP);
		_timeOut = nvholder;
	}

	void registerNothing(const std::string&){}
};

#endif __ZQTianShan_PLUGINConfig_H__