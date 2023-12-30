
#include "RtspSocket.h"

#ifndef MAKEDEPEND

#if defined(_WINDOWS)

# include <stdio.h>
# include <winsock2.h>
//# pragma lib(WS2_32.lib)

# define EINPROGRESS	WSAEINPROGRESS
# define EWOULDBLOCK	WSAEWOULDBLOCK
# define ETIMEDOUT	    WSAETIMEDOUT
#else
extern "C" {
# include <unistd.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <errno.h>
# include <fcntl.h>
}
#endif  // _WINDOWS

#endif // MAKEDEPEND

#if defined(_WINDOWS)
  
static void initWinSock()
{
  static bool wsInit = false;
  if (! wsInit)
  {
    WORD wVersionRequested = MAKEWORD( 2, 0 );
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
    wsInit = true;
  }
}

#else

#define initWinSock()

#endif // _WINDOWS


// These errors are not considered fatal for an IO operation; the operation will be re-tried.
bool
RtspSocket::nonFatalError(int errorcode)
{
	return ( errorcode == NO_ERROR || errorcode == EINPROGRESS || errorcode == EAGAIN || errorcode == EWOULDBLOCK || errorcode == EINTR);
}



int
RtspSocket::socket()
{
  initWinSock();

  SOCKET s = INVALID_SOCKET;

#if defined(_WINDOWS)
  s = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
  s=  ::socket(AF_INET, SOCK_STREAM, 0);
#endif	// _WINDOWS
  
  return (int) s;
}


void
RtspSocket::close(int fd)
{
	struct linger linger;

	// -- modified by Bernie, to set the linger option
	// -- this will immediately close the socket
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&linger,
		sizeof(linger));

#if defined(_WINDOWS)
	shutdown(fd, SD_BOTH);
	closesocket(fd);
#else
	::close(fd);
#endif // _WINDOWS
}




bool
RtspSocket::setNonBlocking(int fd)
{
#if defined(_WINDOWS)
  unsigned long flag = 1;
  return (ioctlsocket((SOCKET)fd, FIONBIO, &flag) == 0);
#else
  return (fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
#endif // _WINDOWS
}


bool
RtspSocket::setReuseAddr(int fd)
{
  // Allow this port to be re-bound immediately so server re-starts are not delayed
  int sflag = 1;
  return (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sflag, sizeof(sflag)) == 0);
}

// Bind to a specified port and address
bool RtspSocket::bind(int fd, int port, const char* strIp)
{
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;

  unsigned long ulAddr;
  if (NULL != strIp)
	ulAddr= inet_addr(strIp);
  else
	  ulAddr = INADDR_ANY;

  saddr.sin_addr.s_addr = ulAddr;
  saddr.sin_port = htons((u_short) port);
  return (::bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) == 0);
}


// Set socket in listen mode
bool 
RtspSocket::listen(int fd, int backlog)
{
  return (::listen(fd, backlog) == 0);
}


int
RtspSocket::accept(int fd)
{
  struct sockaddr_in addr;
#if defined(_WINDOWS)
  int
#else
  socklen_t
#endif
    addrlen = sizeof(addr);

  return (int) ::accept(fd, (struct sockaddr*)&addr, &addrlen);
}


    
// Connect a socket to a server (from a client)
bool
RtspSocket::connect(int fd, std::string& host, int port)
{
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;

  struct hostent *hp = gethostbyname(host.c_str());
  if (hp == 0) return false;

  saddr.sin_family = hp->h_addrtype;
  memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
  saddr.sin_port = htons((u_short) port);

  // For asynch operation, this will return EWOULDBLOCK (windows) or
  // EINPROGRESS (linux) and we just need to wait for the socket to be writable...
  int result = ::connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));
  if(result ==0)
  {
	  return true;
  }
  else
  {
	  int errorcode = getError(fd);
	  return nonFatalError(errorcode);
  }
}



// Read available text from the specified socket. Returns false on error.
int 
RtspSocket::nbRead(int fd, std::string& s, bool *eof, int& error)
{
  int READ_SIZE = 4096;   // Number of bytes to attempt to read at a time
  char* readBuf;
  readBuf = new char[READ_SIZE];

  *eof = false;
  error = 0;

  int n;

  while ( ! *eof) 
  {
#if defined(_WINDOWS)
    n= recv(fd, readBuf, READ_SIZE-1, 0);
#else
    n = read(fd, readBuf, READ_SIZE-1);
#endif

	if (n > 0) 
	{
		readBuf[n] = 0;
		s.append(readBuf, n);
		*eof = true;	// already got all bytes
    }
	else if(n <= 0) 
	{
		int err = WSAGetLastError();
		if(err==WSAEMSGSIZE)
		{
			delete []readBuf;
			READ_SIZE*=2;	// buffer too small, double it
			readBuf = new char[READ_SIZE];
			*eof = false;
		}
		else	// if(err!=NO_ERROR)
		{
			error = err;
			*eof = true;	// some error did occur
		}
	}
	
  }

  delete []readBuf;
  return n;
}


// Write text to the specified socket. Returns false on error.
int 
RtspSocket::nbWrite(int fd, std::string& s, int *bytesSoFar)
{
  int nToWrite = int(s.length()) - *bytesSoFar;
  char *sp = const_cast<char*>(s.c_str()) + *bytesSoFar;
  
  int n;
  
  while ( nToWrite > 0) {
#if defined(_WINDOWS)
    n = send(fd, sp, nToWrite, 0);
#else
    int n = write(fd, sp, nToWrite);
#endif

    if (n > 0) {
      sp += n;
      *bytesSoFar += n;
      nToWrite -= n;
    } 
	else {
      return -1;   // Error
    }
  }
  
  return n;
}


// Returns last errno
int 
RtspSocket::getError(int socket)
{
int nSpecify = 0;
int nGeneral = ::WSAGetLastError();

if(nGeneral==0)
{
	int nLen = sizeof(nSpecify);
	::getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&nSpecify, &nLen);
}

return (nSpecify==0)?nGeneral:nSpecify;
}

// Returns message corresponding to errno... well, it should anyway
std::string 
RtspSocket::getErrorMsg(int error)
{
  char err[60];
  _snprintf(err,sizeof(err),"error %d", error);
  return std::string(err);
}


