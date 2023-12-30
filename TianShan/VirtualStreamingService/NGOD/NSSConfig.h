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
// $Log: /ZQProjs/TianShan/VirtualStreamingService/NGOD/NSSConfig.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 7     09-08-11 13:42 Xiaoming.li
// 
// 6     09-07-22 12:26 Xiaoming.li
// add HTTP request path
// 
// 5     09-07-21 15:17 Xiaoming.li
// 
// 4     09-07-21 15:03 Xiaoming.li
// merge from 1.8
// 
// 6     09-07-21 11:30 Xiaoming.li
// 
// 5     09-07-21 11:02 Xiaoming.li
// add netID for multiple NSS instance
// 
// 4     09-07-16 17:27 Xiaoming.li
// 
// 3     09-04-28 13:55 Xiaoming.li
// after CCUR integration
// 
// 2     09-04-17 17:00 Xiaoming.li
// change for CCUR
// 
// 1     09-02-20 16:14 Xiaoming.li
// 
// 8     08-12-18 11:40 Xiaoming.li
// correct config file format
// 
// 7     08-12-17 14:09 Xiaoming.li
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
struct IceStormConfig
{
	::std::string	endPoint;
	static void structure(ZQ::common::Config::Holder< IceStormConfig > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + IceStormLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &IceStormConfig::endPoint, NULL, ZQ::common::Config::optReadOnly);
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

//configuration of ngod_NSS
static ::std::string NSSLayer = "NSS";

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
		//static ::std::string Layer = NSSLayer + Slash + IcePropertiesLayer + Slash + PropLayer;
		static ::std::string Layer = IcePropertiesLayer + Slash + PropLayer;
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
		std::string _key;
		std::string _type;

		typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

		static void structure(ZQ::common::Config::Holder<PublishLog>& holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "key", &PublishLog::_key, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "type", &PublishLog::_type, "", ZQ::common::Config::optReadOnly);
		}
	};
	typedef PublishLog::PublishLogHolder PublishLogHolder;
	typedef std::vector<PublishLogHolder> PublishLogHolderVec;
	PublishLogHolderVec _logDatas;

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
		//static ::std::string Layer = NSSLayer + Slash + PublishedLogsLayer + Slash + PublishLogLayer;
		static ::std::string Layer = PublishedLogsLayer + Slash + PublishLogLayer;
		holder.addDetail(Layer.c_str(), &PublishedLogs::readPublishedLogs, &PublishedLogs::registerPublishedLogs);
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
		//static ::std::string Layer = NSSLayer + Slash + nssBindLayer;
		static ::std::string Layer = nssBindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

//jms configuration only for 1.8 build
static std::string A3JMSEventsLayer = "JMSEvents";
struct A3JMSEvents
{
	int enable;
	std::string server;
	int port;
	int UTCStamp;
	int reconnectInterval;

	struct JMSEvent
	{
		std::string name;
		std::string topic;
		struct parameter
		{
			std::string key;
			std::string type;
			std::string value;

			typedef ZQ::common::Config::Holder< parameter > parameterHolder;
			static void structure(parameterHolder &holder)
			{
				static ::std::string Layer = "";
				holder.addDetail(Layer.c_str(), "key", &parameter::key, NULL, ZQ::common::Config::optReadOnly);
				holder.addDetail(Layer.c_str(), "type", &parameter::type, NULL, ZQ::common::Config::optReadOnly);
				holder.addDetail(Layer.c_str(), "value", &parameter::value, NULL, ZQ::common::Config::optReadOnly);
			}
		};

		static void structure(ZQ::common::Config::Holder< JMSEvent > &holder)
		{
			::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "topic", &JMSEvent::topic, NULL, ZQ::common::Config::optReadOnly);
			Layer = "parameter";
			holder.addDetail(Layer.c_str(), &JMSEvent::readParams, &JMSEvent::registerParams);
		}

		typedef std::list<parameter::parameterHolder> parameters;
		parameters params;
		void readParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			parameter::parameterHolder serverholder("");
			serverholder.read(node, hPP);
			params.push_back(serverholder);
		}
		void registerParams(const std::string &full_path)
		{
			for (parameters::iterator it = params.begin(); it != params.end(); ++it)
			{
				it->snmpRegister(full_path);
			}
		}
	};
	typedef ZQ::common::Config::Holder< JMSEvent > JMSEventHolder;
	JMSEventHolder StateChanged;
	void readStateChanged(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		StateChanged.read(node, hPP);
		StateChanged.name = "StateChanged";
	}
	void registerStateChanged(const std::string &full_path)
	{
		StateChanged.snmpRegister(full_path);
	}

	JMSEventHolder Progress;
	void readProgress(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		Progress.read(node, hPP);
		StateChanged.name = "Progress";
	}
	void registerProgress(const std::string &full_path)
	{
		Progress.snmpRegister(full_path);
	}

	JMSEventHolder Destroy;
	void readDestroy(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		Destroy.read(node, hPP);
		StateChanged.name = "Destroy";
	}
	void registerDestroy(const std::string &full_path)
	{
		Destroy.snmpRegister(full_path);
	}

	static void structure(ZQ::common::Config::Holder< A3JMSEvents > &holder)
	{
		//static ::std::string Layer = NSSLayer + Slash + A3JMSEventsLayer;
		static ::std::string Layer = A3JMSEventsLayer;
		//get JMSEvent property
		holder.addDetail(Layer.c_str(), "enable", &A3JMSEvents::enable, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "server", &A3JMSEvents::server, NULL);
		holder.addDetail(Layer.c_str(), "port", &A3JMSEvents::port, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "UTCStamp", &A3JMSEvents::UTCStamp, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "reconnectInterval", &A3JMSEvents::reconnectInterval, "", ZQ::common::Config::optReadOnly);


		::std::string sLayer = Layer + Slash + "StateChanged";
		holder.addDetail(sLayer.c_str(), &A3JMSEvents::readStateChanged, &A3JMSEvents::registerStateChanged);

		sLayer = Layer + Slash + "Progress";
		holder.addDetail(sLayer.c_str(), &A3JMSEvents::readProgress, &A3JMSEvents::registerProgress);

		sLayer = Layer + Slash + "Destroy";
		holder.addDetail(sLayer.c_str(), &A3JMSEvents::readDestroy, &A3JMSEvents::registerDestroy);
	}
};


//definition about MediaCluster
static ::std::string NSSVideoServerLayer = "VideoServer";
//static ::std::string VideoServerLayer = NSSLayer + Slash + NSSVideoServerLayer;
static ::std::string VideoServerLayer = NSSVideoServerLayer;
static ::std::string SessionInterfaceLayer = VideoServerLayer + Slash + "SessionInterface";
static ::std::string FixedSpeedSetLayer = SessionInterfaceLayer + Slash + "FixedSpeedSet";
static ::std::string strBlank = " ";
static ::std::string ContentInterfaceLayer = VideoServerLayer + Slash + "ContentInterface";
static ::std::string FeedbackLayer = ContentInterfaceLayer + Slash + "Feedback";
static ::std::string StoreReplicaLayer = ContentInterfaceLayer + Slash + "StoreReplica";
static ::std::string DatabaseCacheLayer = ContentInterfaceLayer + Slash + "DatabaseCache";
static ::std::string LogLayer = ContentInterfaceLayer + Slash + "Log";
static ::std::string EventLogLayer = ContentInterfaceLayer + Slash + "EventLog";
static ::std::string VolumesLayer = ContentInterfaceLayer + Slash + "Volumes";
static ::std::string VolLayer = VolumesLayer + Slash + "Vol";
struct VideoServer
{
	//::std::string	netId;
	::std::string	vendor;
	::std::string	model;

	//session interface
	::std::string	SessionInterfaceIp;
	int32			SessionInterfacePort;
	int32			SessionInterfaceMaxSessionGroup;
	int32			SessionInterfaceMaxSessionsPerGroup;
	int32			SessionInterfaceRequestTimeout;

	//FixedSpeedSet
	int32				FixedSpeedSetEnable;
	std::string			FixedSpeedSetForward;
	std::string			FixedSpeedSetBackward;
	std::vector<float>	FixedSpeedSetForwardSet;
	std::vector<float>	FixedSpeedSetBackwardSet;

	//content interface
	::std::string	ContentInterfaceIp;
	int32			ContentInterfacePort;
	::std::string	ContentInterfacePath;
	int32			ContentInterfaceSyncInterval;
	::std::string	ContentInterfaceMode;

	//<Feedback>
	::std::string	FeedbackIp;
	int32			FeedbackPort;

	//<StoreReplica>
	::std::string	StoreReplicaGroupId;
	::std::string	StoreReplicaReplicaId;
	int32			StoreReplicaReplicaPriority;
	int32			StoreReplicaTimeout;

	//<DatabaseCache>
	int32	DatabaseCacheVolumeSize;
	int32	DatabaseCacheContentSize;
	int32	DatabaseCacheContentSavePeriod;
	int32	DatabaseCacheContentSaveSizeTrigger;

	////<Log>
	//::std::string	LogPath;
	//int32			LogLevel;
	//int32			LogMaxCount;
	//int32			LogSize;
	//int32			LogBufferSize;
	//int32			LogFlushTimeout;

	////<EventLog>
	//::std::string	EventLogPath;
	//int32			EventLogLevel;
	//int32			EventLogMaxCount;
	//int32			EventLogSize;
	//int32			EventLogBufferSize;
	//int32			EventLogFlushTimeout;

	typedef struct Vol
	{
		::std::string	mount;
		::std::string	targetName;
		int32			defaultVal;//1=true,0=false;
		int32			defaultBitRate;

		typedef ZQ::common::Config::Holder< Vol > VolHolder;
		static void structure(VolHolder &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "mount", &Vol::mount, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "targetName", &Vol::targetName, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "default", &Vol::defaultVal, "1", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "defaultBitRate", &Vol::defaultBitRate, "37500000", ZQ::common::Config::optReadOnly);
		}		
	}Vol;

	typedef ::std::vector<Vol::VolHolder> VolList;
	VolList vols;

	typedef ZQ::common::Config::Holder<VideoServer> VideoServerHolder;
	void readVol(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		Vol::VolHolder volHolder("");
		volHolder.read(node, hPP);
		vols.push_back(volHolder);
	}
	void registerVol(const std::string &full_path)
	{
		for (VolList::iterator it = vols.begin(); it != vols.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(VideoServerHolder &holder)
	{
		//load videoserver attribute
		//holder.addDetail(VideoServerLayer.c_str(), "netId", &VideoServer::netId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(VideoServerLayer.c_str(), "vendor", &VideoServer::vendor, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(VideoServerLayer.c_str(), "model", &VideoServer::model, NULL, ZQ::common::Config::optReadOnly);

		//load SessionInterface attribute
		holder.addDetail(SessionInterfaceLayer.c_str(), "ip", &VideoServer::SessionInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "port", &VideoServer::SessionInterfacePort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "maxSessionGroup", &VideoServer::SessionInterfaceMaxSessionGroup, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "maxSessionsPerGroup", &VideoServer::SessionInterfaceMaxSessionsPerGroup, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "requestTimeout", &VideoServer::SessionInterfaceRequestTimeout, "", ZQ::common::Config::optReadOnly);

		//load FixedSpeedSet attribute
		holder.addDetail(FixedSpeedSetLayer.c_str(), "enable", &VideoServer::FixedSpeedSetEnable, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(FixedSpeedSetLayer.c_str(), "forward", &VideoServer::FixedSpeedSetForward, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(FixedSpeedSetLayer.c_str(), "backward", &VideoServer::FixedSpeedSetBackward, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail(ContentInterfaceLayer.c_str(), "ip", &VideoServer::ContentInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "port", &VideoServer::ContentInterfacePort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "path", &VideoServer::ContentInterfacePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "syncInterval", &VideoServer::ContentInterfaceSyncInterval, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "mode", &VideoServer::ContentInterfaceMode, NULL, ZQ::common::Config::optReadOnly);

		//load Feedback attribute
		holder.addDetail(FeedbackLayer.c_str(), "ip", &VideoServer::FeedbackIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(FeedbackLayer.c_str(), "port", &VideoServer::FeedbackPort, "", ZQ::common::Config::optReadOnly);

		//load StoreReplica attribute
		holder.addDetail(StoreReplicaLayer.c_str(), "groupId", &VideoServer::StoreReplicaGroupId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "replicaId", &VideoServer::StoreReplicaReplicaId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "replicaPriority", &VideoServer::StoreReplicaReplicaPriority, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "timeout", &VideoServer::StoreReplicaTimeout, "", ZQ::common::Config::optReadOnly);

		//load DatabaseCache attribute
		holder.addDetail(DatabaseCacheLayer.c_str(), "volumeSize", &VideoServer::DatabaseCacheVolumeSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSize", &VideoServer::DatabaseCacheContentSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSavePeriod", &VideoServer::DatabaseCacheContentSavePeriod, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSaveSizeTrigger", &VideoServer::DatabaseCacheContentSaveSizeTrigger, "", ZQ::common::Config::optReadOnly);

		////load Log attribute
		//holder.addDetail(LogLayer.c_str(), "path", &VideoServer::LogPath, NULL, ZQ::common::Config::optReadOnly);
		//holder.addDetail(LogLayer.c_str(), "level", &VideoServer::LogLevel, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(LogLayer.c_str(), "maxCount", &VideoServer::LogMaxCount, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(LogLayer.c_str(), "size", &VideoServer::LogSize, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(LogLayer.c_str(), "bufferSize", &VideoServer::LogBufferSize, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(LogLayer.c_str(), "flushTimeout", &VideoServer::LogFlushTimeout, "", ZQ::common::Config::optReadOnly);

		////load EventLog attribute
		//holder.addDetail(EventLogLayer.c_str(), "path", &VideoServer::EventLogPath, NULL, ZQ::common::Config::optReadOnly);
		//holder.addDetail(EventLogLayer.c_str(), "level", &VideoServer::EventLogLevel, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(EventLogLayer.c_str(), "maxCount", &VideoServer::EventLogMaxCount, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(EventLogLayer.c_str(), "size", &VideoServer::EventLogSize, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(EventLogLayer.c_str(), "bufferSize", &VideoServer::EventLogBufferSize, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(EventLogLayer.c_str(), "flushTimeout", &VideoServer::EventLogFlushTimeout, "", ZQ::common::Config::optReadOnly);

		//load Volumes attribute
		holder.addDetail(VolLayer.c_str(), &VideoServer::readVol, &VideoServer::registerVol);
	}

	static void parseSpeedStr(std::string &strSpeed, std::vector<float> &speedSet)
	{
		if (strSpeed.empty())
			return;
		size_t beginIdx = 0;
		size_t endIdx;
		endIdx = strSpeed.find_first_of(strBlank, beginIdx);
		while (endIdx != std::string::npos)
		{
			std::string value = strSpeed.substr(beginIdx, endIdx - beginIdx);
			speedSet.push_back(atof(value.c_str()));
			endIdx++;
			beginIdx = endIdx;
			endIdx = strSpeed.find_first_of(strBlank, beginIdx);
		}
		if (beginIdx + 1 != strSpeed.length())
		{
			std::string value = strSpeed.substr(beginIdx);
			speedSet.push_back(atof(value.c_str()));
		}
	}
};//struct VideoServer

struct NSSBaseConfig
{
	struct NSS
	{
		typedef ::ZQ::common::Config::Holder< IceProperties >	IcePropertiesHolder;
		typedef ::ZQ::common::Config::Holder< PublishedLogs >	PublishedLogHolder;
		typedef ::ZQ::common::Config::Holder< stBind >			BindHodler;
		//typedef ::ZQ::common::Config::Holder< A3JMSEvents >		A3JMSEventsHolder;
		typedef ::ZQ::common::Config::Holder< VideoServer >		VideoServerHolder;

		::std::string netId;
		IcePropertiesHolder _iceProperty;
		PublishedLogHolder	_publishedLog;
		BindHodler			_bind;
		//A3JMSEventsHolder	_a3JMSEvents;
		VideoServerHolder	_videoServer;

		static void structure(::ZQ::common::Config::Holder< NSS > &holder)
		{
			//load NSS
			holder.addDetail("", "netId", &NSS::netId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", &readIceProperty, &registerIceProperty);
			holder.addDetail("", &readPublishedLog, &registerPublishedLog);
			holder.addDetail("", &readBind, &registerBind);
			//holder.addDetail("", &readA3JMSEvents, &registerA3JMSEvents);
			holder.addDetail("", &readVideoServer, &registerVideoServer);
		}

		void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			IcePropertiesHolder holder("");
			holder.read(node, hPP);
			_iceProperty = holder;
		}
		void registerIceProperty(const std::string &full_path)
		{
			_iceProperty.snmpRegister(full_path);
		}

		void readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			PublishedLogHolder holder("");
			holder.read(node, hPP);
			_publishedLog = holder;
		}
		void registerPublishedLog(const std::string &full_path)
		{
			_publishedLog.snmpRegister(full_path);
		}

		void readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			BindHodler holder("");
			holder.read(node, hPP);
			_bind = holder;
		}
		void registerBind(const std::string &full_path)
		{
			_bind.snmpRegister(full_path);
		}

		//void readA3JMSEvents(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		//{
		//	A3JMSEventsHolder holder("");
		//	holder.read(node, hPP);
		//	_a3JMSEvents = holder;
		//}
		//void registerA3JMSEvents(const std::string &full_path)
		//{
		//	_a3JMSEvents.snmpRegister(full_path);
		//}

		void readVideoServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			VideoServerHolder holder("");
			holder.read(node, hPP);
			_videoServer = holder;
		}
		void registerVideoServer(const std::string &full_path)
		{
			_videoServer.snmpRegister(full_path);
		}
	};

	typedef ::ZQ::common::Config::Holder< NSS > NSSHolder;
	typedef ::std::vector< NSSHolder > NSSHolderVec;
	NSSHolderVec NSSVec;

	static void structure(::ZQ::common::Config::Holder< NSSBaseConfig > &holder)
	{
		//load NSS
		holder.addDetail(NSSLayer.c_str(), &readNSS, &registerNSS);
	}

	void readNSS(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		NSSHolder holder("");
		holder.read(node, hPP);
		NSSVec.push_back(holder);
	}
	void registerNSS(const std::string &full_path)
	{
		for (NSSHolderVec::iterator it = NSSVec.begin(); it != NSSVec.end(); it++)
			it->snmpRegister(full_path);
	}
};

class NSSConfig
{
public:
	NSSConfig(const char *filepath);
	virtual ~NSSConfig();

	void ConfigLoader();
	
	//default config
	ZQ::common::Config::Loader< CrashDump >			_crashDump;
	ZQ::common::Config::Loader< IceTrace >			_iceTrace;
	ZQ::common::Config::Loader< IceStormConfig >	_iceStorm;
	//ZQ::common::Config::Loader< IceProperties >		_iceProperties;
	ZQ::common::Config::Loader< Database >			_dataBase;
	//ZQ::common::Config::Loader< PublishedLogs >		_publishedLogs;

	//NSS config
	//ZQ::common::Config::Loader< stBind >			_stBind;
	ZQ::common::Config::Loader< NSSBaseConfig >		_NSSBaseConfig;

private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace NSS

}//namespace ZQTianShan

#endif __ZQTianShan_NSSConfig_H__