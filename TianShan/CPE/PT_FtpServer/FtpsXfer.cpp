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
// Ident : $Id: FtpsXfer.cpp,v 1.8 2004/08/17 02:52:08 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Deal with the file transfer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpsXfer.cpp $
// 
// 2     5/15/12 2:01p Li.huang
// fix bug  16451
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 8     10-11-02 14:29 Li.huang
//  megre from 1.10
// 
// 7     09-12-15 16:31 Yixin.tian
// delete warning
// 
// 7     09-12-10 17:56 Yixin.tian
// delete warning
// 
// 6     09-08-18 14:28 Yixin.tian
// 
// 5     09-06-15 16:53 Xia.chen
// 
// 4     08-12-19 17:03 Yixin.tian
// merge for Linux OS
// 
// 3     08-07-18 16:50 Jie.zhang
// 
// 2     08-03-28 16:12 Build
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 3     07-12-21 16:52 Fei.huang
// minor fixes, remove warning of size mismatch in copy expression while
// compiling
// 
// 2     07-12-19 15:38 Fei.huang
// comment out some operations that make no sense to Vstream storage
// 
// 2     07-09-13 17:23 Fei.huang
// return file found in vstream to the client
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 3     07-04-12 19:18 Jie.zhang
// 
// 2     07-03-22 18:36 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 1     05-09-06 13:57 Jie.zhang
// 
// 4     05-03-29 19:47 Jie.zhang
// 
// 3     05-02-06 17:03 Jie.zhang
// 
// 2     04-12-08 15:50 Jie.zhang
// 
// 3     04-11-19 17:02 Jie.zhang
// Revision 1.8  2004/08/17 02:52:08  jshen
// no message
//
// Revision 1.7  2004/08/12 09:07:38  jshen
// Remove Ended Thread List
//
// Revision 1.6  2004/07/29 06:21:45  jshen
// before release
//
// Revision 1.5  2004/07/23 09:04:36  jshen
// For QA test
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:18:10  jshen
// add comments
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================
#include "FtpsXfer.h"
#include "FtpSite.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>      //added for O_RDONLY, O_WRONLY, O_TRUNC, O_CREAT, O_BINARY

#ifdef WIN32
#include <io.h>       //for open(), read(), write(), close()
#else
#include <unistd.h>   //for open(), read(), write(), close()
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Class for Transfering Directory listings/Files (FtpsXfer)
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Function used to start the new transfer thread
//////////////////////////////////////////////////////////////////////

//NOTE: This function does not check the validity of the buffers
//      _context.path, _context.cwd, or _context.userroot.
//      The pointers psiteinfo and pftps are also not checked.
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FtpsXfer::FtpsXfer(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, ZQ::common::NativeThreadPool& Pool, context_t* pContext/*=NULL*/)
: ThreadRequest(Pool), _conn(ftps),_site(site), _pasv_sock(pasv_sock), _port_sock(port_sock)
{
	//initialize the connection parameters
	_dataSd			 = SOCK_INVALID;

	//initialize the transfer speed parameters
	_maxUlSpeed		 = 0;
	_maxDlSpeed		 = 0;
	_bytesXfered	 = 0;
	_xferTimer		 = 0;

	//initialize the transfer rate parameters
	_lastXferRateUpdt	= 0;
	_bytesSinceRateUpdt = 0;

	if (pContext != NULL)
		this->copyContext(pContext, &_context);		
	else
		memset(&_context, 0, sizeof(_context));
}

#ifdef _DEBUG
inline void DbgString(_TCHAR *format, ...)
{
	_TCHAR msg[4096];
	va_list	ap;
	va_start(ap, format);
	_vstprintf(msg, format, ap);
#ifdef ZQ_OS_MSWIN
	OutputDebugString(msg);
#else
	perror(msg);
#endif
}
#endif 

FtpsXfer::~FtpsXfer()
{
#ifdef _DEBUG
	DbgString(_T("~FtpsXfer()\n"));
#endif 
	if (_context.flagpasv != 0)
	{
		if (_pasv_sock)
		{
			delete _pasv_sock;
			_pasv_sock = NULL;
		}
	}
	else
	{
		if (_port_sock)
		{
			delete _port_sock;
			_port_sock = NULL;
		}
	}
}

bool FtpsXfer::init(void)
{
	return true;
}

int FtpsXfer::run(void)
{
	const char *args[2] = {"XFER", "UNKNOWN"};

	switch (_context.command)
	{
	case 'L':
		sendList();
		break;

	case 'R':
		recvFile();
		break;

	case 'S':
		sendFile();
		//reduce the data connection count
//		_conn.decDataThreads();
		break;

	default:
		_conn.eventHandler(2,args,"ERROR: unknown command type.","501",1,1,0);
		break;
	} // end switch

	return 0;
}

void FtpsXfer::final(int retcode, bool bCancelled)
{
	//	_context.path = _context.cwd = _context.userroot = NULL;
	//wait();
	delete this;
}

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

//Sends the directory listing using the control connection
//This is used by STAT
bool FtpsXfer::sendListCtrl()
{
	size_t nbytes = 0;

	if (_conn.getSocketDesc() == SOCK_INVALID)
		return false;

	//use the control socket desc as the data socket desc
	_dataSd = _conn.getSocketDesc();

	sendListData(nbytes);

	return true;
}

bool FtpsXfer::sendList() {

	char buffer[MAX_PATH];
	unsigned long ltime1, ltime2;
	size_t nbytes = 0;    //number of bytes sent in the dir listing

	const char* args[2] = {"LIST", NULL};
	args[1] = _context.path;

	//create the data connection
	if (_context.flagpasv != 0) {

		//wait for FTPS_STDSOCKTIMEOUT seconds to see if
		//_sock.Accept() will block.

		if (_pasv_sock->CheckStatus(FTPS_STDSOCKTIMEOUT) <= 0) {
			_conn.eventHandler(2,args,"Can't open data connection.","425",1,1,0);
			return false;
		}

		//accept the connection
		if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID) {
			_conn.eventHandler(2,args,"Can't open data connection.","425",1,1,0);
			return false;
		}
	}
	else {
		//open a connection (as a client)

		if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,FTPS_STDSOCKTIMEOUT)) == SOCK_INVALID) {
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			return false;
		}
	}

	if (checkFXP(0) == 0) {
		_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
		FtpSock::Close(_dataSd);
		return false;  //FXP is not allowed for the user
	}

	_conn.eventHandler(2,args,"Opening ASCII mode data connection for directory listing.","150",1,1,0);

	ltime1 = _site.timeval();   //time how long it takes to send the listing
	sendListData(nbytes);
	ltime2 = _site.timeval();   //stop timing

	//close the data connection
	FtpSock::Close(_dataSd);

	_conn._lastActive = time(NULL); //update the last active time

	//TODO: GENERALIZE THIS
	snprintf(buffer,79,"[Bytes: %ld][Time: %.2f s][Speed: %.2f K/s]",nbytes,_site.timeDiff2Sec(ltime1,ltime2),(nbytes/1024.0)/_site.timeDiff2Sec(ltime1,ltime2));
	buffer[79] = '\0';
	_conn.eventHandler(2,args,buffer,"226",1,1,0);

	//decrement the list transfer thread counter
	if (_conn._nListThreads >0)
		_conn._nListThreads--;

	return true;
}

bool FtpsXfer::recvFile()
{
	unsigned long ltime1, ltime2;
	long nbytes = 0;    //number of bytes received
	char cmd[] ="STOR";
	const char *args[2] = {cmd, NULL};

	char buffer[MAX_PATH];
	char *filepath = buffer;

	args[1] = _context.path;

	//allocate memory/build the full directory/file path
	if (!FSUtils::buildPath(filepath, MAX_PATH, _context.userroot,_context.cwd,_context.path))
	{
		_conn.eventHandler(2,args,"ERROR: out of memory.","425",1,1,1);
		return false;
	}

	if (_context.mode == 1)
	{
		//if STOU was used (mode = 1), make sure the filename is unique
		strcpy(cmd,"STOU");
		int extnum;
		if ((extnum = getUniqueExtNum(filepath)) != 0)
		{
			std::string fp = filepath;
			//build the new display path
			sprintf(filepath,"%s.%u",fp.c_str(),extnum);
			args[1] = filepath;
		}
	}
	else if (_context.mode == 2)
	{
		FSUtils::fileInfo_t fileinfo;
		//if APPE was used (mode = 2), set the reset offset to the end of the file
		strcpy(cmd,"APPE");
		if (FSUtils::getFileStats(filepath, &fileinfo) != 0)
			_context.restoffset = fileinfo.size;
	}

	//open a file for writing
	int fdw, oflag;
	oflag = ((_context.restoffset > 0) ? (O_APPEND | O_RDWR) : (O_TRUNC | O_WRONLY)) | O_CREAT;
#ifdef WIN32 //include O_BINARY for WINDOWS
	oflag |= O_BINARY;
#endif
	if ((fdw = open(filepath,oflag,0666)) < 0)
	{
		sprintf(buffer,"%s: The system cannot write to the file specified.",_context.path);
		_conn.eventHandler(2,args,buffer,"425",1,1,0);
		return false;
	}

	//add the file to the server (initial size is 0 bytes)
	_site.addFile(filepath, _conn.getLogin(), 0);

	//create the data connection
	if (_context.flagpasv != 0)
	{
		//wait for FTPS_STDSOCKTIMEOUT seconds to see if
		//_sock.Accept() will block.
		if (_pasv_sock->CheckStatus(FTPS_STDSOCKTIMEOUT) <= 0)
		{
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdw);
			return false;
		}
		//accept the connection
		if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
		{
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdw);
			return false;
		}
	}
	else
	{
		//open a connection (as a client)
		if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,FTPS_STDSOCKTIMEOUT)) == SOCK_INVALID)
		{
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdw);
			return false;
		}
	}

	if (checkFXP(1) == 0)
	{
		_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
		::close(fdw);
		FtpSock::Close(_dataSd);
		return false; //FXP is not allowed
	}

	//display which mode of transfer is being used.
	if (_context.type == 'A')
	{
		sprintf(buffer,"Opening ASCII mode data connection for %s.",_context.path);
		_conn.eventHandler(2,args,buffer,"150",1,1,0);
	}
	else
	{
		sprintf(buffer,"Opening BINARY mode data connection for %s.",_context.path);
		_conn.eventHandler(2,args,buffer,"150",1,1,0);
	}

	//set the max upload speed for the user
	_maxUlSpeed = _site.getMaxULSpeed(_conn.getLogin());

	ltime1 = _site.timeval();   //time how long it takes to receive the data
	nbytes = recvFileData(fdw);
	ltime2 = _site.timeval();   //stop timing

	//close the data connection and file descriptor
	FtpSock::Close(_dataSd);
	::close(fdw);

	//Update the file on the server (get the filesize from the OS)
	_site.addFile(filepath,_conn.getLogin());

	//reset the current transfer rate to 0
	_site.setCurrentXferRate(_conn.getUserID(),0);

	_conn._lastActive = time(NULL); //update the last active time

	//set the modification time, if one was specified
	if (_context.timemod != 0)
		FSUtils::setFileTime(filepath,_context.timemod);

	//update the upload statistics
	_site.updateULStats(_conn.getUserID(),_conn.getLogin(),nbytes,(int)(nbytes/_site.timeDiff2Sec(ltime1,ltime2)));
	//update the user's credits
	_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),nbytes,1);

	//TODO: GENERALIZE THIS
	snprintf(buffer,79,"[Bytes: %lu][Time: %.2f s][Speed: %.2f K/s]",nbytes,_site.timeDiff2Sec(ltime1,ltime2),(nbytes/1024.0)/_site.timeDiff2Sec(ltime1,ltime2));
	buffer[79] = '\0';
	_conn.eventHandler(2,args,buffer,"226",1,1,0);

	//decrement the data transfer thread counter
	if (_conn._nDataThreads > 0)
		_conn._nDataThreads --;

	return true;
}

bool FtpsXfer::sendFile()
{
	FSUtils::fileInfo_t finfo;
	char buffer[MAX_PATH], *filepath=buffer;
	unsigned long ltime1, ltime2;
	long nbytes = 0;    //number of bytes sent
	int fdr;
	const char *args[2], cmd[] = "RETR";

	args[0] = cmd; args[1] = _context.path;

	//allocate memory/build the full directory/file path
	if (!FSUtils::buildPath(buffer, MAX_PATH, _context.userroot, _context.cwd, _context.path))
	{
		_conn.eventHandler(2,args,"ERROR: out of memory.","425",1,1,1);
		return false;
	}

	//open a file for reading
#ifdef WIN32 //include O_BINARY for WINDOWS
	if ((fdr = open(filepath,O_RDONLY | O_BINARY,0666)) < 0)
	{
#else
	if ((fdr = open(filepath,O_RDONLY,0666)) < 0)
	{
#endif
		sprintf(buffer,"%s: The system cannot find the file specified.",_context.path);
		_conn.eventHandler(2,args,buffer,"425",1,1,0);
		return false;
	}

	if (FSUtils::getFileStats(filepath,&finfo) == 0)
	{
		_conn.eventHandler(2,args,"The system cannot find the file specified.","425",1,1,0);
		::close(fdr);
		return false;
	}

	//update the user's credits (deduct the credits for this file)
//	if (_site.removeCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0) == 0)
//	{
//		_conn.eventHandler(2,args,"Insufficient credits.","550",1,1,0);
//		::close(fdr);
//		return false;
//	}

	if (_context.flagpasv != 0)
	{
		//wait for FTPS_STDSOCKTIMEOUT seconds to see if
		//_sock.Accept() will block.
		if (_pasv_sock->CheckStatus(FTPS_STDSOCKTIMEOUT) <= 0)
		{
			_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0); //restore the credits
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdr);
			return false;
		}
		//accept the connection
		if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
		{
			_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0); //restore the credits
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdr);
			return false;
		}
	}
	else
	{
		//open a connection (as a client)
		if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,FTPS_STDSOCKTIMEOUT)) == SOCK_INVALID)
		{
			_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0); //restore the credits
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			::close(fdr);
			return false;
		}
	}

	if (checkFXP(0) == 0)
	{
		_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0); //restore the credits
		_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
		FtpSock::Close(_dataSd);
		::close(fdr);
		return false;  //FXP is not allowed
	}

	//display which mode of transfer is being used.
	if (_context.type == 'A')
	{
		sprintf(buffer,"Opening ASCII mode data connection for %s ("FMT64 " bytes).",_context.path,finfo.size);
		_conn.eventHandler(2,args,buffer,"150",1,1,0);
	}
	else
	{
		sprintf(buffer,"Opening BINARY mode data connection for %s ("FMT64" bytes).",_context.path,finfo.size);
		_conn.eventHandler(2,args,buffer,"150",1,1,0);
	}

	//set the max download speed for the user
	_maxDlSpeed = _site.getMaxDLSpeed(_conn.getLogin());

	ltime1 = _site.timeval();   //time how long it takes to send the data
	nbytes = sendFileData(fdr);
	ltime2 = _site.timeval();   //stop timing

	//close the data connection and file descriptor
	FtpSock::Close(_dataSd);
	::close(fdr);

	//reset the current transfer rate to 0
	_site.setCurrentXferRate(_conn.getUserID(),0);

	_conn._lastActive = time(NULL); //update the last active time

	//if the transfer was successful
	if ((nbytes + _context.restoffset) >= finfo.size)
	{
		//update the download statistics
		_site.updateDLStats(_conn.getUserID(),_conn.getLogin(),finfo.size,(int)(nbytes/_site.timeDiff2Sec(ltime1,ltime2)));
	}
	else
	{
		//restore the credits
		_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),finfo.size,0);
	}

	//TODO: GENERALIZE THIS
	snprintf(buffer,79,"[Bytes: %lu][Time: %.2f s][Speed: %.2f K/s]",nbytes,_site.timeDiff2Sec(ltime1,ltime2),(nbytes/1024.0)/_site.timeDiff2Sec(ltime1,ltime2));
	buffer[79] = '\0';
	_conn.eventHandler(2,args,buffer,"226",1,1,0);

	//decrement the data transfer thread counter
	if (_conn._nDataThreads > 0)
		_conn._nDataThreads --;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////

//Sends the directory listing to the client
//"nbytes" will contain the total number of bytes sent
void FtpsXfer::sendListData(size_t& nbytes)
{
	sendListData(_context, nbytes);
}

void FtpsXfer::sendListData(context_t& ctxt,size_t& nbytes) {

//	char buffer[MAX_PATH], *fullpath=buffer; //linebuf[FTPSXFER_MAXDIRENTRY];
	char packet[FTPSXFER_MAXPACKETSIZE]; //, thisyear[8];
	size_t packetlen = 0;
//	long handle;

	//build the full directory/file path
//	if (!FSUtils::buildPath(fullpath,MAX_PATH, ctxt.userroot,ctxt.cwd,ctxt.path))
//		return;

	//Get the current year.  This is needed to determine if a file is more
	//than a year old

	//check if the fullpath is a file.
	std::string listline;
	if (buildListLine(
				listline, 
				"/", 
				"" , 
				2, 
				_site.timeString("%Y").c_str()) && !listline.empty()) {

		size_t pos = listline.find_first_of('\r');
		if (pos != std::string::npos)
//		if (pos == std::string::npos)
		{
			listline = listline.substr(0, pos) + "\r\n";    //end the line
			nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, listline.c_str(), listline.length() ,ctxt.bencdata, 1);
		}

		return; //send the listing for the file and return
	}

	//If the recursive option (-R) was selected, display the path and check permissions
//	if (strchr(ctxt.options,'R') != NULL)
//	{
//		FSUtils::checkSlashEnd(fullpath, strlen(fullpath) +2);
//		//check if the user has permission to list the directory
//		if (_site.checkPermissions(_conn.getLogin(), fullpath, "lvx") != 0)
//		{
//			char tmpbuf[MAX_PATH];
//			//user has permission
//			sprintf(tmpbuf, "%s:\r\n", ctxt.path);
//			FSUtils::checkSlashUNIX(tmpbuf);
//			nbytes += addToSendBuffer(packet,FTPSXFER_MAXPACKETSIZE, packetlen,tmpbuf,strlen(tmpbuf),ctxt.bencdata);
//		}
//		else
//		{
//			//permission denied
//			char tmpbuf[MAX_PATH];
//			sprintf(tmpbuf,"%s: Permission denied.\r\n",ctxt.path);
//			FSUtils::checkSlashUNIX(tmpbuf);
//			nbytes += addToSendBuffer(packet,FTPSXFER_MAXPACKETSIZE, packetlen,tmpbuf,strlen(tmpbuf),ctxt.bencdata,1);
//			return; //send the permission denied message and return
//		}
//	}

	/*
	BaseIOI* pBaseIO = BaseIOI::CreateInstance(BaseIOI::IO_VSTRM);

	// check the existence in Vstrm
	char* path = (ctxt.path[0] == '/' || ctxt.path[0] == '\\') ? ctxt.path+1 : ctxt.path;
	if(!pBaseIO->Open(path, BaseIOI::BF_READ)) {
		pBaseIO->Release();

		return;
	}

	pBaseIO->Release();

	buildListLine(listline, "", path, 2, thisyear);
	nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, listline.c_str(), listline.length(), ctxt.bencdata);

	return;
*/
	//List all the directories
/*
	if ((handle = FSUtils::dirGetFirstFile(fullpath, linebuf, FTPSXFER_MAXDIRENTRY)) <= 0)
		return;

	do
	{
		if (_conn.isAbor())  //check for ABOR
			return;

		buildListLine(listline, fullpath, linebuf, 1, thisyear);
		nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, listline.c_str(), listline.length(), ctxt.bencdata);

	} while (FSUtils::dirGetNextFile(handle,linebuf,FTPSXFER_MAXDIRENTRY) > 0);

	FSUtils::dirClose(handle);

	//List all the files
	if ((handle = FSUtils::dirGetFirstFile(fullpath, linebuf, FTPSXFER_MAXDIRENTRY)) <= 0)
		return;

	do
	{
		if (_conn.isAbor())  //check for ABOR
			return;

		buildListLine(listline, fullpath ,linebuf, 2, thisyear);
		nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, listline.c_str(), listline.length(), ctxt.bencdata);

	} while (FSUtils::dirGetNextFile(handle,linebuf,FTPSXFER_MAXDIRENTRY) > 0);

	FSUtils::dirClose(handle);

	//if the recursive option (-R) was selected do a second pass
	//to go through the sub-directories.

	if (strchr(ctxt.options,'R') != NULL)
	{
		//force any remaining data in the buffer to be sent (and add a \r\n)
		nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, "\r\n", 2, ctxt.bencdata, 1);

		//List all the directories
		if ((handle = FSUtils::dirGetFirstFile(fullpath, linebuf, FTPSXFER_MAXDIRENTRY)) <= 0)
			return;

		do
		{
			buildListLine(listline, fullpath, linebuf, 1, thisyear);
			if (!listline.empty())
			{
				context_t tmpxferinfo;
				copyContext(&tmpxferinfo, &ctxt);
				if (tmpxferinfo.path[0] != '\0')
					FSUtils::checkSlashEnd(tmpxferinfo.path, sizeof(tmpxferinfo.path));

				strcpy(tmpxferinfo.path + strlen(tmpxferinfo.path), linebuf);
				
				FSUtils::checkSlashUNIX(tmpxferinfo.path);
				sendListData(tmpxferinfo,nbytes);   //Recurse
			}
		} while (FSUtils::dirGetNextFile(handle, linebuf, FTPSXFER_MAXDIRENTRY) > 0);

		FSUtils::dirClose(handle);
	}
	else
	{
		//force any remaining data in the buffer to be sent
		nbytes += addToSendBuffer(packet,FTPSXFER_MAXPACKETSIZE, packetlen,"",0,ctxt.bencdata,1);
	}
*/
}

bool FtpsXfer::copyContext(const context_t* from, context_t* to)
{
	if (from == NULL || to ==NULL)
		return false;

	memcpy(to, from, sizeof(context_t));	

	return true;
}


long FtpsXfer::recvFileData(int fdw)
{
	char packet[FTPSXFER_MAXPACKETSIZE], *tmppacket;
	long nbytes = 0;
	int packetlen = 0, tmppacketlen = 0;
#ifdef ZQ_OS_MSWIN
	char lastchar = ' ';
#endif

	//move the file pointer to the proper restart position
	//(used if command is APPE or REST was specified)
#ifdef ZQ_OS_MSWIN
	if (_context.restoffset > 0)
		_lseeki64(fdw,_context.restoffset,0);
#else
	if (_context.restoffset > 0)
		lseek64(fdw,_context.restoffset,0);
#endif
	//initialize the transfer rate parameters
	_site.setCurrentXferRate(_conn.getUserID(),0);
	_bytesSinceRateUpdt = 0;
	_lastXferRateUpdt = _site.timeval();
	_bytesXfered = 0;
	_xferTimer = _site.timeval();

	do
	{
		if (_conn.isAbor())
			return false;

		if (FtpSock::CheckStatus(_dataSd, 10*FTPS_STDSOCKTIMEOUT, 0) <= 0)
			return false;  //if the data stops being sent

		if ((packetlen = recvData(packet,FTPSXFER_MAXPACKETSIZE)) < 0)
			return false;  //if there was an error receiving the data

		if (_context.type == 'A')
		{
			//Recv in ASCI mode
#ifdef ZQ_OS_MSWIN
			//For WINDOWS OS an ASCII file will always contain \r\n (NOT \n)
			if ((tmppacket = BtoA(lastchar,packet,packetlen,&tmppacketlen)) != NULL)
			{
				lastchar = tmppacket[tmppacketlen-1];
				write(fdw,tmppacket,tmppacketlen);
				delete tmppacket;
			}
			else
				return false;    //out of memory
#else                   //Recv in BINARY mode
			//For UNIX OS an ASCII file will always contain \n (NOT \r\n)
			if (AtoU(packet,packetlen,tmppacketlen))
			{
				write(fdw,packet,tmppacketlen);
				delete[] tmppacket;
			}
			else
				return false;    //out of memory
#endif
		}
		else
			write(fdw,packet,packetlen);

		nbytes += packetlen;

	} while(packetlen == FTPSXFER_MAXPACKETSIZE);

	return(nbytes);
}

long FtpsXfer::sendFileData(int fdr)
{
	char lastchar = ' ', packet[FTPSXFER_MAXPACKETSIZE];
	long nsent, nbytes = 0;
	int packetlen = 0, tmppacketlen = 0;

	//move the file pointer to the proper restart position if REST was specified
#ifdef ZQ_OS_MSWIN
	if (_context.restoffset > 0)
		_lseeki64(fdr,_context.restoffset,0);
#else
	if (_context.restoffset > 0)
		lseek64(fdr,_context.restoffset,0);
#endif
	//initialize the transfer rate parameters
	_site.setCurrentXferRate(_conn.getUserID(),0);
	_bytesSinceRateUpdt = 0;
	_lastXferRateUpdt = _site.timeval();
	_bytesXfered = 0;
	_xferTimer = _site.timeval();

	do
	{
		if (_conn.isAbor())
			return false;

		packetlen = read(fdr,packet,FTPSXFER_MAXPACKETSIZE);
		if (_context.type == 'A')
		{
			char *tmppacket;
			if ((tmppacket = BtoA(lastchar,packet,packetlen,&tmppacketlen)) != NULL)
			{
				lastchar = tmppacket[tmppacketlen-1];
				if ((nsent = sendData(tmppacket,tmppacketlen)) < 0)
				{
					delete tmppacket;
					return false;
				}
				else nbytes += nsent;

				delete tmppacket;
			}
			else lastchar = ' ';
		}
		else
		{
			if ((nsent = sendData(packet,packetlen)) < 0)
				return false;
			else nbytes += nsent;
		}
	} while(packetlen == FTPSXFER_MAXPACKETSIZE);

	return(nbytes);
}

//Adds "data" to the "sendbuf" and sends out the buffer on "_dataSd"/"m_sslinfo"
//when "sendbuf" reaches "maxsendbuf".
//The "sendbufoffset" is used to keep track of the current position
//in "sendbuf" where the data ends.
//if "flagforcesend" != 0, the buffer will always be sent.
//if bencdata != 0, the data will be sent over the ssl connection.
//Returns the number of bytes sent.  -1 is returned on error.
size_t FtpsXfer::addToSendBuffer(char* sendbuf, const size_t maxsendbuf, size_t& sendbufoffset, const char *dataptr, const size_t datasize, const bool bencdata, const bool flagforcesend /*= false*/)
{
	size_t dataoffset =0, nbytes =0;

	if (sendbuf == NULL || maxsendbuf == 0)
		return false;

	while (true)
	{
		size_t nleftsend = maxsendbuf - sendbufoffset;  //number of bytes left on the send buffer
		if ((datasize - dataoffset) > nleftsend)
		{
			//fill the remaining free space in the send buffer
			memcpy(sendbuf+sendbufoffset, dataptr+dataoffset, nleftsend);
			dataoffset += nleftsend;
			//send the buffer
			if (bencdata == 0)
				nbytes += FtpSock::SendN(_dataSd, sendbuf,maxsendbuf);
			sendbufoffset = 0;
		}
		else
		{
			//copy the data to the send buffer
			memcpy(sendbuf+sendbufoffset, dataptr+dataoffset, datasize-dataoffset);
			sendbufoffset += (datasize - dataoffset);
			break;
		}
	}

	//"" the send buffer
	if (flagforcesend)
	{
		if (!bencdata)
			nbytes += FtpSock::SendN(_dataSd, sendbuf, sendbufoffset);
		sendbufoffset = 0;
	}

	return(nbytes);
}

//Loads "listline" with a directory listing line that should be returned.
//"options" are the directory listing options that were specified.
//"flagdir" is used for selecting only directories (=1), files (=2), or
//both (=3).
bool FtpsXfer::buildListLine(
				std::string& line, 
				const char *fullpath, 
				const char *filename, 
				const int flagdir, 
				const char *thisyear) {

	line = "";

	FSUtils::fileInfo_t finfo;
	char filepath[MAX_PATH];

	//do not display the "." and ".." entries
	if (strcmp(filename,".") == 0 || strcmp(filename,"..") == 0)
		return false;

	if (strchr(fullpath,'*') != NULL)   //check if wildcard
		FSUtils::getDirPath(filepath,strlen(fullpath)+1,fullpath); //get only the directory name
	else
		strcpy(filepath,fullpath);

	FSUtils::checkSlashEnd(filepath,strlen(fullpath)+strlen(filename)+2);
	strcat(filepath,filename);
	FSUtils::checkSlash(filepath);

	if (FSUtils::getFileStats(filepath,&finfo) == 0)
		return false;

//	if (finfo.mode & S_IFDIR) {
//		if ((flagdir & 1) == 0)
//			return false;   //only file lines should be returned
//	}
//	else if ((flagdir & 2) == 0) {
//		return false;   //only directory lines should be returned
//	}

	if (strchr(_context.options,'N') != NULL)
		line = filename;
	else {
		// build a full LIST line
		char linebuf[MAX_PATH] ="", filename[MAX_PATH];
		FSUtils::fileInfo_t finfo;

		char *ptr, permissions[10], user[9], group[9], filetype;

		if (FSUtils::getFileStats(filepath,&finfo) == 0)
			return false;

		FSUtils::getFileName(filename, strlen(filepath)+1, filepath);

		filetype = (finfo.mode & S_IFDIR) ? 'd' : '-';

		char timebuf[64];
		strcpy(timebuf, _site.timeString("%Y %b %d %H:%M", finfo.timecreate).c_str());
		if ((ptr = strrchr(timebuf,' ')) != NULL)
		{
			ptr++;
			if (thisyear != NULL)
			{
				if (strncmp(timebuf,thisyear,4) != 0)
				{
					*ptr = ' ';
					memmove(ptr+1,timebuf,4);     //overwrite the time of day with the year
				}
			}
			ptr = timebuf + 5;  //move to the start of the month name
			FSUtils::getPrmString(finfo.mode,permissions,sizeof(permissions));
			FSUtils::getUsrName(finfo.userid,user,sizeof(user));
			FSUtils::getGrpName(finfo.groupid,group,sizeof(group));
#ifdef ZQ_OS_MSWIN
			snprintf(linebuf, MAX_PATH,"%c%9s %3ld %-8s %-8s %10lld %s ",filetype,permissions,finfo.nlinks,user,group,finfo.size,ptr);
#else
			snprintf(linebuf, MAX_PATH,"%c%9s %3ld %-8s %-8s %10ld %s ",filetype,permissions,finfo.nlinks,user,group,finfo.size,ptr);
		
#endif
			line = linebuf;
			line += filename;
		}
	}

	line +="\r\n";        //end the line

	return true;
}

//Converts the contents of inbuf from a BINARY mode to an
//ASCII mode (all "\n" are converted to "\r\n").
//"lastchar" is the last character in the previous buffer
//-- this is needed to detect a "\r\n" across buffers
//A buffer containing the ASCII version is returned.
//The size of the returned buffer is loaded into "outbufsize"
//NOTE: The returned buffer must be deleted
char *FtpsXfer::BtoA(char lastchar, char *inbuf, int inbufsize, int *outbufsize)
{
	char *outbuf = NULL;
	int i, nlcount = 0, noutbuf = 0;

	if (outbufsize != NULL)
		*outbufsize = 0;    //initialize the out buffer size

	if (inbuf == NULL || inbufsize == 0)
		return NULL;

	if (*inbuf == '\n' && lastchar != '\r')
		nlcount++;
	for (i = 1; i < inbufsize; i++)
	{
		if (*(inbuf+i) == '\n' && *(inbuf+i-1) != '\r')
			nlcount++;
	}

	if ((outbuf = new char[inbufsize+nlcount]) == NULL)
		return NULL;

	//copy data to the new buffer
	if (*inbuf == '\n' && lastchar != '\r')
	{
		memcpy(outbuf,"\r\n",2);
		noutbuf+= 2;
	}
	else
	{
		*outbuf = *inbuf;
		noutbuf++;
	}
	for (i = 1; i < inbufsize; i++)
	{
		if (*(inbuf+i) == '\n' && *(inbuf+i-1) != '\r')
		{
			memcpy(outbuf+noutbuf,"\r\n",2);
			noutbuf += 2;
		}
		else
		{
			*(outbuf + noutbuf) = *(inbuf + i);
			noutbuf++;
		}
	}

	if (outbufsize != NULL)
		*outbufsize = noutbuf;

	return(outbuf);
}

//Converts the contents of inbuf from a FTP ASCII mode (\r\n) to a
//Unix ASCII mode (\n) (all "\r\n" are converted to "\n").
//A buffer containing the Unix ASCII version is returned.
//The size of the returned buffer is loaded into "outbufsize"
bool FtpsXfer::AtoU(char *buf, int bufsize, int& changedLen)
{
	int i, noutbuf = 0;

	if (buf == NULL || bufsize <= 0)
		return false;

	//copy data to the new buffer
	for (i = 0; i < bufsize; i++)
	{
		if (buf[i] != '\r')
		{
			buf[noutbuf++] = buf[i];
			continue;
		}

		if (i < (bufsize-1) && buf[i+1] != '\n')
			buf[noutbuf++] = buf[i];   //keep only '\r' w/o following '\n'
	}

	buf[noutbuf] = '\0';
	changedLen = noutbuf;
	return(true);
}

//updates the current transfer rate
void FtpsXfer::updateXferRate(long bytessent)
{
	double currxferrate;

	if (bytessent > 0)
		_bytesSinceRateUpdt += bytessent;

	if (_site.timeDiff2Sec(_lastXferRateUpdt,_site.timeval()) >= FTPSXFER_XFERRATEUPDTRATE)
	{
		currxferrate = _bytesSinceRateUpdt / _site.timeDiff2Sec(_lastXferRateUpdt,_site.timeval());
		_site.setCurrentXferRate(_conn.getUserID(),(long)currxferrate);
		_bytesSinceRateUpdt = 0;
		_lastXferRateUpdt = _site.timeval();
	}
}

int FtpsXfer::sendData(char *buffer, int bufsize)
{
	int nbytessent = 0, nbytestosend, offset = 0, packetlen = 0;
	int usermaxbytesperinterval, sitemaxbytesperinterval, useravail, siteavail;
	unsigned long xferupdtmsec, timediff;

	//Max number of bytes that can be sent within "FTPSXFER_XFERRATEUPDTRATE" sec.
	//Max bytes for the current user.
	usermaxbytesperinterval = (int)((double)_maxDlSpeed * FTPSXFER_XFERRATEUPDTRATE);
	//Max bytes for the whole site (all users combined).
	sitemaxbytesperinterval = (int)((double)_site._maxDlSpeed * FTPSXFER_XFERRATEUPDTRATE);

	xferupdtmsec = (unsigned)(FTPSXFER_XFERRATEUPDTRATE * 1000); //convert to msec

	if (usermaxbytesperinterval == 0 && sitemaxbytesperinterval == 0)
	{
		if (_context.bencdata == 0)
			nbytessent = FtpSock::SendN(_dataSd, buffer,bufsize);
		updateXferRate(nbytessent);
		return(nbytessent); //no limits
	}

	usermaxbytesperinterval = (usermaxbytesperinterval == 0) ? 0x7FFFFFFF : usermaxbytesperinterval;
	sitemaxbytesperinterval = (sitemaxbytesperinterval == 0) ? 0x7FFFFFFF : sitemaxbytesperinterval;

	while (offset < bufsize)
	{
		useravail = usermaxbytesperinterval - _bytesXfered;
		siteavail = sitemaxbytesperinterval - _site._bytesSent;
		useravail = (useravail <= 0) ? 0 : useravail;
		siteavail = (siteavail <= 0) ? 0 : siteavail;

		if (useravail <= siteavail)
		{
			//user limitation
			nbytestosend = ((bufsize - offset) <= useravail) ? (bufsize - offset) : useravail;
			_bytesXfered += nbytestosend;
			useravail = usermaxbytesperinterval - _bytesXfered;
			if (useravail <= 0)
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::SendN(_dataSd, buffer+offset,nbytestosend);

				if (packetlen < 0)
					return(packetlen);
				timediff = _site.timeDiff(_xferTimer,_site.timeval());
				if (xferupdtmsec > timediff)
#ifdef ZQ_OS_MSWIN
					Sleep(xferupdtmsec - timediff);
#else
					usleep((xferupdtmsec - timediff)*1000ll);
#endif
				_xferTimer = _site.timeval();
				_bytesXfered = 0;
			}
			else
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::SendN(_dataSd, buffer+offset,nbytestosend);

				if (packetlen < 0)
					return(packetlen);
			}
			packetlen = (packetlen <= 0) ? 0 : packetlen;
			nbytessent += packetlen;
			offset += packetlen;
			updateXferRate(packetlen);
		}
		else
		{
			//site limitation
			//NOTE: maybe send out data in smaller pieces in case of slow connection
			nbytestosend = ((bufsize - offset) <= siteavail) ? (bufsize - offset) : siteavail;
			_site._bytesSent += nbytestosend;
			siteavail = sitemaxbytesperinterval - _site._bytesSent;
			if (siteavail <= 0)
			{
				Guard<Mutex> op(_site._sendMutex);
				
				if (_context.bencdata == 0)
					packetlen = FtpSock::SendN(_dataSd, buffer+offset,nbytestosend);

				if (packetlen < 0)
					return(packetlen);
				timediff = _site.timeDiff(_site._sendTimer,_site.timeval());
				if (xferupdtmsec > timediff)
#ifdef ZQ_OS_MSWIN
					Sleep(xferupdtmsec - timediff);
#else
					usleep((xferupdtmsec - timediff)*1000ll);
#endif
				_site._sendTimer = _site.timeval();
				_site._bytesSent = 0;
			}
			else
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::SendN(_dataSd, buffer+offset,nbytestosend);

				if (packetlen < 0)
					return(packetlen);
			}
			packetlen = (packetlen <= 0) ? 0 : packetlen;
			nbytessent += packetlen;
			offset += packetlen;
			updateXferRate(packetlen);
		}

	}

	return(nbytessent);
}

int FtpsXfer::recvData(char *buffer, int bufsize)
{
	int nbytesrecv = 0, nbytestorecv = 0, offset = 0, packetlen = 0;
	int usermaxbytesperinterval, sitemaxbytesperinterval, useravail, siteavail;
	unsigned long xferupdtmsec, timediff;

	//Max number of bytes that can be sent within "FTPSXFER_XFERRATEUPDTRATE" sec.
	//Max bytes for the current user.
	usermaxbytesperinterval = (int)((double)_maxUlSpeed * FTPSXFER_XFERRATEUPDTRATE);
	//Max bytes for the whole site (all users combined).
	sitemaxbytesperinterval = (int)((double)_site._maxUlSpeed * FTPSXFER_XFERRATEUPDTRATE);

	xferupdtmsec = (unsigned)(FTPSXFER_XFERRATEUPDTRATE * 1000); //convert to msec

	if (usermaxbytesperinterval == 0 && sitemaxbytesperinterval == 0)
	{
		if (_context.bencdata == 0)
			nbytesrecv = FtpSock::RecvN_Timeout(_dataSd, buffer,bufsize);

		updateXferRate(nbytesrecv);
		return(nbytesrecv);  //no limits
	}

	usermaxbytesperinterval = (usermaxbytesperinterval == 0) ? 0x7FFFFFFF : usermaxbytesperinterval;
	sitemaxbytesperinterval = (sitemaxbytesperinterval == 0) ? 0x7FFFFFFF : sitemaxbytesperinterval;

	while (offset < bufsize && packetlen == nbytestorecv)
	{

		useravail = usermaxbytesperinterval - _bytesXfered;
		siteavail = sitemaxbytesperinterval - _site._bytesRecv;
		useravail = (useravail <= 0) ? 0 : useravail;
		siteavail = (siteavail <= 0) ? 0 : siteavail;

		if (useravail <= siteavail)
		{
			//user limitation
			nbytestorecv = ((bufsize - offset) <= useravail) ? (bufsize - offset) : useravail;
			_bytesXfered += nbytestorecv;
			useravail = usermaxbytesperinterval - _bytesXfered;
			if (useravail <= 0)
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::RecvN_Timeout(_dataSd, buffer+offset,nbytestorecv);

				if (packetlen < 0)
					return(packetlen);
				timediff = _site.timeDiff(_xferTimer,_site.timeval());
				if (xferupdtmsec > timediff)
#ifdef ZQ_OS_MSWIN
					Sleep(xferupdtmsec - timediff);
#else
					usleep((xferupdtmsec - timediff)*1000ll);
#endif
				_xferTimer = _site.timeval();
				_bytesXfered = 0;
			} 
			else 
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::RecvN_Timeout(_dataSd, buffer+offset,nbytestorecv);

				if (packetlen < 0)
					return(packetlen);
			}
			packetlen = (packetlen <= 0) ? 0 : packetlen;
			nbytesrecv += packetlen;
			offset += packetlen;
			updateXferRate(packetlen);
		}
		else
		{
			//site limitation
			//NOTE: maybe receive in smaller pieces in case of slow connection
			nbytestorecv = ((bufsize - offset) <= siteavail) ? (bufsize - offset) : siteavail;
			_site._bytesRecv += nbytestorecv;
			siteavail = sitemaxbytesperinterval - _site._bytesRecv;
			if (siteavail <= 0)
			{
				Guard<Mutex> op(_site._recvMutex);
				
				if (_context.bencdata == 0)
					packetlen = FtpSock::RecvN_Timeout(_dataSd, buffer+offset,nbytestorecv);
				if (packetlen < 0)
					return(packetlen);
				timediff = _site.timeDiff(_site._recvtimer,_site.timeval());
				if (xferupdtmsec > timediff)
#ifdef ZQ_OS_MSWIN
					Sleep(xferupdtmsec - timediff);
#else
					usleep((xferupdtmsec - timediff)*1000ll);
#endif
				_site._recvtimer = _site.timeval();
				_site._bytesRecv = 0;

			}
			else
			{
				if (_context.bencdata == 0)
					packetlen = FtpSock::RecvN_Timeout(_dataSd, buffer+offset,nbytestorecv);

				if (packetlen < 0)
					return(packetlen);
			}
			packetlen = (packetlen <= 0) ? 0 : packetlen;
			nbytesrecv += packetlen;
			offset += packetlen;
			updateXferRate(packetlen);
		}

	}

	return(nbytesrecv);
}

//returns the number to add as an extension to make the filename unique
//if the file DNE, 0 is returned (no extension needed)
int FtpsXfer::getUniqueExtNum(char *filepath)
{
	if (FSUtils::validatePath(filepath) == 0)
		return false;  //the file DNE

	char *buffer = new char[strlen(filepath)+12];
	if (buffer == NULL)
		return false;

	strcpy(buffer,filepath);
	size_t extoffset = strlen(filepath);

	int i;
	for (i = 1; i < 0x7FFFFFFF; i++)
	{
		sprintf(buffer+extoffset,".%u",i);
		if (FSUtils::validatePath(buffer) == 0)
			break;
	}

	delete buffer;
	return(i);
}

//checks if FXP is being used and if FXP is allowed
int FtpsXfer::checkFXP(int flagupload) {
	char ipaddr[SOCK_IPADDRLEN], port[SOCK_PORTLEN];
	int retval = 0;

	//get the client's address and port based on the data socket desc
	FtpSock::getPeerAddrPort(_dataSd, ipaddr,sizeof(ipaddr),port,sizeof(port));

	if (strcmp(ipaddr,_conn.clientIP()) == 0)
		return true;  //FXP is not being used

	retval = _site.isAllowedFXP(_conn.getLogin(),flagupload,_context.flagpasv);

	return retval;
}

