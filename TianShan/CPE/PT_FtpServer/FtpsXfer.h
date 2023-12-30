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
// Ident : $Id: FtpsXfer.h,v 1.6 2004/08/12 09:07:34 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Deal with the file transfer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpsXfer.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-12-19 17:03 Yixin.tian
// 
// 2     08-11-18 11:11 Jie.zhang
// merge from TianShan1.8
// 
// 2     08-09-17 14:01 Xia.chen
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 3     07-12-21 16:52 Fei.huang
// minor fixes, remove warning of size mismatch in copy expression while
// compiling
// 
// 2     07-12-19 15:38 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 3     07-04-12 19:18 Jie.zhang
// 
// 2     07-03-22 18:36 Jie.zhang
// 
// 3     05-04-11 19:03 Jie.zhang
// 
// 2     04-12-08 15:50 Jie.zhang
// Revision 1.6  2004/08/12 09:07:34  jshen
// Remove Ended Thread List
//
// Revision 1.5  2004/07/23 09:04:36  jshen
// For QA test
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:18:06  jshen
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
#ifndef __FTPSXFER_H__
#define __FTPSXFER_H__

#include "FtpConnection.h"

#define FTPSXFER_MAXDIRENTRY   256  //max length of a line in a dir listing
#define FTPSXFER_MAXPACKETSIZE 4096 //max size of a sending packet (4KB)

#define FTPSXFER_XFERRATEUPDTRATE .5   //transfer rate update rate (in seconds)

#include "NativeThreadPool.h"
#include "FtpSock.h"

#include <string>

class FtpSite;    //dummy declaration (needed to avoid compiler errors)
class EndThreadHandleList;

///////////////////////////////////////////////////////////////////////////////
// Supplemental class used for directory listing/file transfer
///////////////////////////////////////////////////////////////////////////////
class FtpsXfer : public ThreadRequest
{
public:
	typedef struct
	{
		char command;       //L = dir list, R = receive data, S = send data
		int mode;           //0 = STOR, 1 = STOU, 2 = APPE (only applies to FTP STOR command)
		char options[16];   //command options (Ex. ls -la -> options = "la")
		char path[512];         //file or dir path for the transfer command
		char cwd[512];          //current working directory of the user)
		char userroot[512];     //user's root directory
		int flagpasv;       //0 = active mode, 1 = passive mode
		SOCKET pasvsd;      //socket desc for the passive data connection
		char portaddr[SOCK_IPADDRLEN];  //stores the PORT addr to use for data connect
		char portport[SOCK_PORTLEN];    //stores the PORT port to use for data connect
		char type;          //A = ASCII, I = Image/Binary
		int64 restoffset;    //offset to use for resuming transfers
		bool bencdata;    //if bencdata != 0, encrypt the data channel (set w/ PROT)
		long timemod;       //used to force the mod time of a STORed file (0 = let OS set the mod time)
	} context_t;

public:
	FtpsXfer(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, ZQ::common::NativeThreadPool& Pool, context_t* pContext=NULL);
	virtual ~FtpsXfer();

	// implementation of Thread
	virtual bool init(void);
	virtual int run(void);
	void final(int retcode =0, bool bCancelled =false);

	context_t* getContext() { return &_context; }
	static bool copyContext(const context_t* from, context_t* to);

	virtual bool sendListCtrl();
	virtual bool sendList();
	virtual bool recvFile();
	virtual bool sendFile();
	void setMaxDLBitrate(int bitrate){_maxDlSpeed = bitrate/8;}

protected:
	void sendListData(size_t& nbytes);
	long recvFileData(int fdw);
	long sendFileData(int fdr);
	void sendListData(context_t& ctxt,size_t& nbytes);
	size_t addToSendBuffer(char* sendbuf, const size_t maxsendbuf, size_t& sendbufoffset, const char *dataptr, const size_t datasize, const bool bencdata, const bool flagforcesend= false);
	bool buildListLine(std::string& line, const char *fullpath, const char *filename, const int flagdir, const char *thisyear);
	char *BtoA(char lastchar, char *inbuf, int inbufsize, int *outbufsize);
	bool AtoU(char *buf, int bufsize, int& changedLen);
	void updateXferRate(long bytessent);
	int sendData(char *buffer, int bufsize);
	int recvData(char *buffer, int bufsize);
	int getUniqueExtNum(char *filepath);
	int checkFXP(int flagupload);

protected:
	FtpConnection& _conn;           //FTP connection
	FtpSite& _site;   //site enviroment class (needed for sync and xfer rate functions)
public:	
	context_t _context;
protected:
	FtpSock* _pasv_sock;
	FtpSock* _port_sock;
	SOCKET _dataSd;            //socket desc for the data connection

	long _maxUlSpeed;  //max upload speed (bytes/sec)
	long _maxDlSpeed;  //max download speed (bytes/sec)
	long _bytesXfered; //num bytes xfered in the current sending cycle
	unsigned long _xferTimer;   //used for transfer rate timing

	unsigned long _lastXferRateUpdt;   //last time the transfer rate was updated
	double _bytesSinceRateUpdt;        //num bytes xfered since the last xfer rate update
};

#endif //__FTPSXFER_H__
