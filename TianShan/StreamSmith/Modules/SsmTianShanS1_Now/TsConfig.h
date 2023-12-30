#ifndef __TsConfig_H__
#define __TsConfig_H__

#include "ConfigHelper.h"
#include <string>
#include <map>
#include <vector>

namespace TianShanS1
{

//FixedSpeedSet

struct FixedSpeedSet 
{
	int32			enable;
	int32			enableSpeedLoop;
	int32			errorcode;
	std::string		strForwardSpeeds;
	std::string		strBackwardSpeeds;
	std::vector<float>	forwardSpeeds;
	std::vector<float>	backwardSpeeds;
	typedef ZQ::common::Config::Holder<FixedSpeedSet> FixedSpeedSetHolder;
	static void structure(FixedSpeedSetHolder& holder );
};

//	<IceStorm endpoint="${PrimeIceStormEndpoint}" />

struct IceStorm
{
	std::string _endpoint;

	typedef ZQ::common::Config::Holder<IceStorm> IceStormHolder;

	static void structure(IceStormHolder& holder);
};
typedef IceStorm::IceStormHolder IceStormHolder;

//	<IceTrace enabled="1" level="7" size="50000000" maxCount="10" />

struct IceTrace
{
	int32 _enabled;
	int32 _level;
	int32 _size;
	int32 _maxCount;

	typedef ZQ::common::Config::Holder<IceTrace> IceTraceHolder;

	static void structure(IceTraceHolder& holder);
};
typedef IceTrace::IceTraceHolder IceTraceHolder;

//	<Database path="${TianShanDatabaseDir}" runtimePath="${TianShanDatabaseDir}\runtime" />

struct Database
{
	std::string _path;
	std::string _runtimePath;
    std::string _fatalRecover;
    std::string _checkpointPeriod;
    std::string _saveSizeTrigger;
    std::string _savePeriod;
    int32 _evictorSize;

	typedef ZQ::common::Config::Holder<Database> DatabaseHolder;

	static void structure(DatabaseHolder& holder);
};
typedef Database::DatabaseHolder DatabaseHolder;

//	<LogFile size="50000000" level="7" maxCount="10" bufferSize="8192" />

struct LogFile
{
	int32 _size;
	int32 _level;
	int32 _maxCount;
	int32 _bufferSize;

	typedef ZQ::common::Config::Holder<LogFile> LogFileHolder;

	static void structure(LogFileHolder& holder);
};
typedef LogFile::LogFileHolder LogFileHolder;

//<Announce notifyTransition="1" notifyTrickRestriction="1" notifyItemSkipped="1" notifyStateChanged="1" 
//	notifyScaleChanged="1" notifyBOS="1" notifyEOS="1" notifyExit="1" notifyErrors="1" notifyAds = "0"/>

struct AnnounceCfg
{
	//int32 _notifyTransition;
	//int32 _notifyTrickRestriction;
	int32 _notifyItemSkipped;
	int32 _notifyStateChanged;
	int32 _notifyScaleChanged;
	int32 _notifyBOS;
	int32 _notifyEOS;
	int32 _notifyExit;
	//int32 _notifyErrors;
	int32 _notifyAds;

	typedef ZQ::common::Config::Holder<AnnounceCfg> AnnounceCfgHolder;

	static void structure(AnnounceCfgHolder& holder);
};
typedef AnnounceCfg::AnnounceCfgHolder AnnounceCfgHolder;

//	<SessionManager endpoint="SessionManager:tcp -h ${PartitionServerNetIf} -p 10001" />
struct NodeGroup
{
	int32	_rangeStart;
	int32	_rangeEnd;
	std::string	_sm;

	typedef ZQ::common::Config::Holder<NodeGroup> NodeGroupHolder;

	static void structure(NodeGroupHolder& holder);
};

struct SessionManager
{
	std::string _endpoint;
	std::vector<NodeGroup::NodeGroupHolder> _ngodGroups;

	typedef ZQ::common::Config::Holder<SessionManager> SessionManagerHolder;

	static void structure(SessionManagerHolder& holder);

	void readNodeGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerNodeGroups(const std::string &full_path);
};
typedef SessionManager::SessionManagerHolder SessionManagerHolder;

//	<Bind endpoint="tcp -h ${ServerNetIf}" dispatchSize="5" dispatchMax="30" />

struct Bind
{
	std::string _endpoint;
	int32 _dispatchSize;
	int32 _dispatchMax;

	typedef ZQ::common::Config::Holder<Bind> BindHolder;

	static void structure(BindHolder& holder);
};
typedef Bind::BindHolder BindHolder;

//	<RTSPSession timeout="1200" cacheSize="1000" monitorThreads="5" />

struct RTSPSession
{
	int32 _timeout;
	int32 _cacheSize;
	int32 _monitorThreads;
	int32 _setupMsgTimeoutInterval;
	int32 _maxPendingAnnounce;
	int32 _announceResubscribeAtIdle;
	int32 _pingCacheSize;
	
	RTSPSession()
	{
		_maxPendingAnnounce = 100;
	}

	typedef ZQ::common::Config::Holder<RTSPSession> RTSPSessionHolder;

	static void structure(RTSPSessionHolder& holder);
};
typedef RTSPSession::RTSPSessionHolder RTSPSessionHolder;

//	<DefaultParameter>
//		<param name="Transport" value="MP2T/DVBC/QAM" />
//	</DefaultParameter>

struct DefaultParam
{
	std::string _name;
	std::string _value;

	

	typedef ZQ::common::Config::Holder<DefaultParam> DefaultParamHolder;

	static void structure(DefaultParamHolder& holder);
};
typedef DefaultParam::DefaultParamHolder DefaultParamHolder;

struct DefaultParams
{
	std::string _axiomMsgDefaultPath;

	std::vector<DefaultParamHolder> _paramDatas;

	typedef ZQ::common::Config::Holder<DefaultParams> DefaultParamsHolder;

	static void structure(DefaultParamsHolder& holder);

    void readDefaultParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerDefaultParams(const std::string &full_path);
};
typedef DefaultParams::DefaultParamsHolder DefaultParamsHolder;

//	<Response maxFieldLen="150" />

struct Response
{
	int32 _maxFieldLen, _maxResponseTimeout;
	int32 _dummyEndNpt;
	int32 _exportPlaylist;

	typedef ZQ::common::Config::Holder<Response> ResponseHolder;

	static void structure(ResponseHolder& holder);
};
typedef Response::ResponseHolder ResponseHolder;

//	<DoSProtection timeWindow="10" maxRequests="100" />

struct DoSProtection
{
	int32 _timeWindow;
	int32 _maxRequests;

	typedef ZQ::common::Config::Holder<DoSProtection> DoSProtectionHolder;

	static void structure(DoSProtectionHolder& holder);
};
typedef DoSProtection::DoSProtectionHolder DoSProtectionHolder;

//	<IceProperties>
//		<prop name="Ice.Trace.Network"                    value="0" /> <!--  //!!! former pmProp  -->
//		<prop name="Ice.Logger.Timestamp"                 value="1" />
//		<prop name="Ice.Override.Timeout"                 value="5000"/>
//		<prop name="Ice.Override.ConnectTimeout"          value="5000"/>
//		<prop name="Ice.ThreadPool.Server.Size"           value="5"/>
//		<prop name="Ice.ThreadPool.Server.SizeMax"        value="15"/>
//		<prop name="Ice.ThreadPool.Client.Size"           value="5"/>
//		<prop name="Ice.ThreadPool.Client.SizeMax"        value="15"/>
//	</IceProperties>

struct IceProperty
{
	std::string _name;
	std::string _value;

	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;

	static void structure(IcePropertyHolder& holder);
};
typedef IceProperty::IcePropertyHolder IcePropertyHolder;

struct IcePropertys
{
	std::vector<IceProperty::IcePropertyHolder> _propDatas;

	typedef ZQ::common::Config::Holder<IcePropertys> IcePropertysHolder;

	static void structure(IcePropertysHolder& holder);

    void readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerIcePropertys(const std::string &full_path);
};
typedef IcePropertys::IcePropertysHolder IcePropertysHolder;

struct LogPublish
{
	std::string		logPath;
	std::string		syntaxPath;
	std::string		key;
	std::string		type;
	static void structure(  ZQ::common::Config::Holder<LogPublish>& holder );	
};

struct StreamCtrl
{
    typedef ZQ::common::Config::Holder<StreamCtrl> StreamCtrlHolder;

    std::string     exportEndpoint;
    static void structure(  ZQ::common::Config::Holder<StreamCtrl>& holder );	
};
typedef StreamCtrl::StreamCtrlHolder StreamCtrlHolder;

struct VirtualServiceGroup
{
	typedef ZQ::common::Config::Holder<VirtualServiceGroup> VirtualServiceGroupHolder;
	std::string		group;
	static void structure ( ZQ::common::Config::Holder<VirtualServiceGroup>& holder );
};
typedef VirtualServiceGroup::VirtualServiceGroupHolder VirtualServiceGroupHolder;

struct SessionGroup
{
	std::string expression;
	std::string group;

	typedef ZQ::common::Config::Holder<SessionGroup> SessionGroupHolder;

	static void structure(SessionGroupHolder& holder);
};
typedef SessionGroup::SessionGroupHolder SessionGroupHolder;

struct IpSubnet
{
	std::string ip;
	std::string mask;
	std::string group;

	typedef ZQ::common::Config::Holder<IpSubnet> IpSubnetHolder;
	static void structure(IpSubnetHolder& holder);
};
typedef IpSubnet::IpSubnetHolder IpSubnetHolder;

struct SessionGroups
{
	std::vector<SessionGroup::SessionGroupHolder> sessionGroups;
	std::vector<IpSubnet::IpSubnetHolder> ipSubnets;
	int32 defaultServiceGroup;

	typedef ZQ::common::Config::Holder<SessionGroups> SessionGroupsHolder;

	static void structure(SessionGroupsHolder& holder);

	void readSessionGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSessionGroups(const std::string &full_path);

	void readIpSubnets(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIpSubnets(const std::string &full_path);
};
typedef SessionGroups::SessionGroupsHolder SessionGroupsHolder;

struct NGODS1
{
	std::string defaultAppPath;
    StreamCtrlHolder    _streamCtrl;
	SessionGroupsHolder _sessionGroups;
	
	typedef ZQ::common::Config::Holder<NGODS1> NGODS1Holder;

	static void structure(NGODS1Holder& holder);

	void readSessionGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerSessionGroups(const std::string &full_path);
    void readStreamCtrl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerStreamCtrl(const std::string &full_path);
};
typedef NGODS1::NGODS1Holder NGODS1Holder;

// <C2Stream enable="0" sessionTTL="14400000" keyfile="${TianShanConfDir}/keyfile.xml" defaultC2Server="192.168.81.2" defaultC2Port="12000" />

struct C2Stream
{
	int32 enable;
	int32 sessionTTL;
	std::string keyfile;
	std::string defaultC2Server;
	int32       defaultC2Port;

	typedef ZQ::common::Config::Holder<C2Stream> C2StreamHolder;

	static void structure(C2StreamHolder& holder);
};
typedef C2Stream::C2StreamHolder C2StreamHolder;

struct TSConfig
{
	IceStormHolder _iceStorm;
	IceTraceHolder _iceTrace;
	DatabaseHolder _database;
	LogFileHolder _pluginLog;
	AnnounceCfgHolder _announce;
	SessionManagerHolder _sessionManager;
	BindHolder _bind;
	RTSPSessionHolder _rtspSession;
	DefaultParamsHolder _defaultParams;
	ResponseHolder _response;
	DoSProtectionHolder _doSProtect;
	IcePropertysHolder _iceProps;
	VirtualServiceGroupHolder _virtualServiceGroup;
	FixedSpeedSet::FixedSpeedSetHolder		_fixedSpeedSet;
	NGODS1Holder	_ngods1;
	C2StreamHolder  _c2stream;
	std::string     _NetId;
	int             scale0Paused;
	
	typedef std::vector< ZQ::common::Config::Holder<LogPublish> >		LogPublishs;

	LogPublishs			_logpublish;

	typedef ZQ::common::Config::Holder<TSConfig> TSConfigHolder;

	static void structure(TSConfigHolder& holder);
	
	void readFixedSpeedSet( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );

	void regsiterFixedSpeedSet(const std::string& full_path );

	void readIceStrom(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceStrom(const std::string &full_path);

	void readIceTrace(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceTrace(const std::string &full_path);

	void readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerDatabase(const std::string &full_path);

	void readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLogFile(const std::string &full_path);

	void readAnnounceCfg(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerAnnounceCfg(const std::string &full_path);

	void readSessionManager(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSessionManager(const std::string &full_path);

	void readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerBind(const std::string &full_path);

	void readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerRTSPSession(const std::string &full_path);

	void readDefaultParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerDefaultParams(const std::string &full_path);

	void readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerResponse(const std::string &full_path);

	void readDoSProtection(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerDoSProtection(const std::string &full_path);

	void readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIcePropertys(const std::string &full_path);

	void readLogPublish( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void readVirtualServiceGroup(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerVirtualServiceGroup(const std::string &full_path);

	void readNGODS1(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerNGODS1(const std::string &full_path);

	void readC2Stream(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerC2Stream(const std::string &full_path);
};

}

extern ZQ::common::Config::Loader<TianShanS1::TSConfig> _tsConfig;

#endif // #define __TsConfig_H__

