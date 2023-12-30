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
// Ident : $Id: ZqSentryIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/ZqSentryIce.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 5     09-02-18 10:01 Xiaohui.chai
// change log parsing scheme
// 
// 4     08-06-06 12:06 Xiaohui.chai
// added getWebView() to SentryService
// 
// 3     07-05-22 17:30 Hui.shao
// added exporting logger information
// 
// 2     07-05-21 11:52 Hui.shao
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice  

#ifndef __ZQ_Node_ICE__
#define __ZQ_Node_ICE__

#include "TianShanIce.ICE"

#include <Ice/Identity.ice>

module ZqSentryIce
{

struct LoggerInfo
{
    string logFile; // the full path of the log file
    string logType; // type of the log file. keep empty for default.
    string syntaxFile; // the full path of the syntax definition file
    string syntaxKey; // the key of the syntax in the definition file
    ::TianShanIce::Properties ctx;  // the context info for the log
};

sequence <LoggerInfo> LoggerInfos;

// -----------------------------
// callbacks AdapterCB
// -----------------------------
/// callbacks of the ZQ-wrapped adapter. The SpotService may query the spot thru this callback
interface AdapterCB
{

	/// get active time
	///@return the active time stamp
	long getActiveTime();

	/// get endpoint
	///@return the endpoint to the adapter instance
	string getEndpoint();

	/// get adapterId
	///@return the unique string id of the adapter
	string getId();

	/// get the interface names that are serving thru the adapter
	///@return  a list of the interface names that are serving thru the adapter
	::TianShanIce::StrValues listInterfaces();

	/// list all the log files in this process
	///@return a list of LoggerInfo records
	LoggerInfos listLoggers();
};

// -----------------------------
// loopback interface AdapterCollector
// -----------------------------
/// the interface of NodeService bound on the loopback NIC for the local services
/// to publish their served interfaces.
interface AdapterCollector
{
	/// update an active adapter
	///@param processId the id of the OS process that the adapter belongs to
	///@param adapterId a unique string id to identify the adapter
	///@param identAdapterCB ident to access the adapterCB for further query
	///@return the timeout in sec for next update.
	int updateAdapter(int processId, string adapterId, long lastChange, Ice::Identity identAdapterCB);

	/// get the root admin URL.
	/// When the NodeService is the administration web server of the local service,
	/// each local service should host its admininstration pages under this root URL of NodeService
	string getRootUrl();
};

struct WebEntry
{
    string name;
    string URL;
};

sequence <WebEntry> WebEntries;

struct WebView
{
    WebEntry base;
    WebEntries services;
};

struct ServiceInfo
{
	string name;
	string proxystr;
	string adapterId;
	int    processId;
};

sequence <ServiceInfo> ServiceInfos;

// -----------------------------
// public interface SentryService
// -----------------------------
interface SentryService extends TianShanIce::BaseService
{
	/// get the group address. A number of NodeService work as a group, they 
	/// are using a common IP multicast messaging to notify the others in the group
	///@param[out] McastIp the multicast IP address for the group
	///@param[out] port the multicast port number for the group
	void getGroupAddress(out string McastIp, out int port);

	/// get the processor info
	///@param[out] processor a string description of the processor
	///@param[out] count	the count of processors on the machine
	///@param[out] frequencyMhz	the clock frequency of the processor
	void getProcessorInfo(out string processor, out int count, out long frequencyMhz);

	/// get the operating system info
	///@return a string descrption of the OS. i.e. "Windows NT Version 5.1"
	string getOSType();

	/// get the root admin URL.
	/// When the NodeService is the administration web server of the local services,
	/// each local service should host its admininstration pages under this root URL of NodeService
	string getRootUrl();

    /// get the machine's view of web
    /// @return the web entrances of the server and the TianShan services on the machine
    WebView getWebView();

	/// list the published services on the machine
	ServiceInfos listServices();

	/// stop a daemon by its process idservice
	///@param[in] pid the current process id of the daemon about to stop
	///@parma[in] restart true if this stop requires an immediate start follows
	///@return    0 if the daemon is stopped, otherwise, the new process id
	int stopDaemonByPid(int pid, bool restart);
};

};

#endif // __ZQ_Node_ICE__
