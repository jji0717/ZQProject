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
// Ident : $Id: DODContentStore.ice$
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : 
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/ice/DODContentStore.ice $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 3     09-03-05 10:46 Li.huang
// 
// 2     08-12-09 17:24 Li.huang
// 
// 1     08-12-08 11:10 Li.huang
// 
// 7     07-11-26 16:23 Ken.qian
// 
// 6     07-11-21 11:23 Ken.qian
// 
// 5     07-04-23 15:37 Ken.qian
// change tag from string type to int type
// 
// 4     07-04-10 12:34 Ken.qian
// 
// 2     07-03-14 15:55 Ken.qian
// 
// 1     07-02-28 16:41 Ken.qian
// 


#ifndef __ZQ_DataOnDemand_DODContentStore_ICE__
#define __ZQ_DataOnDemand_DODContentStore_ICE__

#include "TsStorage.ICE"

module DataOnDemand {

// the parameters that DODContentStore requred to complete a provision.
struct DataWrappingParam
{
	int         esPID;            // the PID of the element stream, it is specified by ChannelId
	int         streamType;       // the stream type of specified channel
	int         subStreamCount;   // the sub-stream count, sub-stream¡¯s PID is esPID+index[0, streamCount)
	int         dataType;         // the channel type, such as navigation, EPG, portal, message, etc
	int         withObjIndex;     // Whether add the object index info at the front of  objects TS data, it is specified by 
								  // SendWithDestination in the channel configuration
	int         objTag;           // object Tag, such as ¡®nav¡¯, ¡®pic¡¯, etc
	int         encryptType;      // the encryption type, 0 ¨C no encryption.
	int         versionNumber;    // version of the wrapped data, from 0 to 31
};

["freeze:write"]
class DODContent extends TianShanIce::Storage::Content
{
	/// set the data wrapping parameters before invoking provision.
	void setDataWrappingParam(DataWrappingParam param);
	string name;
	
};

};

#endif // __ZQ_DataOnDemand_DODContentStore_ICE__
