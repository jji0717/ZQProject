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
// Ident : $Id: MetaLibCmds.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibCmds.h $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_MetaLibCmds_H__
#define __ZQTianShan_MetaLibCmds_H__

#include "../common/TianShanDefines.h"

#include "MetaLibImpl.h"

namespace ZQTianShan {
namespace MetaLib {

// -----------------------------
// class ProvisionCmd
// -----------------------------
///
class BaseCmd : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
    BaseCmd(MetaLibImpl& lib);
	virtual ~BaseCmd();

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) { return 0; }
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	MetaLibImpl&     _lib;
};

// -----------------------------
// class LookupCmd
// -----------------------------
///
class LookupCmd : public BaseCmd
{
public:

	/// constructor
    LookupCmd(MetaLibImpl& lib, const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr& amdCB, const ::std::string& type, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames);

protected: // overwrite of ProvisionCmd

	virtual int run(void);

protected:

	::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr _amdCB;
	::Ice::Identity _dummyIdentObj;
	::TianShanIce::Properties _searchForMetaData;
	::TianShanIce::StrValues _expectedMetaDataNames;
	IdentCollection _objsFound;
};

}} // namespace

#endif // __ZQTianShan_MetaLibCmds_H__

