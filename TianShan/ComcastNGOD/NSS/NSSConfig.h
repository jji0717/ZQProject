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
// Ident : $Id: NSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSConfig.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 11    08-12-18 11:39 Xiaoming.li
// correct config file format
// 
// 10    08-12-15 15:29 Xiaoming.li
// add snmp, change config element from ngod_NSS to NSS
// 
// 9     08-11-27 14:32 Xiaoming.li
// 
// 8     08-11-14 11:48 Xiaoming.li
// add version info, change data folder setting and service name
// 
// 6     08-11-11 17:22 Xiaoming.li
// 
// 5     08-11-11 16:43 Xiaoming.li
// 
// 4     08-11-06 11:49 Xiaoming.li
// 
// 7     08-11-05 11:14 Xiaoming.li
// 
// 6     08-11-05 10:08 Xiaoming.li
// 
// 5     08-11-04 10:34 Xiaoming.li
// 
// 4     08-10-21 15:12 Xiaoming.li
// 
// 3     08-08-20 17:30 Xiaoming.li
// 
// 2     08-07-14 14:54 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-22 13:45 xiaoming.li
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_NSSConfig_H__
#define __ZQTianShan_NSSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace ZQTianShan{

namespace NSS{

//root configuration

//global configuration

//default configuration
static ::std::string DefaultLayer = "default";
static ::std::string Slash ="/";
static ::std::string CrashDumpLayer = "CrashDump";

struct CrashDump
{
	::std::string	path;
	int32			enabled;
	static void structure(ZQ::common::Config::Holder< CrashDump > &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + CrashDumpLayer;
		holder.addDetail(Layer.c_str(), "path", &CrashDump::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enabled", &CrashDump::enabled, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceTraceLayer = "IceTrace";
struct IceTrace
{
	int32	enabled;
	int32	level;
	int32	size;
	static void structure(ZQ::common::Config::Holder< IceTrace > &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + IceTraceLayer;
		holder.addDetail(Layer.c_str(), "enabled", &IceTrace::enabled, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(Layer.c_str(), "logfilesuffix", &IceTrace::logfilesuffix, NULL);
		holder.addDetail(Layer.c_str(), "level", &IceTrace::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &IceTrace::size, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceStormLayer = "EventChannel";
struct IceStorm
{
	::std::string	endPoint;
	static void structure(ZQ::common::Config::Holder< IceStorm > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + IceStormLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &IceStorm::endPoint, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IcePropertiesLayer = "IceProperties";
static ::std::string PropLayer = ::std::string("prop");
struct IceProperties
{
	struct prop
	{
		::std::string	name;
		::std::string	value;
		static void structure(ZQ::common::Config::Holder< prop > &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &prop::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "value", &prop::value,NULL, ZQ::common::Config::optReadOnly);
		}
	};
	typedef ::std::list< ZQ::common::Config::Holder<prop> > props;
	props _props;
	void readProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<prop> dptholder("");
        dptholder.read(node, hPP);
        _props.push_front(dptholder);
    }
    void registerProp(const std::string &full_path)
    {
        for(props::iterator it = _props.begin(); it != _props.end(); ++it)
            it->snmpRegister(full_path);
    }
    static void structure(ZQ::common::Config::Holder<IceProperties> &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + IcePropertiesLayer + Slash + PropLayer;
        holder.addDetail(Layer.c_str(), &IceProperties::readProp, &IceProperties::registerProp);
    }
};

static ::std::string DatabaseLayer = "Database";
struct Database
{
	::std::string	path;
	::std::string	runtimePath;
	static void structure(ZQ::common::Config::Holder< Database > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + DatabaseLayer;
		holder.addDetail(Layer.c_str(), "path", &Database::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "runtimePath", &Database::runtimePath, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string PublishedLogsLayer = "PublishedLogs";
static ::std::string PublishLogLayer = "Log";
struct PublishedLogs
{
	struct PublishLog
	{
		std::string _path;
		std::string _syntax;

		typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

		static void structure(ZQ::common::Config::Holder<PublishLog>& holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
		}
	};
	typedef PublishLog::PublishLogHolder PublishLogHolder;
	std::vector<PublishLogHolder> _logDatas;

	void readPublishedLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		PublishLogHolder logHolder("path");
		logHolder.read(node, hPP);
		_logDatas.push_back(logHolder);
	}

	void registerPublishedLogs(const std::string &full_path)
	{
		for (std::vector<PublishLogHolder>::iterator it = _logDatas.begin(); it != _logDatas.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(ZQ::common::Config::Holder< PublishedLogs > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + PublishedLogsLayer + Slash + PublishLogLayer;
		holder.addDetail(Layer.c_str(), &PublishedLogs::readPublishedLogs, &PublishedLogs::registerPublishedLogs);
	}
};

//configuration of ngod_NSS
static ::std::string NSSLayer = "NSS";

//LogFile
static ::std::string LogFileLayer = "LogFile";
struct LogFile
{
	::std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			bufferSize;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< LogFile > &holder)
	{
		static ::std::string Layer = NSSLayer + Slash + LogFileLayer;
		holder.addDetail(Layer.c_str(), "path", &LogFile::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &LogFile::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//bind
static ::std::string nssBindLayer = "Bind";
struct stBind
{
	::std::string	endPoint;
	int32			dispatchSize;
	int32			dispatchMax;
	int32			evictorSize;
	int32			threadPoolSize;
	static void structure(ZQ::common::Config::Holder< stBind > &holder)
	{
		static ::std::string Layer = NSSLayer + Slash + nssBindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string nssIceLogLayer = "IceLog";
struct nssIceLog
{
	::std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	bufferSize;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< nssIceLog > &holder)
	{
		static ::std::string Layer = NSSLayer + Slash + nssIceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &nssIceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &nssIceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &nssIceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &nssIceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &nssIceLog::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &nssIceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//LocalInfo
static ::std::string LocalInfoLayer = "LocalInfo";
struct LocalInfo
{
	::std::string	strAddr;
	int32			port;
	static void structure(ZQ::common::Config::Holder< LocalInfo > &holder)
	{
		static ::std::string Layer = NSSLayer + Slash + LocalInfoLayer;
		holder.addDetail(Layer.c_str(), "addr", &LocalInfo::strAddr, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &LocalInfo::port, "", ZQ::common::Config::optReadOnly);
	}
};

//definition about MediaCluster
static ::std::string MediaClusterLayer = "MediaCluster";
static ::std::string ServerLayer = "Server";
struct MediaCluster
{
	struct Server
	{
		::std::string	name;
		::std::string	address;
		int32			port;
		static void structure(ZQ::common::Config::Holder< Server > &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &Server::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "address", &Server::address, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "port", &Server::port, NULL, ZQ::common::Config::optReadOnly);
		}
	};

	typedef std::vector< ZQ::common::Config::Holder< Server > > Servers;
    Servers _servers;
	static void structure(ZQ::common::Config::Holder< MediaCluster > &holder)
    {
		static ::std::string Layer = NSSLayer + Slash + MediaClusterLayer + Slash + ServerLayer;
		holder.addDetail(Layer.c_str(), &MediaCluster::readServer, &MediaCluster::registerServers);
    }
    void readServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<Server> serverholder("");
        serverholder.read(node, hPP);
        _servers.push_back(serverholder);
    }
    void registerServers(const std::string &full_path)
    {
        for (Servers::iterator it = _servers.begin(); it != _servers.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};

//definition of timeout
static ::std::string timeOut = "TimeOut";

struct TimeOut
{
	int32	time;
	static void structure(ZQ::common::Config::Holder< TimeOut > &holder)
	{
		static ::std::string Layer =  NSSLayer + Slash + timeOut;
		holder.addDetail(Layer.c_str(), "time", &TimeOut::time, NULL, ZQ::common::Config::optReadOnly);
	}
};

//definition of session group
static ::std::string SessionGroupLayer = "SessionGroup";
static ::std::string GroupLayer = "Group";

struct SessionGroup
{
	//int32 cacheSize;
	struct Group
	{
		::std::string	name;
		int32			maxSession;
		static void structure(ZQ::common::Config::Holder< Group > &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &Group::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "maxSession", &Group::maxSession, NULL, ZQ::common::Config::optReadOnly);
		}
	};
	typedef std::vector< ZQ::common::Config::Holder< Group > > Groups;
    Groups _groups;
	static void structure(ZQ::common::Config::Holder< SessionGroup > &holder)
    {
		static ::std::string Layer = NSSLayer + Slash + SessionGroupLayer;
		//holder.addDetail(Layer.c_str(), "cacheSize", &SessionGroup::cacheSize, NULL, ZQ::common::Config::optReadOnly);
		Layer += Slash + GroupLayer;
		holder.addDetail(Layer.c_str(), &SessionGroup::readGroup, &SessionGroup::registerGroups);
    }
    void readGroup(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<Group> groupholder("");
        groupholder.read(node, hPP);
        _groups.push_back(groupholder);
    }
    void registerGroups(const std::string &full_path)
    {
        for (Groups::iterator it = _groups.begin(); it != _groups.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};

//definition of DEBUG
//static ::std::string strDebug = "DEBUG";
//
//struct Debug
//{
//	int32	enable;
//	int32	minTime;
//	int32	maxTime;
//	static void structure(ZQ::common::Config::Holder< Debug > &holder)
//	{
//		static ::std::string Layer =  NSSLayer + Slash + strDebug;
//		holder.addDetail(Layer.c_str(), "enable", &Debug::enable, NULL, ZQ::common::Config::optReadOnly);
//		holder.addDetail(Layer.c_str(), "minTime", &Debug::minTime, NULL, ZQ::common::Config::optReadOnly);
//		holder.addDetail(Layer.c_str(), "maxTime", &Debug::maxTime, NULL, ZQ::common::Config::optReadOnly);
//	}
//};

class NSSConfig
{
public:
	NSSConfig(const char *filepath);
	virtual ~NSSConfig();

	void ConfigLoader();
	
	//default config
	ZQ::common::Config::Loader< CrashDump >		_crashDump;
	ZQ::common::Config::Loader< IceTrace >		_iceTrace;
	ZQ::common::Config::Loader< IceStorm >		_iceStorm;
	ZQ::common::Config::Loader< IceProperties >	_iceProperties;
	ZQ::common::Config::Loader< Database >		_dataBase;
	ZQ::common::Config::Loader< PublishedLogs >	_publishedLogs;

	//NSS config
	ZQ::common::Config::Loader< stBind >		_stBind;
	//ZQ::common::Config::Loader< nssLog >		_nssLog;
	ZQ::common::Config::Loader< nssIceLog >		_nssIceLog;
	ZQ::common::Config::Loader< TimeOut >		_timeOut;
	ZQ::common::Config::Loader< MediaCluster >	_mediaCluster;
	ZQ::common::Config::Loader< SessionGroup >	_sessionGroup;

	//DEBUG mode
	//ZQ::common::Config::Loader< Debug>			_debug;

private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace NSS

}//namespace ZQTianShan

#endif __ZQTianShan_NSSConfig_H__