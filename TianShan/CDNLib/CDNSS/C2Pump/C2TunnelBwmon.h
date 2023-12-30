#ifndef __bwmeter_monitor_header_file_h__
#define __bwmeter_monitor_header_file_h__

#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <libiptc/libiptc.h>
#include <string>
#include <vector>
#include <Locks.h>
#include <algorithm>
#include <NativeThread.h>

#undef max //make sure boost regex can be compiled
#include <boost/regex.hpp>

namespace C2Streamer{
#ifdef CENTOS_5
#	define IPTC_HANDLE_TYPE iptc_handle_t
#	define iptc_handle_inst	&mTableHandle
#else
#	define IPTC_HANDLE_TYPE iptc_handle*
#	define iptc_handle_inst mTableHandle
#endif


class IptableQuerying
{
public:
	IptableQuerying( );
	virtual ~IptableQuerying();
	bool      query(const std::string& chainname = "", const std::string& tablename="filter" );
	long long getBytes() const
	{
		return mBytes;
	}
	long long getPackets() const
	{
		return mPackets;
	}
private:
	void clear();
private:
	IPTC_HANDLE_TYPE		mTableHandle;
	long long				mBytes;
	long long				mPackets;
};


class C2TunnelBWMon
{
public:
	C2TunnelBWMon();
	virtual ~C2TunnelBWMon();
	
	long long querybw( const std::string& chainname, const std::string& tablename = "filter");
	long long queryPackets(){ return mLastPackets; }
	long long queryBytes() { return mLastBytes; }

private:
	long long now();
private:
	long long mLastBytes;
	long long mLastPackets;
	long long mBitrate;
	long long mLastMeasuredTime;
};

class C2StreamerEnv;

class C2IpConnTackMon : public ZQ::common::NativeThread
{
public:
	C2IpConnTackMon( C2StreamerEnv& env );
	virtual ~C2IpConnTackMon();

	bool	start( );
	
	void	stop();

	//query bandwidth used, if ip is empty, whole pass-through bandwidth will be returned
	int64	queryBW( const std::string& ip ) const;

	void	addRules( const std::string& downstreamIp, unsigned short downStreamPort,
						const std::string& upstreamLocalIp, const std::string& upstreamPeerIp,
						unsigned short upstreamPeerPort );

private:
	int		run();

	void	readInfos();

	void	compareInfos();

	struct BWINFO
	{
		long long	sessCount; // it is tcp connection count
		long long	usedBW;
		long long	usedBytes;
		BWINFO()
		{
			sessCount	= 0;
			usedBW		= 0;
			usedBytes	= 0;
		}
	};
	typedef std::map<std::string, BWINFO > BWINFOMAP;

	void	addUsage( BWINFOMAP& infomap, const std::string& ip,  long long usedBytes );
	
	C2StreamerEnv&			mEnv;
	bool					mbQuit;
	char*					mLineBuffer;

	struct RegexInfo
	{
		boost::regex*		re;
		std::string			downstreamIp;
		unsigned short		downstreamPort;
		std::string			upstreamLocalIp;
		std::string			upstreamPeerIp;
	};
	//I want to get result from regex via name clientip localip downbytes
	//std::vector<boost::regex*>	mRegexs;
	std::vector<RegexInfo>		mRegexs;


private:
	struct IpConnInfo
	{
		std::string			clientIp;
		unsigned short		clientTcpPort;
		std::string			localIp;
		long long			transferedBytes;
		long long			bitrate;
		IpConnInfo()
		{
			bitrate		 = 0;
			transferedBytes = 0 ;
		}
		bool operator<(const IpConnInfo& b) const
		{
			if( clientIp == b.clientIp )
			{
				return clientTcpPort < b.clientTcpPort;
			}
			else
			{
				return clientIp < b.clientIp;
			}
		}
		bool operator == ( const IpConnInfo& b ) const
		{
			return (clientIp == b.clientIp) && (clientTcpPort == b.clientTcpPort);
		}
	};
	typedef std::vector<IpConnInfo> IPCONNINFOARRAY;
	IPCONNINFOARRAY*		mOldConnInfos;
	IPCONNINFOARRAY*		mNewConnInfos;

	BWINFOMAP	mLocalPortBWUsage;//local port bandwidth usage for C2Tunnel
	BWINFOMAP	mClientBWUsage;
	int64		mLastTouch;

	ZQ::common::Mutex		mBWUsageLocker;
	ZQ::common::Semaphore	mSem;
};

void lowerstring( std::string& str);

class C2Service;
class IptablesRule
{
public:

	IptablesRule( C2StreamerEnv& env , C2Service& svc );
	virtual ~IptablesRule();

	void	start( );

	void	stop();

	void	setPortRange( const std::string& ip, unsigned short portBase, unsigned short portCount );

	unsigned short	getPort( const std::string& localIp, const std::string& peerIp, unsigned short peerPort, const std::string& outIp );

protected:
	void	clearAllRules();

	bool	createDNat( const std::string& localIp, unsigned short localPort, const std::string& peerIp, unsigned short peerShort ,
						const std::string& outIp );

	std::string convertDomainToIp( const std::string& dn );

private:

	struct AddressInfo
	{
		std::string ipname;
		unsigned short port;
		AddressInfo()
		{
			port = 0;
		}
		AddressInfo( const std::string& ip, unsigned short p )
		{
			ipname = ip;
			port = p;
		}
		bool operator < ( const AddressInfo& b ) const
		{
			if( port < b.port )
				return true;
			else if ( port == b.port )
			{
				return ipname < b.ipname;
			}
			else
				return false;
		}
	};

	struct AddressResource
	{
		std::string ipname;
		unsigned short portbase;
		unsigned short portcount;
		unsigned short nextavailport;
		AddressResource()
			:portbase(0),portcount(0),nextavailport(0)
		{
		}
	};

	typedef std::map<AddressInfo, AddressInfo> DNATINFOMAP;

	typedef std::map<std::string, AddressResource> DNATResourceMap;

	C2StreamerEnv&					mEnv;
	C2Service&						mSvc;
	ZQ::common::Mutex				mLocker;
	DNATINFOMAP						mDnatInfos;
	DNATResourceMap					mResMap;
};

}//namespace C2Streamer

#endif// __bwmeter_monitor_header_file_h__

