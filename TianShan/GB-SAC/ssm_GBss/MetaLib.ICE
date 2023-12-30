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
// $Log: /ZQProjs/TianShan/GBss/ssm_GBss/MetaLib.ICE $
// 
// 1     6/17/11 3:26p Haoyuan.lu
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 6     10-05-10 14:14 Haoyuan.lu
// 
// 5     10-04-12 16:44 Haoyuan.lu
// 
// 4     10-04-12 16:36 Hui.shao
// 
// 3     09-04-01 16:56 Hui.shao
// check-in works as of 4/1/2009
// 
// 2     08-03-18 15:48 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir .. $(InputDir)$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::Repository::ValueIdx,TianShanIce::Repository::LibMetaValue,value,case-insensitive" ValueIdx --output-dir .. $(InputDir)$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::Repository::TypeIdx,TianShanIce::Repository::LibMetaObject,type,case-sensitive" TypeIdx --output-dir .. $(InputDir)$(InputName).ice
// outputs:
//	../$(InputName).h
//	../$(InputName).cpp
//  ../ContentToProvision.cpp
//  ../ContentToProvision.h

#ifndef __ZQ_TianShanIce_MetaDataLib_ICE__
#define __ZQ_TianShanIce_MetaDataLib_ICE__

#include "TsRepository.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

module Repository
{

/// defines a meta object structure.
["freeze:write"] 
class LibMetaValue
{
	Ice::Identity identObj; ///< identity to find meta object. @sa LibMetaObject::ident

	string value;	///< copied from MetaDataValue definition, the value of a metadata will always be stored in string format
	ValueType hintedType;  ///< copied from MetaDataValue definition, this field indicate how to covert the value date

	["cpp:const", "freeze:read"] Ice::Identity getObjId();
	["cpp:const", "freeze:read"] MetaDataValue get();
	void set(MetaDataValue value);
};

/// defines a meta object structure.
["freeze:write"] 
class LibMetaObject
{
	Ice::Identity	ident; ///< ident.name = objectId and ident.name = objectType
	string			type;  ///< objectType, case sensitive
	long			stampCreated; ///< timestamp as of created
	long			timeout; ///< timeout of this object
	TianShanIce::Repository::MetaDataMap		metaDataMap; ///< store all meta data

	["cpp:const", "freeze:read"] idempotent Ice::Identity getObjId();
	["cpp:const", "freeze:read"] idempotent string getType();
	["cpp:const", "freeze:read"] idempotent string getCreatedTime(); ///< return the created timestamp in ISO8601 format

	["cpp:const", "freeze:read"] idempotent MetaDataValue get(string key);
	void set(string key, MetaDataValue value);

	["cpp:const", "freeze:read"] idempotent TianShanIce::Repository::MetaDataMap getMetaDataMap();
};

// -----------------------------
// interface MetaLib
// -----------------------------
/// the MetaDataLibraryEx extends MetaDataLibrary to cover extended options. This interface usually will not
/// be exported externally.
class MetaLib implements MetaDataLibrary
{
	void registerMetaClass(string type, MetaDataMap metaDataSchema)
	throws
		NotSupported,
		InvalidParameter;

	void setDefaultClassType(string type)
	throws
		NotSupported,
		InvalidParameter;

	string getDefaultClassType();

	void removeObject(string objId)
	throws
		EntityNotFound,
		NotSupported,
		InvalidParameter;

	void removeObjects(StrValues objIds)
	throws
		EntityNotFound,
		NotSupported,
		InvalidParameter;

	["ami"] void setMetaData(string objId, string name, MetaDataValue value, bool skipSearch)
	throws
		EntityNotFound,
		NotSupported,
		InvalidParameter,
		ServerError;

	["ami"] void setMetaDatas(string objId, MetaDataMap valueMap, bool skipSearch)
	throws
		EntityNotFound,
		NotSupported,
		InvalidParameter,
		ServerError;

};

};
};

#endif // __ZQ_TianShanIce_MetaDataLib_ICE__
