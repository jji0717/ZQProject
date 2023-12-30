#include <ZQ_common_conf.h>
#include <assert.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <arpa/inet.h>
#include <SystemUtils.h>
#include "C2SessionHelper.h"
#include "C2StreamerEnv.h"
#include "PortManager.h"
#include "C2StreamerService.h"

namespace C2Streamer
{
PortManager::PortManager( C2StreamerEnv& env  , C2Service& svc)
:mEnv(env),mSvc(svc)
{
	mNlh	= NULL;
	mNlcm	= NULL;
	mLink_nlc = NULL;
	mAddr_nlc = NULL;
}


PortManager::~PortManager(void)
{
}

uint64 PortManager::get_config_portspeed( const std::string& name) const
{
	uint64 capacity = 0;
	const C2EnvConfig& conf 		= getEnvironment()->getConfig();
	const PORTSPEEDCONF& portSpeedMap	= conf.portSpeed;
	PORTSPEEDCONF::const_iterator itPort = portSpeedMap.find( name );
	if( itPort != portSpeedMap.end())
	{
		capacity =  itPort->second.speed;
	}
	return capacity;
}

std::string PortManager::portNameToIp( const std::string& portName ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	std::map<std::string, std::string>::const_iterator it = mPortToIps.find(portName);
	if( it == mPortToIps.end() )
		return std::string("");
	else
		return it->second;
}

std::string PortManager::ipToPortName( const std::string& ip ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	IPTOPORTMAP::const_iterator it =  mIp2Ports.find( ip );
	if( it == mIp2Ports.end() )
	{
		return std::string("");
	}
	else
	{
		return it->second;
	}
}

int64 PortManager::queryBW( const std::string& ip, int64* totalBW ) const
{
	int64 bw = 0;
	int64 total = 0;
	ZQ::common::MutexGuard gd(mMutex);
	if( ip.empty() )
	{
		PORTS::const_iterator it = mPorts.begin();
		for( ; it != mPorts.end() ; it ++ )
		{
			bw += it->second.usedBW;
			if( it->second.bConf )
				total += it->second.capacity;
		}
		if( totalBW ) *totalBW = total;
	}
	else
	{
		PORTS::const_iterator it = mPorts.find( ip );
		if( it == mPorts.end() )
			return 0;
		else
			it->second.usedBW;
	}
	return bw;
}

void PortManager::unregisterSession( const std::string& ip , const std::string& sessId )
{
	std::string portName = ipToPortName( ip );
	if( portName.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(PortManager,"unregisterSession() failed to find portName with ip[%s] for session[%s]"),
			ip.c_str() , sessId.c_str() );
		return;
	}

	ZQ::common::MutexGuard gd(mMutex);
	PORTS::iterator itPort = mPorts.find( portName );
	if( itPort == mPorts.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(PortManager,"unregisterSession() failed to find port with portName[%s] ip[%s] for session[%s]"),
			portName.c_str() , ip.c_str() , sessId.c_str() );
		return;
	}
	PortAttr& portAttr = itPort->second;
	SESSIONATTRS& sessionInfos = portAttr.sessionInfos;
	SESSIONATTRS::iterator itSess = sessionInfos.find( sessId );
	if( itSess == sessionInfos.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(PortManager,"unregisterSession() failed to find session[%s]'s record: ip[%s] portName[%s]"),
			sessId.c_str() , ip.c_str() , portName.c_str() );
		return;
	}
	portAttr.usedBW	-= itSess->second.usedBW;
	portAttr.usedBW = MAX( 0 , portAttr.usedBW );// adjust to >= 0
	portAttr.usedCount --;	

	sessionInfos.erase( itSess );

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PortManager,"unregisterSession() session[%s] unregistered : capacity["FMT64"] usedBW["FMT64"] sessionCount[%lld] ip[%s] portName[%s]"),
		sessId.c_str() , portAttr.capacity , portAttr.usedBW , portAttr.usedCount, ip.c_str() , portName.c_str() );

}
struct PortAvailInfo
{
	std::string portName;
	int weight;
	PortAvailInfo()
		:weight(0){
	}
	bool operator<(const PortAvailInfo& b) const
	{
		 return weight < b.weight;
	}
};

std::vector<std::string> PortManager::getAvailPorts(int64 bw, const std::string& sessId) const
{
	std::vector<std::string> ports;
	std::vector<PortAvailInfo> availPorts;
	ZQ::common::MutexGuard gd(mMutex);
	PORTS::const_iterator it = mPorts.begin();
	for( ; it != mPorts.end(); it ++)
	{
		const PortAttr& attr = it->second;

		std::string ip;
		if( attr.ipv6.empty() && attr.ipv4.empty())
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PortManager,"getAvailPorts(), do not select [%s] due to no ip address"),
					it->first.c_str() );
			continue;
		}
		ip = attr.ipv4.empty() ? attr.ipv6[0]: attr.ipv4[0];

		int64 natBW = mSvc.getC2TunnelMon().queryBW( ip );
		if( !attr.bUp || attr.capacity <= 0 || attr.capacity < (attr.usedBW + bw + natBW ) )
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PortManager,"getAvailPorts(), do not select [%s] due to:status[%s] capacity[%lld] usedBW[%lld] natBW[%lld] requestBW[%lld]"),
					it->first.c_str(), attr.bUp?"UP":"DOWN", attr.capacity, attr.usedBW, natBW, bw);
			continue;
		}
		PortAvailInfo info;
		info.portName = ip;
		info.weight =  (attr.capacity - attr.usedBW) * 1000 /attr.capacity;
		availPorts.push_back(info);
	}
	std::sort(availPorts.begin(), availPorts.end());
	std::vector<PortAvailInfo>::const_iterator itPort = availPorts.begin();
	for( ; itPort != availPorts.end(); itPort ++)
	{
		ports.push_back(itPort->portName);
	}
	return ports;
}

bool PortManager::registerSession( const std::string& ip , const std::string& sessId , int64 requestBW )
{
	std::string portName = ipToPortName( ip );
	if( portName.empty() )
	{
		MLOG( ZQ::common::Log::L_ERROR, CLOGFMT( PortManager, "registerSession() failed to convert Ip[%s] to port name: session[%s] requestBW["FMT64"]" ) ,
			ip.c_str() , sessId.c_str() , requestBW );
		return false;
	}

	ZQ::common::MutexGuard gd(mMutex);

	PORTS::iterator itPort = mPorts.find( portName );
	if( itPort == mPorts.end() )
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT( PortManager, "registerSession() failed to find port[%s] with ip[%s]: session[%s] requestBW["FMT64"]" ) ,
			portName.c_str() , ip.c_str() , sessId.c_str() , requestBW );
		return false;
	}
	
	if( !itPort->second.bUp )
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT( PortManager , "registerSession() port[%s] is down: ip[%s] session[%s] requestBW["FMT64"]"),
			portName.c_str() , ip.c_str() , sessId.c_str() , requestBW);
		return false;
	}

	PortAttr&		portAttr		= itPort->second;
	SESSIONATTRS& sessionInfos		= itPort->second.sessionInfos;
	assert( sessionInfos.find(sessId) == sessionInfos.end() );

	int64 natBW = mSvc.getC2TunnelMon().queryBW( ip );

	if( portAttr.capacity < (requestBW + portAttr.usedBW + natBW) )
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT( PortManager , "registerSession() not enough bandwidth for session[%s] requstBW["FMT64"] : capacity["FMT64"] usedBW["FMT64"] natBW["FMT64"]"),
			sessId.c_str() , requestBW , portAttr.capacity , portAttr.usedBW, natBW );
		return false;
	}

	portAttr.usedBW	+= requestBW;
	portAttr.usedCount++;
	
	sessionAttr attr;
	attr.sessionId		= sessId;
	attr.usedBW			= requestBW;

	sessionInfos.insert( SESSIONATTRS::value_type( sessId ,attr ) );

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PortManager,"register new session [%s] portName[%s] requestBW["FMT64"]: capacity["FMT64"] usedBW["FMT64"] natBW["FMT64"] sessionCount[%lld] ip[%s] "),
		sessId.c_str() , portName.c_str(), requestBW , portAttr.capacity ,  portAttr.usedBW , natBW, portAttr.usedCount, ip.c_str() );
	return true;
}

bool PortManager::getPortAttr( const std::string& ipOrPortname , PortAttr& attr ) const
{
	std::string portName = ipToPortName( ipOrPortname );
	if( portName.empty() )
		portName = ipOrPortname;
	ZQ::common::MutexGuard gd(mMutex);
	PORTS::const_iterator it = mPorts.find( portName );
	if( it == mPorts.end() )
	{
		return false;
	}
	else
	{
		attr = it->second;
		return true;
	}
}

void PortManager::getPortsAttr( PORTS& ports ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	ports = mPorts;
}

void PortManager::updateIpPortNameMap(  const PortAttr& attr )
{
	ZQ::common::MutexGuard gd(mMutex);
	if( !attr.ipv4.empty() )
	{
		std::vector<std::string>::const_iterator it = attr.ipv4.begin();
		for( ; it != attr.ipv4.end() ; it ++ )
		{
			if( !it->empty() )
			{
				mIp2Ports[*it] = attr.portName;
				mPortToIps[attr.portName] = *it;
			}
		}
	}
	if( !attr.ipv6.empty() )
	{
		std::vector<std::string>::const_iterator it = attr.ipv6.begin();
		for( ; it != attr.ipv6.end() ; it ++ )
		{
			if( !it->empty() )
				mIp2Ports[*it] = attr.portName;
		}
	}
}

void PortManager::updatePort( const PortAttr& attr )
{
	updateIpPortNameMap(attr);
	updateIdxPortNameMap( attr );

	bool bChanged = attr.bConf;

	PortStateUpdateEventPtr e = new PortStateUpdateEvent();

	e->portName = attr.portName;
	e->portAddressV4 = attr.ipv4;
	e->portAddressV6 = attr.ipv6;
	e->tcpPortNumber = attr.tcpPortNumber;
	e->capacity = attr.capacity;
	e->portState = attr.bUp ? PORT_STATE_UP : PORT_STATE_DOWN;
	e->activeBandiwidth = attr.capacity - attr.usedBW;
	e->activeTransferCount = attr.sessionInfos.size();

	ZQ::common::MutexGuard gd(mMutex);

	PORTS::iterator itPort = mPorts.find( attr.portName );
	if( itPort == mPorts.end() )
	{
		mPorts.insert( PORTS::value_type( attr.portName , attr) );
	}
	else
	{
		PortAttr& portAttr = itPort->second;	

		portAttr.bUp		= attr.bUp;
		portAttr.capacity	= attr.capacity;
		portAttr.ipv4		= attr.ipv4;
		portAttr.ipv6		= attr.ipv6;
		portAttr.portName	= attr.portName;
		portAttr.tcpPortNumber = attr.tcpPortNumber;		
	}
	
	if( bChanged)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(PortManager,"port attr changed: portName[%s] state[%s] ipv4[%s] ipv6[%s] capacity["FMT64"]"),
			attr.portName.c_str() , attr.bUp ? "UP":"DOWN", showStringVector(attr.ipv4).c_str() , showStringVector(attr.ipv6).c_str() , attr.capacity );
		mSvc.getEventPublisher().post(e);
	}
}

void PortManager::updateIdxPortNameMap( const PortAttr& attr )
{
	ZQ::common::MutexGuard gd(mMutex);
	mIdx2Ports[attr.portIndex] = attr.portName;
}

std::string PortManager::indexToPortName( int portIndex ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	INDEXTOPORTNAME::const_iterator it = mIdx2Ports.find( portIndex );
	if( it != mIdx2Ports.end() )
	{
		return it->second;
	}
	return "";
}

void PortManager::addIpAddress(const std::string& portName, const std::string& ipaddr, bool bIpv6)
{
	ZQ::common::MutexGuard gd(mMutex);
	PORTS::iterator itPort = mPorts.find(portName );
	if( itPort == mPorts.end() )
		return;
	
	bool bFound = false;
	PortAttr& attr = itPort->second;
	std::vector<std::string>& ips =bIpv6 ? attr.ipv6 : attr.ipv4;
	std::vector<std::string>::iterator it = ips.begin();
	for( ; it != ips.end() ; it++ )
	{
		if( *it == ipaddr )
		{
			bFound = true;
			break;
		}
	}
	if(!bFound)
	{
		ips.push_back(ipaddr);
		updateIpPortNameMap( itPort->second);
	}

	bool bFireEvent = get_config_portspeed( portName ) > 0 ;

	PortStateUpdateEventPtr e = new PortStateUpdateEvent();

	e->portName = attr.portName;
	e->portAddressV4 = attr.ipv4;
	e->portAddressV6 = attr.ipv6;
	e->tcpPortNumber = attr.tcpPortNumber;
	e->capacity = attr.capacity;
	e->portState = attr.bUp ? PORT_STATE_UP : PORT_STATE_DOWN;
	e->activeBandiwidth = attr.capacity - attr.usedBW;
	e->activeTransferCount = attr.sessionInfos.size();

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(PortManager,"addIpAddress() port attr changed: portName[%s] state[%s] ipv4[%s] ipv6[%s] capacity["FMT64"]"),
		attr.portName.c_str() , attr.bUp ? "UP":"DOWN", showStringVector(attr.ipv4).c_str() , showStringVector(attr.ipv6).c_str() , attr.capacity );
	if( bFireEvent )
		mSvc.getEventPublisher().post(e);
}

void PortManager::removeIpAddress(const std::string& portName, const std::string& ipaddr, bool bIpv6)
{
	ZQ::common::MutexGuard gd(mMutex);
	PORTS::iterator itPort = mPorts.find(portName );
	if( itPort == mPorts.end() )
		return;
	PortAttr& attr = itPort->second;

	PortStateUpdateEventPtr e = new PortStateUpdateEvent();

	bool bFireEvent = get_config_portspeed( portName ) > 0 ;

	e->portName = attr.portName;
	e->portAddressV4 = attr.ipv4;
	e->portAddressV6 = attr.ipv6;
	e->tcpPortNumber = attr.tcpPortNumber;
	e->capacity = attr.capacity;
	e->portState = attr.bUp ? PORT_STATE_UP : PORT_STATE_DOWN;
	e->activeBandiwidth = attr.capacity - attr.usedBW;
	e->activeTransferCount = attr.sessionInfos.size();

	std::vector<std::string>& ips =bIpv6 ? attr.ipv6 : attr.ipv4;
	std::vector<std::string>::iterator it = ips.begin();
	for( ; it != ips.end() ; it++ )
	{
		if( *it == ipaddr )
		{
			ips.erase(it);
			mIp2Ports.erase( ipaddr );
			break;
		}
	}
	
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(PortManager,"removeIpAddress() port attr changed: portName[%s] state[%s] ipv4[%s] ipv6[%s] capacity["FMT64"]"),
		attr.portName.c_str() , attr.bUp ? "UP":"DOWN", showStringVector(attr.ipv4).c_str() , showStringVector(attr.ipv6).c_str() , attr.capacity );
	if( bFireEvent )
		mSvc.getEventPublisher().post(e);
}


#define PLOG (*(getEnvironment()->getLogger()))

typedef struct addr 
{
  int af;
  union 
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } u;
} addr_t;
//typedef struct port port_t;

PortManager& getPortManager()
{
	return getC2StreamerService()->getPortManager();
}

static unsigned long long get_port_speed(char *name, unsigned long long* phycap)
{
	if(phycap)
		*phycap = 0 ;
	unsigned long long capacity = 0;
	if( ! ( name && name[0] != 0  ) )
	{
		PLOG(ZQ::common::Log::L_ERROR,CLOGFMT(get_port_speed,"invalid name passed in"));
		return 0;
	}
	/* See if there's a config item 'port_capacity_<name>', and if so use that. */
	
	///FIXME: if there is a configuration which tell us that user has already set the speed of port, USE IT
	{
		const C2EnvConfig& conf 		= getEnvironment()->getConfig();
		const PORTSPEEDCONF& portSpeedMap	= conf.portSpeed;
		PORTSPEEDCONF::const_iterator itPort = portSpeedMap.find(std::string(name));
		if( itPort != portSpeedMap.end())
		{
			PLOG(ZQ::common::Log::L_INFO, CLOGFMT(get_port_speed,"use configuration speed [%lld] for port[%s]"),
				 itPort->second.speed, name);
			capacity =  itPort->second.speed;
		}
	}
	if( capacity == 0 )
		PLOG(ZQ::common::Log::L_INFO, CLOGFMT(get_port_speed,"no configuration is associated with port [%s], set speed[0]"),
			name);

	if(!phycap)
		return capacity;

	/* If no config item, check ethtool. */
	int s = -1;
	struct ifreq ifreq;
	struct ethtool_cmd ethtool;
	memset( &ifreq, 0, sizeof(ifreq));
	memset( &ethtool , 0,sizeof(ethtool));

	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(get_port_speed,"get_port_speed: Failed to create socket:[%s]"), SYS::getErrorMessage().c_str() );
		return capacity;
	}


	strncpy(ifreq.ifr_name, name, sizeof(ifreq.ifr_name));
	ifreq.ifr_name[sizeof(ifreq.ifr_name)-1] = '\0';
	ifreq.ifr_data = (caddr_t) &ethtool;

	ethtool.cmd = ETHTOOL_GSET;

	int error = ioctl(s, SIOCETHTOOL, &ifreq);
	if ( error < 0 )
	{
		PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(get_port_speed,"Failed to read port settings:[%s] for port[%s]"), SYS::getErrorMessage().c_str() , name );		
		close(s);
		return capacity;
	}	
	
	if( s>= 0 )
	{
		close(s);
		s = -1;
	}
	

	switch (ethtool.speed) 
	{
	case SPEED_10:    *phycap =		10000000ULL;
	case SPEED_100:   *phycap =		100000000ULL;
	case SPEED_1000:  *phycap =		1000000000ULL;
	case SPEED_2500:  *phycap =		2500000000ULL;
	case SPEED_10000: *phycap =		10000000000ULL;
	case (__u16)-1:   *phycap =		0ULL;
	default:
		PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(get_port_speed,"get_port_speed: Unknown physical speed %d, reporting 0"), ethtool.speed);
		return capacity;
	}
}
static void link_callback(struct nl_cache *link_nlc, struct nl_object *objp, int action)
{
	struct rtnl_link *linkp = (struct rtnl_link *)objp;
	char *name = rtnl_link_get_name(linkp);
	unsigned int flags;
	unsigned long long capacity;
	bool online;
	int ifindex = rtnl_link_get_ifindex(linkp);	
	
	PortManager::PortAttr portattr;

	
	switch (action) 
	{
	case NL_ACT_NEW:
	case NL_ACT_CHANGE:
		{
			PLOG(ZQ::common::Log::L_INFO, CLOGFMT(link_callback, "Got add or change notification for interface '%s'"), name);
			flags = rtnl_link_get_flags(linkp);
			//if ((flags & (IFF_NOARP | IFF_LOOPBACK)) != 0) 
			if ((flags & (IFF_NOARP)) != 0) 
			{
				PLOG(ZQ::common::Log::L_WARNING, CLOGFMT(link_callback,"Ignoring interface '%s' (either noarp or loopback)"), name);
				return;
			}
			unsigned long long physicalSpeed = 0;
			capacity = get_port_speed(name, &physicalSpeed );
			online = (flags & (IFF_UP | IFF_RUNNING)) == (IFF_UP | IFF_RUNNING) ? true : false;
		
			PortManager& pm = getPortManager();
			std::string portName =  pm.indexToPortName(ifindex);
			pm.getPortAttr( portName , portattr );
			
			portattr.bUp 		= online;
			portattr.bConf		= capacity > 0 ;
			portattr.capacity	= capacity;
			portattr.physicalCapability = physicalSpeed;
			portattr.portIndex	= ifindex;
			portattr.portName	= name;	
			
			getPortManager().updatePort( portattr );
			
		}
	break;

	case NL_ACT_DEL:
		{		
			PortManager& pm = getPortManager();
			std::string portName =  pm.indexToPortName(ifindex);
			pm.getPortAttr( portName , portattr );
			
			portattr.bUp 		= false;
			portattr.capacity	= 0;		
			portattr.portIndex	= ifindex;
			portattr.portName	= name;			
			getPortManager().updatePort( portattr );
			
		}
	break;

	default:
		{
			PLOG(ZQ::common::Log::L_ERROR, CLOGFMT( link_callback," unknown action %d"),  action);
		}
		break;
	}
}


static void addr_callback(struct nl_cache *link_nlc, struct nl_object *objp, int action)
{
	struct rtnl_addr *addrp = (struct rtnl_addr *)objp;	
	struct nl_addr *localp = rtnl_addr_get_local(addrp);
	int ifindex = rtnl_addr_get_ifindex(addrp);
	addr_t addr;
	int len;
	void *binaryp;

	if (localp == NULL)
	{
		PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback,"failed to get local address") );
		return;
	}
	len = nl_addr_get_len(localp);
	binaryp = nl_addr_get_binary_addr(localp);
	if (binaryp == NULL)
	{
		PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback ," failed to get binary representation of address"));
		return;
	}
	
	bool isIpv6 = false;
	
	memset(&addr, 0, sizeof(addr));
	switch (nl_addr_get_family(localp)) 
	{
	case AF_INET:
		if (len != sizeof(addr.u.ipv4_addr)) 
		{
			PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback,"AF_INET addr has bogus len %d, expected %d"), 
												   len, sizeof(addr.u.ipv4_addr) );
			return;
		}
		addr.af = AF_INET;
		memcpy(&addr.u.ipv4_addr, binaryp, len);
		break;

	case AF_INET6:
		if (len != sizeof(addr.u.ipv6_addr))
		{
			PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback,"AF_INET addr has bogus len %d, expected %d"),
													len, sizeof(addr.u.ipv6_addr) );
			return;
		}
		addr.af = AF_INET6;
		isIpv6 	= true;
		memcpy(&addr.u.ipv6_addr, binaryp, len);
		break;

	default:
		{
			PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback,"invalid address family %d, expected %d (ipv4) or %d (ipv6)"),
						nl_addr_get_family(localp), AF_INET, AF_INET6 );
		}
			
		break;
	}

	char ipbuf[INET6_ADDRSTRLEN];

	if (inet_ntop(addr.af, &addr.u, ipbuf, sizeof(ipbuf)) == NULL) 
	{
		strcpy(ipbuf, "<\?\?\?>");
	}
	PLOG(ZQ::common::Log::L_INFO, CLOGFMT(addr_callback,"Got %s request for address '%s' ifindex %d"),
			((action == NL_ACT_NEW) || (action == NL_ACT_CHANGE)) ? "add" :
			((action == NL_ACT_DEL) ? "delete" : "UNKNOWN"),
			ipbuf, ifindex );

	PortManager& pm = getPortManager();
	std::string portName =  pm.indexToPortName(ifindex);
	if( portName.empty() )
		return;
		
	switch (action) 
	{
	case NL_ACT_NEW:	
	case NL_ACT_CHANGE:
		{
			pm.addIpAddress( portName , ipbuf , isIpv6 );
		}
		break;

	case NL_ACT_DEL:
		{			
			pm.removeIpAddress( portName , ipbuf , isIpv6 );		
		}
		break;

	default:
		{			
			PLOG(ZQ::common::Log::L_ERROR, CLOGFMT(addr_callback,"unknown action %d"),action);
		}
		break;
	}
}

static void add_initial_links(struct nl_object *objp, void *arg)
{
	link_callback( (nl_cache*)arg, objp , NL_ACT_NEW );	
	
}
static void add_initial_addrs(struct nl_object *objp, void *arg)
{
	addr_callback( (nl_cache*)arg , objp , NL_ACT_NEW );
}

void PortManager::clearResource()
{
	if( mNlcm  )
	{
		nl_cache_mngr_free(mNlcm);
		mNlcm = NULL;
	}
	mNlh 		= NULL;
	mLink_nlc 	= NULL;
	mAddr_nlc	= NULL;	
}

void PortManager::stop( )
{
	if( !mbQuit )
	{
		mbQuit = true;
		waitHandle( 10 * 1000 );
	}
	clearResource();
}

bool PortManager::getMtus()
{
	int sock = socket(AF_INET, SOCK_STREAM, 0 );
	if( sock < 0 )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PortManager,"getMtus() failed to create socket"));
		return false;
	}
	PORTS::iterator it = mPorts.begin();
	for( ; it != mPorts.end(); it ++ )
	{
		if( !it->second.bConf )
			continue;

		struct ifreq ifr;
		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name,it->first.c_str(),IFNAMSIZ);
		if( ioctl(sock,SIOCGIFMTU,&ifr) < 0 )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PortManager,"getMtus(), failed to get mtu for [%s] due to [%s]"),
				it->first.c_str(), strerror(errno) );
			continue;
		}		
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(PortManager,"getMtus() , get mtu[%d] for [%s]"),
			ifr.ifr_mtu, it->first.c_str());
		it->second.ethMtu = (ifr.ifr_mtu > 40) ? (ifr.ifr_mtu-40):ifr.ifr_mtu;
		//                                         remove tcp header size
	}
	return true;
}

bool PortManager::start( )
{
	clearResource();
	mNlh = nl_handle_alloc();
	mNlcm = nl_cache_mngr_alloc(mNlh, NETLINK_ROUTE, NL_AUTO_PROVIDE);
	if ( mNlcm == NULL) 
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PortManager,"start() Failed to initialize netlink interface (cache manager creation)"));
		return false;
	}

	mLink_nlc = nl_cache_mngr_add(mNlcm, "route/link", link_callback);
	if ( mLink_nlc == NULL) 
	{		
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PortManager,"start() Failed to initialize netlink interface (link cache creation)"));
		return false;
	}

	mAddr_nlc = nl_cache_mngr_add(mNlcm, "route/addr", addr_callback);
	if ( mAddr_nlc == NULL) 
	{		
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PortManager,"start() Failed to initialize netlink interface (address cache creation)"));
		return false;
	}
	
	nl_cache_foreach(mLink_nlc, add_initial_links, this);
	nl_cache_foreach(mAddr_nlc, add_initial_addrs, this);	

	mbQuit = false;	
	getMtus();
	return ZQ::common::NativeThread::start();
}

int PortManager::run()
{		
	while (!mbQuit) 
	{
		if (nl_cache_mngr_poll( mNlcm, 500 ) < 0) 
		{
		/* An EINTR error is normal & expected, just retry the poll.
		Anything else should be logged. */
			if (errno != EINTR)
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PortManager,"run() netlink interface poll failed"));
		}
	}	
	return 1;
}

}//namespace C2Streamer
