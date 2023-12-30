
#ifndef _XMLRPCSOURCE_H_
#define _XMLRPCSOURCE_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

// added by lorenzo, 2005-05-30
// for sockaddr
#if defined(_WINDOWS)
# include <winsock2.h>
#else
extern "C" {
# include <sys/socket.h>
}
#endif  // _WINDOWS
// add end

namespace XmlRpc {

#define ST_TCP SOCK_STREAM
#define ST_UDP SOCK_DGRAM
#define ST_OTHER 100

  //! An RPC source represents a file descriptor to monitor
  class XmlRpcSource {
  protected:
	  
	// added by lorenzo, 2005-05-30
	//! Specify the socket type to iSockType. SOCK_STREAM/SOCK_DGRAM
	void setSockType(int iSockType) { _sockType = iSockType; }
	//! Specify the socket type as SOCK_STREAM
	void setTcp() { setSockType(SOCK_STREAM); }
    //! Specify the socket type as SOCK_DGRAM
	void setUdp() { setSockType(SOCK_DGRAM); }
  public:

    //! Constructor
    //!  @param fd The socket file descriptor to monitor.
    //!  @param deleteOnClose If true, the object deletes itself when close is called.
    XmlRpcSource(int fd = -1, bool deleteOnClose = false, int st = ST_TCP);

    //! Destructor
    virtual ~XmlRpcSource();

    //! Return the file descriptor being monitored.
    int getfd() const { return _fd; }
    //! Specify the file descriptor to monitor.
    void setfd(int fd) { _fd = fd; }

    //! Return whether the file descriptor should be kept open if it is no longer monitored.
    bool getKeepOpen() const { return _keepOpen; }
    //! Specify whether the file descriptor should be kept open if it is no longer monitored.
    void setKeepOpen(bool b=true) { _keepOpen = b; }

	//! Return socket type, SOCK_STREAM/SOCK_DGRAM
	int getSockType() const { return _sockType; }
	//! Return whether the socket type is SOCK_STREAM
	bool isTcp() { return (getSockType() == SOCK_STREAM) ? true : false; }
	//! Return whether the socket type is SOCK_DGRAM
	bool isUdp() { return (getSockType() == SOCK_DGRAM) ? true : false; }
	// add end

    //! Close the owned fd. If deleteOnClose was specified at construction, the object is deleted.
    virtual void close();

    //! Return true to continue monitoring this source
    virtual unsigned handleEvent(unsigned eventType) = 0;

  private:

    // Socket. This should really be a SOCKET (an alias for unsigned int*) on windows...
    int _fd;

    // In the server, a new source (XmlRpcServerConnection) is created
    // for each connected client. When each connection is closed, the
    // corresponding source object is deleted.
    bool _deleteOnClose;

    // In the client, keep connections open if you intend to make multiple calls.
    bool _keepOpen;

  // added by lorenzo, 2005-05-30
  // in order to support tcp/udp
  protected:
	// socket type, SOCK_STREAM/SOCK_DGRAM
    int _sockType;

	// to reserve the remote sockaddr in udp mode
	sockaddr _sockaddr;
  // add end
  };
} // namespace XmlRpc

#endif //_XMLRPCSOURCE_H_
