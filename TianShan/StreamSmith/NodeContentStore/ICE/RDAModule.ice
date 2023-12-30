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
// $Log: /ZQProjs/TianShan/StreamSmith/NodeContentStore/ICE/RDAModule.ice $
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
// 
// 1     07-09-18 16:56 Fei.huang
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

	struct FTPServerInfo
	{
		string	strFtpUrl;
		string	strFtpId;			//unique id for ftp server(or edge card)
		int		maxSessionNum;		//max push session number
		int		maxBitrate;			//max bitrate can process
		int		curSessionNum;
		int		curBitrate;
	};
	sequence <FTPServerInfo>	FtpServerInfoSet;

	struct RDASetting
	{
		int		leaseTerm;
		int		bAllowAnonymous;
		int		bSkipAuthentication;
		int		timeWindow;
	};

	struct RDABookedSession
	{
		string content;
		int	startTime;
		int	endTime;
		int	bitrate;
	};

	sequence<RDABookedSession> BookedSessionSet;

	struct RDASessionProgress
	{
		string content;
		long size;
	};

	sequence <RDASessionProgress> SessionProgressSet;

	interface RDAManager
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


		/// Called when ftp ingestion done and before sessionStop
		/// @param[in] filename - the filename is uploading
		/// @return false means session not found, true for success
		bool sessionResouceClear(string filename);

		/// Report session started
		/// @param[in] filename - the filename is uploading
		/// @param[in] fileBitrate - the actual file bitRate
		/// @return false means session not found, true for success
		bool sessionStarted(string filename, 
			int fileBitrate
			);

		/// Report session's progress
		/// @param[in] sp - session progress list
		void sessionProgress(SessionProgressSet sp);		

		/// RDS report instance to RDAManager
		/// @param[in] nodeId - the id of a node, string to identify a node
		/// @param[in] ftpSrvs - the information of Ftp servers
		/// @param[out] setting         - setting returned from RDAManager
		/// @param[out] sessions      - booked session list
		void reportInstance(  string            nodeId,
							  FtpServerInfoSet  ftpSrvs,
							out RDASetting		setting,
							out BookedSessionSet	sessions);		

	};

};
};
};

#endif