#ifndef __ZQTianShan_A3Config_H__
#define __ZQTianShan_A3Config_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace ZQTianShan{

namespace A3{

//root configuration

//global configuration

//default configuration
static ::std::string A3ServerLayer = "A3Server";
static ::std::string Slash ="/";
static ::std::string A3LogLayer = "Log";

struct A3Log
{
	::std::string path;
	int32	level;
	int32	logNum;
	int32	size;
	int32	buffer;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< A3Log > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3LogLayer;
		holder.addDetail(Layer.c_str(), "path", &A3Log::path, NULL);
		holder.addDetail(Layer.c_str(), "level", &A3Log::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "logNum", &A3Log::logNum, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &A3Log::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &A3Log::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &A3Log::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string A3EventLogLayer = "EventLog";

struct A3EventLog
{
	::std::string path;
	int32	level;
	int32	logNum;
	int32	size;
	int32	buffer;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< A3EventLog > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3EventLogLayer;
		holder.addDetail(Layer.c_str(), "path", &A3EventLog::path, NULL);
		holder.addDetail(Layer.c_str(), "level", &A3EventLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "logNum", &A3EventLog::logNum, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &A3EventLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "buffer", &A3EventLog::buffer, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &A3EventLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

static std::string A3StoreInfoLayer = "StoreInfo";
struct A3StoreInfo
{
	std::string netId;
	std::string type;
	std::string streamableLength;

	std::string groupId;
	std::string replicaId;
	int			replicaPriority;
	int			timeout;
	int			contentSize;
	int			volumeSize;

	static void structure(ZQ::common::Config::Holder< A3StoreInfo > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3StoreInfoLayer;
		holder.addDetail(Layer.c_str(), "netId", &A3StoreInfo::netId, NULL);
		holder.addDetail(Layer.c_str(), "type", &A3StoreInfo::type, NULL);
		holder.addDetail(Layer.c_str(), "streamableLength", &A3StoreInfo::streamableLength,NULL);


		Layer += Slash + "StoreReplica";
		holder.addDetail(Layer.c_str(), "groupId", &A3StoreInfo::groupId, NULL);
		holder.addDetail(Layer.c_str(), "replicaId", &A3StoreInfo::replicaId, NULL);
		holder.addDetail(Layer.c_str(), "replicaPriority", &A3StoreInfo::replicaPriority, NULL);
		holder.addDetail(Layer.c_str(), "timeout", &A3StoreInfo::timeout, NULL);
		holder.addDetail(Layer.c_str(), "contentSize", &A3StoreInfo::contentSize, NULL);
		holder.addDetail(Layer.c_str(), "volumeSize", &A3StoreInfo::volumeSize, NULL);
	}
};


static ::std::string A3LocalInfoLayer = "LocalInfo";
struct A3LocalInfo
{
	::std::string ip;
	int32	port;
	static void structure(ZQ::common::Config::Holder< A3LocalInfo > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3LocalInfoLayer;
		holder.addDetail(Layer.c_str(), "ip", &A3LocalInfo::ip, NULL);
		holder.addDetail(Layer.c_str(), "port", &A3LocalInfo::port, "", ZQ::common::Config::optReadOnly);
	}

};

static ::std::string A3ServerInfoLayer = "ServerInfo";
struct A3ServerInfo
{
	::std::string ip;
	int32	port;
	static void structure(ZQ::common::Config::Holder< A3ServerInfo > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3ServerInfoLayer;
		holder.addDetail(Layer.c_str(), "ip", &A3ServerInfo::ip, NULL);
		holder.addDetail(Layer.c_str(), "port", &A3ServerInfo::port, "", ZQ::common::Config::optReadOnly);
	}

};

static ::std::string A3VolumeInfoLayer = "VolumeInfo";
struct A3VolumeInfo
{
	::std::string name;
	::std::string path;
	static void structure(ZQ::common::Config::Holder< A3VolumeInfo > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3VolumeInfoLayer;
		holder.addDetail(Layer.c_str(), "name", &A3VolumeInfo::name, NULL);
		holder.addDetail(Layer.c_str(), "path", &A3VolumeInfo::path, NULL);
	}

};

static ::std::string A3HeartBeatLayer = "HeartBeat";
struct A3HeartBeat
{
	int32	interval;
	static void structure(ZQ::common::Config::Holder< A3HeartBeat > &holder)
	{
		static ::std::string Layer = A3ServerLayer + Slash + A3HeartBeatLayer;
		holder.addDetail(Layer.c_str(), "interval", &A3HeartBeat::interval, "", ZQ::common::Config::optReadOnly);
	}

};

}//namespace A3

}//namespace ZQTianShan

#endif __ZQTianShan_A3Config_H__