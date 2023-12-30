
#include <sys/time.h>
#include <TimeUtil.h>
#include "C2StreamerEnv.h"
#include "C2TunnelBwmon.h"
#include "C2StreamerService.h"
#include "C2SessionHelper.h"

namespace C2Streamer{

IptableQuerying::IptableQuerying()
	:mTableHandle(0)
{
}

IptableQuerying::~IptableQuerying()
{
	clear();
}

void IptableQuerying::clear()
{
	if(mTableHandle)
	{
		iptc_free(iptc_handle_inst);
		mTableHandle = 0 ;
	}
	mPackets = 0;
	mBytes = 0;
}

bool IptableQuerying::query(const std::string& chainname , const std::string& tablename)
{
	clear();
	mTableHandle =  iptc_init(tablename.c_str());
	if(!mTableHandle)
	{
		return false;//should I log some error message here?
	}
	const char * chain = NULL;
	for( chain = iptc_first_chain(iptc_handle_inst); chain ; chain = iptc_next_chain(iptc_handle_inst) )
	{
		//if( iptc_builtin(chain , mTableHandle ))
		//{
		//	continue;//built in chain is not considered
		//}

		if( chainname.length() > 0 )
		{
			if( std::string(chain) == chainname )
			{
				for(int i = 1 ; i ; i++ )
				{
					struct ipt_counters *counters = iptc_read_counter(chain,i,iptc_handle_inst);
					if( !counters )
					{
						return true;
					}
					mPackets += counters->pcnt;
					mBytes += counters->bcnt;
				}
			}
		}
		else
		{
			for( int i = 1 ; i ; i++ )
			{
				struct ipt_counters *counters = iptc_read_counter(chain,i,iptc_handle_inst);
				if( !counters )
				{
					break;
				}
				mPackets += counters->pcnt;
				mBytes += counters->bcnt;
			}
		}
	}
	return true;
}

C2TunnelBWMon::C2TunnelBWMon()
	:mLastBytes(0),
	mLastPackets(0),
	mBitrate(0),
	mLastMeasuredTime(0)
{
}

C2TunnelBWMon::~C2TunnelBWMon()
{
}

long long C2TunnelBWMon::now()
{
	struct timeval n;timerclear(&n);
	gettimeofday(&n,0);
	long long t = (long long)n.tv_sec * 1000 + (long long)n.tv_usec / 1000;
	return t;
}

long long C2TunnelBWMon::querybw(const std::string& chainname, const std::string& tablename)
{
	IptableQuerying q;
	if( !q.query(chainname, tablename))
	{
		return 0;
	}
	long long bitrate = 0 ;
	if( mLastMeasuredTime > 0 )
	{
		long long deltaBytes = q.getBytes() - mLastBytes;
		long long deltaTime = now() - mLastMeasuredTime;
		if( deltaTime > 0 )
		{
			bitrate = deltaBytes * 8000 / deltaTime;
		}
	}
	mLastBytes = q.getBytes();
	mLastPackets = q.getPackets();
	mLastMeasuredTime = now();
	if( bitrate < 0 )
		bitrate = 0;
	return bitrate;
}


///////////////////////////////////////////////////////////////////////////////////////////
//C2IpConnTackMon


#define LINE_BUFFER_SIZE 4096

C2IpConnTackMon::C2IpConnTackMon( C2StreamerEnv& env)
	:mEnv(env),
	mbQuit(true)
{
	mOldConnInfos =  mNewConnInfos = 0;
	mLineBuffer = (char*)malloc( LINE_BUFFER_SIZE );
}

C2IpConnTackMon::~C2IpConnTackMon()
{
	stop();
	if(mLineBuffer)
	{
		free(mLineBuffer);
		mLineBuffer = 0;
	}
}

void C2IpConnTackMon::stop()
{
	if( mbQuit)
		return;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2IpConnTackMon,"stopping TrackMon"));
	mbQuit = true;
	mSem.post();
	waitHandle(-1);
	std::vector<RegexInfo>::iterator it = mRegexs.begin();
	for( ; it != mRegexs.end() ; it ++ )
	{
		delete it->re;
	}
	mRegexs.clear();
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2IpConnTackMon,"stopped"));
}
/*
 * tcp      6 431990 ESTABLISHED src=10.15.10.50 dst=172.16.20.40 sport=34889 dport=9527 packets=3 bytes=140 src=10.2.232.68 dst=10.2.232.63 sport=22 dport=34889 packets=2 bytes=112 [ASSURED] mark=0 secmark=0 use=1
 *
 * */

void C2IpConnTackMon::addRules( const std::string& downstreamIp, unsigned short downstreamPort,
						const std::string& upstreamLocalIp, const std::string& upstreamPeerIp,
						unsigned short upstreamPeerPort )
{
	char szBuffer[4096];
	sprintf(szBuffer,"tcp.*?ESTABLISHED src=(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}).*?dst=%s.*?sport=(\\d*) dport=%hu.*?src=%s.*?dst=%s.*?packets=.*?bytes=(\\d*?) \\[.*",
			downstreamIp.c_str(), downstreamPort, upstreamPeerIp.c_str(),upstreamLocalIp.c_str());
	boost::regex* re = new boost::regex(szBuffer);
	if( re->empty() )
	{
		delete re;
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2IpConnTackMon,"bad regex rule: %s"), szBuffer);
		return;
	}
	RegexInfo info;
	
	info.re = re;
	info.downstreamIp = downstreamIp;
	info.downstreamPort = downstreamPort;
	info.upstreamLocalIp = upstreamLocalIp;
	info.upstreamPeerIp = upstreamPeerIp;

	{	
		ZQ::common::MutexGuard gd(mBWUsageLocker);
		mRegexs.push_back(info);
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2IpConnTackMon,"add new regex rule: %s"), szBuffer);
}

bool C2IpConnTackMon::start( )
{
	mbQuit = false;
	return ZQ::common::NativeThread::start();
}

#define MAX_NAT_CONN 40960

int C2IpConnTackMon::run()
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2IpConnTackMon,"start to monitor /proc/net/ip_conntrack"));

	mOldConnInfos = new IPCONNINFOARRAY();
	mNewConnInfos = new IPCONNINFOARRAY();
	
	mOldConnInfos->reserve(MAX_NAT_CONN);
	mNewConnInfos->reserve(MAX_NAT_CONN);

	while(!mbQuit)
	{
		readInfos();
		compareInfos();

		//swap old info with new infos
		IPCONNINFOARRAY* p = mNewConnInfos;
		mNewConnInfos = mOldConnInfos;
		mOldConnInfos = p;

		mSem.timedWait( 500 );
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2IpConnTackMon,"stop monitoring /proc/net/ip_conntrack"));
	delete mOldConnInfos;
	delete mNewConnInfos;
	return 0;
}

static const char* const ipconnfile = "/proc/net/ip_conntrack";
static const char* const nfconnfile = "/proc/net/nf_conntrack";

void C2IpConnTackMon::readInfos()
{
	FILE* f = fopen( ipconnfile, "rb" );
	if(!f)
	{
		f = fopen( nfconnfile, "rb");
		if(!f) {
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2IpConnTackMon,"failed to open file %s or %s, please make sure iptables service is started"),
					ipconnfile, nfconnfile);
			return;
		}
	}

	std::vector<RegexInfo> regInfo;

	{
		ZQ::common::MutexGuard gd(mBWUsageLocker);
		regInfo = mRegexs;
	}

	std::vector<RegexInfo>::iterator itRegInfo = regInfo.begin();

	char*pbuf = mLineBuffer;
	size_t len = LINE_BUFFER_SIZE  - 1;
	
	mNewConnInfos->clear();

	ssize_t length = 0 ;
	size_t lineCount = 0 ;
	while( ( length = getline(&pbuf, &len, f) ) >= 0 )
	{
		lineCount ++;
		if ( length < 10 )
			continue;

		for( itRegInfo = regInfo.begin() ; itRegInfo != regInfo.end() ; itRegInfo++ )
		{
			boost::cmatch result;
			RegexInfo& reInfo = *itRegInfo;
			boost::regex* re = reInfo.re;
			if( !boost::regex_match( pbuf, result, *re ) )
			{
				continue;
			}
			if ( result.size() < 4 )
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2IpConnTackMon,"regex matched but less than 4 sections: %s"), pbuf );
				continue;
			}
			IpConnInfo info;
			info.clientIp =  result.str(1);
			info.clientTcpPort = (unsigned short)atoi(result.str( 2 ).c_str());
			info.localIp = reInfo.downstreamIp;
			info.transferedBytes = atoll( result.str( 3 ).c_str() );

			mNewConnInfos->push_back( info );


			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2IpConnTackMon,"got nat conn: client[%s:%hu] downIp[%s] bytes[%lld]"),
					info.clientIp.c_str(), info.clientTcpPort,
					info.localIp.c_str(), info.transferedBytes);
		}
		if( pbuf != mLineBuffer )
		{
			free(pbuf);
			mLineBuffer = (char*)malloc( LINE_BUFFER_SIZE );
		}
		pbuf = mLineBuffer;
		len = LINE_BUFFER_SIZE - 1;
	}

	fclose(f);
}


void C2IpConnTackMon::addUsage( BWINFOMAP& infomap, const std::string& ip, long long usedBytes )
{
	BWINFOMAP::iterator it = infomap.find(ip);
	if( it == infomap.end() )
	{
		BWINFO info;
		info.sessCount = 1;
		info.usedBytes = usedBytes;
		infomap[ip] = info;
	}
	else
	{
		BWINFO& info = it->second;
		++ info.sessCount;
		info.usedBytes += usedBytes;
	}	
}

void C2IpConnTackMon::compareInfos()
{
	ZQ::common::MutexGuard gd( mBWUsageLocker );
	
	int64 timenow = ZQ::common::now();
	if( mLastTouch <= 0 )
	{
		mLastTouch = timenow;
	}
	int64 delta = timenow - mLastTouch;
	if( delta < 250 )
		return;
	mLastTouch = timenow;

	mLocalPortBWUsage.clear();
	mClientBWUsage.clear();

	std::sort( mNewConnInfos->begin(), mNewConnInfos->end() );
	IPCONNINFOARRAY::const_iterator itOld = mOldConnInfos->begin();
	IPCONNINFOARRAY::const_iterator itNew = mNewConnInfos->begin();
	while( itOld != mOldConnInfos->end() && itNew != mNewConnInfos->end() )
	{
		if( *itOld < *itNew )
		{
			// connection lost
			itOld ++;
		}
		else if( *itOld == *itNew )
		{
			long long d = itNew->transferedBytes - itOld->transferedBytes;
			if( d > 0 )
			{
				addUsage( mLocalPortBWUsage, itOld->localIp, d );
				addUsage( mClientBWUsage, itOld->clientIp, d );
			}
			itOld ++;
			itNew ++;
		}
		else
		{
			//addUsage( mLocalPortBWUsage, itNew->localIp, itNew->transferedBytes );
			//addUsage( mClientBWUsage, itNew->clientIp, itNew->transferedBytes );
			itNew ++;
		}
	}
	//Only increment bytes will be calculated for bandwidth
	
	//while( itNew != mNewConnInfos->end() )
	//{
	//	addUsage( mLocalPortBWUsage, itNew->localIp, itNew->transferedBytes );
	//	addUsage( mClientBWUsage, itNew->clientIp, itNew->transferedBytes );
	//	itNew ++;
	//}
	
	BWINFOMAP::iterator it = mLocalPortBWUsage.begin();
	for( ; it != mLocalPortBWUsage.end() ; it ++ )
	{
		BWINFO& info = it->second;
		info.usedBW = info.usedBytes * 8000 / delta;
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2IpConnTackMon,"upstream [%s] bw usage: bw[%ld] bytes[%ld], delta[%ld]"),
			   	it->first.c_str(), info.usedBW, info.usedBytes, delta ) ;
	}

	it = mClientBWUsage.begin();
	for( ; it != mClientBWUsage.end() ; it ++ )
	{
		BWINFO& info = it->second;
		info.usedBW = info.usedBytes * 8000 / delta;
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2IpConnTackMon,"downstream [%s] bw usage: bw[%ld] bytes[%ld]"),
			   	it->first.c_str(), info.usedBW, info.usedBytes);
	}
}

int64 C2IpConnTackMon::queryBW( const std::string& ip ) const 
{
	ZQ::common::MutexGuard gd( mBWUsageLocker );
	if( ip.empty() )
	{
		BWINFOMAP::const_iterator it = mLocalPortBWUsage.begin();
		int64 bw = 0;
		for( ; it != mLocalPortBWUsage.end(); it ++ )
		{
			bw += it->second.usedBW;
		}
		return bw;
	}
	BWINFOMAP::const_iterator it = mLocalPortBWUsage.find(ip);
	if( it == mLocalPortBWUsage.end() )
	{
		it = mClientBWUsage.find( ip );
		if( it == mClientBWUsage.end() )
			return 0;
		else
			return it->second.usedBW;
	}
	else
	{
		return it->second.usedBW;
	}
}

void lowerstring( std::string& str)
{
	std::transform( str.begin(), str.end(), str.begin(), ::tolower);
}

IptablesRule::IptablesRule( C2StreamerEnv& env , C2Service& svc)
	:mEnv(env), mSvc(svc)
{
}

IptablesRule::~IptablesRule()
{
}

std::string  IptablesRule::convertDomainToIp( const std::string& dn )
{
	AddrInfoHelper aih;
	if(!aih.getinfo(dn,"0"))
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IptablesRule,"start(), unrecognized domain name or ip, [%s]"), dn.c_str());
		return "";
	}
	return aih.ip();
}

void IptablesRule::stop()
{
	clearAllRules();
}

void IptablesRule::clearAllRules()
{
	const NicsConf& portAttr = mEnv.getConfig().nics.nics;
	NicsConf::const_iterator it = portAttr.begin();
	for( ; it != portAttr.end(); it ++ )
	{
		const NicAttr& sa = it->second;

		char	szCommandStr[1024];
		sprintf(szCommandStr,"%s %d %d","/opt/TianShan/bin/cleanupc2tunnel.py ",sa.natPortBase,sa.natPortBase+sa.natPortCount);
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IptablesRule,"clearAllRules() clear dnat rules [%s]"),szCommandStr);
		system(szCommandStr);		
	}
}

void IptablesRule::start()
{
	//clear all DNAT rules before creating new one
	clearAllRules();
	const NicsConf& portAttr = mEnv.getConfig().nics.nics;
	NicsConf::const_iterator it = portAttr.begin();
	for( ; it != portAttr.end(); it ++ )
	{
		const NicAttr& sa = it->second;

		std::string ipStr = it->first;//convertDomainToIp( it->first );
		std::string ip = mSvc.getPortManager().portNameToIp( ipStr );
		if( ip.empty() )
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(IptablesRule,"start() no ip is found for port[%s]"),
					it->first.c_str());
			continue;
		}
		if( sa.natPortBase <= 0 || sa.natPortCount <= 0 )
		{
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(IptablesRule,"start() port[%s/%s] natPortBase[%hu] natPortCount[%hu], ignored"),
					it->first.c_str(), ip.c_str(), sa.natPortBase, sa.natPortCount);
			continue;
		}
		setPortRange( ip, sa.natPortBase, sa.natPortCount );
	}
}

void IptablesRule::setPortRange( const std::string& ip, unsigned short portBase, unsigned short portCount )
{
	if( portCount <= 0  || portBase <= 0 )
	{
		MLOG( ZQ::common::Log::L_ERROR, CLOGFMT(IptablesRule, "setPortRange() invalid portbase[%d] or portcount[%d] passed in, reject"),
				portBase, portCount);
		return;
	}

	AddressResource res;
	res.ipname = ip;
	lowerstring( res.ipname );
	res.portbase = portBase;
	res.portcount = portCount;
	res.nextavailport = res.portbase;

	ZQ::common::MutexGuard gd(mLocker);
	mResMap[ res.ipname ] = res;

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(IptablesRule,"setPortRange() set port[%s] range[%hu-%hu)"),ip.c_str(), portBase, portBase+portCount );
}

unsigned short IptablesRule::getPort( const std::string& localIp, const std::string& peerIpStr, unsigned short peerPort, const std::string& outIp )
{
	unsigned short port = 0;
	std::string localipname =  convertDomainToIp( localIp );
	if(localipname.empty( ) )
		return 0;
	lowerstring( localipname);

	std::string peerIp = convertDomainToIp( peerIpStr );
	if(peerIp.empty())
		return 0;

	AddressInfo peerInfo( peerIp, peerPort );
	ZQ::common::MutexGuard gd(mLocker);
	DNATINFOMAP::iterator it = mDnatInfos.find( peerInfo );

	if( it == mDnatInfos.end() )
	{
		DNATResourceMap::iterator itRes = mResMap.find( localipname );
		if( itRes == mResMap.end() )
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IptablesRule,"getPort(), localIp[%s] is not exist in configuration"), localipname.c_str() );
			return 0;
		}
		AddressResource& res = itRes->second;
		if( res.nextavailport > res.portbase + res.portcount )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IptablesRule,"getPort(), localIp[%s] portbase[%d] portCount[%d] no available port"),
					localipname.c_str(), res.portbase, res.portcount );
			return 0;
		}
		port = res.nextavailport;
		if(createDNat( localIp, port, peerIp,peerPort,outIp))
		{
			res.nextavailport ++;
			return port;
		}
		else
			return 0;
	}
	else
	{
		AddressInfo& localInfo = it->second;
		port = localInfo.port;

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IptablesRule,"getPort(), peerIp[%s] peerPort[%d], return localIp[%s] localPort[%d]"),
				peerIp.c_str(), peerPort, localInfo.ipname.c_str(), port);
		return port;
	}
}

bool IptablesRule::createDNat( const std::string& localIp, unsigned short localPort, 
								const std::string& peerIp, unsigned short peerPort, 
								const std::string& upLocalIp )
{
	std::string outIp = convertDomainToIp( upLocalIp );
	if( outIp.empty() )
		return false;
	std::string portName = mSvc.getPortManager().ipToPortName( outIp );
	if(portName.empty())
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IptablesRule,"createDNat() unknown UpStream local bind ip[%s]"),outIp.c_str());
		return false;
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(IptablesRule,"createDNat() trying to create DNAT localAddress[%s:%hu] outEth[%s/%s] peerAddress[%s:%hu]"),
			localIp.c_str(), localPort, portName.c_str(),outIp.c_str(), peerIp.c_str(), peerPort);
	char strCommand[1024];
	strCommand[sizeof(strCommand) -1 ] = 0;
	snprintf(strCommand,sizeof(strCommand)-1,"/opt/TianShan/bin/createc2tunnel.py %s %s %s %d %s %d",
			portName.c_str(), outIp.c_str(),
			localIp.c_str(), localPort,
			peerIp.c_str(), peerPort );
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IptablesRule,"createDNat() command[%s]"), strCommand);
	int ret = system(strCommand);
	if( ret == -1 )
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IptablesRule,"createDNat() failed to execute command[%s]"), strCommand );
		return false;
	}
	ret = WEXITSTATUS(ret);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(IptablesRule,"createDNat() command executed, return value %d"),ret);
	if( ret == 0)
	{
		mSvc.getC2TunnelMon().addRules( localIp, localPort, outIp, peerIp, peerPort );
		ZQ::common::MutexGuard gd(mLocker);
		AddressInfo peerInfo(peerIp, peerPort);
		AddressInfo localInfo( localIp, localPort);
		mDnatInfos[peerInfo] = localInfo;
		return true;
	}
	return false;
}

}//namespace C2Streamer

