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
// Ident : $Id: CVSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSSConfig.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 4     09-01-21 16:59 Xiaoming.li
// change config format
// 
// 3     09-01-20 9:50 Xiaoming.li
// delete timeout
// 
// 2     08-12-16 14:58 Xiaoming.li
// add snmpRegister
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CVSSConfig_H__
#define __ZQTianShan_CVSSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace ZQTianShan{

namespace CVSS{

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
static ::std::string PropLayer = "prop";
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
        _props.push_back(dptholder);
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


//configuration of CVSS
static ::std::string CVSSLayer = "CVSS";

//LogFile
static ::std::string LogFileLayer = "LogFile";
struct LogFile
{
	::std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			buffer;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< LogFile > &holder)
	{
		static ::std::string Layer = CVSSLayer + Slash + LogFileLayer;
		holder.addDetail(Layer.c_str(), "path", &LogFile::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &LogFile::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//bind
static ::std::string CVSSBindLayer = "Bind";
struct stBind
{
	::std::string	endPoint;
	int32			dispatchSize;
	int32			dispatchMax;
	int32			evictorSize;
	int32			threadPoolSize;
	static void structure(ZQ::common::Config::Holder< stBind > &holder)
	{
		static ::std::string Layer = CVSSLayer + Slash + CVSSBindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string nssIceLogLayer = "IceLog";
struct IceLog
{
	::std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	buffer;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< IceLog > &holder)
	{
		static ::std::string Layer = CVSSLayer + Slash + nssIceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &IceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &IceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &IceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &IceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &IceLog::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &IceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//definition of timeout
static ::std::string RTSPPropLayer = "RTSPProp";
struct RTSPProp
{
	int32	threadPoolSize;
	int32	timeOut;
	int32	bufferMaxSize;
	static void structure(ZQ::common::Config::Holder< RTSPProp > &holder)
	{
		static ::std::string Layer =  CVSSLayer + Slash + RTSPPropLayer;
		holder.addDetail(Layer.c_str(), "threadPoolSize", &RTSPProp::threadPoolSize, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "timeOut", &RTSPProp::timeOut, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferMaxSize", &RTSPProp::bufferMaxSize, NULL, ZQ::common::Config::optReadOnly);
	}
};

//definition of StreamingServer
static ::std::string streamingServer = "StreamingServer";
struct StreamingServer
{
	::std::string name;
	::std::string ip;
	int32	port;
	static void structure(ZQ::common::Config::Holder< StreamingServer > &holder)
	{
		static ::std::string Layer =  CVSSLayer + Slash + streamingServer;
		holder.addDetail(Layer.c_str(), "name", &StreamingServer::name, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "ip", &StreamingServer::ip, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &StreamingServer::port, NULL, ZQ::common::Config::optReadOnly);
	}
};

//definition of SoapLog
static ::std::string soapLog = "SoapLog";
struct SoapLog
{
	::std::string path;
	::std::string syntax;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	buffer;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< SoapLog > &holder)
	{
		static ::std::string Layer = CVSSLayer + Slash + soapLog;
		holder.addDetail(Layer.c_str(), "path", &SoapLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &SoapLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &SoapLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &SoapLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &SoapLog::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &SoapLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//definition of StoreInfo
static ::std::string StoreInfoLayer = "StoreInfo";
struct StoreInfo
{
	::std::string netId;
	::std::string type;
	::std::string streamableLength;

	//for StoreReplica
	struct StoreReplica
	{
		std::string groupId;
		std::string replicaId;
		int			replicaPriority;
		int			timeout;
		int			contentSize;
		int			volumeSize;

		static void structure(ZQ::common::Config::Holder< StoreReplica > &holder)
		{
			static ::std::string StoreReplicaLayer = "StoreReplica";
			//static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer + Slash + StoreReplicaLayer;
			holder.addDetail(StoreReplicaLayer.c_str(), "groupId", &StoreInfo::StoreReplica::groupId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(StoreReplicaLayer.c_str(), "replicaId", &StoreInfo::StoreReplica::replicaId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(StoreReplicaLayer.c_str(), "replicaPriority", &StoreInfo::StoreReplica::replicaPriority, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(StoreReplicaLayer.c_str(), "timeout", &StoreInfo::StoreReplica::timeout, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(StoreReplicaLayer.c_str(), "contentSize", &StoreInfo::StoreReplica::contentSize, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(StoreReplicaLayer.c_str(), "volumeSize", &StoreInfo::StoreReplica::volumeSize, NULL, ZQ::common::Config::optReadOnly);
		}
	};
	::ZQ::common::Config::Holder<StoreReplica> _storeReplica;
	void readStoreReplica(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		_storeReplica.read(node, hPP);
	}
	void registerStoreReplica(const std::string &full_path)
	{
		_storeReplica.snmpRegister(full_path);
	}

	//for LocalSoapInfo
	struct LocalSoapInfo
	{
		::std::string ip;
		int32	port;

		static void structure(ZQ::common::Config::Holder< LocalSoapInfo > &holder)
		{
			static ::std::string LocalSoapInfoLayer = "LocalSoapInfo";
			//static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer + Slash + LocalSoapInfoLayer;
			holder.addDetail(LocalSoapInfoLayer.c_str(), "ip", &StoreInfo::LocalSoapInfo::ip, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(LocalSoapInfoLayer.c_str(), "port", &StoreInfo::LocalSoapInfo::port, "", ZQ::common::Config::optReadOnly);
		}
	};
	::ZQ::common::Config::Holder< LocalSoapInfo > _localSoapInfo;
	void readLocalSoapInfo(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		_localSoapInfo.read(node, hPP);
	}
	void registerLocalSoapInfo(const std::string &full_path)
	{
		_localSoapInfo.snmpRegister(full_path);
	}

	//for ServerInfo
	struct ServerInfo
	{
		::std::string ip;
		int32	port;

		static void structure(ZQ::common::Config::Holder< ServerInfo > &holder)
		{
			static ::std::string ServerInfoLayer = "ServerInfo";
			//static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer + Slash + ServerInfoLayer;
			holder.addDetail(ServerInfoLayer.c_str(), "ip", &StoreInfo::ServerInfo::ip, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(ServerInfoLayer.c_str(), "port", &StoreInfo::ServerInfo::port, "", ZQ::common::Config::optReadOnly);
		}
	};
	::ZQ::common::Config::Holder< ServerInfo > _serverInfo;
	void readServerInfo(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		_serverInfo.read(node, hPP);
	}
	void registerServerInfo(const std::string &full_path)
	{
		_serverInfo.snmpRegister(full_path);
	}

	//for VolumeInfo
	struct VolumeInfo
	{
		::std::string name;
		::std::string path;

		static void structure(ZQ::common::Config::Holder< VolumeInfo > &holder)
		{
			static ::std::string VolumeInfoLayer = "VolumeInfo";
			//static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer + Slash + VolumeInfoLayer;
			holder.addDetail(VolumeInfoLayer.c_str(), "name", &StoreInfo::VolumeInfo::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(VolumeInfoLayer.c_str(), "path", &StoreInfo::VolumeInfo::path, NULL, ZQ::common::Config::optReadOnly);
		}
	};
	::ZQ::common::Config::Holder< VolumeInfo > _volumeInfo;
	void readVolumeInfo(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		_volumeInfo.read(node, hPP);
	}
	void registerVolumeInfo(const std::string &full_path)
	{
		_volumeInfo.snmpRegister(full_path);
	}

	//for HeartBeat
	struct HeartBeat
	{
		int32	interval;

		static void structure(ZQ::common::Config::Holder< HeartBeat > &holder)
		{
			static ::std::string HeartBeatLayer = "HeartBeat";
			//static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer + Slash + HeartBeatLayer;
			holder.addDetail(HeartBeatLayer.c_str(), "interval", &StoreInfo::HeartBeat::interval, "", ZQ::common::Config::optReadOnly);
		}
	};
	::ZQ::common::Config::Holder< HeartBeat > _heartBeat;
	void readHeartBeat(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		_heartBeat.read(node, hPP);
	}
	void registerHeartBeat(const std::string &full_path)
	{
		_heartBeat.snmpRegister(full_path);
	}

	//read whole structure
	static void structure(ZQ::common::Config::Holder<StoreInfo> &holder)
	{
		static ::std::string Layer = CVSSLayer + Slash + StoreInfoLayer;
		holder.addDetail(Layer.c_str(), "netId", &StoreInfo::netId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "type", &StoreInfo::type, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "streamableLength", &StoreInfo::streamableLength,NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail(Layer.c_str(), &StoreInfo::readStoreReplica, &StoreInfo::registerStoreReplica);
		holder.addDetail(Layer.c_str(), &StoreInfo::readLocalSoapInfo, &StoreInfo::registerLocalSoapInfo);
		holder.addDetail(Layer.c_str(), &StoreInfo::readServerInfo, &StoreInfo::registerServerInfo);
		holder.addDetail(Layer.c_str(), &StoreInfo::readVolumeInfo, &StoreInfo::registerVolumeInfo);
		holder.addDetail(Layer.c_str(), &StoreInfo::readHeartBeat, &StoreInfo::registerHeartBeat);
	}
};


class CVSSConfig
{
public:
	CVSSConfig(const char *filepath);
	virtual ~CVSSConfig();

	void ConfigLoader();
	
	//default config
	::ZQ::common::Config::Loader< CrashDump >		_crashDump;
	::ZQ::common::Config::Loader< IceTrace >		_iceTrace;
	::ZQ::common::Config::Loader< IceStorm >		_iceStorm;
	::ZQ::common::Config::Loader< IceProperties >	_iceProperties;
	::ZQ::common::Config::Loader< Database >		_dataBase;
	::ZQ::common::Config::Loader< PublishedLogs >	_publishedLogs;
	::ZQ::common::Config::Loader< RTSPProp >		_rtspProp;

	//CVSS config
	::ZQ::common::Config::Loader< LogFile >		_cLogFile;
	::ZQ::common::Config::Loader< stBind >		_bind;
	::ZQ::common::Config::Loader< IceLog >		_iceLog;
	//::ZQ::common::Config::Loader< TimeOut >		_timeOut;
	::ZQ::common::Config::Loader< StreamingServer >		_streamingServer;
	::ZQ::common::Config::Loader< SoapLog >		_soapLog;
	::ZQ::common::Config::Loader< StoreInfo >	_storeInfo;

private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace CVSS

}//namespace ZQTianShan

#endif __ZQTianShan_CVSSConfig_H__