// ========================================================================
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
// Ident : $Id:  $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp general headers
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspHeaders.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 7     05-03-24 14:51 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 6     05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 5     04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 4     10/14/04 11:23a Jie.zhang
// 
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#ifndef _RTSPHEADERS_H_
#define _RTSPHEADERS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#include <string>
#include <map>
#include <vector>

// rtsp message category
#define		RTSP_UNKNOWN_MSG	0
#define		RTSP_REQUEST_MSG	1
#define		RTSP_RESPONSE_MSG	2

// rtsp default values
#define		RTSP_DEFAULT_PORT	554		// default RTSP server port: 554
#define		RTSP_DEFAULT_NSEC	20000	// default timeout: 20 seconds
#define		RTSP_DEFAULT_HB		300		// default heartbeat timeout: 300 seconds
#define		RTSP_DEFAULT_HB_FAC	500		// default heartbeat timeout factor: timeout*FAC/1000
#define		RTSP_DEFAULT_HBFAC	500		// default heartbeat timeout factor: 0.5*timeout
#define		RTSP_DEFAULT_FREQ	1000	// default select() frequency: 1 second

#define		MAX_SELECTTIME			1000

#define		MAX_BUFFER_SIZE		4096
#define		MAX_ERROR_LENGTH	256
#define		MAX_RECOVER_TIMES	5

#define		RTSP_VALID_TYPE		6

// heartbeat setting
#define		RTSP_HB_MSG			("\x00D\x00A")
#define		RTSP_HB_MSG_LEN		2

// request string
const std::string	RTSP_REQUEST_TYPE[RTSP_VALID_TYPE]= { "SETUP","PLAY","PAUSE","TEARDOWN","GET_PARAMETER","ANNOUNCE" };

// error code
#define	RTSP_SOCKET_OPEN_ERROR  "RTSP socket open failed :"
#define	RTSP_SENDING_ERROR      "RTSP sending data failed :"
#define	RTSP_RECVING_ERROR      "RTSP receiving data failed :"

// characters definition
#define CH_CR		 (0x0D)	// carriage return
#define CH_LF		 (0x0A)	// line feed

// string definition
#define KEY_ENTER  ("\n")
#define KEY_CRLF   ("\x00D\x00A")

//SetupMessage 
#define KEY_CSEQ  "CSeq"
#define KEY_SEACHANGEVERSION  "SeaChange-Version"
#define KEY_TRANSPORT  "Transport"
#define KEY_SEACHANGEMAYNOTIFY  "SeaChange-MayNotify"
#define KEY_SEACHANGEENTITLEMENT  "SeaChange-Entitlement"
#define KEY_SESSION  "Session"
#define KEY_SEACHANGESERVERDATA  "SeaChange-Server-Data"
#define KEY_SEACHANGEMODDATA  "SeaChange-Mod-Data"
#define KEY_SEACHANGEFORCEDNP  "SeaChange-ForceDNP"
#define KEY_SEACHANGETRANSPORT  "SeaChange-Transport"
#define KEY_SEACHANGECAS  "SeaChange-CAS"
#define KEY_RTSPVERSION  "RTSP/"

//PlayMessage 
#define KEY_RANGE  "Range"
#define KEY_RANGE_NPT  "npt"
#define KEY_SCALE  "Scale"
#define KEY_XPLAYNOW  "x-playNow"

//TeardownMessage
#define KEY_SEACHANGEREASON  "SeaChange-Reason"
#define KEY_XREASON  "x-reason"

//AnnounceRequest 
#define KEY_SEACHANGENOTICE  "SeaChange-Notice"

//GetParameterMessage 
#define KEY_CONTENTTYPE  "Content-Type"
#define KEY_CONTENTLENGTH  "Content-Length"
#define KEY_PARAM_PRESENTATIONSTATE  "presentation_state"
#define KEY_PARAM_POSITION  "Position"
#define KEY_PARAM_SCALE  "Scale"

//Transport
#define KEY_UNICAST  "unicast"
#define KEY_MULTICAST  "multicast"
#define KEY_DESTINATION  "destination"
#define KEY_CLIENTPORT  "client_port"
#define KEY_CLIENTMAC  "client_mac"

//Session
#define KEY_TIMEOUT  "timeout"

//SeaChangeServerData 
#define KEY_NODEGROUPID  "node-group-id"
#define KEY_PURCHASEID  "purchase-id"
#define KEY_SMARTCARDID  "smartcard-id"
#define KEY_TYPEINST  "typeinst"
#define KEY_DEVICEID  "device-id"

//SeaChangeTransport
#define KEY_TRANSPORTSTREAMID  "transport-stream-id"
#define KEY_PROGRAMNUMBER  "program-number"
#define KEY_FREQUENCY  "frequency"
#define KEY_QAMMODE  "qam-mode"
#define KEY_VIRTUALCHANNEL  "virtual-channel"

//SeaChangNotice
#define KEY_EVENTDATE  "event-date"

//SeaChangeModeData
#define KEY_BILLINGID  "billing-id"
#define KEY_PURCHASETIME  "purchase-time"
#define KEY_TIMEREMAINING  "time-remaining"
#define KEY_HOMEID  "home-id"

//SeaChangeForceDNP
#define KEY_PIPEID  "pipe-id"
#define KEY_SLICEID  "slice-id"
#define KEY_SLICEGROUPID  "slice-group-id"
#define KEY_CLUSTERID  "cluster-id"

//SeaChangeCAS
#define KEY_EMM  "emm"
#define KEY_EXPIRES  "expires"

#endif // _RTSPHEADERS_H_