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
// Ident : $Id: FtpConnection.cpp,v 1.15 2004/08/12 09:06:39 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpConnection.cpp $
// 
// 10    8/26/15 11:22a Li.huang
// 
// 9     5/20/14 2:25p Li.huang
// fix empty string .size()-1
// 
// 8     12/12/13 1:48p Hui.shao
// %lld
// 
// 7     5/15/12 2:01p Li.huang
// fix bug  16451
// 
// 6     5/14/12 2:34p Li.huang
// add log for monitor increas and reduce connections number
// 
// 5     5/04/12 4:51p Li.huang
// 
// 7     5/04/12 4:50p Li.huang
// 
// 6     5/04/12 4:47p Li.huang
// add log
// 
// 5     4/09/12 2:10p Li.huang
// add log 
// 
// 4     10-12-15 11:11 Fei.huang
// * fix: filename not visible on Windows
// 
// 3     10-12-08 18:04 Fei.huang
// * polish dup codes for stricmp
// * use mapVirtualDirectory to map volume name to a real full path on the
// server
// 
// 2     10-11-15 18:46 Fei.huang
// - comment out FtpOverStream flag on linux
// 
// 18    10-11-09 19:19 Fei.huang
// * fix: append ftp root before content
// * fix: using 64bit format to write large filesize to buffer.
// 
// 17    10-11-02 14:29 Li.huang
//  megre from 1.10
// 
// 16    10-10-22 16:14 Fei.huang
// * recognize file extension of VVC 
// bug 13445
// 
// 15    09-07-24 15:08 Xia.chen
// 
// 14    09-06-26 17:12 Yixin.tian
// 
// 13    09-06-16 17:11 Xia.chen
// 
// 12    09-06-15 16:55 Xia.chen
// support rest command
// 
// 11    09-01-21 14:27 Jie.zhang
// change SYST return string for window
// 
// 10    09-01-09 11:29 Yixin.tian
// modify warning
// 
// 9     08-12-19 16:30 Yixin.tian
// merge for Linux OS
// 
// 8     08-11-18 11:11 Jie.zhang
// merge from TianShan1.8
// 
// 15    08-11-07 15:47 Xia.chen
// 
// 14    08-10-25 12:35 Xia.chen
// 
// 13    08-09-17 14:00 Xia.chen
// 
// 12    08-09-17 11:59 Jie.zhang
// add some log when authorization failure
// 
// 11    08-09-11 17:43 Jie.zhang
// getExportUrl add transfer bitrate
// 
// 10    08-09-03 17:29 Fei.huang
// 
// 9     08-08-28 18:43 Fei.huang
// verify trick files of the content while downloading
// 
// 8     08-08-15 17:35 Xia.chen
// 
// 7     08-07-18 17:01 Jie.zhang
// fixed doSize no pasv port 
// 
// 6     08-07-11 13:51 Jie.zhang
// add export content support
// 
// 5     08-06-24 19:18 Jie.zhang
// fix an issue maybe cause using CPU 100%, checkstatus did not process
// error
// 
// 4     08-04-28 13:47 Jie.zhang
// 
// 3     08-03-27 16:50 Jie.zhang
// 
// 3     08-03-07 18:38 Jie.zhang
// 
// 2     08-02-18 18:29 Jie.zhang
// changes check in
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:38 Fei.huang
// remove permission check
// 
// 2     07-09-13 17:22 Fei.huang
// get size from vstream IO
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================
//#define _WIN32_WINNT (0x403)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>  //for isdigit()
#include <stdarg.h> //for va_list, etc.

#include "FtpConnection.h"
#include "FtpsXfer.h"
#include "FtpSite.h"
//#include "Crypto.h"
#include "utils.h"

#include "CmdLine.h"
#include "FtpPushSess.h"
#include "FtpXferEx.h"
#include "CPECfg.h"
#include "TianShanDefines.h"

#include "FTPAccount.h"
#include "SystemUtils.h"
#define _MAINLOOPFREQ 2     //number of seconds the main loop runs at
#define _USERLOOPFREQ 1     //the freq of the loop in the user thread

//#define USE_EXTEND_XFER
#ifdef USE_EXTEND_XFER
typedef FtpsXferExtension FtpXferClass;
#else
typedef FtpsXfer FtpXferClass;
#endif

#include "CECommon.h"


#define FtpConn			"FtpConn"

#define MOLOG		glog

using namespace ZQ::common;


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// FTP connection class
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
FtpConnection::FtpConnection(SOCKET sd, FtpSite& site, int MaxConnetion, NativeThreadPool& Pool, char *serverip /*=NULL*/, int resolvedns /*=1*/)
: ThreadRequest(Pool), TermService(sd), _site(site), MaxConnectionNum(MaxConnetion), _Pool(Pool)
{

	//set the client's socked descriptor
	_sd			= sd;
	_cli_sock   = new FtpSock(_sd);
	_cli_sock->SetTcpNoDelay(true);

	_bEncData	= 0;  //initialize to use a clear data connection


	// init the file data transfer socket 
	_pasv_sock	= NULL;
	_port_sock	= NULL;

	//Initialize the user information
	*_login		 = '\0';        //initialize the login to an "" string
	_userId		 = 0;           //initialize the user ID to 0
	*_clientName = '\0';   //initialize the user's address name
	*_serverName = '\0';   //initialize the server's address name

	//initialize the user's root directory to the default home directory
// 	_site.buildPathHome(_userroot);      //absolute path for the site home dir
	_userroot = _site._defaultRoot;
	//Initialize the control information
	_bQuit		= false;             //if _bQuit != 0, the client wants to quit
	_bAbor		= false;             //used to signal an ABOR command
	_lastActive = time(NULL);  //last time the user was active

	//Initialize the thread counters (total threads = _nListThreads + _nDataThreads)
	_nListThreads = 0;       //number of active list threads
	_nDataThreads = 0;       //number of active data threads

	//Initialize the transfer mode information
	_fPasv		  = 0;				//initialize to active mode
	_pasvSd		  = SOCK_INVALID;   //initialize the socket desc to an invalid socket
	*_portAddr	  = '\0';	  //initialize the PORT addr to use for data connect
	*_portPort	  = '\0';     //initialize the PORT port to use for data connect
	_type		  = 'A';      //initialize the transfer type to 'A' -> ASCII mode
	_restOffset	  = 0;		  //initialize the offset to use for resuming transfers

	*_linebuffer  = '\0';	  //initialize the line buffer

	//get the user's address
	_cli_sock->getPeerAddrPort(_clientIp,sizeof(_clientIp),_clientPort,sizeof(_clientPort));

	//set the local address
	_cli_sock->getAddrPort(_serverip,sizeof(_serverip),_serverPort,sizeof(_serverPort));
	if (serverip != NULL)
	{
		strncpy(_serverip,serverip,sizeof(_serverip)-1);
		_serverip[sizeof(_serverip)-1] = '\0';
	}

	resolvedns = 0;  //add by Jie 10/06/2005 this will take long time
	//get the user's address name if resolvedns is not 0
	if (resolvedns != 0)
	{
		_cli_sock->GetAddrName(_clientIp,_clientName,sizeof(_clientName));
		_cli_sock->GetAddrName(_serverip,_serverName,sizeof(_serverName));
	}
}

FtpConnection::~FtpConnection()
{
	if (_cli_sock)
		delete _cli_sock;
}


bool FtpConnection::init(void)
{
	return true;
}

int FtpConnection::run(void)
{
	//FtpSock sock;          //Sockets class (contains all the network func)
	CmdLine cmdline;       //Stores the FTP received command line

	char **argv;        //array of pointers to the individual command line arguments
	int argc = 0;       //the number of command line arguments

	int sockstatus;     //the status of the socket (has data been received)

	int flaguser = 0;   //is the user allowed access (1 = user is allowed)
	bool bAuth	 = false;   //is the user authenticated  (1 = user if authenticated)


	if (_site._ftpSkipAuthentication)
		bAuth = true;
	
//	InterlockedIncrement(&_site._nConnections);    //increment the number of current client connections
	{
		ZQ::common::MutexGuard gd(_site._lockConCount);
		_site._nConnections++;
	}

	//Set the socket to keepalive to detect a dead socket
	if (_cli_sock->SetKeepAlive() == 0)
		MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "User[%s]: Failed to set socket to KeepAlive"), getLogin());

	//Set the socket to receive OOB data (for ABOR)
	if (_cli_sock->SetRecvOOB() == 0)
		MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "User[%s]: Failed to set socket to receive OOB data"), getLogin());

	//Send the Login string to the client
	eventHandler(0,NULL,_LOGINSTR,"220",1,1,0);
    

	_site.clientConnAdd(this);


	while (!isQuit() && ! _site._bQuit)
	{
		sockstatus = _cli_sock->CheckStatus(_USERLOOPFREQ);
		if (sockstatus > 0)
		{
			//receive the FTP command and check if the connection was broken
			if (!recvCommand(cmdline._cmdline))
			{
				if (!_site._bQuit)
				{
#ifdef ZQ_OS_MSWIN
					int dwRet = WSAGetLastError();
					MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "Error receive ftp command from client (%s disconnected), error[%s] code[%d]"), 
						getLogin(), getErrMsg(dwRet).c_str(), dwRet);
#else
					MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "Error receive ftp command from client (%s disconnected), error[%s] code[%d]"), 
						getLogin(), getErrMsg().c_str(), SYS::getLastErr());
#endif
				}

				doQUIT(0,NULL);
				break;
			}
			//update the user's current information
			_lastActive = time(NULL);  //update the last active time
		}
		else if (sockstatus == 0)
		{
			cmdline._cmdline="~";      //used to indicate no activity
		}
		else
		{
			//sockstatus < 0, error
			if (!_site._bQuit)
			{
#ifdef ZQ_OS_MSWIN
				int dwRet = WSAGetLastError();
				MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "Error receive ftp command from client (%s disconnected), error[%s] code[%d]"), 
					getLogin(), getErrMsg(dwRet).c_str(), dwRet);
#else
				MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "Error receive ftp command from client (%s disconnected), error[%s] code[%d]"), 
					getLogin(), getErrMsg().c_str(), errno);
#endif
			}

			doQUIT(0,NULL);
			break;
		}
		
		if (isQuit() || _site._bQuit)
		{
			break;
		}

		//parse the input from the user
		argv = cmdline.parse(&argc);

		if (argc <=0)  //nothing was entered
		{
			eventHandler(argc,(const char**)argv,"command not understood","500",1,1,0);
			continue;
		}


		//Make sure the user is logged in
		//(RDS allows any login and password)
		//If the user is not logged in only allow the commands
		//USER, PASS, or QUIT
		if (!bAuth)
		{
			//user is not authenticated
			switch (*(argv[0]))
			{
			case '~':
				{
					if (sockstatus > 0) //if a command was received
						eventHandler(argc,(const char**)argv,"Please login with USER and PASS.","530",1,1,0);
					//else Do Nothing (idle indicator)
				}
				break;

			case 'U': case 'u':
				{
					if (stricmp(argv[0],"USER") == 0)
						flaguser = doUSER(argc,argv);
					else
						eventHandler(argc,(const char**)argv,"Please login with USER and PASS.","530",1,1,0);
				} break;

			case 'P': case 'p':
				{
					if (flaguser != 0 && stricmp(argv[0],"PASS") == 0)
					{
						if (doPASS(argc,argv) == 0)
						{
							flaguser = false;
							bAuth = false;  //the user was not authenticated
						}
						else
							bAuth = true;  //the user was authenticated
					}
					else if(stricmp(argv[0],"PORT") == 0)
					{
						doPORT(argc,argv);
					}
					else
						eventHandler(argc,(const char**)argv,"Please login with USER and PASS.","530",1,1,0);
				}
				break;

			case 'Q': case 'q':
				{
					if (stricmp(argv[0],"QUIT") == 0)
						doQUIT(argc,argv);
					else
						eventHandler(argc,(const char**)argv,"Please login with USER and PASS.","530",1,1,0);
				}
				break;

			default:
					eventHandler(argc,(const char**)argv,"Please login with USER and PASS.","530",1,1,0);
				break;

			} // end switch
		}
		else //user is authenticated
		{
			switch (*(argv[0]))
			{
			case '~':
				{
					if (sockstatus > 0)     //if a command was received
						execFtp(argc,argv);
					//else Do Nothing (idle indicator)
				}
				break;

			case 'P': case 'p':
				{
					if (stricmp(argv[0],"PASS") == 0)
					{
						if (doPASS(argc,argv) == 0)
						{
							flaguser = 0;
							bAuth = false;  //the user was not authenticated
						}
					}
					else
						execFtp(argc,argv);
				}
				break;

			case 'U': case 'u':
				{
					if (stricmp(argv[0],"USER") == 0)
					{
						bAuth = false;  //reset the authorization
						flaguser = doUSER(argc,argv);
					}
					else
						execFtp(argc,argv);
				}
				break;

			default:
				{
					//check if the user entered a data transfer command (Ex. RETR, STOR)
					bool bDlUlCmd = false;
					char commandbuffer[5];

					//Extract the FTP command
					strncpy(commandbuffer,cmdline._cmdline.c_str(),sizeof(commandbuffer)-1);
					commandbuffer[sizeof(commandbuffer)-1] = '\0';
					static char* rscmd[] = {"RETR", "STOR", "STOU", "APPE"};
					for (int i = 3; i>=0; i--)
						if (stricmp(commandbuffer,rscmd[i]) ==0)
							bDlUlCmd = true;

					if (bDlUlCmd)
					{
						//only allow 1 data transfer at a time
						if (nDataThreads() > 0)
						{
/*  Jun 10 2005 remove by Jie because the data connection thread may not mcppublish, and the client will put again.
							eventHandler(argc,(const char**)argv,"Only 1 transfer per control connection is allowed.","550",1,1,0);
							break;
*/						}
					}
					//execute the FTP command
					execFtp(argc,argv);
				}
				break;

			} // end switch
		}
	}
	
	_cli_sock->Close(); //close the control connection
	MOLOG(Log::L_INFO, CLOGFMT(FtpConn, "Connection closed (%u), %s disconnected"), getSocketDesc(),getLogin());


	//DON'T delete the FtpConnection class until ALL connection threads have exited
	setAbor(true); setQuit(true); //stop all threads


	if (_nListThreads + _nDataThreads > 0)
	{
		doQUIT(0, NULL);
	}
	
	
//	InterlockedDecrement(&_site._nConnections);     //decrement the number of current client connections
	{
		ZQ::common::MutexGuard gd(_site._lockConCount);
		_site._nConnections--;
	}

	// remove myself from the connections vector
	_site.clientConnRemove(this);


	return 0;
}

void FtpConnection::final(int retcode, bool bCancelled)
{
	//delete this;
}


//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// Executes the FTP commands
////////////////////////////////////////

bool FtpConnection::execFtp(int argc, char **argv)
{
	bool retval = false;     //Stores the return value of the FTP function

	if (argc < 1)
		return false;

	switch (*(argv[0]))
	{
	case 'A': case 'a':
		{
			if (stricmp(argv[0],"ABOR") == 0)
				retval = doABOR(argc,argv);
			else if (stricmp(argv[0],"ACCT") == 0)
				retval = doACCT(argc,argv);
			else if (stricmp(argv[0],"ALLO") == 0)
				retval = doALLO(argc,argv);
			else if (stricmp(argv[0],"APPE") == 0)
				retval = doAPPE(argc,argv);
			else if (stricmp(argv[0],"AUTH") == 0)
				retval = doAUTH(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'C': case 'c':
		{
			if (stricmp(argv[0],"CDUP") == 0)
				retval = doCDUP(argc,argv);
			else if (stricmp(argv[0],"CWD") == 0)
				retval = doCWD(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'D': case 'd':
		{
			if (stricmp(argv[0],"DELE") == 0)
				retval = doDELE(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'F': case 'f':
		{
			if (stricmp(argv[0],"FEAT") == 0)
				retval = doFEAT(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'H': case 'h':
		{
			if (stricmp(argv[0],"HELP") == 0)
				retval = doHELP(argc,argv);
			else
				defaultResp(argc,argv);
		} break;

	case 'L': case 'l':
		{
			if (stricmp(argv[0],"LIST") == 0)
				retval = doLIST(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'M': case 'm':
		{
			if (stricmp(argv[0],"MDTM") == 0)
				retval = doMDTM(argc,argv);
			else if (stricmp(argv[0],"MKD") == 0)
				retval = doMKD(argc,argv);
			else if (stricmp(argv[0],"MODE") == 0)
				retval = doMODE(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'N': case 'n':
		{
			if (stricmp(argv[0],"NLST") == 0)
				retval = doNLST(argc,argv);
			else if (stricmp(argv[0],"NOOP") == 0)
				retval = doNOOP(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'O': case 'o':
		{
			if (stricmp(argv[0],"OPTS") == 0)
				retval = doOPTS(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'P': case 'p':
		{
			if (stricmp(argv[0],"PASS") == 0)
				retval = doPASS(argc,argv);
			else if (stricmp(argv[0],"PASV") == 0)
				retval = doPASV(argc,argv);
			else if (stricmp(argv[0],"PBSZ") == 0)
				retval = doPBSZ(argc,argv);
			else if (stricmp(argv[0],"PORT") == 0)
				retval = doPORT(argc,argv);
			else if (stricmp(argv[0],"PROT") == 0)
				retval = doPROT(argc,argv);
			else if (stricmp(argv[0],"PWD") == 0)
				retval = doPWD(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'Q': case 'q':
		{
			if (stricmp(argv[0],"QUIT") == 0)
				doQUIT(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'R': case 'r':
		{
			if (stricmp(argv[0],"REIN") == 0)
				retval = doREIN(argc,argv);
			else if (stricmp(argv[0],"REST") == 0)
				retval = doREST(argc,argv);
			else if (stricmp(argv[0],"RETR") == 0)
				retval = doRETR(argc,argv);
			else if (stricmp(argv[0],"RMD") == 0)
				retval = doRMD(argc,argv);
			else if (stricmp(argv[0],"RNFR") == 0)
				retval = doRNFR(argc,argv);
			else if (stricmp(argv[0],"RNTO") == 0)
				retval = doRNTO(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	case 'S': case 's':
		{
			if (stricmp(argv[0],"SITE") == 0)
				retval = doSITE(argc,argv);
			else if (stricmp(argv[0],"SIZE") == 0)
				retval = doSIZE(argc,argv);
			else if (stricmp(argv[0],"SMNT") == 0)
				retval = doSMNT(argc,argv);
			else if (stricmp(argv[0],"STAT") == 0)
				retval = doSTAT(argc,argv);
			else if (stricmp(argv[0],"STOR") == 0)
				retval = doSTOR(argc,argv);
			else if (stricmp(argv[0],"STOU") == 0)
				retval = doSTOU(argc,argv);
			else if (stricmp(argv[0],"SYST") == 0)
				retval = doSYST(argc,argv);
			else
				defaultResp(argc,argv);
		} break;

	case 'T': case 't':
		{
			if (stricmp(argv[0],"TYPE") == 0)
				retval = doTYPE(argc,argv);
			else
				defaultResp(argc,argv);
		} break;

	case 'U': case 'u':
		{
			if (stricmp(argv[0],"USER") == 0)
				retval = doUSER(argc,argv);
			else
				defaultResp(argc,argv);
		} break;

	case 'X': case 'x':
		{
			if (stricmp(argv[0],"XCUP") == 0)
				retval = doCDUP(argc,argv);
			else if (stricmp(argv[0],"XCWD") == 0)
				retval = doCWD(argc,argv);
			else if (stricmp(argv[0],"XMKD") == 0)
				retval = doMKD(argc,argv);
			else if (stricmp(argv[0],"XPWD") == 0)
				retval = doPWD(argc,argv);
			else if (stricmp(argv[0],"XRMD") == 0)
				retval = doRMD(argc,argv);
			else
				defaultResp(argc,argv);
		} break;

	default:
		{
			//Recheck for the ABOR command.
			//When ABOR is sent as an urgent message it usually has some misc
			//characters preceeding it.
			if (strstr(argv[0],"ABOR") != NULL || strstr(argv[0],"abor") != NULL)
				retval = doABOR(argc,argv);
			else
				defaultResp(argc,argv);
		}
		break;

	} // end switch

	return retval;
}

////////////////////////////////////////
// FTP functions
//
// Implementations of the FTP commands
////////////////////////////////////////

bool FtpConnection::doABOR(int argc, char **argv)
{
	//indicate all operations should be aborted
	_bAbor = true;
	eventHandler(argc,(const char**)argv,"ABOR command successful.","225",1,1,0);
	return true;
}

bool FtpConnection::doACCT(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"ACCT command not implemented.","502",1,1,0);
	return true;
}

bool FtpConnection::doALLO(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"ALLO command ignored.","202",1,1,0);
	return true;
}

bool FtpConnection::doAPPE(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'APPE': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	return(doSTOR(argc,argv,2));
}

bool FtpConnection::doAUTH(int argc, char **argv)
{
	bool retval = false;
//	int errcode = 0, privkeysize = 0, certsize = 0;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'AUTH': Invalid number of parameters.","500",1,1,0);
		return false;
	}

#ifdef _SSL
	if (getUseSSL() != 0)
	{
		eventHandler(argc,(const char**)argv,"Control connection is already encrypted.","500",1,1,0);
		return false;
	}
#else
	{
		eventHandler(argc,(const char**)argv,"Unsupported command.","500",1,1,0);
		return false;
	}
#endif

	return retval;
}

bool FtpConnection::doCDUP(int argc, char **argv)
{
	char *tmpargv[3], cwd[] = "CWD", dotdot[] = "..";

	tmpargv[0] = cwd;
	tmpargv[1] = dotdot;
	tmpargv[2] = NULL;

	doCWD(2,tmpargv);

	return true;
}

bool FtpConnection::doCWD(int argc, char **argv)
{
	char buffer[MAX_PATH*2], *tmppath = buffer;
	int retval = 0, mode = 0;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"CWD command successful.","250",1,1,0);
		return true;
	}

	//combine all the arguments
	std::string dirname;
	CmdLine::combineArgs(dirname, argc,(const char**)argv);
	if (dirname.empty())
	{
		eventHandler(argc,(const char**)argv,"ERROR: out of memory.","451",1,1,1);
		return false;
	}
	FSUtils::checkSlashEnd(dirname);

	//check if the user has permission to change into the directory
//	if (checkPermissions("CWD",dirname.c_str(),"cx") == 0)
//		return false;

	//build the new CWD
	if (!FSUtils::buildPath(tmppath, MAX_PATH, "",_cwd.c_str(),dirname.c_str()))
	{
		eventHandler(argc,(const char**)argv,"Unable to set CWD.","530",1,1,0);
		return false;
	}

	//make sure the CWD is not too long
	if (strlen(tmppath) > FTPS_MAXCWDSIZE)
	{
		eventHandler(argc,(const char**)argv,"Unable to set CWD.","531",1,1,0);
		return false;
	}

	//validate the new path
	char *p = tmppath + strlen(tmppath)+10;

	strcpy(p,_userroot.c_str());
	strcat(p,tmppath);
	if ((mode = FSUtils::validatePath(p)) == 1)
	{
		//if a valid directory
		_cwd = tmppath;    //set the new CWD (the path is valid)
		retval = 1;
		//printf("CWD = %s\n",buffer); //DEBUG
	}

	if (retval != 0)
	{
		eventHandler(argc,(const char**)argv,"CWD command successful.","250",1,1,0);
		return true;
	}

	FSUtils::checkSlashUNIX(tmppath);    //display the path w/UNIX slashes
	if (mode == 2)
	{
		//if the path points to a file
		FSUtils::removeSlashEnd(tmppath);
		writeToLineBuffer("/%s: Not a directory.",tmppath);
	}
	else
	{
		//if the path DNE
		writeToLineBuffer("/%s: No such file or directory.",tmppath);
	}

	eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	return false;
}

bool FtpConnection::doDELE(int argc, char **argv)
{
	char tmppath[MAX_PATH];
	bool retval;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'DELE': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string filename;
	CmdLine::combineArgs(filename, argc,(const char**)argv);

	if (filename.empty())
	{
		eventHandler(argc,(const char**)argv,"ERROR: out of memory.","451",1,1,1);
		return false;
	}

	//check the user's permissions for the path
//	if (checkPermissions("DELE",filename.c_str(),"wo") == 0)
//		return false;

	if (!FSUtils::buildPath(tmppath, MAX_PATH, _userroot.c_str(),_cwd.c_str(),filename.c_str()))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set file path.","550",1,1,0);
		return false;
	}

	retval = FSUtils::delFile(tmppath);  //delete the file

	if (retval == 0)
	{
		FSUtils::checkSlashUNIX(filename);    //display the path w/UNIX slashes
		writeToLineBuffer("%s: The system cannot find the file specified.",filename.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	}
	else eventHandler(argc,(const char**)argv,"DELE command successful.","250",1,1,0);

	return retval;
}

bool FtpConnection::doFEAT(int argc, char **argv)
{
	sendResponse("Extensions supported","211",1);
	sendResponse("  AUTH SSL","211",1);
	sendResponse("  PBSZ","211",1);
	sendResponse("  PROT","211",1);
	eventHandler(argc,(const char**)argv,"FEAT command successful.","211",1,1,0);

	return true;
}

bool FtpConnection::doHELP(int argc, char **argv)
{
	sendResponse("The following commands are recognized (* =>'s unimplemented).","214",1);
	sendResponse("  ABOR   *ACCT   *ALLO    APPE ","214",1);
	sendResponse("  CDUP    CWD     DELE    FEAT ","214",1);
	sendResponse("  HELP    LIST    MDTM    MKD  ","214",1);
	sendResponse("  MODE    NLST    NOOP   *OPTS ","214",1);
	sendResponse("  PASS    PASV    PORT    PWD  ","214",1);
	sendResponse("  QUIT   *REIN    REST    RETR ","214",1);
	sendResponse("  RMD     RNFR    RNTO    SITE ","214",1);
	sendResponse("  SIZE   *SMNT    STAT    STOR ","214",1);
	sendResponse("  STOU    SYST    TYPE    USER ","214",1);
	eventHandler(argc,(const char**)argv,"HELP command successful.","214",1,1,0);

	return true;
}

bool FtpConnection::doLIST(int argc, char **argv) 
{
	//create the thread to transfer the directory listing
	FtpsXfer *xfer;
#ifdef ZQ_OS_MSWIN
	if (_gCPECfg._dwEnableFtpOverVstrm)
	{
		xfer = new FtpXferEx(*this, _site, _pasv_sock, _port_sock, _Pool);
	}
	else
	{
		xfer = new FtpsXfer(*this, _site, _pasv_sock, _port_sock, _Pool);
	}
#else
	xfer = new FtpsXfer(*this, _site, _pasv_sock, _port_sock, _Pool);
#endif

	FtpsXfer::context_t *xferinfo = xfer->getContext();
	char tmppath[MAX_PATH], permissions[4];

	//extract the options
	size_t optionlen=0;
	for (int i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			//if the argument is an option
			optionlen += strlen(argv[i]+1);
			if (optionlen < (int)sizeof(xferinfo->options))
				strcat(xferinfo->options, argv[i]+1);
		}
	}

	//build path
	CmdLine::combineArgs(xferinfo->path, argc,(const char**)argv,'-');

	//check the user's permissions for the path
	if (strchr(xferinfo->options, 'R') != NULL)
		strcpy(permissions,"vx");   //permissions needed for recursive listing
	else
		strcpy(permissions,"lvx");   //permissions needed for non-recursive listing

//	if (checkPermissions("LIST",xferinfo->path, permissions,1) == 0)
//		return false;

	//check if the path is valid
	FSUtils::buildPath(tmppath, MAX_PATH, _userroot.c_str(), _cwd.c_str(), xferinfo->path);

// no need to do check, since there is no folder hierarchy in Vstrm
/* 
	if (FSUtils::validatePath(tmppath) == 0 && strchr(tmppath,'*') == NULL)
	{
		FSUtils::checkSlashUNIX(xferinfo->path); //display the path w/UNIX slashes
		writeToLineBuffer("%s: No such file or directory.",xferinfo->path);
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		return false;
	}
*/
	//allocate memory for the current working dir
	strcpy(xferinfo->cwd, _cwd.c_str());    //set the current working directory

	//allocate memory for the user's root directory
	strcpy(xferinfo->userroot, _userroot.c_str());  //set the user's root directory

	//set the remaining values of the transfer info
	xferinfo->command = 'L';    //indicate the command is a LIST
	xferinfo->flagpasv = _fPasv;
	xferinfo->pasvsd = _pasvSd;
	strcpy(xferinfo->portaddr,_portAddr);
	strcpy(xferinfo->portport,_portPort);
	xferinfo->type = _type;
	xferinfo->restoffset = _restOffset;
	xferinfo->bencdata = _bEncData;

	_bAbor = 0;     //reset the abort flag
	_nListThreads++; //increment the number of listing threads

	xfer->start();

	return true;
}

bool FtpConnection::doMDTM(int argc, char **argv)
{
	FSUtils::fileInfo_t fileinfo;
	char path[MAX_PATH];
	bool retval = false;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'MDTM': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string filename;
	CmdLine::combineArgs(filename, argc,(const char**)argv);

//	if (checkPermissions("MDTM",filename.c_str(),"drlvx") == 0)
//		return false;

	if (!FSUtils::buildPath(path, MAX_PATH, _userroot.c_str(),_cwd.c_str(),filename.c_str())) {
		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
		return false;
	}

	if (FSUtils::getFileStats(path,&fileinfo) != 0)
		eventHandler(argc,(const char**)argv,_site.timeString("%Y%m%d%H%M%S").c_str(),"213",1,1,0);
	else
	{
		writeToLineBuffer("%s: No such file or directory.",filename.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	}

	return retval;
}

bool FtpConnection::doMKD(int argc, char **argv)
{
	char tmppath[MAX_PATH];
	int i;
	bool retval = false;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'MKD': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string dirname;
	CmdLine::combineArgs(dirname, argc,(const char**)argv);

	FSUtils::removeSlashEnd(dirname);

	//check the user's permissions for the path
//	if (checkPermissions("MKD",dirname.c_str(),"wm",1) == 0)
//		return false;

	if (!FSUtils::buildPath(tmppath, MAX_PATH, _userroot.c_str(),_cwd.c_str(),dirname.c_str()))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
		return false;
	}

	FSUtils::checkSlashUNIX(dirname);    //display the path w/UNIX slashes

	i = FSUtils::validatePath(tmppath);
	if (i == 1)
	{
		//if the directory already exists
		writeToLineBuffer("%s: Directory exits.",dirname.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"521",1,1,0);
	}
	else if (i == 2)
	{
		//if a file was specified
		writeToLineBuffer("%s: Is a file.",dirname.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"521",1,1,0);
	}
	else
	{
		if (FSUtils::mkDir(tmppath) == 0)
		{
			//make the directory
			writeToLineBuffer("%s: Unable to create directory.",dirname.c_str());
			eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		}
		else
		{
			writeToLineBuffer("\"%s\" new directory created.",dirname.c_str());
			eventHandler(argc,(const char**)argv,_linebuffer,"257",1,1,0);
			retval = 1;
		}
	}

	return retval;
}

bool FtpConnection::doMODE(int argc, char **argv)
{
	bool retval = false;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'MODE': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	if (strcmp(argv[1],"b") == 0 || strcmp(argv[1],"B") == 0)
	{
		eventHandler(argc,(const char**)argv,"Unimplemented MODE type.","502",1,1,0);
	}
	else if (strcmp(argv[1],"s") == 0 || strcmp(argv[1],"S") == 0)
	{
		eventHandler(argc,(const char**)argv,"MODE S ok.","200",1,1,0);
		retval = true;
	}
	else
	{
		writeToLineBuffer("'MODE %s': command not understood.",argv[1]);
		eventHandler(argc,(const char**)argv,_linebuffer,"500",1,1,0);
	}

	return retval;
}

bool FtpConnection::doNLST(int argc, char **argv)
{
	int i;
	bool retval;
	char* tmp_argv[100];

	//add a -N option to the entered arguments
	//NOTE: the -N option indicates only the names should be displayed
	tmp_argv[0] = "LIST";
	tmp_argv[1] = "-N";

	for (i = 1; i < argc; i++)
		tmp_argv[i+1] = argv[i];
	tmp_argv[i+1] = NULL;

	retval = doLIST(argc+1,tmp_argv);
	
	return retval;
}

bool FtpConnection::doNOOP(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"NOOP command successful.","200",1,1,0);
	return true;
}

bool FtpConnection::doOPTS(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"OPTS command not implemented.","502",1,1,0);
	return true;
}

bool FtpConnection::doPASS(int argc, char **argv)
{
	bool flagauth = true;

	if (argc == 1)
	{
		strncpy(_pwd,"",sizeof(_pwd)-1);
		_pwd[sizeof(_pwd)-1] = '\0';
	}
	else
	{
		strncpy(_pwd,argv[1],sizeof(_pwd)-1);
		_pwd[sizeof(_pwd)-1] = '\0';
	}

	if (_gCPECfg._enableAuthorization)
	{
		flagauth = ExportAccountGen::verify(_login,_pwd);
	}

	if (flagauth)
	{
		//user was successfully authenticated
		writeToLineBuffer("User %s logged in.",_login);
		eventHandler(1,(const char**)argv,_linebuffer,"230",1,1,0);
	}
	else
	{
		//user was not authenticated
		SYS::sleep(FTPS_LOGINFAILDELAY * 1000); //delay after failed login
		writeToLineBuffer("User %s cannot login.",_login);
		eventHandler(1,(const char**)argv,_linebuffer,"530",1,1,0);
	}

	return(flagauth);
}

//NOTE: If a PASV socket is left open when the user logs out,
//      it will be closed at logout.
bool FtpConnection::doPASV(int argc, char **argv)
{
	char *ptr1, *ptr2, bindport[SOCK_PORTLEN], bindaddr[SOCK_IPADDRLEN];
	unsigned short p, ph,pl;
	int retval;

	//if a PASV socket has already been created, close it.
	if (_pasvSd != SOCK_INVALID)
	{
		if (_site.freetDataPort((int)_pasvSd) != 1)
			FtpSock::Close(_pasvSd); //close directly if freetDataPort() is unimplemented/fails
	}
	_fPasv = 1;     //set to use passive mode

	//Create a socket and bind to it and get the port number
	//that was binded to.
	//NOTE: NULL is used for the "bindip" field to allow PASV mode when
	//      acting as an external (IP) machine from behind a NATed firewall.
	//      This allows PASV mode to be used with an IP that is not assigned
	//      to the machine running the server (using NULL binds to all IPs).
	retval = _site.dataPort(bindport,sizeof(bindport),(int *)(&_pasvSd),NULL);
	if (retval == -1)
	{
		_pasv_sock = new FtpSock(); // create a new passive mode socket
		//dataPort() is not implemented.  Try to bind directly
		if ((_pasvSd = _pasv_sock->BindServer(bindport,sizeof(bindport),_serverip)) == SOCK_INVALID)
		{
			eventHandler(argc,(const char**)argv,"Unable to bind to a port.","500",1,1,1);
			delete _pasv_sock;
			_pasv_sock = NULL;
			return false;      //there was an error in the binding
		}
	}
	else if (retval == 0)
	{
		//Unable to bind to an available port
		eventHandler(argc,(const char**)argv,"Unable to bind to a port.","500",1,1,1);
		return false;      //there was an error in the binding
	}

	//listen on the socket descriptor
	if (_pasv_sock->ListenServer() == 0)
	{
		writeToLineBuffer("Unable to listen on port %s (sd = %d).",bindport,_pasvSd);
		eventHandler(argc,(const char**)argv,_linebuffer,"500",1,1,1);
		if (_site.freetDataPort((int)_pasvSd) != 1)
			FtpSock::Close(_pasvSd); //close directly if freetDataPort() is unimplemented/fails
		delete _pasv_sock;
		_pasv_sock = NULL;
		_pasvSd = SOCK_INVALID;    //invalidate the socket desc
		return false;      //there was an error in listening
	}

	//replace the '.' with ',' in the IP address
	strcpy(bindaddr,_serverip);
	ptr1 = strchr(bindaddr,'.');
	*ptr1 = ',';
	ptr1++;
	ptr2 = strchr(ptr1,'.');
	*ptr2 = ',';
	ptr2++;
	ptr1 = strchr(ptr2,'.');
	*ptr1 = ',';

	//generate the high and low parts of the port number
	p = atoi(bindport);
	ph = p / 256;
	pl = p % 256;

	writeToLineBuffer("Entering Passive Mode (%s,%hu,%hu).",bindaddr,ph,pl);
	eventHandler(argc,(const char**)argv,_linebuffer,"227",1,1,0);

	return true;
}

bool FtpConnection::doPBSZ(int argc, char **argv)
{
	bool retval = false;

	if (getUseSSL() == 0)
	{
		eventHandler(argc,(const char**)argv,"Bad sequence of commands.","503",1,1,0);
	}
	else
	{
		if (argc < 2)
			eventHandler(argc,(const char**)argv,"'PBSZ': Invalid number of parameters.","500",1,1,0);
		else
		{
			eventHandler(argc,(const char**)argv,"Command ok.  PBSZ=0","200",1,1,0);
			retval = true;
		}
	}

	return retval;
}

bool FtpConnection::doPORT(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'PORT': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	writeToLineBuffer("%s",argv[1]);

	if (parsePortAddr(_linebuffer,_portAddr,sizeof(_portAddr),_portPort,sizeof(_portPort)) == 0)
	{
		eventHandler(argc,(const char**)argv,"Invalid PORT command.","500",1,1,0);
		return false;
	}

	_fPasv = 0;     //set to use active mode

	//if a socket was created for the PASV command, close it.
	if (_pasvSd != SOCK_INVALID)
	{
		if (_site.freetDataPort((int)_pasvSd) != 1)
			FtpSock::Close(_pasvSd); //close directly if freetDataPort() is unimplemented/fails
		_pasvSd = SOCK_INVALID;
	}
    _port_sock = new FtpSock(); // create a active mode socket
	eventHandler(argc,(const char**)argv,"PORT command successful.","200",1,1,0);

	return true;
}

bool FtpConnection::doPROT(int argc, char **argv)
{
	if (getUseSSL() == 0)
	{
		eventHandler(argc,(const char**)argv,"Bad sequence of commands.","503",1,1,0);
		return false;
	}

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'PROT': Invalid number of parameters.","500",1,1,0);
		return false;
	}
	
	if (stricmp(argv[1],"C") == 0)
	{
		eventHandler(argc,(const char**)argv,"Not Encrypting Data Channel","200",1,1,0);
		_bEncData = 0;  //use a clear data channel
		return true;
	}

	if (stricmp(argv[1],"S") == 0 ||
		stricmp(argv[1],"E") == 0 ||
		stricmp(argv[1],"P") == 0)
	{
		eventHandler(argc,(const char**)argv,"Encrypting Data Channel","200",1,1,0);
		_bEncData = 1;  //use an encrypted data channel
		return true;
	}

	writeToLineBuffer("'PROT %s': command not understood.",argv[1]);
	eventHandler(argc,(const char**)argv,_linebuffer,"500",1,1,0);
	return false;
}

bool FtpConnection::doPWD(int argc, char **argv)
{
	//check the user's permissions
//	if (checkPermissions("PWD",NULL,"px") == 0)
//		return false;

	writeToLineBuffer("\"/%s\" is current directory.",_cwd.c_str());
	FSUtils::checkSlashUNIX(_linebuffer);     //display the path w/UNIX slashes
	eventHandler(argc,(const char**)argv,_linebuffer,"257",1,1,0);

	return true;
}

bool FtpConnection::doQUIT(int argc, char **argv)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(FtpConn, "User[%s]: doQUIT enter, nListThreads(%d), nDataThreads(%d)"), getLogin(), nListThreads(), nDataThreads());
	
	//stop any current transfers
	_bAbor = 1;         //stop all threads

//	DWORD dw1 = GetTickCount();
	int64 nStamp = ZQTianShan::now();
	
	// must wait for data thread to finish work. this time out only for _nListThreads > 0
	if ((_nListThreads+_nDataThreads) > 0)
	{
		SYS::sleep(300);
		//print the goodbye message and logout
		eventHandler(argc,(const char**)argv,"Exit.","221",1,1,0);
		setQuit(true);  //set flag to indicate the user is quiting

		if (_cli_sock)
		{
			_cli_sock->Close();
		}
		MOLOG(Log::L_DEBUG, CLOGFMT(FtpConn, "User[%s]: Ctrl Connection closed, nListThreads(%d), nDataThreads(%d)"), getLogin(), nListThreads(), nDataThreads());
		

		// must wait for data thread to finish work. this time out only for _nListThreads > 0
		while ((_nListThreads+_nDataThreads) > 0)
		{
			//if there are any active transfer threads
			SYS::sleep(300);    //wait until all threads have stopped	
		}
	}
	else
	{
		//print the goodbye message and logout
		eventHandler(argc,(const char**)argv,"Exit.","221",1,1,0);
		setQuit(true);  //set flag to indicate the user is quiting
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(FtpConn, "User[%s]: doQUIT leave, nListThreads(%d), nDataThreads(%d), waited(%d)s"), 
		getLogin(), nListThreads(), nDataThreads(), (ZQTianShan::now()-nStamp)/1000);
	
	_nListThreads = 0;
	_nDataThreads = 0;

	return true;
}

bool FtpConnection::doREIN(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"'REIN': command not implemented.","504",1,1,0);
	return true;
}

bool FtpConnection::doREST(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'REST': Invalid number of parameters.","500",1,1,0);
		return false;
	}
    
	// check if argv[1] is number
	const char *p = argv[1];
	bool isNum = isdigit(*p);

	if (isNum)
	{
		_restOffset = atol(argv[1]);
		writeToLineBuffer("Restarting at %s.  Send STOR or RETR to initiate transfer.",argv[1]);
		eventHandler(argc,(const char**)argv,_linebuffer,"350",1,1,0);
		return true;
	}

 //   // we do not support resume broken downloads or uploads
	writeToLineBuffer("'REST %s': command not understood.",argv[1]);
	eventHandler(argc,(const char**)argv,_linebuffer,"500",1,1,0);

	return false;
}

bool FtpConnection::doRETR(int argc, char **argv)
{
	if (argc < 2) {
		eventHandler(argc,(const char**)argv,"'RETR': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	/* get file path to be transfered */
	std::string real_root, basename;
	{
	std::string path;
	CmdLine::combineArgs(path, argc,(const char**)argv);
	mapVirtualDir(path, real_root, basename);
	}

	int nTransferBitrate = 0;
	if (_gCPECfg._enableAuthorization) {
		std::string file = basename;
		std::string::size_type pos2 = file.find_last_of('.');
	
		/* found a file with extension, get main file name */
		if(pos2 != std::string::npos) {
			std::string ext = file.substr(pos2+1);
			for(std::string::size_type i = 0; i < ext.length(); ++i) {
				ext[i] = toupper(ext.at(i));
			}

			if(!ext.compare(0, 2, "VV") || 
			   !ext.compare(0, 2, "FF") ||
			   !ext.compare(0, 2, "FR") ||
			   !ext.compare("INDEX")    || 
			   !ext.compare("0X0000"))   {
				
			   file = file.substr(0, pos2);
			}
		}

		std::string filename = file;
		std::string::size_type pos3 = filename.rfind('/');
		if(pos3 != std::string::npos)
		{
			filename = filename.substr(pos3 + 1);
		}

		int nExpiredTime;
		if (!ExportAccountGen::verify(_login, _pwd, filename.c_str(), nExpiredTime, nTransferBitrate)) {
			eventHandler(argc,(const char**)argv,"ERROR: no permission to download the specified file.","560",1,1,0);
			MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, 
						"User[%s]/Password[%s] incorrect [%s]"), _login, _pwd, filename.c_str());
			return false;
		}	

		int nTimeNow = (int)time(0);
		if (nExpiredTime<nTimeNow) {
			char t1[32], t2[32];
			struct tm* pTM = localtime((time_t*)&nExpiredTime);
			if (pTM) {
				sprintf(t1, "%4d-%02d-%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1,pTM->tm_mday,pTM->tm_hour,pTM->tm_min,pTM->tm_sec);
			}
			else t1[0]='\0';

			pTM = localtime((time_t*)&nTimeNow);
			if (pTM) {
				sprintf(t2, "%4d-%02d-%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1,pTM->tm_mday,pTM->tm_hour,pTM->tm_min,pTM->tm_sec);
			}
			else t2[0]='\0';

			eventHandler(argc,(const char**)argv,"ERROR:the download TTL expired for the user.","560",1,1,0);
			MOLOG(Log::L_WARNING, CLOGFMT(FtpConn, "User[%s]/Password[%s] expired at[%s], current time is[%s]"), _login, _pwd, t1, t2);
			return false;
		}
	}

	if (!nTransferBitrate) {
		nTransferBitrate = _gCPECfg._dwExportBitrate;
	}
	if (nTransferBitrate > _gCPECfg._dwMaxBitrate) {
		nTransferBitrate = _gCPECfg._dwMaxBitrate;
	}

	FtpsXfer *xfer;
#ifdef ZQ_OS_MSWIN
	if (_gCPECfg._dwEnableFtpOverVstrm)
	{
		xfer = new FtpXferEx(*this, _site, _pasv_sock, _port_sock, _Pool);
	}
	else
	{
		xfer = new FtpsXfer(*this, _site, _pasv_sock, _port_sock, _Pool);
	}
#else
	xfer = new FtpsXfer(*this, _site, _pasv_sock, _port_sock, _Pool);
#endif
    xfer->setMaxDLBitrate(nTransferBitrate);

	FtpsXfer::context_t *xferinfo = xfer->getContext();

	/* base name only */
	strcpy(xferinfo->path, basename.c_str());

	strcpy(xferinfo->cwd, _cwd.c_str());
	strcpy(xferinfo->userroot, real_root.c_str());

	/* indicates data should be sent */
	xferinfo->command = 'S';    
	xferinfo->flagpasv = _fPasv;
	xferinfo->pasvsd = _pasvSd;
	strcpy(xferinfo->portaddr,_portAddr);
	strcpy(xferinfo->portport,_portPort);
	xferinfo->type = _type;
	xferinfo->restoffset = _restOffset;
	xferinfo->bencdata = _bEncData;

	_bAbor = false; 

//	MOLOG(Log::L_DEBUG, CLOGFMT(FtpConn, "doRETR() User[%s]/Password[%s] data connection count(increase)[%lld]"),_login, _pwd,_site._nConnections);
//	incDataThreads(); 
	_restOffset = 0;  

	xfer->start();

	return true;
}

bool FtpConnection::doRMD(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'RMD': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string dirname;
	CmdLine::combineArgs(dirname, argc,(const char**)argv);

	//check the user's permissions for the path
//	if (checkPermissions("RMD",dirname.c_str(),"wn") == 0)
//		return false;

	char tmppath[MAX_PATH];

	if (!FSUtils::buildPath(tmppath,MAX_PATH,_userroot.c_str(),_cwd.c_str(),dirname.c_str()))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
		return false;
	}

	//do not allow the user to delete the root directory
	if (strcmp(tmppath,_userroot.c_str()) == 0)
	{
		eventHandler(argc,(const char**)argv,"Cannot delete the root directory.","500",1,1,0);
		return false;
	}

	FSUtils::checkSlashUNIX(dirname);    //display the path w/UNIX slashes

	int i = FSUtils::validatePath(tmppath);
	if (i == 0)
	{
		//if the directory does not exist
		writeToLineBuffer("%s: No such file of directory.",dirname.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		return false;
	}
	
	if (i == 2)
	{
		//if path is a file
		writeToLineBuffer("%s: Not a directory.",dirname.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		return false;
	}

	//the directory exists
	//check if the directory is ""
	if (FSUtils::dirIsEmpty(tmppath) != 0)
	{
		writeToLineBuffer("%s: The directory is not "".",dirname.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		return false;
	}

	if (FSUtils::rmDir(tmppath) != 0)
	{
		eventHandler(argc,(const char**)argv,"RMD command successful.","250",1,1,0);
		return true;
	}

	writeToLineBuffer("%s: Unable to remove directory.",dirname.c_str());
	eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	return false;
}

bool FtpConnection::doRNFR(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'RNFR': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string filename;
	CmdLine::combineArgs(filename, argc,(const char**)argv);

	//check the user's permissions for the path
//	if (checkPermissions("RNFR",filename.c_str(),"wa") == 0)
//		return false;

	char tmppath[MAX_PATH];

	if (!FSUtils::buildPath(tmppath, MAX_PATH, _userroot.c_str(),_cwd.c_str(),filename.c_str()))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set file path.","550",1,1,0);
		return false;
	}

	FSUtils::checkSlashUNIX(filename);    //display the path w/UNIX slashes

	int i = FSUtils::validatePath(tmppath);
	if (i == 0)
	{
		//if the file/directory does not exist
		writeToLineBuffer("%s: No such file or directory.",filename.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		return false;
	}

	_rendFilename = tmppath;
	eventHandler(argc,(const char**)argv,"File exists, ready for destination name.","350",1,1,0);
	return true;
}

bool FtpConnection::doRNTO(int argc, char **argv)
{
	int i;
	bool retval = false;

	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'RNTO': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	if (_rendFilename.empty())

	{
		eventHandler(argc,(const char**)argv,"Bad sequence of commands.","503",1,1,0);
		return false;
	}

	char tmppath[MAX_PATH];
	std::string filename;
	CmdLine::combineArgs(filename, argc,(const char**)argv);
	FSUtils::removeSlashEnd(filename);

	//check the user's permissions for the path
//	if (checkPermissions("RNTO",filename.c_str(),"wa",1) == 0)
//		return false;

	if (!FSUtils::buildPath(tmppath, MAX_PATH, _userroot.c_str(),_cwd.c_str(),filename.c_str()))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set file path.","550",1,1,0);
		return false;
	}

	FSUtils::checkSlashUNIX(filename);    //display the path w/UNIX slashes

	i = FSUtils::validatePath(tmppath);
	if (i == 0)
	{
		//if the file/directory does not exist
		if (FSUtils::rename(_rendFilename.c_str(),tmppath) != 0)

		{
			eventHandler(argc,(const char**)argv,"RNTO command successful.","250",1,1,0);
			retval = true;
		}
		else
		{
			writeToLineBuffer("%s: Unable to rename file.",filename.c_str());
			eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
		}
	}
	else
	{
		writeToLineBuffer("%s: Cannot create a file when that file already exists.",filename.c_str());
		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	}

	_rendFilename = "";

	return retval;
}

bool FtpConnection::doSIZE(int argc, char **argv) 
{
	FSUtils::fileInfo_t fileinfo;
	bool retval = false;

	if (argc < 2) {
		eventHandler(argc,(const char**)argv,"'SIZE': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	std::string filename, real_root, basename;
	{
	CmdLine::combineArgs(filename, argc,(const char**)argv);
	mapVirtualDir(filename, real_root, basename);
	}

	char path[MAX_PATH];
 	if (!FSUtils::buildPath(path, MAX_PATH, real_root.c_str(), _cwd.c_str(), basename.c_str())) {
 		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
 		return false;
 	}

#ifdef ZQ_OS_MSWIN
	if (_gCPECfg._dwEnableFtpOverVstrm)
	{
		//no passive or port socket created, do not pass it in, or it will be deleted
		FtpXferEx xferEx(*this, _site, NULL, NULL, _Pool);
		int64 filesize = xferEx.GetFileSize(filename.c_str());
		writeToLineBuffer("%lld",filesize);
		eventHandler(argc,(const char**)argv,_linebuffer,"213",1,1,0);
		retval = true;
	}
	else
	{
#endif
 		if (FSUtils::getFileStats(path,&fileinfo) != 0)
 		{
 			if (fileinfo.mode & S_IFDIR)
	 
 			{
 				writeToLineBuffer("%s: not a plain file.",basename.c_str());
 				eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
 			}
 			else
 			{
 				writeToLineBuffer(FMT64U,fileinfo.size);
 				eventHandler(argc,(const char**)argv,_linebuffer,"213",1,1,0);
 				retval = true;
 			}
 		}
 		else
 		{
 			writeToLineBuffer("%s: No such file or directory.", path);
 			eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
	 
 			retval = false;
 		}
#ifdef ZQ_OS_MSWIN
	}
#endif

	return retval;
}

bool FtpConnection::doSMNT(int argc, char **argv)
{
	eventHandler(argc,(const char**)argv,"'SMNT': command not implemented.","504",1,1,0);
	return true;
}

bool FtpConnection::doSTAT(int argc, char **argv)
{
//	FtpXferClass ftpsxfer(*this, _site, _pasv_sock, _port_sock, DataThreadPool);
//	FtpXferClass::context_t *xferinfo = ftpsxfer.getContext();
	FtpsXfer ftpsxfer(*this, _site, _pasv_sock, _port_sock, _Pool);
	FtpsXfer::context_t *xferinfo = ftpsxfer.getContext();

	*_linebuffer = '\0';

	if (argc == 1)
	{
		//check the user's permissions
//		if (checkPermissions("STAT",NULL,"sx") == 0)
//			return false;

		if (*_serverName != '\0')
			appendLineBuf(_serverName);
		else
			appendLineBuf(_serverip);
		appendLineBuf(" FTP server status:");
		sendResponse(_linebuffer,"211",1);
		*_linebuffer = '\0';
		appendLineBuf("Version ");
		appendLineBuf(_site.progName());
		appendLineBuf("-");
		appendLineBuf(_site.progVersion());
		appendLineBuf(" (");
		appendLineBuf(_site.coreVersion());
		appendLineBuf(")");
		appendLineBuf(_site.timeString(" %a %b %d %H:%M:%S %Y").c_str());
		sendResponse(_linebuffer,"",2);
		*_linebuffer = '\0';
		appendLineBuf("Connected to user-");
		appendLineBuf(_clientName);
		appendLineBuf(" (");
		appendLineBuf(_clientIp);
		appendLineBuf(")");
		sendResponse(_linebuffer,"",2);
		*_linebuffer = '\0';
		appendLineBuf("Logged in as ");
		appendLineBuf(_login);
		sendResponse(_linebuffer,"",2);
		*_linebuffer = '\0';
		appendLineBuf("TYPE: ");
		appendLineBuf((_type == 'A')?"ASCII, FORM: Nonprint; ":"Image; ");
		appendLineBuf("STRUcture: File; transfer MODE: Stream");
		sendResponse(_linebuffer,"",2);
		*_linebuffer = '\0';
		if ((_nListThreads+_nDataThreads) == 0)   //if there are any active transfer threads
			appendLineBuf("No data connection");
		else
			appendLineBuf("Data transfer in progress");
		sendResponse(_linebuffer,"",2);
		eventHandler(argc,(const char**)argv,"End of status.","211",1,1,0);
	}
	else
	{
		//Do dir listing
		std::string filename;
		CmdLine::combineArgs(filename, argc,(const char**)argv);
		FSUtils::removeSlashEnd(filename);
		//check the user's permissions
//		if (checkPermissions("STAT",filename.c_str(),"sx",1) == 0)
//			return false;
		//check the user's permissions for the path
//		if (checkPermissions("STAT",filename.c_str(),"lx") != 0)
//
//		{
			appendLineBuf("status of ");
			appendLineBuf(filename.c_str());
			appendLineBuf(":");
			sendResponse(_linebuffer,"213",1);
			//send the dir listing over the control connection
			strcpy(xferinfo->path, filename.c_str());
			strcpy(xferinfo->cwd, _cwd.c_str());
			strcpy(xferinfo->userroot, _userroot.c_str());
			xferinfo->flagpasv = _fPasv;
			xferinfo->pasvsd = _pasvSd;
			xferinfo->bencdata = getUseSSL();
			_bAbor = false; //reset the abort flag
			ftpsxfer.sendListCtrl();
			eventHandler(argc,(const char**)argv,"End of status.","213",1,1,0);
//		}
	}

	return true;
}

//mode = 0 --> do normal STOR command
//mode = 1 --> do STOU (store to unique name)
//mode = 2 --> do APPE (append to a file --or create)
// bool FtpConnection::doSTOR(int argc, char **argv, int mode /*=0*/)
// {
// 	char buffer[MAX_PATH], *tmppath;
// 	int i;
// 
// 	if (argc < 2)
// 	{
// 		eventHandler(argc,(const char**)argv,"'STOR': Invalid number of parameters.","500",1,1,0);
// 		return false;
// 	}
// 
// 	FtpXferClass *xfer = new FtpXferClass(*this, _site, _pasv_sock, _port_sock, _Pool);
// 	if (xfer == NULL)
// 	{
// 		//if unable to create the new thread, clean up.
// 		eventHandler(argc,(const char**)argv,"ERROR: unable to create a new transfer thread.","530",1,1,1);
// 		_nListThreads--;
// 		return false;
// 	}
// 
// 	FtpXferClass::context_t *xferinfo = xfer->getContext();
// 
// 	CmdLine::combineArgs(xferinfo->path, argc,(const char**)argv);
// 	FSUtils::removeSlashEnd(xferinfo->path);
// 
// 	//check the user's permissions for the path
// //	if (checkPermissions("STOR",xferinfo->path, "uw",1) == 0)
// //		return false;
// 
// 	//check if the path is valid
// 	if (!FSUtils::buildPath(buffer, MAX_PATH,_userroot.c_str(),_cwd.c_str(),xferinfo->path))
// 	{
// 		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
// 		return false;
// 	}
// 
// 	tmppath = buffer + strlen(buffer) +2;
// 
// 	FSUtils::getDirPath(tmppath,strlen(buffer),buffer);  //get the directory portion of the path
// 	//if the path is not a directory
// 	if ((i = FSUtils::validatePath(tmppath)) != 1)
// 	{
// 		FSUtils::checkSlashUNIX(xferinfo->path); //display the path w/UNIX slashes
// 		writeToLineBuffer("%s: The system cannot find the path specified.",xferinfo->path); //path DNE
// 		eventHandler(argc,(const char**)argv,_linebuffer,"550",1,1,0);
// 		return false;
// 	}
// 
// 	strcpy(xferinfo->cwd, _cwd.c_str());    //set the current working directory
// 	strcpy(xferinfo->userroot, _userroot.c_str());  //set the user's root directory
// 
// 	//set the remaining values of the transfer info
// 	xferinfo->command = 'R';    //indicates data should be received
// 	xferinfo->mode = mode;
// 	xferinfo->flagpasv = _fPasv;
// 	xferinfo->pasvsd = _pasvSd;
// 	strcpy(xferinfo->portaddr,_portAddr);
// 	strcpy(xferinfo->portport,_portPort);
// 	xferinfo->type = _type;
// 	xferinfo->restoffset = _restOffset;
// 	xferinfo->bencdata = _bEncData;
// 
// 	_bAbor = false;     //reset the abort flag
// 	_nDataThreads++; //increment the number of data transfer threads
// 	_restOffset = 0;   //reset the transfer reset offset
// 
// 	//start the thread to transfer the file
// 	xfer->start();
// 
// 	return true;
// }
bool FtpConnection::doSTOR(int argc, char **argv, int mode /*=0*/)
{
	char buffer[MAX_PATH], *tmppath;
	
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'STOR': Invalid number of parameters.","500",1,1,0);
		return false;
	}
	
	FtpsPushXfer *xfer = new FtpsPushXfer(*this, _site, _pasv_sock, _port_sock, _Pool);
	if (xfer == NULL)
	{
		//if unable to create the new thread, clean up.
		eventHandler(argc,(const char**)argv,"ERROR: unable to create a new transfer thread.","530",1,1,1);
		_nListThreads--;
		return false;
	}
	
	FtpXferClass::context_t *xferinfo = xfer->getContext();
	
	CmdLine::combineArgs(xferinfo->path, argc,(const char**)argv);
	FSUtils::removeSlashEnd(xferinfo->path);
	
	//check if the path is valid
	if (!FSUtils::buildPath(buffer, MAX_PATH,_userroot.c_str(),_cwd.c_str(),xferinfo->path))
	{
		eventHandler(argc,(const char**)argv,"ERROR: unable to set directory path.","550",1,1,0);
		delete xfer;
		return false;
	}
	
	tmppath = buffer + strlen(buffer) +2;
	
	FSUtils::getDirPath(tmppath,strlen(buffer),buffer);  //get the directory portion of the path
	strcpy(xferinfo->cwd, _cwd.c_str());    //set the current working directory
	strcpy(xferinfo->userroot, _userroot.c_str());  //set the user's root directory
	
	//set the remaining values of the transfer info
	xferinfo->command = 'R';    //indicates data should be received
	xferinfo->mode = mode;
	xferinfo->flagpasv = _fPasv;
	xferinfo->pasvsd = _pasvSd;
	strcpy(xferinfo->portaddr,_portAddr);
	strcpy(xferinfo->portport,_portPort);
	xferinfo->type = _type;
	xferinfo->restoffset = _restOffset;
	xferinfo->bencdata = _bEncData;
	
	_bAbor = false;     //reset the abort flag
	_restOffset = 0;   //reset the transfer reset offset
	
	if (!xfer->recvFile())
	{
		delete xfer;
		return false;
	}

	return true;
}

bool FtpConnection::doSTOU(int argc, char **argv)
{
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'STOU': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	return(doSTOR(argc,argv,1));
}

bool FtpConnection::doSYST(int argc, char **argv)
{
	//check the user's permissions
//	if (checkPermissions("SYST",NULL,"sx") == 0)
//		return false;

#ifdef WIN32
	eventHandler(argc,(const char**)argv,"Windows_NT","215",1,1,0);
#else
	eventHandler(argc,(const char**)argv,"UNIX","215",1,1,0);
#endif

	return true;
}

bool FtpConnection::doTYPE(int argc, char **argv)
{
	char strtype[2], *args[2];

	if (argc > 1)
	{
		if (stricmp(argv[1],"e") == 0)
		{
			eventHandler(argc,(const char**)argv,"Type E not implemented.","504",1,1,0);
			return false;
		}
		if (stricmp(argv[1],"a") == 0)
			_type = 'A';
		else if (stricmp(argv[1],"i") == 0)
			_type = 'I';
		else if (stricmp(argv[1],"l") == 0)
			_type = 'I';   //"L 8" or "L" is the same as "I"
	}

	//as a default leave the current setting.
	sprintf(_linebuffer,"Type is set to %c.",_type);
	strtype[0] = _type; strtype[1] = '\0';
	args[0] = argv[0]; args[1] = strtype;
	eventHandler(2,(const char**)args,_linebuffer,"200",1,1,0);

	return true;
}

bool FtpConnection::doUSER(int argc, char **argv)
{
	MOLOG(Log::L_INFO, CLOGFMT(FtpConn, "do USER connections[%d], MaxConnectionNum[%d]"), int(_site._nConnections), MaxConnectionNum);
	if (_site._nConnections > MaxConnectionNum)
	{
		setQuit(true);
		eventHandler(argc,(const char**)argv,"Too many users - please try again later.","421",1,1,0);
		return false;
	}
	if (argc < 2)
	{
		eventHandler(argc,(const char**)argv,"'USER': Invalid number of parameters.","500",1,1,0);
		return false;
	}

	//store the user name
	strncpy(_login,argv[1],sizeof(_login)-1);
	_login[sizeof(_login)-1] = '\0';

	//always ask for the password (check authentication in DoPASS())
	writeToLineBuffer("Password required for %s.",argv[1]);
	eventHandler(argc,(const char**)argv,_linebuffer,"331",1,1,0);

	return true;
}

////////////////////////////////////////
// Functions that should be overwritten
// by a class that inherits from FtpConnection
////////////////////////////////////////

bool FtpConnection::doSITE(int argc, char **argv)
{
	//check the user's permissions
//	if (checkPermissions("SITE",NULL,"tx") == 0)
//		return false;

	eventHandler(argc,(const char**)argv,"'SITE': command not implemented.","504",1,1,0);

	return true;
}


////////////////////////////////////////
// Functions used to get FTP server
// state
//
// NOTE: Although returning the internal
//       data pointers in functions such
//       as getLogin() violates proper 
//       information hiding, it saves
//       memory and complexity that
//       would be needed to constantly
//       create/update temp buffers.
//       Although it would be possible
//       to write to the pointers 
//       returned by the functions, 
//       this is never done in this
//       application.
////////////////////////////////////////

SOCKET FtpConnection::getSocketDesc()
{
	return(_sd);
}

bool FtpConnection::isEncData()
{
	return(_bEncData);
}

char *FtpConnection::serverName()
{
	return(_serverName);
}

char *FtpConnection::serverIP()
{
	return(_serverip);
}

char *FtpConnection::serverPort()
{
	return(_serverPort);
}

char *FtpConnection::clientName()
{
	return(_clientName);
}

char *FtpConnection::clientIP()
{
	return(_clientIp);
}

char *FtpConnection::clientPort()
{
	return(_clientPort);
}

FtpSite *FtpConnection::rtEnv()
{
	return &_site;
}

void FtpConnection::setLogin(char *login)
{
	if (login != NULL)

	{
		strncpy(_login,login,sizeof(_login)-1);
		_login[sizeof(_login)-1] = '\0';
	}
	else *_login = '\0';
}

char *FtpConnection::getLogin()
{
	return(_login);
}

void FtpConnection::setUserID(int uid)
{
	_userId = uid;
}

int FtpConnection::getUserID()
{
	return(_userId);
}

int FtpConnection::fPASV()
{
	return(_fPasv);
}

void FtpConnection::setQuit(bool quit)
{
	if (quit)
		MOLOG(Log::L_INFO, CLOGFMT(FtpConn, "User[%s]: setQuit(true)"), getLogin());
	else
		MOLOG(Log::L_INFO, CLOGFMT(FtpConn, "User[%s]: setQuit(false)"), getLogin());

	_bQuit = quit;
}

int FtpConnection::isQuit()
{
	return(_bQuit);
}

void FtpConnection::setAbor(bool abor)
{
	_bAbor = abor;
}

int FtpConnection::isAbor()
{
	return(_bAbor);
}

int FtpConnection::nListThreads()
{
	return(_nListThreads);
}

int FtpConnection::nDataThreads()
{
	return(_nDataThreads);
}

time_t FtpConnection::lastActive()
{
	return _lastActive;
}

////////////////////////////////////////
// Functions used to write to the log
// files and send responses back to the
// FTP client.
////////////////////////////////////////

//Handles events based on flagclient, flagaccess, and flagprog.
//flagclient determines if information will be sent back to the client.
//flagaccess determines if information will be written to the access log.
//flagprog determines if information will be written to the program log.
//respmode specifies what kind of response to send to the user (intermediate or final)
bool FtpConnection::eventHandler(const int argc, const char **argv, const char *mesg, char *ftpcode, int flagclient, int flagaccess, int flagprog, int respmode /*=0*/)
{
	const char *cmdptr;
	std::string argptr;

	if (argc == 0 || argv == NULL)
	{
		argptr = "";
		cmdptr = "";
	}
	else
	{
		cmdptr = argv[0];
		CmdLine::combineArgs(argptr, argc,(const char**)argv);
	}

	//write to the program log
	if (flagprog != 0 && mesg != NULL)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FtpConn, "User[%s] \"%s\" occured in '%s %s'"),
			getLogin(), mesg, cmdptr, argptr.c_str());
	}
	
	//send the message back to the client
	if (flagclient != 0 && mesg != NULL && ftpcode != NULL)
		sendResponse(mesg,ftpcode,respmode);

	return true;
}

//Writes to _linebuffer without exceeding FTPS_LINEBUFSIZE
//This function works just like sprintf()
int FtpConnection::writeToLineBuffer(char *lineformat, ...)
{
	va_list parg;
	int nchars;

	//get the argument list
	va_start(parg,lineformat);

	//write the arguments into the buffer
#ifdef WIN32    //for Windows
	nchars = _vsnprintf((char*)_linebuffer,sizeof(_linebuffer),lineformat,parg);
#else   //for UNIX
	nchars = vsnprintf((char*)_linebuffer,sizeof(_linebuffer),lineformat,parg);
#endif

	va_end(parg);

	//make sure the string is terminated
	_linebuffer[sizeof(_linebuffer)-1] = '\0';

	return(nchars);
}

////////////////////////////////////////
// Static Utility Functions
////////////////////////////////////////

//Parses the address used with the PORT (and PASV) command
//NOTE: "rawaddr" is modified by this function
int FtpConnection::parsePortAddr(char *rawaddr, char *addr, int maxaddr, char *port, int maxport)
{
	char *ptr1, *ptr2;
	char h1[4],h2[4],h3[4],h4[4];
	unsigned int p1i,p2i;

	if (rawaddr == NULL || addr == NULL || port == NULL)
		return false;

	if (maxaddr < 16 || maxport < 6)
		return false;

	//seek to the first digit
	ptr1 = rawaddr;
	while (isdigit(*ptr1) == 0 && *ptr1 != '\0')
		ptr1++;

	//extract the IP address
	if ((ptr2 = strchr(ptr1,',')) == NULL)
		return false;
	*ptr2 = '\0';
	strncpy(h1,ptr1,sizeof(h1)-1);
	h1[sizeof(h1)-1] = '\0';
	ptr1 = ptr2 + 1;
	if ((ptr2 = strchr(ptr1,',')) == NULL)
		return false;
	*ptr2 = '\0';
	strncpy(h2,ptr1,sizeof(h2)-1);
	h2[sizeof(h2)-1] = '\0';
	ptr1 = ptr2 + 1;
	if ((ptr2 = strchr(ptr1,',')) == NULL)
		return false;
	*ptr2 = '\0';
	strncpy(h3,ptr1,sizeof(h3)-1);
	h3[sizeof(h3)-1] = '\0';
	ptr1 = ptr2 + 1;
	if ((ptr2 = strchr(ptr1,',')) == NULL)
		return false;
	*ptr2 = '\0';
	strncpy(h4,ptr1,sizeof(h4)-1);
	h4[sizeof(h4)-1] = '\0';

	//extract the port number
	ptr1 = ptr2 + 1;
	if ((ptr2 = strchr(ptr1,',')) == NULL)
		return false;
	*ptr2 = '\0';
	p1i = atoi(ptr1);
	ptr1 = ptr2 + 1;
	p2i = atoi(ptr1);

	sprintf(addr,"%s.%s.%s.%s",h1,h2,h3,h4);
	sprintf(port,"%u",p1i*256+p2i);
	return true;
}

//////////////////////////////////////////////////////////////////////
// Protected Methods
//////////////////////////////////////////////////////////////////////

//Sends the default response to a FTP command that is not understood
void FtpConnection::defaultResp(int argc, char **argv)
{
	if (argc == 0 || argv == NULL)
	{
		eventHandler(argc,(const char**)argv,"command not understood","500",1,1,0);
		return;
	}

	std::string argline;
	CmdLine::combineArgs(argline, argc,(const char**)argv);

	writeToLineBuffer("'%s %s': command not understood",argv[0],argline.c_str());
	eventHandler(argc,(const char**)argv,_linebuffer,"500",1,1,0);
}

//checks the user's permissions, given a path, to see if "pflags" are present.
//if flagdir != 0, the directory part of the path will be checked if the
//full path does not exist.
//path = _userroot + _cwd + arg
//int FtpConnection::checkPermissions(const char *cmd, const char *arg, const char *pflags, const int flagdir /*=0*/)
//{
//	const char*args[2];
//	int type, argc;
//	bool retval = false;
//
//	if (cmd == NULL || pflags == NULL)
//	{
//		eventHandler(0,NULL,"ERROR: Invalid parameters while checking permissions.","451",1,1,1);
//		return false;
//	}
//
//	args[0] = cmd; args[1] = arg;
//
//	char path[MAX_PATH];
//	if (arg != NULL)
//	{
//		if ((FSUtils::buildPath(path, MAX_PATH, _userroot.c_str(),_cwd.c_str(),arg)) != NULL)
//		{
//			type = FSUtils::validatePath(path);
//			if (type == 2) //path is a file
//				FSUtils::removeSlashEnd(path);       //make sure files don't end in a slash
//			else if (type == 1) //path is directory
//				FSUtils::checkSlashEnd(path,strlen(path)+2); //make sure directories end in a slash
//			else
//
//			{
//				//path does not exist
//				if (flagdir != 0)   //check the directory part
//					FSUtils::getDirPath(path,strlen(path)+1,path); //get the directory part of the path
//			}
//			//check the user's permissions for the path
//			if (FSUtils::validatePath(path) != 0)
//				retval = _site.checkPermissions(_login,path,pflags);
//			else
//				retval = true; //return success to let the calling function send the response
//		}
//		else
//		{
//			eventHandler(2,args,"ERROR: out of memory.","451",1,1,1);
//			return false;
//		}
//		argc = 2;
//	}
//	else
//	{
//		//check the user's permissions (not path specific)
//		retval = _site.checkPermissions(_login,NULL,pflags);
//		argc = 1;
//	}
//
//	//print the permission denied message on failure
//	if (retval == false)
//	{
//		writeToLineBuffer("%s: Permission denied.",(arg != NULL) ? arg : "");
//		FSUtils::checkSlashUNIX(_linebuffer);  //display UNIX style paths
//		eventHandler(argc,args,_linebuffer,"550",1,1,0);
//	}
//
//	return retval;
//}

bool FtpConnection::appendLineBuf(const char *str)
{
	if (str == NULL)
		return false;

	size_t lenleft = sizeof(_linebuffer) - strlen(_linebuffer) -1;

	if (lenleft <=0)
		return false;  //the line buffer is full

	strncat(_linebuffer, str, lenleft);

	_linebuffer[sizeof(_linebuffer)-1] = '\0';

	return true;
}

void FtpConnection::setLastActive(time_t ti)
{
	_lastActive =(ti>0)? ti : time(NULL);
}

void FtpConnection::decDataThreads()
{
//	InterlockedDecrement((LONG volatile*)&_nDataThreads);
	ZQ::common::MutexGuard gd(_site._lockConCount);
	_site._nConnections--;
}

void FtpConnection::incDataThreads()
{
//	InterlockedIncrement((LONG volatile*)&_nDataThreads);
	ZQ::common::MutexGuard gd(_site._lockConCount);
	_site._nConnections++;
}

void FtpConnection::Close()
{
	_cli_sock->Close();
}

void FtpConnection::mapVirtualDir(const string& path, std::string& real_root, std::string& basename) {
	/* extract virtual dir name and content name */
	std::string::size_type pos = path.find_last_of(FNSEPC);
	/* seperator found, there is a volume */
	if(pos != std::string::npos) {
		/* extract volume name and try to match a virtual dir */
		std::string dirName = path.substr(0, pos);
		std::map<std::string, std::string>::const_iterator iter = _gCPECfg.dirInfos.find(dirName);
		/* fall back to default root if no match */
		if(iter  == _gCPECfg.dirInfos.end()) {
			real_root = _userroot;
		}
		else {
			real_root = iter->second; 	
		}
	}
	/* for the default volume */
	else {
		real_root = _userroot;
	}

	if(real_root.length()> 0  && real_root[real_root.length()-1] != FNSEPC) {
		real_root += FNSEPC;
	}
	basename = path.substr(pos+1);
}
