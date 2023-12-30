
#ifndef _cdnss_c2streamer_network_interface_manager_h__
#define _cdnss_c2streamer_network_interface_manager_h__

#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <map>
#include <vector>
#include <Locks.h>
#include <NativeThread.h>
#include "C2TunnelBwmon.h"


namespace C2Streamer
{

class C2StreamerEnv;
class C2Service;


class PortManager : public ZQ::common::NativeThread
{
public:
	PortManager( C2StreamerEnv& env , C2Service& svc );
	virtual ~PortManager( );

	struct sessionAttr 
	{
		std::string		sessionId;
		int64			usedBW;
		sessionAttr()
		{
			usedBW = 0;
		}
	};

	typedef std::map< std::string , sessionAttr> SESSIONATTRS;
	/*                sessionId ,    sessionAttr*/

	struct PortAttr 
	{
		std::string						portName;
		std::vector<std::string>		ipv4;
		std::vector<std::string>		ipv6;
		int								portIndex;
		uint16							tcpPortNumber;
		int64							capacity;
		int64							usedBW;
		int64							usedCount;
		int64							physicalCapability;
		bool							bUp;
		bool							bConf;
		SESSIONATTRS					sessionInfos;
		int								ethMtu;
		PortAttr()
		{
			portIndex	= -1;
			capacity	= 0 ;
			usedBW		= 0;
			bUp			= false;
			bConf		= false;
			tcpPortNumber=0;
			usedCount	= 0;
			physicalCapability = 0;
			ethMtu		= 1460;
		}
	};
	
	typedef std::map< std::string ,  PortAttr> PORTS;
	/*                portName , portAttr*/
	
	typedef std::map< int , std::string > INDEXTOPORTNAME;
	/*				ifindex  port_name	*/
	

public:

	bool			start( );
	
	void			stop( );
	
	bool			scanNetworkInterface( );

	std::string		ipToPortName( const std::string& ip ) const;
	
	std::string		indexToPortName( const int portIndex ) const;

	std::string		portNameToIp( const std::string& portName ) const;

	///register session with request bandwidth
	///return false if no available bandwidth can suit the session, return true other wise
	bool			registerSession( const std::string& ip , const std::string& sessId , int64 requestBW );

	void			unregisterSession( const std::string& ip , const std::string& sessId );

	void			updatePort( const PortAttr& attr );

	bool			getPortAttr( const std::string& ipOrPortname , PortAttr& attr ) const;

	void			getPortsAttr( PORTS& ports ) const;

	void			addIpAddress( const std::string& portName ,  const std::string& ipaddr , bool bIpv6 );
	void			removeIpAddress( const std::string& portName , const std::string& ipaddr , bool bIpv6 );
	
	std::vector<std::string> getAvailPorts(int64 bw, const std::string& sessId) const;

	int64			queryBW( const std::string& ip, int64* totalBW ) const;

protected:
	
	int				run();//override base class run method
  
	void			updateIpPortNameMap(  const PortAttr& attr );
	void			updateIdxPortNameMap( const PortAttr& attr );

	void			clearResource();
	uint64			get_config_portspeed( const std::string& name) const;

	bool			getMtus();

private:	

	typedef std::map< std::string ,  std::string > IPTOPORTMAP;
	/*                 ip ,           portName   */
	

	PORTS					mPorts;
	IPTOPORTMAP				mIp2Ports;
	INDEXTOPORTNAME			mIdx2Ports;
	std::map<std::string, std::string> mPortToIps;
	C2StreamerEnv&			mEnv;
	C2Service&				mSvc;
	ZQ::common::Mutex		mMutex;
	bool					mbQuit;
	
	struct nl_handle 		*mNlh;
	struct nl_cache_mngr 	*mNlcm;
	struct nl_cache 		*mLink_nlc, *mAddr_nlc;
};

}//namespace C2Streamer

#endif//_cdnss_c2streamer_network_interface_manager_h__

