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
// Ident : $Id: TsLayout.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/TsLayout.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 11    09-06-23 16:14 Fei.huang
// 
// 10    08-03-24 18:03 Xiaohui.chai
// 
// 9     07-10-31 16:47 Xiaohui.chai
// 
// 8     07-09-13 17:36 Xiaohui.chai
// 
// 7     07-07-23 13:20 Xiaohui.chai
// add community string to interface
// 
// 6     07-06-07 14:39 Jie.zhang
// add oid to struct
// 
// 5     07-06-06 16:06 Jie.zhang
// change interface
// 
// 5     07-06-05 16:52 Jie.zhang
// 
// 4     07-06-04 14:12 Hongquan.zhang
// 
// 3     07-05-29 11:19 Jie.zhang
// 
// 2     07-05-25 15:02 Hui.shao
// added comments
// 
// 1     07-05-25 13:45 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_TsLayout_H__
#define __ZQTianShan_TsLayout_H__


#define	SNMPATTR_VARVALUE_MAXLEN	512
#define	SNMPATTR_VARNAME_MAXLEN		256
#define	SNMPATTR_VARTYPE_INT		0
#define SNMPATTR_VARTYPE_STRING		1
#define SNMPATTR_VARRW_READONLY		1
#define SNMPATTR_VARRW_WRITABLE		0
#define SNMPATTR_VARVALUE_NOCHANGE	0
#define SNMPATTR_VARVALUE_CHANGED	1
#define SNMPATTR_VARNAMES_MAXLEN	128*1024		//buffer length for ALL variable names
#define SNMPATTR_OID_MAXLEN			64

/// namespace of ZQ's implmentation of TianShan Architecture
namespace ZQTianShan {
/// namespace of layout
namespace Layout {

// -----------------------------
// Interface ILayoutCtx
// -----------------------------
///@brief defines the method of a layout context, it is implemented in the layout module and exported to datasource module
class ILayoutCtx
{

public:
    virtual ~ILayoutCtx() {}

public:
	/// set the column names of a grid view. it is required before addRow() is called
	///@param[in] colnames a list of column names in NULL-terminated strings
	///@parma[in] colcount the count of columns
	///@return true if succeed. the layout module may reject the invocation if it is after addRow() calls
	virtual bool setColumns(const char* colnames[], const int colcount) =0;

	/// add a row to the grid view
	///@param[in] colvalues the column values, must in the order of the column definition in previous setColumns()
	///@parma[in] ref       the URI that this row refers to. The layout module may use it to build up the hyper reference. NULL if non-appliable
	///@return true if succeed. the layout module may reject the invocation if no setColumns() was called or the column count lead access illegal memory range
	virtual bool addRow(const char* colvalues[], const char* ref=0) =0;
	
#pragma pack(4)
	typedef struct _lvalue
	{
		char	value[SNMPATTR_VARVALUE_MAXLEN];
		char	oid[SNMPATTR_OID_MAXLEN];
		int		type;				
		int		readonly;			
		int		modify;	
	}lvalue;
#pragma pack()

	virtual void set(const char*key, const lvalue& val) =0;

	/// get a context attribute
	///@param[in] key		the key name of the attribute in NULL-terminated string
	///@param[out] val		the value returned
	///@return value		return true means success, false means the key is not found
	virtual bool get(const char* key, lvalue& val) =0;


	///reset the context attribute
	///@param[in] key the key of attribute which you want to clear
	///If key is NULL all attribute will be cleared
	virtual	void clear(const char* key) = 0;



	/// list all context attribute names
	///@param[in] buf buffer allocated in datasource module to receive the attribute names
	///@param[in] bufSize the size of buf
	///@param[out] attrnames pointer to the names of attributes, NULL if reaches the end
	///@return count of the names of the attributes
	virtual int list(char* buf, const int bufsize, char*** pattrnames) =0;
};

/// pointer to ILayoutCtx
typedef ILayoutCtx* PLayoutCtx;


/// module entry definition to initialize the dll library
/// @param[in] szLogFile the full path name of the log file
/// @return 0 for success, none-zero for error
typedef int (*EntryFunc_Initialize)(const char* szLogFile);


/// module entry definition to uninitialize the dll library
typedef void (*EntryFunc_Uninitialize)();


// -----------------------------
// Prototype EntryFunc_FillGrid
// -----------------------------
/// module entry definition to fill grid data, exported from datasource module, will be invoked by
/// the layout module when it tries to fill in data into its 2D grid view
/// The implementation in datasource may
/// @li query the necessary attributes from the context
/// @li set the column definition
/// @li add rows
/// @li set necessary attributes
/// @param[in] ctx pointer to the Layout context object in the layout module
/// @return count of the rows
typedef int (*EntryFunc_FillGrid) (PLayoutCtx ctx);


/// module entry definition to query snmp variables, exported from datasource module, will be invoked by
/// the layout module when it tries to populate the snmp variables of a services
/// The implementation in datasource may follow the sample below:
/// @param[in] ctx pointer to the Layout context object in the layout module
/// @param[in] baseOid the oid of the base SNMP node, required. in NULL-terminated string
/// @param[in] bPost   true if this populating is also a post. In a post method, the datasource is responsible to update SNMP variables
/// @param[in] snmpServer   specify a remote SNMP server-side in "<IP>:<Port>". NULL if the target SNMP server is local MS SNMP server
/// @return count of oid leaves
/// @note see the @ref EntryFunc_PopulateSnmpVariables_sample "sample"
typedef int (*EntryFunc_PopulateSnmpVariables)(PLayoutCtx ctx, const char* baseOid, bool bPost, const char* snmpServer, const char *community);


}} // namespaces

#endif  //__ZQTianShan_TsLayout_H__

