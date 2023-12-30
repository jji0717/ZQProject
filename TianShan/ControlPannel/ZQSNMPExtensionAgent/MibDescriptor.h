#ifndef __MIBDESCRIPTOR_H
#define __MIBDESCRIPTOR_H
#pragma once

#include <snmp.h>   
#include <wchar.h>
#include <tchar.h>
#include <stdio.h>

enum SME_STATUS
{
	SME_SUCCESS,
	SME_FAILED,
	SME_OID_STRING_SYNTAX_ERROR,
	SME_OID_CORRUPT,
	SME_SOCKET_ERROR,
	SME_NO_DATA_RETURNED
};


#define TopLevelVariables			1
// Isrm stuff
#define IsrmSessionTable			2
#define SIZEOFOID( Oid )      ( sizeof ( Oid ) / sizeof( UINT ) )


/////////////////////////////////////////////////////////////////////////////////////////////
//
// MIB actions. These are the three SNMP operations that may
// be performed on the managed objects in a MIB.

#define MIB_ACTION_GET				ASN_RFC1157_GETREQUEST
#define MIB_ACTION_SET				ASN_RFC1157_SETREQUEST
#define MIB_ACTION_GETNEXT			ASN_RFC1157_GETNEXTREQUEST
 

// MIB Variable access privileges
#define MIB_ACCESS_READONLY         0
#define MIB_ACCESS_WRITEONLY        1
#define MIB_ACCESS_READWRITE        2
#define MIB_ACCESS_NOTACCESSIBLE    3

#define VAR_TYPELEN                 33
#define DISPLAY_STRING				256

//-----------------------------------------------------------------------------
//  SnmpVariable
//
//	This object is inherited from SnmpVarBind (touch SnmpVarBind and hit  
//  f12 now). The purposeof the object is to add some member function 
//  to make it convenient to manipulate.

class SnmpVariable : public	SnmpVarBind
{
public:
	SnmpVariable();
	~SnmpVariable();

	// OID methods
	SME_STATUS			ClearOID();
	SME_STATUS			SetOID(AsnObjectIdentifier* InputOID);
	SME_STATUS			SetOID(UINT Length, UINT* Ids);
	SME_STATUS			SetOID(TCHAR* InputOID);

	AsnObjectIdentifier* 
						GetOID();
	int					CompareOID(AsnObjectIdentifier* InputOID);
	int					CompareOID(AsnObjectIdentifier* InputOID, int MaxLen);

	// Value Sets and Gets
	SME_STATUS			SetValue(const WCHAR*	InputValue);
	SME_STATUS			SetValue(const char*	InputValue);
	SME_STATUS			SetValue(AsnInteger32	InputValue);
	SME_STATUS			SetValue(AsnUnsigned32	InputValue);
	SME_STATUS			SetValue(AsnCounter64	InputValue);
	SME_STATUS			SetValue(AsnOctetString InputValue);

		
	SME_STATUS			GetValue(AsnInteger32*	OutputValue);
	SME_STATUS			GetValue(AsnUnsigned32* OutputValue);
	SME_STATUS			GetValue(AsnCounter64*	OutputValue);
	SME_STATUS			GetValue(AsnOctetString**
												OutputValue);
	
	// Type sets and gets
	SME_STATUS			SetType(BYTE InputValue);
	BYTE				GetType();
};


//-----------------------------------------------------------------------------
//  MibDescriptor
//
//	This object is inherited from SnmpVariable (touch SnmpVariable and hit f12 now). 
//  The purpose of the object is to be the object that forms the table of 
//	descriptors that maps Mibs to OID's to ManUtil paths. This is the 
//  heart of the dll.
//  .
class MibDescriptor : public SnmpVariable
{
public:
	MibDescriptor();
	~MibDescriptor();

	// TBD:  add other accessor functions as needed.
	AsnObjectSyntax		TempVar;			// temp storage variable
	bool				Modified;			// TRUE if variable as been modified 
	bool				m_CacheIsGood;		// for caching, if false, need to retrieve from service
	bool				m_ValueIsGood;		// we attempted to reach the service and did not get it
	bool				m_IsTable;
	bool				m_IsTableVal;
	bool				m_IsTableCol;
	

	// vars for refreshing the value
	UINT				RetrievalCount;
	time_t				LastModifiedTime;
	DWORD				RefreshInterval;

	signed int          MinVal;				// Minimum possible value 
	DWORD               MaxVal;				// Maximum possible value 
	UINT               (*pMibFunc)( BYTE, class MibDescriptor*, RFC1157VarBind* );

	TCHAR              * pszStorageName;   //变量的描述符
	INT                 chType;            //变量的类型
	UINT                Access;				// Access specifier
	class MibDescriptor* pMibNext;			// Pointer to next entry in the MibVars list
};
#endif


