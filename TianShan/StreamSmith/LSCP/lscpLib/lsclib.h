
#ifndef  _LSC_PROTOCOL_LIBRARY_HEADER_H__
#define _LSC_PROTOCOL_LIBRARY_HEADER_H__

#include <WS2tcpip.h>
#include <tchar.h>

namespace lsc
{
typedef struct _ConnInfo
{

	int						type;

	union
	{
		TCHAR				buffer[MAX_PATH];
		sockaddr_storage	sAddr;
	}
	int						iAddrLen;	//size of the sAddr

}ConnInfo;


typedef ConnInfo PeerInfo ;
typedef ConnInfo LocalInfo ;


class IDialog;

class IConn
{
public:
	///send out data using TCP
	///@return total number of bytes sent
	///@param buf buffer address
	///@param size buffer size in byte
	virtual int		send (void* buf ,  int size ) = 0;

	///send out data using UDP
	///@return total number of bytes sent
	///@param connInfo target info
	///@param buf buffer address
	///@param size buffer size in byte
	virtual int		sendTo(ConnInfo* connInfo ,  void* buf , int size ) = 0;

	///receive data through TCP connection
	///@return the total number of bytes received
	///@param buf buffer to hold the data
	///@param size buffer size in byte
	virtual int		recv( void* buf , int size ) = 0;

	///receive data through UDP
	///@return total number of bytes received
	///@param connInfo target info
	///@param buf buffer to hold the data
	///@param size buffer size in bytes
	virtual int		recvFrom(ConnInfo* connInfo , void* buf , int size ) =0;

	///close the connection
	virtual int		close() = 0;

	///get peer information
	virtual bool	getPeerInfo(PeerInfo& info ) = 0;

	///get local information
	virtual bool	getLocalInfo(LocalInfo& info) = 0;

	///current connection is active or not	
	virtual	bool	isActive( ) = 0;

	///get connection Identity
	///The connection Identity is a global unique ID
	///And every single Data gram can have a different Connection ID although it's not a Connection Oriented
	///@no connection identity is available in UDP protocol
	virtual unsigned __int64 getConnectionIdentity( ) = 0;

	///get attached LscDialog 
	IDialog*		getDialog( ) =0;
};

class IDialog
{
public:	
	virtual void onConnected(IN IConn* conn) = 0;

	virtual void onRequest(IN IConn* conn, IN const void* buf, IN int size) = 0;		

	virtual void onConnectionDestroyed(IN IConn* conn) = 0;	

	virtual void onDatagram(IN IConn* conn , IN const void* buf , int size ) = 0;
};

class ILscDialogFactory
{
public:
	enum
	{
		DIALOG_TYPE_STREAM,
		DIALOG_TYPE_DATAGRAM
	};
	///Create Dialog with the specified type
	///TYPE can only be DIALOG_TYPE_STREAM or DIALOG_TYPE_DATAGRAM
	///@return the created dialog
	virtual IDialog*		createDialog( int type ) =0;
	
	///release the dialog
	///If it's a connection oriented ,dialog will be released when connection is down
	///If it's not,dialog will be released when application down ^_^
	virtual void			releaseDialog( ) =0;

};

}//namespace lsc

#endif//_LSC_PROTOCOL_LIBRARY_HEADER_H__