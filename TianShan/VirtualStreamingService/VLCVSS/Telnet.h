#ifndef __ZQ_COM_TELNET_H__
#define __ZQ_COM_TELNET_H__

#define	IAC		255		       /* interpret as command: */
#define	DONT	254		       /* you are not to use option */
#define	DO		253		       /* please, you use option */
#define	WONT	252		       /* I won't use option */
#define	WILL	251		       /* I will use option */
#define	SB		250		       /* interpret as subnegotiation */
#define	SE		240		       /* end sub negotiation */

#define GA      249		       /* you may reverse the line */
#define EL      248		       /* erase the current line */
#define EC      247		       /* erase the current character */
#define	AYT		246		       /* are you there */
#define	AO		245		       /* abort output--but let prog finish */
#define	IP		244		       /* interrupt process--permanently */
#define	BREAK	243		       /* break */
#define DM      242		       /* data mark--for connect. cleaning */
#define NOP     241		       /* nop */
#define EOR     239		       /* end of record (transparent mode) */
#define ABORT   238		       /* Abort process */
#define SUSP    237		       /* Suspend process */
#define xEOF    236		       /* End of file: EOF is already used... */

#define TELOPT_BINARY			0	       /* 8-bit data path */
#define TELOPT_ECHO				1	       /* echo */
#define	TELOPT_RCP				2	       /* prepare to reconnect */
#define	TELOPT_SGA				3	       /* suppress go ahead */
#define	TELOPT_NAMS				4	       /* approximate message size */
#define	TELOPT_STATUS			5	       /* give status */
#define	TELOPT_TM				6	       /* timing mark */
#define	TELOPT_RCTE				7	       /* remote controlled transmission and echo */
#define TELOPT_NAOL 			8	       /* negotiate about output line width */
#define TELOPT_NAOP 			9	       /* negotiate about output page size */
#define TELOPT_NAOCRD			10	       /* negotiate about CR disposition */
#define TELOPT_NAOHTS			11	       /* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD			12	       /* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD			13	       /* negotiate about formfeed disposition */
#define TELOPT_NAOVTS			14	       /* negotiate about vertical tab stops */
#define TELOPT_NAOVTD			15	       /* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD			16	       /* negotiate about output LF disposition */
#define TELOPT_XASCII			17	       /* extended ascic character set */
#define	TELOPT_LOGOUT			18	       /* force logout */
#define	TELOPT_BM				19	       /* byte macro */
#define	TELOPT_DET				20	       /* data entry terminal */
#define	TELOPT_SUPDUP			21	       /* supdup protocol */
#define	TELOPT_SUPDUPOUTPUT		22	       /* supdup output */
#define	TELOPT_SNDLOC			23	       /* send location */
#define	TELOPT_TTYPE			24	       /* terminal type */
#define	TELOPT_EOR				25	       /* end or record */
#define	TELOPT_TUID				26	       /* TACACS user identification */
#define	TELOPT_OUTMRK			27	       /* output marking */
#define	TELOPT_TTYLOC			28	       /* terminal location number */
#define	TELOPT_3270REGIME		29	       /* 3270 regime */
#define	TELOPT_X3PAD			30	       /* X.3 PAD */
#define	TELOPT_NAWS				31	       /* window size */
#define	TELOPT_TSPEED			32	       /* terminal speed */
#define	TELOPT_LFLOW			33	       /* remote flow control */
#define TELOPT_LINEMODE			34	       /* Linemode option */
#define TELOPT_XDISPLOC			35	       /* X Display Location */
#define TELOPT_OLD_ENVIRON		36	       /* Old - Environment variables */
#define	TELOPT_AUTHENTICATION	37         /* Authenticate */
#define	TELOPT_ENCRYPT			38	       /* Encryption option */
#define TELOPT_NEW_ENVIRON		39	       /* New - Environment variables */
#define TELOPT_TN3270E			40	       /* TN3270 enhancements */
#define TELOPT_XAUTH			41
#define TELOPT_CHARSET			42	       /* Character set */
#define TELOPT_RSP				43	       /* Remote serial port */
#define TELOPT_COM_PORT_OPTION	44		   /* Com port control */
#define TELOPT_SLE				45	       /* Suppress local echo */
#define TELOPT_STARTTLS			46	       /* Start TLS */
#define TELOPT_KERMIT			47	       /* Automatic Kermit file transfer */
#define TELOPT_SEND_URL			48
#define TELOPT_FORWARD_X		49
#define TELOPT_PRAGMA_LOGON		138
#define TELOPT_SSPI_LOGON		139
#define TELOPT_PRAGMA_HEARTBEAT	140
#define	TELOPT_EXOPL			255	       /* extended-options-list */

#define	TELQUAL_IS		0	       /* option is... */
#define	TELQUAL_SEND	1	       /* send option */
#define	TELQUAL_INFO	2	       /* ENVIRON: informational version of IS */
#define BSD_VAR			1
#define BSD_VALUE		0
#define RFC_VAR			0
#define RFC_VALUE		1

#define CR 13
#define LF 10
#define NUL 0

#include "TCPSocket.h"
#include "Locks.h"

namespace ZQ {
namespace common {

#define iswritable(x) \
	( (x) != IAC && \
	(TCPSocket->opt_states[o_we_bin.index] == ACTIVE || (x) != CR))

#define TELNETRECVLEN 2048

class ZQ_COMMON_API Telnet
{
public:
	Telnet();

	Telnet(const InetAddress &bind, tpport_t port = 23);

	~Telnet();

	/// set the peer address to send message packets to. This can be set
	/// before every connectToServer() call if necessary.
	/// @param host address to send packets to.
	/// @param port number to deliver packets to.
	void setPeer(const InetAddress &host, tpport_t port);

	/// set the telnet server login password
	/// before every connectToServer() call if necessary
	/// @param login password
	void setPWD(const char *strPWD);

	/// connect to the remote telnet server
	/// @param connect timeout in second, -1 means no timeout until server return
	/// @return value connect success will return true, otherwise return false
	bool connectToServer(int32 timeOut = -1);

	/// send the specified telnet command and get the response from telnet server
	/// @param the command to send
	/// @param the response message get from remote telnet server
	/// @param the function will timeout if set timeOut>0, otherwise will wail until server response
	/// @return value send ok will return true, otherwise return false
	bool sendCMD(const char* strCMD,std::string &strResMsg, int32 timeOut = -1);

	/// get the last error from telnet server
	/// @return the error string
	std::string getLastErr();

	::ZQ::common::Mutex _mutex;
	bool _bUsed;

private:
	TCPSocket _TCPSocket;
	std::string _strPWD;
	char _recvBuf[TELNETRECVLEN];
	std::string _strLastErr;
	bool _bConnected;
};

}//namespace common

}//namespace ZQ
#endif __ZQ_COM_TELNET_H__