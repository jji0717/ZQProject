#ifndef _XMLRPCSOCKET_H_
#define _XMLRPCSOCKET_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#ifndef MAKEDEPEND
# include <string>
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

  //! A platform-independent socket API.
  class XmlRpcSocket {
  public:

	// modified by lorenzo, 2005-05-30
    //! Creates a socket. Default type is stream. Returns -1 on failure.
    static int socket(int iSockType = SOCK_STREAM);
	// modify end

    //! Closes a socket.
    static void close(int socket);


    //! Sets socket to perform non-blocking IO. Returns false on failure.
    static bool setNonBlocking(int socket);

    //! Read text from the specified socket. Returns false on error.
    static bool nbRead(int socket, std::string& s, bool *eof);

    //! Write text to the specified socket. Returns false on error.
    static bool nbWrite(int socket, std::string& s, int *bytesSoFar);

	// Added by lorenzo, 2005-05-30
    //! Read text from the specified socket. Returns false on error.
    static bool nbReadFrom(int socket, std::string& s, bool *eof, sockaddr *from, int *fromlen);

    //! Write text to the specified socket. Returns false on error.
    static bool nbWriteTo(int socket, std::string& s, int *bytesSoFar, sockaddr *to, int tolen);
	// add end

    // The next four methods are appropriate for servers.

    //! Allow the port the specified socket is bound to to be re-bound immediately so 
    //! server re-starts are not delayed. Returns false on failure.
    static bool setReuseAddr(int socket);

	//! Bind to a specified port with localhost
    static bool bind(int socket, int port, const char* strIp = NULL);

    //! Set socket in listen mode
    static bool listen(int socket, int backlog);

    //! Accept a client connection request
    static int accept(int socket);


    //! Connect a socket to a server (from a client)
    static bool connect(int socket, std::string& host, int port);


    //! Returns last errno
    static int getError();

    //! Returns message corresponding to last error
    static std::string getErrorMsg();

    //! Returns message corresponding to error
    static std::string getErrorMsg(int error);
  };

} // namespace XmlRpc

#endif
