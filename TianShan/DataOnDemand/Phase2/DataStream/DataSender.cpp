// DataSender.cpp: implementation of the DataSender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataSender.h"
#include "BufferManager.h"
#include "DataDef.h"
#include "ws2tcpip.h"
#include "datastreamcfg.h"

extern ZQ::common::Config::Loader<DataStreamCfg > gDataStreamConfig;
namespace DataStream {
	
	Transfer::Transfer()
	{
		_ref = 1;
	}
	
	bool Transfer::init()
	{
		return true;
	}
	
	long Transfer::refer()
	{
		return InterlockedIncrement(&_ref);
	}
	
	long Transfer::release()
	{
		InterlockedDecrement(&_ref);
		if (_ref <= 0) {
			delete this;
		}
		
		return _ref;
	}
	
	size_t Transfer::send(void* buf, size_t len)
	{
		// report to DataProfile
		return len;
	}
	bool Transfer::InitSocket()
	{
		WSADATA wsd;			
		//初始化WinSock1.1
		if( WSAStartup( MAKEWORD(2,2),&wsd) != 0 )
		{
			return false;
		}
		return true;
	}

	class UDPTransfer: public Transfer {
	public:
		UDPTransfer(const TransferAddress& addr): _addr(addr)
		{
			_sock = INVALID_SOCKET;
		}
		
		bool init()
		{
			_sock = socket(AF_INET, SOCK_DGRAM, 0);
			_saddr.sin_family = AF_INET;
			_saddr.sin_addr.s_addr = _addr.getIP();
			_saddr.sin_port = _addr.getPort();
			
			if(gDataStreamConfig.netWorkcardIP.size() >1)
			{
				int one = 1;
				setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
				
				struct sockaddr_in my_addr;

				my_addr.sin_family = AF_INET;
				my_addr.sin_port = htons(0);
				my_addr.sin_addr.s_addr = inet_addr(gDataStreamConfig.netWorkcardIP.c_str());
				if (bind(_sock, (sockaddr* )&my_addr, sizeof(my_addr)) == SOCKET_ERROR) {
					glog( ZQLIB::Log::L_ERROR,  "bind failed with:%d",WSAGetLastError());
				}
				else
				{
					glog( ZQLIB::Log::L_INFO,  "bind success");
				}
			}

			return true;
		}
		
		virtual size_t send(void* buf, size_t len)
		{
			size_t sendSize;
			size_t remain = len;
			char* bufPos = (char* )buf;
			
			while (true) {
				
				if (remain > TRANSFER_UNIT_SIZE)
					sendSize = TRANSFER_UNIT_SIZE;
				else
					sendSize = remain;
				
				size_t r = sendto(_sock, bufPos, sendSize, 0, 
					(const struct sockaddr* )&_saddr, 
					sizeof(_saddr));
				
				if (r != sendSize) {
					// log error
					return r;
				}
				
				remain -= sendSize;
				if (remain <= 0)
					break;
				bufPos += sendSize;
			}
			
			return Transfer::send(buf, len);
		}
		
	protected:
		TransferAddress		_addr;
		SOCKET				_sock;
		sockaddr_in			_saddr;
	};
	//////////////////////////////////////////////////////////////////////////
	class MulticastTransfer: public Transfer {
	public:
		MulticastTransfer(const TransferAddress& addr): _addr(addr)
		{
			_sock = INVALID_SOCKET;
		}
		
		bool init()
		{    
#define MULTICAST_TTL	(0x40)

			/*if ((_sock = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
			{
				return false;
			}
			_saddr.sin_family = AF_INET;
			_saddr.sin_addr.s_addr = _addr.getIP();
			_saddr.sin_port = _addr.getPort();

			setsockopt(_sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&(_saddr.sin_addr), sizeof(_saddr.sin_addr));

			struct ip_mreq mreq;	
			memset(&mreq,0,sizeof(mreq)); 				
			mreq.imr_multiaddr = _saddr.sin_addr;
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);

			int result;	
			result = setsockopt(_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
			if (result == SOCKET_ERROR) 
			{
				glog( ZQLIB::Log::L_ERROR,  "add_membership error %d",WSAGetLastError());
				return false;
			} */

			int result;	
			if((_sock=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,
				WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|
				WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
			{
				return false;
			}
			if(gDataStreamConfig.netWorkcardIP.size() > 1)
			{
				int one = 1;
				setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
				
				struct sockaddr_in local;
				//将sock绑定到本机某端口上。
				local.sin_family = AF_INET;
				local.sin_port = htons(0) ;
				local.sin_addr.s_addr = inet_addr(gDataStreamConfig.netWorkcardIP.c_str());

				if(bind(_sock,(struct sockaddr*)&local,sizeof(local)) == SOCKET_ERROR )
				{
					glog( ZQLIB::Log::L_ERROR,  "bind failed with:%d",WSAGetLastError());
				}
				else
				{
					glog( ZQLIB::Log::L_INFO,  "bind success");
				}
			}

			//加入多播组
			SOCKET sockM;
			_saddr.sin_family = AF_INET;
			_saddr.sin_addr.s_addr = _addr.getIP();
			_saddr.sin_port = _addr.getPort();
			if(( sockM = WSAJoinLeaf(_sock,(SOCKADDR*)&_saddr,
				sizeof(_saddr),NULL,NULL,NULL,NULL,
				JL_BOTH)) == INVALID_SOCKET)
			{
				return false;
			}
			
			unsigned long ttl = MULTICAST_TTL;
			
			/* Set IP TTL to traverse up to multiple routers */
			result = setsockopt(_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
			if (result == SOCKET_ERROR) 
			{    
				return false;
			}
			
			/* Disable loopback */
			BOOL fFlag = FALSE;
			result = setsockopt(_sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));
			
			if (result == SOCKET_ERROR) 
			{
				return false;
			}
			return true;
		}
		
		virtual size_t send(void* buf, size_t len)
		{
			size_t sendSize;
			size_t remain = len;
			char* bufPos = (char* )buf;
			
			while (true) {
				
				if (remain > TRANSFER_UNIT_SIZE)
					sendSize = TRANSFER_UNIT_SIZE;
				else
					sendSize = remain;
				
				size_t r = sendto(_sock, bufPos, sendSize, 0, 
					(const struct sockaddr* )&_saddr, 
					sizeof(_saddr));
				
				if (r != sendSize) {
					// log error
					return r;
				}
				
				remain -= sendSize;
				if (remain <= 0)
					break;
				bufPos += sendSize;
			}
			
			return Transfer::send(buf, len);
		}
		
	protected:
		TransferAddress		_addr;
		SOCKET				_sock;
		sockaddr_in			_saddr;
	};
	//////////////////////////////////////////////////////////////////////////
	
	class SendDataWorkItem: public ZQLIB::ZQWorkItem {
	public:
		
		SendDataWorkItem(ZQLIB::ZQThreadPool& pool, 
			Transfer& tran, BufferBlock& block): 
		ZQLIB::ZQWorkItem(pool), _tran(tran), _block(block)
		{
			
		}
		
		~SendDataWorkItem()
		{
			
		}
		
		int run()
		{
			size_t len = _block.size();
			size_t r = _tran.send(_block.getPtr(), len);
			if (r <= len) {
				// log
				return -1;
			}
			
			_block.release();
			
			return 0;
		}
		
		void final(int )
		{
			delete this;
		}
		
	protected:
		Transfer&			_tran;
		BufferBlock&		_block;
	};
	
	//////////////////////////////////////////////////////////////////////////
	
	DataSender::DataSender(ZQLIB::ZQThreadPool&	sendPool): _sendPool(sendPool)
	{
		
	}
	
	DataSender::~DataSender()
	{
		
	}
	
	Transfer* DataSender::createTransfer(const TransferAddress& addr)
	{
		Transfer* tran = NULL;
		
		if (addr.getProto() == TRANSFER_UDP) {
			/// log	支持 udp
			tran = new UDPTransfer(addr);
		}
		else
			if (addr.getProto() == TRANSFER_MULTICAST) {
				/// log 支持 multicast
				tran = new MulticastTransfer(addr);
			}
			else
				return NULL;
			
			if (tran->init()) {
				std::pair< TranMap::iterator, bool > r;
				r = _tranMap.insert(TranMap::value_type(addr, tran));
				
				if (!r.second) {
					delete tran;
				}
				
				return tran;
				
			} else {
				
				delete tran;
				return NULL;
			}
	}
	
	Transfer* DataSender::getTransfer(const TransferAddress& addr)
	{
		TranMap::iterator it = _tranMap.find(addr);
		Transfer* res;
		if (it == _tranMap.end()) {
			res = createTransfer(addr);		
		} else  {
			res = it->second;
		}
		
		assert(res);
		res->refer();
		return res;
	}
	
	void DataSender::sendData(Transfer& tran, BufferBlock& block)
	{
		SendDataWorkItem* item = new SendDataWorkItem(_sendPool, 
			tran, block);
		
		item->start();
	}
	
	size_t DataSender::directSendData(Transfer& tran, BufferBlock& block)
	{
		size_t len = block.size();
		size_t r = tran.send(block.getPtr(), len);
		if (r <= len) {
			// log err
		}
		
		block.release();
		
		return r;
	}
	
} // namespace DataStream {
