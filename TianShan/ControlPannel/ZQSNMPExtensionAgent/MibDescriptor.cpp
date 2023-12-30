#include "stdafx.h"
#include "MibDescriptor.h"

//-----------------------------------------------------------------------------
//  SnmpVariable
//
//	This object is inherited from SnmpVarBind (touch SnmpVarBind and hit  
//  f12 now). The purposeof the object is to add some member function 
//  to make it convenient to manipulate.

SnmpVariable::SnmpVariable()
{
	name.idLength = 0;
	name.ids = NULL;
	value.asnValue.string.dynamic = 1;
	value.asnValue.string.length = 0;
	value.asnValue.string.stream = 0;
}

SnmpVariable::~SnmpVariable()
{
	ClearOID();
}


//-----------------------------------------------------------------------------
//  SnmpVariable::ClearOID()
//
//	This method clears the OID member. It takes care of allocated memory.
//	It is assumed that the integer array was allocated.

SME_STATUS SnmpVariable::ClearOID()
{
	name.idLength = 0;

	if (NULL != name.ids)
	{
		delete[] name.ids;
	}
	
	name.ids = NULL;

	return (SME_SUCCESS);
}


//-----------------------------------------------------------------------------
//  SnmpVariable::SetOID()
//
//	This method sets the OID member. It allocates memory for the integer array.
//	It is assumed that the integer array was allocated.

SME_STATUS SnmpVariable::SetOID(AsnObjectIdentifier* InputOID)
{
	ClearOID();

	if ((0 == InputOID->idLength) &&
		(NULL == InputOID->ids))
	{
		return (SME_SUCCESS);
	}

	if (0 == InputOID->idLength)
	{
		// This is an "eyebrow raise", we are pointing
		// to someplace with 0 length.
		// This is permissive in that the above paragraph above checks
		// for null InputOid.

		return (SME_SUCCESS);
	}

	name.idLength = InputOID->idLength;
	name.ids = new (UINT[name.idLength]);
	for (unsigned long i=0; i < name.idLength; i++)
	{
		name.ids[i]	= InputOID->ids[i];
	}

	return (SME_SUCCESS);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::SetOID()
//
//	This method sets the OID member. It allocates memory for the integer array.
//	It is assumed that the integer array was allocated. Here the inputs are the
//  ingredients for a OID, not a whole OID.  This is a convenience function.
SME_STATUS SnmpVariable::SetOID(UINT Length, UINT* Ids)
{
	AsnObjectIdentifier	TempOID;

	TempOID.idLength = Length;
	TempOID.ids = Ids;
	return (SetOID(&TempOID));
}
//-----------------------------------------------------------------------------
//  SnmpVariable::SetOID()
//
//	This method sets the OID member. It allocates memory for the integer array.
//	It is assumed that the integer array was allocated. Here the input is a
//  character string. This is convenient when reading in an OID from a 
//  configuration file.

SME_STATUS SnmpVariable::SetOID(TCHAR* InputOID)
{
	ClearOID();

	if (NULL == InputOID)
	{
		name.idLength = 0;
		name.ids = NULL;
		return (SME_SUCCESS);
	}

	TCHAR*				pPointIntoOID;

	pPointIntoOID = InputOID;

	int					NumberOfDigits = 0;
	bool				FoundAnyChars = false;
	while (*pPointIntoOID)
	{
		if ((_T('.') != *pPointIntoOID)&&
			(!FoundAnyChars))
		{
			FoundAnyChars = true;
			NumberOfDigits++;
		}

		if (_T('.') == *pPointIntoOID)
		{
			if (!FoundAnyChars)
			{
				return (SME_OID_STRING_SYNTAX_ERROR);
			}
			FoundAnyChars = false;
		}
		pPointIntoOID++;
	}

	UINT*				TempIds;
	TempIds = new (UINT[NumberOfDigits]);
	int					ProcessedDigits = 0;
	TCHAR seps[] = _T(".");

	pPointIntoOID = _tcstok(InputOID, seps);

	// now fill up the array 
	int					FieldsProcessed;
	while (pPointIntoOID)
	{
		if (_T('.') != *pPointIntoOID)
		{
			FieldsProcessed = _stscanf(pPointIntoOID, _T("%d"), &TempIds[ProcessedDigits++]);
	        if (1 != FieldsProcessed)
			{
				delete [] TempIds;
				return (SME_OID_STRING_SYNTAX_ERROR);
			}
		}
		pPointIntoOID = _tcstok(NULL, seps);
	}

	if (NumberOfDigits != ProcessedDigits)
	{
		delete [] TempIds;
		return (SME_OID_STRING_SYNTAX_ERROR);
	}

	name.ids = TempIds;
	name.idLength = ProcessedDigits;

	return (SME_SUCCESS);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::GetOID()
//
//	This method returns a pointer to the internally store OID. 

AsnObjectIdentifier* SnmpVariable::GetOID()
{
	return (&name);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::CompareOID()
//
//	This method compares the argument OID to this OID. 
int SnmpVariable::CompareOID(AsnObjectIdentifier* InputOID)
{
	int result =
		SnmpUtilOidCmp(InputOID, &name);
	return (result);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::CompareOID()
//
//	This method compares the argument OID to this OID. Additionally, it compares
//  a specified part of the OID, not necessarily the whole thing.
int SnmpVariable::CompareOID(AsnObjectIdentifier* InputOID, int MaxLen)
{
	int result =
		SnmpUtilOidNCmp(InputOID, &name, MaxLen);
	return (result);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::SetValue()
//
//	This collection of set overloads store a value in an SnmpVariable. The final 
//	storage for all of these is a union. This variable is mimicking the value
//	in a SnmpVarBind, however, there are so many types that are the same, we
//  are going to support a select only the unique ones on the Set collection of
//	overloads. This is fine since ManUtil data is almost all character anyway.
//	
//	The Get collection of overloads can convert the unique types to the desired
//	types to fulfill an SNMP variable request. The purpose of this approach is
//	to simply reduce the number of set members we have to create

SME_STATUS SnmpVariable::SetValue(const WCHAR* InputValue)
{
	switch (value.asnType)
	{

//	case(ASN_INTEGER):				// this is the same number as ASN_INTEGER32
	case(ASN_INTEGER32):
		{
			int FieldsProcessed = swscanf(InputValue, L"%d", &value.asnValue.number);
			return (SME_SUCCESS);
		}

	case(ASN_UNSIGNED32):
		{
			int FieldsProcessed = swscanf(InputValue, L"%u", &value.asnValue.unsigned32);
			return (SME_SUCCESS);
		}

	case(ASN_COUNTER64):
		{
			break;
		}

	case(ASN_OCTETSTRING):
	case(ASN_OBJECTIDENTIFIER):
		{
			char			Temp[1024];
			int ctranslated = WideCharToMultiByte (CP_ACP, 0,
					  InputValue, -1, Temp, sizeof(Temp),
					  NULL, NULL);
			return (SetValue(&Temp[0]));
		}
	case(ASN_BITS):
		{
			char			Temp[1024];
			int ctranslated = WideCharToMultiByte (CP_ACP, 0,
					  InputValue, -1, Temp, sizeof(Temp),
					  NULL, NULL);
			return (SetValue(&Temp[0]));
		}
	default:
		{
			break;
		}
		break;
	};

	return (SME_SUCCESS);
}

SME_STATUS SnmpVariable::SetValue(const char* InputValue)
{
	int			StringLength;
	switch (value.asnType)
	{

//	case(ASN_INTEGER):
	case(ASN_INTEGER32):
		{
			int FieldsProcessed = sscanf(InputValue, "%d", &value.asnValue.number);
			return (SME_SUCCESS);
		}

	case(ASN_UNSIGNED32):
		{
			int FieldsProcessed = sscanf(InputValue, "%u", &value.asnValue.unsigned32);
			return (SME_SUCCESS);
		}

	case(ASN_COUNTER64):
		{
			break;
		}

	case(ASN_OCTETSTRING):
	case(ASN_OBJECTIDENTIFIER):
		StringLength = strlen(InputValue);
//		if ((NULL != value.asnValue.string.stream) &&
//			(value.asnValue.string.dynamic))
		if ((NULL != value.asnValue.string.stream))
		{
			delete[] value.asnValue.string.stream;
			value.asnValue.string.stream = 0;
			value.asnValue.string.length = 0;
		}
		if (StringLength > 0)
		{
			value.asnValue.string.stream = new (BYTE[StringLength]);
			value.asnValue.string.length = StringLength;
			value.asnValue.string.dynamic = false;
			memcpy(value.asnValue.string.stream, 
					InputValue,
					StringLength);
		}
		
		break;
	case (ASN_BITS):
		StringLength = strlen(InputValue);
//		if ((NULL != value.asnValue.bits.stream) &&
//			(value.asnValue.bits.dynamic))
		if ((NULL != value.asnValue.bits.stream))
		{
			delete[] value.asnValue.bits.stream;
			value.asnValue.bits.stream = 0;
			value.asnValue.bits.length = 0;
		}
		if (StringLength > 0)
		{
			value.asnValue.bits.stream = new (BYTE[StringLength]);
			value.asnValue.bits.length = StringLength;
			value.asnValue.bits.dynamic = false;
			memcpy(value.asnValue.bits.stream, 
					InputValue,
					StringLength);
		}
		break;

	default:
		{

		}

	};

	return (SME_SUCCESS);
}


SME_STATUS SnmpVariable::SetValue(AsnInteger32 InputValue)
{
	switch (value.asnType)
	{

	//case(ASN_INTEGER):
	case(ASN_INTEGER32):
		{
			value.asnValue.number = InputValue;
			return (SME_SUCCESS);
		}

	case(ASN_UNSIGNED32):
		{
			value.asnValue.unsigned32 = InputValue;
			return (SME_SUCCESS);
			
		}

	case(ASN_COUNTER64):
		{

		}

	case(ASN_OBJECTIDENTIFIER):
		{

		}

	default:
		{

		}

	};

	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::SetValue(AsnCounter64 InputValue)
{

	value.asnValue.counter64 = InputValue;
	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::SetValue(AsnOctetString InputValue)
{
	int			StringLength;
	switch (value.asnType)
	{
	case ASN_OCTETSTRING:
		StringLength = InputValue.length;
//		if ((NULL != value.asnValue.string.stream) &&
//			(value.asnValue.string.dynamic))
		if ((NULL != value.asnValue.string.stream))
		{
			delete[] value.asnValue.string.stream;
			value.asnValue.string.stream = 0;
		}
		value.asnValue.string.stream = new (BYTE[StringLength]);

		value.asnValue.string.length = StringLength;
		value.asnValue.string.dynamic = false;
		memcpy(value.asnValue.string.stream, 
			   InputValue.stream,
			   StringLength);
		//	value.asnValue.string.stream[StringLength]='\0'; // add by dony for delete the duoyu character
		break;
	case ASN_BITS:
		StringLength = InputValue.length;
//		if ((NULL != value.asnValue.bits.stream) &&
//			(value.asnValue.bits.dynamic))
		if ((NULL != value.asnValue.bits.stream))
		{
			delete[] value.asnValue.bits.stream;
			value.asnValue.bits.stream = 0;
		}
		value.asnValue.bits.stream = new (BYTE[StringLength]);

		value.asnValue.bits.length = StringLength;
		value.asnValue.bits.dynamic = false;
		memcpy(value.asnValue.bits.stream, 
			   InputValue.stream,
			   StringLength);
		//	value.asnValue.bits.stream[StringLength]='\0'; // add by dony for delete the duoyu character
		break;
	}
	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::SetValue(AsnUnsigned32 InputValue)
{
	value.asnValue.unsigned32 = InputValue;
	return (SME_SUCCESS);
	//return (SetValue((AsnInteger32)InputValue));
}
//-----------------------------------------------------------------------------
//  SnmpVariable::GetValue()
//
//	The Get collection of overloads can convert the few unique types to the desired
//	types to fulfill an SNMP variable request. 
//
SME_STATUS	SnmpVariable::GetValue(AsnInteger32* OutputValue)
{
	*OutputValue = value.asnValue.number;
	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::GetValue(AsnUnsigned32* OutputValue)
{
	*OutputValue = value.asnValue.unsigned32;
	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::GetValue(AsnCounter64*	OutputValue)
{
	*OutputValue = value.asnValue.counter64;
	return (SME_SUCCESS);
}

SME_STATUS	SnmpVariable::GetValue(AsnOctetString** OutputValue)
{
	switch (value.asnType)
	{
	case ASN_OCTETSTRING:
		*OutputValue = &value.asnValue.string;
		break;
	case ASN_BITS:
		*OutputValue = &value.asnValue.bits;
		break;
	}
	return (SME_SUCCESS);
}

//-----------------------------------------------------------------------------
//  SnmpVariable::SetType 
//
//	The set the internal type of a variable. You only want to do this once for 
//	the life of the variable. 
//
SME_STATUS SnmpVariable::SetType(BYTE InputValue)
{
	value.asnType = InputValue;
	return (SME_SUCCESS);
}
//-----------------------------------------------------------------------------
//  SnmpVariable::GetType
//
//	The set the internal type of a variable. You only want to do this once for 
//	the life of the variable. 
//
BYTE SnmpVariable::GetType()
{
	return (value.asnType);
}
//-----------------------------------------------------------------------------
//  MibDescriptor
//
//	This object is inherited from MibVars (touch MibVars and hit f12 now). 
//  The purpose of the object is to be the object that forms the table of 
//	descriptors that maps Mibs to OID's to ManUtil paths. This is the 
//  heart of the dll.
//  .
MibDescriptor::MibDescriptor()
{
	m_CacheIsGood = false;
	m_ValueIsGood = false;
	m_IsTable = false;
	m_IsTableVal = false;
	m_IsTableCol = false;
	pMibNext = NULL;
	Modified = FALSE;
	MinVal = 0;
	MaxVal = 0xffffffff;
	Access = MIB_ACCESS_READONLY;
	RetrievalCount = 0;
	LastModifiedTime = 0;
	TempVar.asnValue.string.dynamic = 1;
	TempVar.asnValue.string.length = 0;
	TempVar.asnValue.string.stream = 0;
}
//
// Destructor
//

MibDescriptor::~MibDescriptor()
{
	
}
//
// Clear the m_ManPkgPath variable
//


