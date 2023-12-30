// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: FtpConnection.h,v 1.10 2004/08/17 02:54:10 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpConnection.h $
// 
// 2     10-12-08 18:03 Fei.huang
// + add mapVirtualDirectory 
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     08-12-19 16:30 Yixin.tian
// 
// 3     08-11-18 11:11 Jie.zhang
// merge from TianShan1.8
// 
// 3     08-08-15 17:39 Xia.chen
// 
// 2     08-07-11 13:51 Jie.zhang
// add export content support
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:38 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 3     07-04-12 19:18 Jie.zhang
// 
// 2     07-03-22 18:36 Jie.zhang
// 
// 5     05-04-14 18:14 Jie.zhang
// 
// 4     05-04-11 19:03 Jie.zhang
// 
// 3     05-02-06 17:03 Jie.zhang
// 
// 2     04-12-08 15:50 Jie.zhang
// 
// 5     04-11-22 14:27 Jie.zhang
// 
// 4     04-11-19 17:06 Jie.zhang
// 
// 3     04-08-27 17:54 Jian.shen
// add cluster node info viewer in manUtil
// Revision 1.10  2004/08/17 02:54:10  jshen
// no message
//
// Revision 1.9  2004/08/12 09:06:37  jshen
// remove End Thread list
//
// Revision 1.8  2004/08/09 06:39:12  jshen
// before change to UNICODE
//
// Revision 1.7  2004/08/05 07:08:00  jshen
// no message
//
// Revision 1.6  2004/07/12 06:44:47  jshen
// because of the IP change of the CVS server
//
// Revision 1.5  2004/07/07 08:53:37  jshen
// move EndedFtpXferThreadList class implemetation to .cpp file
//
// Revision 1.4  2004/06/30 03:28:32  jshen
// add IOMoudle and fix some bugs in handling errors
//
// Revision 1.3  2004/06/17 05:53:06  jshen
// *** empty log message ***
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================

#ifndef __FTPCONNECTION_H__
#define __FTPCONNECTION_H__
#include "TermService.h"

#include "utils.h"

#include "FtpSock.h"

#define FTPS_COREVERSION "1.0.9"    //core FTP server version number

#define FTPS_MAXLOGINSIZE 16    //max num of char a login can be
#define FTPS_MAXPWSIZE    16    //max num of char a password can be
#define FTPS_STDSOCKTIMEOUT 10   //standard timeout for a connection (in sec)
#define FTPS_MAXCWDSIZE  256    //max length of the CWD path
#define FTPS_LINEBUFSIZE 256    //max length of a FTP command/response line
#define FTPS_SUBSYSTEMNAME "FTPD"   //subsystem name used for the program log
#define FTPS_LOGINFAILDELAY 2   //number of seconds to delay after a failed login attempt

#include "NativeThreadPool.h"


class FtpSite;    //dummy declaration (needed to avoid compiler errors)
class FtpsXfer;
class FtpXferEx;
class FtpConnection;

using namespace ZQ::common;
///////////////////////////////////////////////////////////////////////////////
// FTP connection class
///////////////////////////////////////////////////////////////////////////////

class FtpConnection : public ThreadRequest, public TermService
{
	friend class FtpsXfer;
	friend class FtpXferEx;
	// public fuction
public:
	FtpConnection(SOCKET sd, FtpSite& site, int MaxConnection, NativeThreadPool& Pool, char *serverip = NULL, int resolvedns = 1);
	virtual ~FtpConnection();

	// implementation of Thread
	virtual bool init(void);
	virtual int run(void);
	void final(int retcode =0, bool bCancelled =false);
	
	void	Close();

	//Executes the FTP commands
	virtual bool execFtp(int argc, char **argv);

	//FTP functions
	virtual bool doABOR(int argc, char **argv);
	virtual bool doACCT(int argc, char **argv);
	virtual bool doALLO(int argc, char **argv);
	virtual bool doAPPE(int argc, char **argv);
	virtual bool doAUTH(int argc, char **argv);
	virtual bool doCDUP(int argc, char **argv);
	virtual bool doCWD(int argc, char **argv);
	virtual bool doDELE(int argc, char **argv);
	virtual bool doFEAT(int argc, char **argv);
	virtual bool doHELP(int argc, char **argv);
	virtual bool doLIST(int argc, char **argv);
	virtual bool doMDTM(int argc, char **argv);
	virtual bool doMKD(int argc, char **argv);
	virtual bool doMODE(int argc, char **argv);
	virtual bool doNLST(int argc, char **argv);
	virtual bool doNOOP(int argc, char **argv);
	virtual bool doOPTS(int argc, char **argv);
	virtual bool doPASS(int argc, char **argv);
	virtual bool doPASV(int argc, char **argv);
	virtual bool doPBSZ(int argc, char **argv);
	virtual bool doPORT(int argc, char **argv);
	virtual bool doPROT(int argc, char **argv);
	virtual bool doPWD(int argc, char **argv);
	virtual bool doQUIT(int argc, char **argv);
	virtual bool doREIN(int argc, char **argv);
	virtual bool doREST(int argc, char **argv);
	virtual bool doRETR(int argc, char **argv);
	virtual bool doRMD(int argc, char **argv);
	virtual bool doRNFR(int argc, char **argv);
	virtual bool doRNTO(int argc, char **argv);
	virtual bool doSIZE(int argc, char **argv);
	virtual bool doSMNT(int argc, char **argv);
	virtual bool doSTAT(int argc, char **argv);
	virtual bool doSTOR(int argc, char **argv, int mode = 0);
	virtual bool doSTOU(int argc, char **argv);
	virtual bool doSYST(int argc, char **argv);
	virtual bool doTYPE(int argc, char **argv);
	virtual bool doUSER(int argc, char **argv);

	//Functions that should be overwritten
	//by a class that inherits from FtpConnection
	virtual bool doSITE(int argc, char **argv);

	//Functions used to set/get FTP server state
	SOCKET getSocketDesc();

	bool isEncData();
	char *serverName();
	char *serverIP();
	char *serverPort();
	char *clientName();
	char *clientIP();
	char *clientPort();
	FtpSite *rtEnv();
	void setLogin(char *login);
	char *getLogin();
	void setUserID(int uid);
	int getUserID();
	int fPASV();
	void setQuit(bool quit);
	int isQuit();
	void setAbor(bool abor);
	int isAbor();
	time_t lastActive();

	int nListThreads();
	int nDataThreads();
	void setLastActive(time_t ti =-1);
	void decDataThreads();
	void incDataThreads();

	//Used for logging and to send responses back to the FTP client
	bool eventHandler(const int argc, const char **argv, const char *mesg, char *ftpcode, int flagclient, int flagaccess, int flagprog, int respmode = 0);
	//Used to build response lines to send back to the FTP client (writes to _linebuffer)
	int writeToLineBuffer(char *lineformat, ...);

	//Used to parse the address in the format used in the PORT (and PASV) command
	static int parsePortAddr(char *rawaddr, char *addr, int maxaddr, char *port, int maxport);

protected:
	void defaultResp(int argc, char **argv);
//	int  checkPermissions(const char *cmd, const char *arg, const char *pflags, const int flagdir = 0);
	bool appendLineBuf(const char *str);

	void mapVirtualDir(const std::string& path, std::string& real_root, std::string& basename); 
public:
	std::string		_strFilename;

protected:
	FtpSite& _site;		  //pointer to the class containing the site information

	SOCKET  _sd;		  //socket desc of the client connection

	int MaxConnectionNum; // the max connection number the ftp allow

	FtpSock *_pasv_sock; //create the sockets class
	FtpSock *_port_sock;
	
	NativeThreadPool	& _Pool;
	
	FtpSock *_cli_sock;

	bool _bEncData;  //if _bEncData != 0, encrypt the data channel (set w/ PROT)

	//Client information
	char _clientIp[SOCK_IPADDRLEN];    //ip address the client is connecting from
	char _clientPort[SOCK_PORTLEN];    //port number the client is using
	char _clientName[256];             //name of the client machine

	//Server information
	char _serverip[SOCK_IPADDRLEN];    //IP addr the server is running on
	char _serverPort[SOCK_PORTLEN];    //port number the server is running on
	char _serverName[256];             //name of the server machine

	//User information
	char _login[FTPS_LINEBUFSIZE];     //client login name
	int _userId;                       //User ID
	std::string _cwd;                        //user's current working directory
	std::string _userroot;                   //root path for the user (absolute path)
	char _pwd[FTPS_LINEBUFSIZE];

	//Control information
	bool _bQuit;             //if _bQuit != 0, the client wants to quit
	bool _bAbor;             //used to signal an ABOR command
	time_t _lastActive;          //last time the user was active

	//Counters used to keep track of the number of transfer threads (total threads = _nListThreads + _nDataThreads)
	long volatile _nListThreads;       //number of active list threads (Ex. LIST, NLST)
	long volatile _nDataThreads;       //number of active data transfer threads (Ex. RETR, STOR)

	//Transfer Mode Information
	int _fPasv;         //is the mode set to PASV (default is active mode)
	SOCKET _pasvSd;     //socket desc for socket created in PASV cmnd
	char _portAddr[SOCK_IPADDRLEN];    //stores the PORT addr to use for data connect
	char _portPort[SOCK_PORTLEN];      //stores the PORT port to use for data connect
	char _type;            //type of transfer "A" = ascii and "I" = image
	long _restOffset;      //offset to use for resuming transfers

	//buffer used for renaming a file
	std::string _rendFilename;

	//buffer used to store temporary strings
	char _linebuffer[FTPS_LINEBUFSIZE];

};

#endif //__FTPCONNECTION_H__
