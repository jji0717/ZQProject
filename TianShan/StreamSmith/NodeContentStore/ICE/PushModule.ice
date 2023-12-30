// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: PushModule.ICE $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamSmith/NodeContentStore/ICE/PushModule.ice $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 1     08-04-28 12:26 Hongquan.zhang
// do not share with \TianShan\ContentStore\ice\
// any longer
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice



#ifndef _PUSHCONTENTMANAGER_
#define _PUSHCONTENTMANAGER_

module TianShanIce
{
module Storage
{

module PushContentModule {

	struct RDSInfo
	{
		string	strFtpUrl;			///<ftp root url
		int		maxSessionNum;		///<max push session number
		int		maxBitrate;			///<max bitrate can process
		int		curSessionNum;		///<current session number
		int		curBitrate;			///<current bitrate used
	};

	/// The settings get from PushManager
	struct PushSetting
	{
		int  leaseTerm;				///<RDS heartbeat lease term
		short bAllowAnonymous;		///<RDS if allow anonymous
		short bSkipAuthentication;	///<RDS if skip authentication
		int  timeWindow;			///<time window for ticket time checking
		int	  enableMultiTrickSpeed;	///<whether to enalbe multi-trick speed, 1 for enable, 0 for disable
	};

	/// The sessions that have been booked on this RDS server
	struct BookedSession
	{
		string	filename;			///<filename in mediacluster node, also the session identify
		int	startTime;				///<ticket start time, in UTC
		int	endTime;				///<ticket end time, in UTC
		int	bitrate;				///<ticket bitrate, in bps
	};
	sequence<BookedSession> BookedSessionSet;

	/// Describe the session progress
	struct SessionProgress
	{
		string			filename;	///<filename in mediacluster node, also the session identify
		long			size;		///<proceed bytes, 64bit
	};
	sequence<SessionProgress> SessionProgressSet;

	/// SubscriberInfo
	struct SubscriberInfo
	{
		string		subPropagationIp;
	};
	sequence<SubscriberInfo> SubscriberInfoSet;

	/// Interface for PushManager and RDS communication
	interface seacPushManager
	{
		/// Get session information
		/// @param[in] filename - the filename is uploading
		/// @param[out] ticketStart - ticket start time 
		/// @param[out] ticketEnd - ticket end time 
		/// @param[out] ticketBitrate         - ticket bitrate
		/// @return false means not found, true for success
		bool attachSession(string filename, out int ticketStart, out int  ticketEnd, out int ticketBitrate);

		/// Report session stop
		/// @param[in] filename - the filename is uploading
		/// @param[in] processedSize - proceed bytes, 64bit
		/// @param[in] statusCode         - status code specify the stop status, success or failed
		/// @param[in] statusDesc      - status description, if failed, it's the failure reason
		/// @return false means session not found, true for success
		bool sessionStop(string filename, long processedSize, int statusCode, string statusDesc);

		/// Report session started
		/// @param[in] filename - the filename is uploading
		/// @param[in] fileBitrate - the actual file bitRate
		/// @param[in] mcpBitrate - the mcp require bitRate
		/// @param[in] videoBitrate - the video bitrate of the file
		/// @param[in] videoH         - the video h
		/// @param[in] videoV      - the video v
		/// @param[in] frameRate       - the frame rate of file
		/// @param[out] mcpDestPort       - the mcp dest port
		/// @param[out] mcpMucastIp       - the mcp multicast ip
		/// @param[out] subIpSet       - the mcp subscribers ip list(propagation ip list)
		/// @return 0 for success, 1 for error "invalid session", 2 for error "no subscriber available"
		
		/// @return false means session not found, true for success
		int sessionStarted(string filename, 
			int fileBitrate,
			int mcpBitrate,
			int videoBitrate,
			int videoH,
			int videoV,
			out int mcpDestPort,
			out string mcpMucastIp,
			out SubscriberInfoSet subIpSet
			);

		/// Report session's progress
		/// @param[in] sp - session progress list
		void sessionProgress(SessionProgressSet sp);		


		/// RDS report instance to PushManager
		/// @param[in] nodeId - the id of a node, string to identify a node
		/// @param[in] rInfo - the information of RDS
		/// @param[out] psetting         - setting returned from PushManager
		/// @param[out] sessions      - booked session list
		void reportInstance( string            nodeId,
							 RDSInfo			rInfo,
							out PushSetting	  psetting,
							out BookedSessionSet	sessions);		

	};
};
};
};

#endif