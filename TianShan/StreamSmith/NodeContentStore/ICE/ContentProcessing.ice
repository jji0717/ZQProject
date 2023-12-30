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
// Ident : $Id: ContentProcessing.ICE $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------


#ifndef __ZQ_TianShanIce_Storage_ContentProcessing_ICE__
#define __ZQ_TianShanIce_Storage_ContentProcessing_ICE__

#include "TianShanIce.ICE"
#include "TsStorage.ICE"
#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

module TianShanIce
{

// -----------------------------
// namespace Storage
// -----------------------------
/// Storage represents a media content storage within the TianShan architecture
module Storage
{

module ContentProcess
{
	/// CPNode Type
	const string cptRTFCPNode = "RTFCPNode";  ///<RTFCPNode
	const string cptVstrmCPNode = "VstrmCPNode"; ///<VstrmCPNode
	const string cptRTICPNode = "RTICPNode"; ///<RTICPNode

	/// node setting
	const string ptyRegister		  = "RegisterInterval";    ///<The interval between reports fom nodes, uint32
	const string ptyFtpBitrate        = "FtpTransferBitrate";  ///<Ftp speed limit uint32

	/// node info
	const string niRunningOnNode      = "RunningOnNode";       ///<Running On Node
	
	/// ContentProcess NOde information 
	struct NodeInfo
	{
		string	                type;            ///<the ContentProcess Node type
		string	                serverName;      ///<the ServerName which ContentProcess Node running on
		string	                netId;           ///<the unique id to specify the ContentProcess Node
		string                  endpoint;        ///<endpoint of the ContentProcess Node
		int                     maxSessionNum;   ///<the max concurrent session number
		int                     maxBitrate;      ///<the max bitrate to process
		int                     curSessionNum;	 ///<current session number
		int                     curBitrate;      ///<current bitrate used
		TianShanIce::Properties additionalInfo;  ///<additional information of the node
	};
		
	/// The booked Session information
	struct SessionInfo
	{
		string                   destinationContentType;  ///<destination content type
		string                   contentName;       ///<the content name of the booked session, the unique key of a session
		string                   sourceURL;         ///<source file URL for the provision, if there is
		int                      startTime;         ///<the start time of this booked session
		int                      endTime;           ///<the end time of this booked session
		int                      bitrate;           ///<the bitrate of processing the booked session in bps
		TianShanIce::Properties  additionalInfo;    ///<addtional information besides above, in map mode
	};
	
	sequence<SessionInfo> SessionSet;	

	/// CPManager is responsible for 
	/// 1. Managing all the CPNode instance in type category
	/// 2. CPNode resource management and session book.
	/// 3. Scheduling the content processing request
	/// 4. When schedule time is coming in, initial the request to the booked CPNode
	interface CPManager
	{
		/// ContentProcess Node register instance to ContentProcess Manager, it is also as the heartbeat between Node and Manager
		/// @param[in] type             - the type of the Node
		/// @param[in] serverName       - the ServerName which the node running on. 
		/// @param[in] netId            - unique id to identify this Node 
		/// @param[in] ninfo            - the basic information of the node, focusing on its resource capacity
		/// @param[out] nodeSetting     - general setting returned from ContentProcess Manager of this type Node
		/// @param[out] bookedSessions  - booked session list, this is for node cache, all these sessions are still driving from ContentProcess Manager
		void registerInstance(string type, 
		                    string serverName, 
		                    string netId, 
		                    NodeInfo ninfo, 
		                    out TianShanIce::Properties nodeSetting, 
		                    out SessionSet bookedSessions);
		
		/// ContentProcess Node query the session 
		/// @param[in] contentName      - content name of the session
		/// @param[out] session         - session information if the session is valid
		/// @return false means not found, true for session found. 
		bool querySession(string contentName, out SessionInfo session); 
		                    
		/// ContentProcess Node report the started event 
		/// @param[in] contentName      - content name of the started session
		/// @param[in] properties       - updated properties of the content, only provide avaliable the properties. 
		void sessionStarted(string contentName, TianShanIce::Properties properties);
		
		/// ContentProcess Node report the streamable event
		///	@param[in] contentName		- content name of the streamable session
		void sessionStreamable(string contentName);

		/// ContentProcess Node report the completed event 
		/// @param[in] contentName      - content name of the completed session
		/// @param[in] retCode          - Node content processing result, succeed or fail. > 0 means succeed, others means failure. 
		/// @param[in] errDesc          - description of retCode, if fail, it is the detail reason.
		/// @param[in] properties       - updated properties of the content. 
		void sessionCompleted(string contentName, int retCode, string retDesc, TianShanIce::Properties properties);
		
		/// ContentProcess Node report the progress event 
		/// @param[in] contentName      - content name of the processing session
		/// @param[in] curSize          - current processed bytes, 64 bit.
		/// @param[in] totalSize        - total size of the content, if it is unknown, set it to be 0. 
		void sessionProgress(string contentName, long curSize, long totalSize);
		
		/// ContentProcess Node update content properties during content processing before completion. 
		/// @param[in] contentName      - content name of the session
		/// @param[in] properties       - updated properties of the content. 
		void updateProperties(string contentName, TianShanIce::Properties properties);
		
	};

	/// CPNode is responsible for content processing.
	interface CPNode
	{
		/// Drive ContentProcess Node to process the specified session. 
		/// @param[in] session          - session information for the processing
		void process(SessionInfo session);
		
		/// Cancel the processing content. 
		/// @param[in] contentName      - content name of the session
		/// @return false means specified session has completed or does not exist, 
		///  true means session found, after cancel completion, Node will report sessionCompleted()
		bool cancel(string contentName);

		/// list the exported files
		string getExportedFileList(string contentName, out Ice::StringSeq fileList);

		/// clear the cache files
		/// @param contentName - content name of the session
		///  if contentName is Null, then clear all cache files
		void clearCache(string contentName);

		/// get the states of current node, for example	current active session
		/// @return		- the dictionary of state
		TianShanIce::Properties getNodeStates();
		
		/// get the information of the session at current node
		/// @return - the sequence of session info
		SessionSet getSessionInfo();
	};
	
	enum NodeSessionStatus
	{
		ssCREATED,
		ssSTARTED,
		ssPROPERTY,
		ssSTREAMABLE,
		ssPROGRESS,
		ssCOMPLETED
	};
	
	struct NodeSessionStatusMsg
	{
		string                    sessName;
		NodeSessionStatus         sessStatus;
		int                       sessRetCode;
		string                    sessRetDesc;
		TianShanIce::Properties   sessProperty;
		long                      sessProcessed;
		long                      sessTotal;
	};

};
};
};
#endif // __ZQ_TianShanIce_Storage_ContentProcessing_ICE__
