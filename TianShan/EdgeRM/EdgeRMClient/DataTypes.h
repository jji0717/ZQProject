#ifndef __ErmManWeb_DataTypes_H__
#define __ErmManWeb_DataTypes_H__

#include "ZQ_common_conf.h"
#include <string>
#include <vector>
#include <Ice/Ice.h>
#include "TsEdgeResource.h"

#define EdgeDeviceInfo     TianShanIce::ObjectInfo
#define EdgeChannelInfo    TianShanIce::StatedObjInfo
#define AllocationInfo     TianShanIce::StatedObjInfo

typedef TianShanIce::EdgeResource::EdgeDeviceInfos::iterator DeviceInfos_iter;
typedef TianShanIce::EdgeResource::EdgeChannelInfos::iterator ChannelInfos_iter;
typedef TianShanIce::EdgeResource::AllocationInfos::iterator AllocationInfos_iter;
typedef TianShanIce::EdgeResource::EdgePortInfos::iterator PortInfos_iter;

#define	Zone						SYS_PROP(Zone)
#define	Device					    SYS_PROP(Name)
#define	Vendor					    SYS_PROP(Vendor)
#define Model					    SYS_PROP(Model)
#define	Description					SYS_PROP(Desc)
#define	TFTP				        SYS_PROP(Tftp)
#define	AdminUrl					SYS_PROP(AdminUrl)
#define	Qam						    SYS_PROP(qam)
#define Admin					    SYS_PROP(admin-state)
#define Freq				        SYS_PROP(Freq)
#define	Power				        SYS_PROP(PowerLevel)
#define Level					    SYS_PROP(interleaver-level)
#define TSID					    SYS_PROP(TSID)
#define PAT_Interval				SYS_PROP(IntervalPAT)
#define PMT_Interval				SYS_PROP(IntervalPMT)
#define NITPID                      SYS_PROP(NITPID)
#define Enabled                     SYS_PROP(Enabled)
#define Address					    SYS_PROP(video-module-address)
#define StartUDP					SYS_PROP(StartUDPPort)
#define PN                          SYS_PROP(StartProgramNumber)
#define MaxSessions                 SYS_PROP(MaxSessions)
#define	UdpSBP				        SYS_PROP(UdpPortStepByPn)
#define LBandWidth					SYS_PROP(LowBandwidthUtilization)
#define HBandWidth					SYS_PROP(HighBandwidthUtilization)
#define RF							SYS_PROP(FreqRF)
#define LastUpdated				    SYS_PROP(StampLastUpdated)

#define UDP                         SYS_PROP(UdpPort)
#define BandWidth					SYS_PROP(Bandwidth)
#define ProgramNumber               SYS_PROP(ProgramNumber)
#define SourceIP				    SYS_PROP(SourceIP)
#define Status					    SYS_PROP(status)
#define StartTime				    SYS_PROP(start-time)
#define MaximumJitter				SYS_PROP(MaxJitter)
#define StpCreated				    SYS_PROP(StampCreated)
#define StpProvisioned				SYS_PROP(StampProvisioned)
#define StpCommitted				SYS_PROP(StampCommitted)
#define Expire				        SYS_PROP(Expiration)
#define Modulation					"modulationFormat"
#define SymRate				        "symbolRate"
#define Fec			                "FEC"
#define Depth						"interleaveDepth"
#define Mode				        "modulationMode"
#define EDeviceName				    "edgeDeviceName"
#define EDeviceIP			        "edgeDeviceIP"
#define EDeviceGroup				"edgeDeviceZone"

enum LevelCode { eLevel0x01 = 1, eLevel0x02, eLevelMax };

enum ModeCode { eMode0x01 = 1, eMode0x02, eMode0x03, eMode0x04, eMode0x05, 
					   eMode0x06, eMode0x07, eMode0x08, eMode0x09, eMode0x0a, 
					   eMode0x0b, eMode0x0c, eModeMax };

enum ModulationCode { eModulation0x00 = 0, eModulation0x01, eModulation0x02, eModulation0x03, eModulation0x04, 
						     eModulation0x05, eModulation0x06, eModulation0x07, eModulation0x08, eModulation0x09, 
							 eModulation0x0a, eModulation0x0b, eModulation0x0c, eModulation0x0d, eModulation0x0e,
							 eModulation0x0f, eModulation0x10, eModulation0x11, eModulationMax };

std::string switchModulation(short  modulationFormat);
std::string switchLevel(short  interleaverMode);
std::string switchMode(short  interleaverLevel);
std::string switchFec(short  fec);
std::string switchState(TianShanIce::State state);

int getModeCode(std::string ActionText);
int getLevelCode(std::string ActionText);
int getModulationCode(std::string ActionText);

//bool localTime2TianShanTime(const char* szTime, int64& lTime);
std::string TianShanTime2String(int64 lTime);

/*
struct DeviceInfo
{
	std::string zone;
	std::string name;
	std::string vendor;
	std::string model;
	std::string desc;
	std::string tftp;
	std::string telnet;
};

struct ChannelInfo
{
	std::string name;
	std::string adminState;
	Ice::Float RF;
	int powerLevel;
	std::string modulation;
	std::string level;
	std::string mode;
	Ice::Int TSID;
	Ice::Int PAT;
	Ice::Int PMT;
};


struct SessionInfo
{
	std::string address;
	Ice::Int port;
	std::string output;
	std::string sourceIP;
	Ice::Int bandwidth;
	std::string state;
	std::string status;
	std::string startTime;
	Ice::Float maxJitter;
};

typedef std::vector<DeviceInfo> DeviceInfos;
typedef std::vector<ChannelInfo> ChannelInfos;
typedef std::vector<SessionInfo> SessionInfos;
typedef std::vector<DeviceInfo>::iterator DeviceInfos_iter;
typedef std::vector<ChannelInfo>::iterator ChannelInfos_iter;
typedef std::vector<SessionInfo>::iterator SessionInfos_iter;

//DeviceInfos  deviceInfos;
//extern ChannelInfos channelInfos;
//SessionInfos sessionInfos;
*/
#endif // __ErmManWeb_DataTypes_H__

