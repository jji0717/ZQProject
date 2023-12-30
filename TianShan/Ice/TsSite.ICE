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
// Ident : $Id: TianShanIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsSite.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 8     08-08-14 14:31 Hui.shao
// merge from 1.7.10
// 
// 8     08-07-07 15:41 Hui.shao
// 
// 7     08-03-18 11:23 Hui.shao
// 
// 6     08-02-19 15:13 Hui.shao
// 
// 5     07-12-27 19:41 Hui.shao
// added resource restriction for virtual site
// 
// 4     07-12-20 13:50 Hui.shao
// added throwing SRM::InvalidResource from resolvePurchase()
// 
// 3     07-12-10 18:46 Hui.shao
// 
// 2     07-03-23 14:43 Hui.shao
// 
// 13    9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 1     06-09-18 19:06 Hui.shao
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice  
// outputs:
//	$(InputName).h
//	$(InputName).cpp

#ifndef __ZQ_TianShanIce_Site_ICE__
#define __ZQ_TianShanIce_Site_ICE__

#include "../Ice/TsSRM.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

module Site
{

// -----------------------------
// exception OutOfQuota
// -----------------------------
/// exception thrown when the system is out of quota for the request
exception OutOfQuota extends ServerError
{
};

// -----------------------------
// Service BusinessRouter
// -----------------------------
/// represents a router of business application logic within TianShan
interface BusinessRouter extends TianShanIce::BaseService
{
	/// resolve a purchase in the business logic for a session
	///@param[in] sess	the session instance about to be resolved
	///@return    a purchase instance along with business logic associated, NULL if failed
	///@note	  the resource with type rtAssetUri in the session (plus the client servicegroup) will be
	///           used by the BusinessRouter to determin the business logic, other variables within the session
	///           may be accessed by the selected individual business application
	///@note	  Session::attachPurchase() will be invoked by BusinessRouter if purchase is resovled
	///@throw SRM::InvaildResource if failed to resolove a purchase due to the resource field of the session, especially rtURL
	///@throw OutOfQuota if the quota management refuse to auth a new purchase
	Application::Purchase* resolvePurchase(SRM::Session* sess)
	   throws 
	      NotSupported,
		  OutOfQuota,
		  ServerError,
		  SRM::InvalidResource;
};

// -----------------------------
// exception NoSuchTxn
// -----------------------------
/// exception thrown when the transaction does not exists in the given transaction database
exception NoSuchTxn extends ServerError
{
};

// -----------------------------
// struct TxnInfo
// -----------------------------
/// The transaction information exported from the TxnService
struct TxnInfo
{
	string sessId;
	TianShanIce::Properties params;
};

/// a collection of TxnInfo
sequence <TxnInfo> TxnInfos;

// -----------------------------
// Service TxnService
// -----------------------------
/// represents a router of business application logic within TianShan
interface TxnService extends TianShanIce::BaseService
{
	/// The transaction records are kept as a set of properties, system properties can only be updated thru
	/// commitStateChange(), their names start with a prefix "sys.", the following are some known properties:
	/// @li sys.vpath		   - the mapped application path
	/// @li sys.stampResolved  - the timestamp when the session has been resolved with application and purchase
	/// @li sys.stampServe     - the timestamp when the session becomes in-service and start serving
	/// @li sys.stampStopped   - the timestamp when the session becomes out-of-service
	/// @li sys.bandwidth      - the bandwidth usage in bps for the session, effective if the session this in-service
	/// @li sys.storageLink    - the GUID of the storage link that served this session
	/// @li sys.streamLink     - the GUID of the stream link that served this session
	/// @li sys.allocateCost   - the evaluated cost when allocation
	/// @li sys.ContentStore   - the net id of the content store that served this session
	/// @li sys.Streamer       - the net id of the streamer that served this session

	/// commit the usage when the session start serving
	///@param[in] state the new state that the transaction has been changed to
	///@param[in] sessId the session id for this transaction, it is used as a key to index transactions
	///@param[in] sess   access to the session object only for referennce, suggest not to access the object frequently
	///@param[in] props  the properties to update in the transaction record, system properties are only allowed to
	///                  update in this method
	///@throw NoSuchTxn  if the transaction with given sessId doesn't exist in the live transaction database
	["ami"] void commitStateChange(string sessId, TianShanIce::State state, SRM::Session* sess, TianShanIce::Properties props)
		throws NoSuchTxn, ServerError;

	/// record an error on the transaction
	///@param[in] sessId the session id for this transaction, it is used as a key to index transactions
	///@param[in] category the category name of this error
	///@param[in] errCode  the error code of the category
	///@param[in] errMsg   the string error message
	///@throw NoSuchTxn  if the transaction with given sessId doesn't exist in the live transaction database
	["ami", "amd"] void trace(string sessId, string category, string eventCode, string eventMsg)
		throws NoSuchTxn, ServerError;

	/// post the transaction into history database
	///@param[in] sessId the session id for this transaction, it is used as a key to index transactions
	///@throw NoSuchTxn  if the transaction with given sessId doesn't exist in the live transaction database
	["ami"] void postYTD(string sessId)
		throws NoSuchTxn, ServerError;

	/// set a user property on a live transaction
	///@param[in] sessId the session id for this transaction, it is used as a key to index transactions
	///@param[in] key    the keyname of the property, will be added with a "user." prefix if the given key doesn't have it
	///@param[in] value  the value of the property
	["ami"] void setUserProperty(string sessId, string key, string value);

	/// list the live transations by site or the application
	///@param[in] siteName specify the site name to list the transactions; empty or "*" will list all regardless virtual sites
	///@param[in] appMount specify the app mount to list the transactions; empty or "*" will list all regardless application
	///@param[in] paramNames specify the name of returning parameters of each transaction
	///@param[in] startId  the start session Id. all the session Id listed in the result will be no smaller than it. empty if 
	///                    wish to list from the beginning
	///@param[in] maxCount the max count of the txns in the list result
	///@return a collection of TxnInfo
	///@throw ServerError
	["cpp:const", "amd", "ami"]
	TxnInfos listLiveTxn(string siteName, string appMount, TianShanIce::StrValues paramNames, string startId, int maxCount)
		throws ServerError;

	/// dump a live transaction
	///@param[in] sessId      the session id for this transaction, it is used as a key to index transactions
	///@param[in] beginFormat the format of the begining part of transaction output string, all transaction
	///                       property names are allowed to format the output
	///@param[in] endFormat   the format of the begining part of transaction output string, keywords
	///                       property names are allowed to format the output 
	///@param[in] traceFormat the format of a trace string in the transaction output, all transaction
	///                       "$event.category", "$event.code", "$event.msg" "$event.stamp" are allowed in this format
	///@throw NoSuchTxn  if the transaction with given sessId doesn't exist in the live transaction database
	///@note in the format definition, the following additional keyword are allowed in any of the above format definitions:
	///@li "$sys.sessId"	  the session id
	///@li "$sys.siteName"	  the name of the virtual site
	///@li "$sys.appName"	  the the associated application name
	///@remark if you wish to dump a transaction as an XML doc like:
	///@code
	///    <txn sessId="dfasobDgab123" start="20061203T145555" end="20061203T150053">
	///		<event code="Session:10001" stamp="20061203T145555" >close to site bandwidth limitaion</event>
	///		<event code="ChannelOnDemand:30012" stamp="20061203T150055" >session terminated by administrator</event>
	///    </txn>
	///@endcode
	/// a possible invocation could like:
	///@code
	///    dumpLiveTxn("dfasobDgab123", "<txn sessId=\"$sys.sessId\" start=\"$sys.stampResolved\" end=\"$sys.stampStopped\">\n",
	///		            "\t<event code=\"$event.category:$event.code\">$event.msg</event>\n",
	///					"</txn>\n");
	///@endcode
	["amd"] string dumpLiveTxn(string sessId, string beginFormat, string traceFormat, string endFormat)
		throws NoSuchTxn, ServerError;

	/// set a user property on a live transaction
	///@param[in] sessId      the session id for this transaction, it is used as a key to index transactions
	///@param[in] beginFormat the format of the begining part of transaction output string, all transaction
	///                       property names are allowed to format the output
	///@param[in] endFormat   the format of the begining part of transaction output string, keywords
	///                       "$err.category", "$err.code", "$err.msg" are allowed in this format
	///@param[in] traceFormat the format of a trace string in the transaction output, all transaction
	///                       property names are allowed to format the output 
	///@throw NoSuchTxn  if the transaction with given sessId doesn't exist in the history transaction database
	///@sa dumpLiveTxn
	// ["amd"] string dumpHistoryTxn(string sessId, string beginFormat, string traceFormat, string endFormat)
	//	throws NoSuchTxn, ServerError;
};

};
};

#endif // __ZQ_TianShanIce_Site_ICE__
