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
// Ident : $Id: GBVSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/GB-SAC/GBvss/GBVSSConfig.h $
// 
// 1     6/23/11 4:15p Xiaohui.chai
// 
// 8     5/03/11 6:16p Fei.huang
// + migrated to linux
// 
// 7     4/27/11 3:08p Haoyuan.lu
// 
// 6     4/27/11 2:21p Fei.huang
// 
// 5     3/11/11 5:18p Haoyuan.lu
// 
// 4     2/09/11 1:59p Haoyuan.lu
// 
// 3     1/28/11 5:23p Haoyuan.lu
// 
// 2     1/19/11 5:00p Haoyuan.lu
// 
// 1     1/10/11 2:40p Haoyuan.lu
// 
// 11    08-12-18 11:39 Xiaoming.li
// correct config file format
// 
// 10    08-12-15 15:29 Xiaoming.li
// add snmp, change config element from ngod_GBVSS to GBVSS
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

#ifndef __ZQTianShan_GBVSSConfig_H__
#define __ZQTianShan_GBVSSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>
#include <set>

namespace ZQTianShan{

namespace GBVSS{

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
		static ::std::string Layer = IcePropertiesLayer + Slash + PropLayer;
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
		static ::std::string Layer = PublishedLogsLayer + Slash + PublishLogLayer;
		holder.addDetail(Layer.c_str(), &PublishedLogs::readPublishedLogs, &PublishedLogs::registerPublishedLogs);
	}
};

//configuration of ngod_GBVSS
static ::std::string GBVSSLayer = "GBVSS";

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
		static ::std::string Layer = GBVSSLayer + Slash + LogFileLayer;
		holder.addDetail(Layer.c_str(), "path", &LogFile::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &LogFile::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//bind
static ::std::string GBVSSBindLayer = "Bind";
struct stBind
{
	::std::string	endPoint;
	int32			dispatchSize;
	int32			dispatchMax;
	int32			evictorSize;
	int32			threadPoolSize;
	static void structure(ZQ::common::Config::Holder< stBind > &holder)
	{
		static ::std::string Layer = GBVSSBindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string GBVSSIceLogLayer = "IceLog";
struct GBVSSIceLog
{
	::std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	bufferSize;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< GBVSSIceLog > &holder)
	{
		static ::std::string Layer = GBVSSIceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &GBVSSIceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &GBVSSIceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &GBVSSIceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &GBVSSIceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &GBVSSIceLog::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &GBVSSIceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//definition of timeout
static ::std::string sessionHistory = "SessionHistory";

struct SessionHistory
{
	int32	enable;
	::std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			bufferSize;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< SessionHistory > &holder)
	{
		static ::std::string Layer =  sessionHistory;
		holder.addDetail(Layer.c_str(), "enable", &SessionHistory::enable, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "path", &SessionHistory::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &SessionHistory::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &SessionHistory::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &SessionHistory::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &SessionHistory::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &SessionHistory::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string postEvent = "PostEvent";

struct PostEvent
{
	int32	enableScaleChangeEvent;
	int32	enableStateChangeEvent;
	static void structure(ZQ::common::Config::Holder< PostEvent > &holder)
	{
		static ::std::string Layer =  postEvent;
		holder.addDetail(Layer.c_str(), "enableScaleChangeEvent", &PostEvent::enableScaleChangeEvent, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enableStateChangeEvent", &PostEvent::enableStateChangeEvent, "1", ZQ::common::Config::optReadOnly);
	}
};

//definition about MediaCluster
static ::std::string GBVSSVideoServerLayer = "VideoServer";
//static ::std::string VideoServerLayer = GBVSSLayer + Slash + GBVSSVideoServerLayer;
static ::std::string VideoServerLayer = GBVSSVideoServerLayer;
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
static ::std::string GBVSSStreamerLayer = SessionInterfaceLayer + Slash + "Streamer";
static ::std::string CDNLayer = VideoServerLayer + Slash + "CDN";
struct VideoServer
{
	//::std::string	netId;
	::std::string	vendor;
	::std::string	model;
	int32			enableMessageBinaryDump;
	int32			streamSyncInterval;
	int32			sessionRenewInterval;


	//session interface
	::std::string	SessionInterfaceIp;
	int32			SessionInterfacePort;
	int32			SessionInterfaceMaxSessionGroup;
	int32			SessionInterfaceMaxSessionsPerGroup;
	int32			SessionInterfaceRequestTimeout;
	int32			SessionInterfaceDisconnectAtTimeout;

	//FixedSpeedSet
	int32				FixedSpeedSetEnable;
	int32				EnableFixedSpeedLoop; // default is 1
	std::string			FixedSpeedSetForward;
	std::string			FixedSpeedSetBackward;
	std::vector<float>	FixedSpeedSetForwardSet;
	std::vector<float>	FixedSpeedSetBackwardSet;
/* CS
	//content interface
	::std::string	ContentInterfaceIp;
	int32			ContentInterfacePort;
	::std::string	ContentInterfacePath;
	int32			ContentInterfaceSyncInterval;
	int32			ContentInterfaceSyncRetry;
	::std::string	ContentInterfaceMode;
	int32			ContentInterfaceHttpTimeOut;
	int32			ContentInterfaceDestroyEnable;
	int32			urlPercentDecodeOnOutgoingMsg;
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

    */
	// <CDN>
	::std::string	libraryVolume;

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


	struct GBVSSStreamer 
	{
		std::string			streamerName;
		static void structure( ZQ::common::Config::Holder<GBVSSStreamer>& holder )
		{
			holder.addDetail( "" , "name",&GBVSSStreamer::streamerName,"",ZQ::common::Config::optReadOnly );
		}
	};
	typedef ZQ::common::Config::Holder<GBVSSStreamer>		GBVSSStreamerHolder;
/*
	struct GBVSSStreamerHolderCmp
	{
		bool operator() (const  GBVSSStreamerHolder& a, const GBVSSStreamerHolder& b ) const
		{
			return a.streamerName < b.streamerName;
		}
	};
*/

//	typedef std::set< GBVSSStreamerHolder, GBVSSStreamerHolderCmp >		GBVSSStreamerHolderSet;
	typedef std::vector< GBVSSStreamerHolder >		GBVSSStreamerHolderSet;
	GBVSSStreamerHolderSet	streamerSet;

	void readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		GBVSSStreamerHolder streamerHolder("");
		streamerHolder.read(node, hPP);
	//	streamerSet.insert(streamerHolder);
		streamerSet.push_back(streamerHolder);
	}
	void registerStreamer(const std::string &full_path) 
	{
		for (GBVSSStreamerHolderSet::iterator it = streamerSet.begin(); it != streamerSet.end(); ++it)
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
		holder.addDetail(VideoServerLayer.c_str(), "enableMessageBinaryDump", &VideoServer::enableMessageBinaryDump, "0", ZQ::common::Config::optReadWrite);
		holder.addDetail(VideoServerLayer.c_str(), "streamSyncInterval", &VideoServer::streamSyncInterval, "3600", ZQ::common::Config::optReadWrite);
		holder.addDetail(VideoServerLayer.c_str(), "sessionRenewInterval", &VideoServer::sessionRenewInterval, "600", ZQ::common::Config::optReadWrite);


		//load SessionInterface attribute
		holder.addDetail(SessionInterfaceLayer.c_str(), "ip", &VideoServer::SessionInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "port", &VideoServer::SessionInterfacePort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "maxSessionGroup", &VideoServer::SessionInterfaceMaxSessionGroup, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "maxSessionsPerGroup", &VideoServer::SessionInterfaceMaxSessionsPerGroup, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "requestTimeout", &VideoServer::SessionInterfaceRequestTimeout, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(SessionInterfaceLayer.c_str(), "disconnectAtTimeout", &VideoServer::SessionInterfaceDisconnectAtTimeout,"20",ZQ::common::Config::optReadOnly);
		holder.addDetail(GBVSSStreamerLayer, &VideoServer::readStreamer, &VideoServer::registerStreamer);

		//load FixedSpeedSet attribute
		holder.addDetail(FixedSpeedSetLayer.c_str(), "enable", &VideoServer::FixedSpeedSetEnable, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(FixedSpeedSetLayer.c_str(), "enableSpeedLoop", &VideoServer::EnableFixedSpeedLoop, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail(FixedSpeedSetLayer.c_str(), "forward", &VideoServer::FixedSpeedSetForward, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(FixedSpeedSetLayer.c_str(), "backward", &VideoServer::FixedSpeedSetBackward, NULL, ZQ::common::Config::optReadOnly);
/* CS
		holder.addDetail(ContentInterfaceLayer.c_str(), "ip", &VideoServer::ContentInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "port", &VideoServer::ContentInterfacePort, "8080", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "path", &VideoServer::ContentInterfacePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "syncInterval", &VideoServer::ContentInterfaceSyncInterval, "200000", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "syncRetry", &VideoServer::ContentInterfaceSyncRetry, "3", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "mode", &VideoServer::ContentInterfaceMode, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "httpTimeOut", &VideoServer::ContentInterfaceHttpTimeOut, "200000", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "destroyEnable", &VideoServer::ContentInterfaceDestroyEnable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "urlPercentDecodeOnOutgoingMsg", &VideoServer::urlPercentDecodeOnOutgoingMsg, "0", ZQ::common::Config::optReadOnly);

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

		//load Volumes attribute
		holder.addDetail(VolLayer.c_str(), &VideoServer::readVol, &VideoServer::registerVol);
*/
		// <VideoServer> <CDN /> </VideoServer>
		holder.addDetail(CDNLayer.c_str(), "libraryVolume", &VideoServer::libraryVolume, "library", ZQ::common::Config::optReadOnly);
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
			speedSet.push_back( (float)atof(value.c_str()));
			endIdx++;
			beginIdx = endIdx;
			endIdx = strSpeed.find_first_of(strBlank, beginIdx);
		}
		if (beginIdx + 1 != strSpeed.length())
		{
			std::string value = strSpeed.substr(beginIdx);
			speedSet.push_back( (float)atof(value.c_str()));
		}
	}
};//struct VideoServer

struct GBVSSBaseConfig
{
	struct GBVSS
	{
		typedef ::ZQ::common::Config::Holder< IceProperties >	IcePropertiesHolder;
		typedef ::ZQ::common::Config::Holder< PublishedLogs >	PublishedLogHolder;
		typedef ::ZQ::common::Config::Holder< stBind >			BindHodler;
		typedef ::ZQ::common::Config::Holder< VideoServer >		VideoServerHolder;
		typedef ::ZQ::common::Config::Holder< SessionHistory >	SessionHistoryHolder;
		typedef ::ZQ::common::Config::Holder< PostEvent >		PostEventHolder;

		::std::string			netId;
		int32					_ForceRefOnly;
		IcePropertiesHolder		_iceProperty;
		PublishedLogHolder		_publishedLog;
		BindHodler				_bind;
		VideoServerHolder		_videoServer;
		SessionHistoryHolder	_sessionHistory;
		PostEventHolder			_postEvent;

		static void structure(::ZQ::common::Config::Holder< GBVSS > &holder)
		{
			//load GBVSS
			holder.addDetail("", "netId", &GBVSS::netId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("","forceRefOnly",&GBVSS::_ForceRefOnly,"0",ZQ::common::Config::optReadOnly);
			holder.addDetail("", &GBVSS::readIceProperty, &GBVSS::registerIceProperty);
			holder.addDetail("", &GBVSS::readPublishedLog, &GBVSS::registerPublishedLog);
			holder.addDetail("", &GBVSS::readBind, &GBVSS::registerBind);
			holder.addDetail("", &GBVSS::readVideoServer, &GBVSS::registerVideoServer);
			holder.addDetail("", &GBVSS::readsessionHistory, &GBVSS::registerstsessionHistory);
			holder.addDetail("", &GBVSS::readPostEvent, &GBVSS::registerstPostEvent);
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

		void readsessionHistory(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<SessionHistory> nvholder("");
			nvholder.read(node, hPP);
			_sessionHistory = nvholder;
		}
		void registerstsessionHistory(const std::string &full_path)
		{
			_sessionHistory.snmpRegister(full_path);
		}

		void readPostEvent(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			PostEventHolder peholder("");
			peholder.read(node, hPP);
			_postEvent = peholder;
		}
		void registerstPostEvent(const std::string &full_path)
		{
			_postEvent.snmpRegister(full_path);
		}
	};

	typedef ::ZQ::common::Config::Holder< GBVSS > GBVSSHolder;
	typedef ::std::vector< GBVSSHolder > GBVSSHolderVec;
	GBVSSHolderVec GBVSSVec;

	static void structure(::ZQ::common::Config::Holder< GBVSSBaseConfig > &holder)
	{
		//load GBVSS
		holder.addDetail(GBVSSLayer.c_str(), &GBVSSBaseConfig::readGBVSS, &GBVSSBaseConfig::registerGBVSS);
	}

	void readGBVSS(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		GBVSSHolder holder("");
		holder.read(node, hPP);
		GBVSSVec.push_back(holder);
	}
	void registerGBVSS(const std::string &full_path)
	{
		for (GBVSSHolderVec::iterator it = GBVSSVec.begin(); it != GBVSSVec.end(); it++)
			it->snmpRegister(full_path);
	}
};


class GBVSSConfig
{
public:
	GBVSSConfig(const char *filepath);
	virtual ~GBVSSConfig();

	void ConfigLoader();

	//default config
	ZQ::common::Config::Loader< CrashDump >			_crashDump;
	ZQ::common::Config::Loader< IceTrace >			_iceTrace;
	ZQ::common::Config::Loader< IceStorm >		    _iceStorm;
	ZQ::common::Config::Loader< Database >			_dataBase;

	//GBVSS config
	ZQ::common::Config::Loader< GBVSSBaseConfig >		_GBVSSBaseConfig;


private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace GBVSS

}//namespace ZQTianShan

#endif //__ZQTianShan_GBVSSConfig_H__
