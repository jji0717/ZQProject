#include "BlockingSocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

///////class BlockingSocketException/////////
BlockingSocketException::BlockingSocketException(char* pchMessage)
{
   m_strMessage = pchMessage;
   m_nError     = WSAGetLastError();
}

BlockingSocketException::~BlockingSocketException()
{
}

bool BlockingSocketException::GetErrorMessage(char* lpstrError, int nMaxError)
{
   if( m_nError == 0 )
      _snprintf(lpstrError, static_cast<size_t>(nMaxError), "%s error", m_strMessage.c_str());
   else
      _snprintf(lpstrError, static_cast<size_t>(nMaxError), "%s error 0x%08x", m_strMessage.c_str(), m_nError);
   return true;
}

std::string BlockingSocketException::GetErrorMessage()
{
   char szBuffer[512] = {0};
   GetErrorMessage(szBuffer, 511);

   return szBuffer;
}



/////class BlockingSocket//////
BlockingSocket::BlockingSocket(void)
{
	_sock = 0;
}

BlockingSocket::~BlockingSocket(void)
{
	Cleanup();
}

BlockingSocket* BlockingSocket::CreateInstance() const
{
   return new BlockingSocket();
}

void BlockingSocket::Cleanup()
{
   // doesn't throw an exception because it's called in a catch block
   if( _sock==0 ) 
      return;

   closesocket(_sock);

   _sock = 0;
}

void BlockingSocket::Create(int nType /* = SOCK_STREAM */)
{
   assert( _sock==0 );
   if( (_sock=socket(AF_INET, nType, 0))==INVALID_SOCKET ) 
   {
      throw BlockingSocketException("Create");
   }
}

void BlockingSocket::Bind(LPCSOCKADDR psa) const
{
   assert( _sock!=0 );
   if( bind(_sock, psa, sizeof(SOCKADDR))==SOCKET_ERROR )
   {
      throw BlockingSocketException("Bind");
   }
}

void BlockingSocket::Listen() const
{
   assert( _sock!=0 );
   if( listen(_sock, 5)==SOCKET_ERROR )
   {
      throw BlockingSocketException("Listen");
   }
}

bool BlockingSocket::Accept(BlockingSocket& sConnect, LPSOCKADDR psa) const
{
   BlockingSocket* pConnect = static_cast<BlockingSocket*>(&sConnect);
   assert( _sock!=0 );
   assert( pConnect->_sock==0 );

   int nLengthAddr = sizeof(SOCKADDR);
   pConnect->_sock = accept(_sock, psa, &nLengthAddr);

   if( pConnect->operator SOCKET()==INVALID_SOCKET )
   {
      // no exception if the listen was canceled
      if( WSAGetLastError() !=WSAEINTR ) 
      {
         throw BlockingSocketException("Accept");
      }
      return false;
   }
   return true;
}

void BlockingSocket::Close()
{
   if( _sock && closesocket(_sock)==SOCKET_ERROR )
   {
      // should be OK to close if closed already
      throw BlockingSocketException("Close");
   }
   _sock = 0;
}

void BlockingSocket::Connect(LPCSOCKADDR psa) const
{
   assert( _sock!=0 );
   // should timeout by itself
   if( connect(_sock, psa, sizeof(SOCKADDR))==SOCKET_ERROR )
   {
      throw BlockingSocketException("Connect");
   }
}

int BlockingSocket::Write(const char* pch, int nSize, int nSecs) const
{
   int         nBytesSent        = 0;
   int         nBytesThisTime;
   const char* pch1              = pch;

   do
   {
      nBytesThisTime = Send(pch1, nSize - nBytesSent, nSecs);
      nBytesSent += nBytesThisTime;
      pch1 += nBytesThisTime;
   } while( nBytesSent<nSize );

   return nBytesSent;
}

int BlockingSocket::Send(const char* pch, int nSize, int nSecs) const
{
   assert( _sock!=0 );
   
   // returned value will be less than nSize if client cancels the reading
   FD_SET  fd = { 1, _sock };
   TIMEVAL tv = { nSecs, 0 };

   if( select(0, NULL, &fd, NULL, &tv)==0 )
   {
      throw BlockingSocketException("Send timeout");
   }

   int nBytesSent;
   if( (nBytesSent=send(_sock, pch, nSize, 0))==SOCKET_ERROR )
   {
      throw BlockingSocketException("Send");
   }

   return nBytesSent;
}

bool BlockingSocket::CheckReadability() const
{
   assert( _sock!=0 );
   
   FD_SET  fd = { 1, _sock };
   TIMEVAL tv = { 0, 0 };

   const int iRet = select(0, &fd, NULL, NULL, &tv);
   
   if( iRet==SOCKET_ERROR )
   {
      throw BlockingSocketException("Socket Error");
   }

   return iRet == 1;
}

int BlockingSocket::Receive(char* pch, int nSize, int nSecs) const
{
   assert(_sock!=0);
   
   FD_SET  fd = { 1, _sock };
   TIMEVAL tv = { nSecs, 0 };

   if( select(0, &fd, NULL, NULL, &tv)==0 )
   {
      throw BlockingSocketException("Receive timeout");
   }

   int nBytesReceived;
   if( (nBytesReceived=recv(_sock, pch, nSize, 0))==SOCKET_ERROR )
   {
      throw BlockingSocketException("Receive");
   }

   return nBytesReceived;
}

int BlockingSocket::ReceiveDatagram(char* pch, int nSize, LPSOCKADDR psa, int nSecs) const
{
   assert( _sock!=0 );

   FD_SET  fd = { 1, _sock };
   TIMEVAL tv = { nSecs, 0 };

   if( select(0, &fd, NULL, NULL, &tv)==0 )
   {
      throw BlockingSocketException("Receive timeout");
   }

   // input buffer should be big enough for the entire datagram
   int nFromSize = sizeof(SOCKADDR);
   int nBytesReceived = recvfrom(_sock, pch, nSize, 0, psa, &nFromSize);

   if( nBytesReceived==SOCKET_ERROR )
   {
      throw BlockingSocketException("ReceiveDatagram");
   }

   return nBytesReceived;
}

int BlockingSocket::SendDatagram(const char* pch, int nSize, LPCSOCKADDR psa, int nSecs) const
{
   assert( _sock!=0 );

   FD_SET  fd = { 1, _sock };
   TIMEVAL tv = { nSecs, 0 };

   if( select(0, NULL, &fd, NULL, &tv)==0 )
   {
      throw BlockingSocketException("Send timeout");
   }

   int nBytesSent = sendto(_sock, pch, nSize, 0, psa, sizeof(SOCKADDR));
   if( nBytesSent==SOCKET_ERROR )
   {
      throw BlockingSocketException("SendDatagram");
   }

   return nBytesSent;
}

void BlockingSocket::GetPeerAddr(LPSOCKADDR psa) const
{
   assert( _sock!=0 );

   // gets the address of the socket at the other end
   int nLengthAddr = sizeof(SOCKADDR);
   if( getpeername(_sock, psa, &nLengthAddr)==SOCKET_ERROR )
   {
      throw BlockingSocketException("GetPeerName");
   }
}

void BlockingSocket::GetSockAddr(LPSOCKADDR psa) const
{
   assert( _sock!=0 );

   // gets the address of the socket at this end
   int nLengthAddr = sizeof(SOCKADDR);
   if( getsockname(_sock, psa, &nLengthAddr)==SOCKET_ERROR ) 
   {
      throw BlockingSocketException("GetSockName");
   }
}

sockaddr_in BlockingSocket::GetHostByName(const char* pchName, int nPort)
{
   hostent* pHostEnt = gethostbyname(pchName);
   
   if( pHostEnt==NULL)
   {
      throw BlockingSocketException("GetHostByName");
   }

   ULONG* pulAddr = (ULONG*) pHostEnt->h_addr_list[0];
   SOCKADDR_IN sockTemp;
   sockTemp.sin_family = AF_INET;
   sockTemp.sin_port = htons(nPort);
   sockTemp.sin_addr.s_addr = *pulAddr; // address is already in network byte order
   return sockTemp;
}

const char* BlockingSocket::GetHostByAddr(LPCSOCKADDR psa)
{
   hostent* pHostEnt = gethostbyaddr((char*) &((LPSOCKADDR_IN) psa)
            ->sin_addr.s_addr, 4, PF_INET);
   
   if( pHostEnt==NULL )
   {
      throw BlockingSocketException("GetHostByAddr");
   }

   return pHostEnt->h_name; // caller shouldn't delete this memory
}
