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
// Ident : $Id: ContentDict.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamSmith/NodeContentStore/ICE/ContentDict.ice $
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
// 1     08-04-28 12:20 Hongquan.zhang
// Do not share with \TianShan\ContentStore\Ice
// any longer
// 
// 9     08-03-12 10:42 Fei.huang
// supportFileSize added
// 
// 9     08-03-11 16:10 Fei.huang
// add supportFileSize attribute
// 
// 8     07-09-04 13:58 Fei.huang
// 1.7 first merge
// 
// 7     07-06-06 16:17 Fei.huang
// 
// 6     07-06-04 13:45 Ken.qian
// change extAttrs type to be TianShan::Properties
// 
// 5     07-04-19 16:04 Fei.huang
// 
// 4     07-04-09 11:04 Fei.huang
// 
// 3     06-09-25 16:46 Ken.qian
// ===========================================================================
// build steps:
//	$(ICE_ROOT)/bin/slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice 
//	$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice --dict Contents,string,ContentDictData::ContentAttrs Data $(InputName).ice 

#ifndef __ZQ_ContentStore_ICE__
#define __ZQ_ContentStore_ICE__

#include <Ice/Identity.ice>
#include "TianShanIce.ice"

module ContentDictData
{

// data structure used inside of Spot
// ----------------------------------


enum ProvStatus
{
	stCREATED,
	stPROCESSING,
	stPROVISIONED,
	stDELETEABLE
};

// attributes about a content in the store
struct ContentAttrs
{
   	string name;

	// time stamps, of GMT

	long stampCreate;		// when the content is created by calling ContentStore::openContent(create=true);
	long stampProvision;	// when the content file is actually be provisioned into the store
	long stampLastAccess;	// when the content has been newly accessed

	string sourceUrl;		// where the content was provisioned from
	string parentName;		// asset if necessary 
	string comments;

	long fileSize;          // file size in bytes
	int  resolutionH;		// pixel resolution
	int  resolutionV;
	int  bitrate;			// in bit-per-second
	long playtime;			// in sec
	long  supportFileSize;    // in bytes
	float fps;				// in frame-per-second

	string   sourceType;       // the content source type
	string   localType;        // the content local type
	string   subType;

	ProvStatus   status;    // the status

	TianShanIce::Properties extAttrs; // some other store specific attributes may defined here
};

};

#endif // __ZQ_ContentStore_ICE__
