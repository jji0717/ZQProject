// $Header: /ZQProjs/TianShan/StreamSmith/NodeContentStore/VstrmTlvTypes.h 1     10-11-12 16:07 Admin $
//
// Copyright (c) 2010 SeaChange Technology
//
// Module Name:
//
//    	VstrmTlvTypes.h
//
// Abstract:
//
//		Vstrm TLV Definitions
//
// Author:
//
//		Bill Stetson
//
// Environment:
//
//		User/Kernel
//
// Revision History:
//

#ifndef _VstrmTlvTypes_h
#define _VstrmTlvTypes_h

// ***************************************************
// Notes
// ***************************************************
//
// Tags are defined with a "VS prefix rather then a "VSTRM" prefix since VSIS
// has internally defined it's tags using the "VSTRM" prefix.causing build errors 
//

// ***************************************************
// Tag data types
// ***************************************************
#define VS_TAG_DATA_TYPE_MASK			0xF000
#define VS_TAG_DATA_TYPE_SHIFT			12

typedef enum _VS_TAG_DATA_TYPE
{
	VS_TAG_DATA_TYPE_UNKNOWN			= 0x0000,
	VS_TAG_DATA_TYPE_BLOCK				= 0x1000,
	VS_TAG_DATA_TYPE_SIGNED_INT			= 0x2000,
	VS_TAG_DATA_TYPE_UNSIGNED_INT		= 0x3000,
	VS_TAG_DATA_TYPE_BOOLEAN			= 0x4000,
	VS_TAG_DATA_TYPE_TEXT				= 0x5000,
	VS_TAG_DATA_TYPE_BINARY_DATA		= 0x6000,
} VS_TAG_DATA_TYPE;

// ***************************************************
// Tag block identification
// ***************************************************
#define VS_TAG_BLOCK_ID_MASK			0x0F00
#define VS_TAG_BLOCK_ID_SHIFT			8

typedef enum _VS_TAG_BLOCK_ID
{
	VS_TAG_BLOCK_ID_UNKNOWN				= 0x0000,
	VS_TAG_BLOCK_ID_VOD_EVENT			= 0x0100,
	VS_TAG_BLOCK_ID_EDGE_EVENT			= 0x0200,
	VS_TAG_BLOCK_ID_FSI_EVENT			= 0x0300,
	VS_TAG_BLOCK_ID_VSIS_EVENT			= 0x0400,
} VS_TAG_BLOCK_ID;

// ***************************************************
// Tag value codes
// ***************************************************
#define VS_TAG_VALUE_CODE_MASK			0x00FF
#define VS_TAG_VALUE_CODE_SHIFT			0

// ***************************************************
// Tags
// ***************************************************
typedef UINT16 	VS_TAG;		// unsigned 16 bit tag identifying value
typedef UINT16 	VS_LENGTH;	// unsigned 16 bit length of value

#define DEF_VS_TAG(blockId, dataType, valueCode) 	((VS_TAG)(blockId | dataType | valueCode ))

// VOD Event Block Tag Definitions
#define	VS_TAG_VOD_EVENT_BLOCK					DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_BLOCK,			0xFF)
#define	VS_TAG_VOD_EVENT_TYPE					DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x01)
#define	VS_TAG_VOD_EVENT_PORT_HANDLE			DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x02)
#define	VS_TAG_VOD_EVENT_PORT_SEQUENCE_NUMBER	DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x03)
#define	VS_TAG_VOD_EVENT_SESSION_ID				DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x04)
#define	VS_TAG_VOD_EVENT_SPEED_NUMERATOR		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_SIGNED_INT,	0x05)
#define	VS_TAG_VOD_EVENT_SPEED_DENOMINATOR		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x06)
#define	VS_TAG_VOD_EVENT_NPT					DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x07)

// VOD (new)
#define	VS_TAG_VOD_EVENT_ASSET_NAME				DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_TEXT,			0x08)
#define	VS_TAG_VOD_EVENT_SESSION_DURATION		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x09)
#define	VS_TAG_VOD_EVENT_COUNT_TO_NORMAL		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0A)
#define	VS_TAG_VOD_EVENT_COUNT_TO_FAST_FORWARD	DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0B)
#define	VS_TAG_VOD_EVENT_COUNT_TO_FAST_REVERSE	DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0C)
#define	VS_TAG_VOD_EVENT_COUNT_TO_PAUSE			DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0D)
#define	VS_TAG_VOD_EVENT_BYTES_NORMAL			DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0E)
#define	VS_TAG_VOD_EVENT_BYTES_FAST_FORWARD		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x0F)
#define	VS_TAG_VOD_EVENT_BYTES_FAST_REVERSE		DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x10)
#define	VS_TAG_VOD_EVENT_BYTES_PAUSE			DEF_VS_TAG (VS_TAG_BLOCK_ID_VOD_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x11)

// Edge Event Block Tag Definitions
#define	VS_TAG_EDGE_EVENT_BLOCK					DEF_VS_TAG (VS_TAG_BLOCK_ID_EDGE_EVENT,	VS_TAG_DATA_TYPE_BLOCK,			0xFF)
#define	VS_TAG_EDGE_EVENT_TYPE					DEF_VS_TAG (VS_TAG_BLOCK_ID_EDGE_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x01)
#define	VS_TAG_EDGE_EVENT_PORT_HANDLE			DEF_VS_TAG (VS_TAG_BLOCK_ID_EDGE_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x02)
#define	VS_TAG_EDGE_EVENT_PORT_SEQUENCE_NUMBER	DEF_VS_TAG (VS_TAG_BLOCK_ID_EDGE_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x03)

// FSI Event Block Tag Definitions
#define	VS_TAG_FSI_EVENT_BLOCK					DEF_VS_TAG (VS_TAG_BLOCK_ID_FSI_EVENT,	VS_TAG_DATA_TYPE_BLOCK,			0xFF)
#define	VS_TAG_FSI_EVENT_TYPE					DEF_VS_TAG (VS_TAG_BLOCK_ID_FSI_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x01)

// VSIS Event Block Tag Definitions
#define	VS_TAG_VSIS_EVENT_BLOCK					DEF_VS_TAG (VS_TAG_BLOCK_ID_VSIS_EVENT,	VS_TAG_DATA_TYPE_BLOCK,			0xFF)
#define	VS_TAG_VSIS_EVENT_TYPE					DEF_VS_TAG (VS_TAG_BLOCK_ID_VSIS_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x01)
#define	VS_TAG_VSIS_ASSET_NAME					DEF_VS_TAG (VS_TAG_BLOCK_ID_VSIS_EVENT,	VS_TAG_DATA_TYPE_TEXT,			0x02)
#define	VS_TAG_VSIS_ASSET_PWE					DEF_VS_TAG (VS_TAG_BLOCK_ID_VSIS_EVENT,	VS_TAG_DATA_TYPE_UNSIGNED_INT,	0x03)


// ***************************************************
// Generic value union
// ***************************************************
#pragma pack(push, tlv_types, 1)

typedef union _VS_VALUE {
	UINT8		vUInt8;			//  8 bit unsigned integer
	UINT16		vUInt16;		// 16 bit unsigned integer
	UINT32		vUInt32;		// 32 bit unsigned integer
	UINT64		vUInt64;		// 64 bit unsigned integer

	INT8		vInt8;			//  8 bit signed integer
	INT16		vInt16;			// 16 bit signed integer
	INT32		vInt32;			// 32 bit signed integer
	INT64		vInt64;			// 32 bit signed integer

	CHAR		vText[1];			// zero terminated ASCII text 
	UINT8		vBinary[1];		// binary data

	UINT8		vBoolean;		// Boolean (8 bit)
} VS_VALUE, *PVS_VALUE;

// ***************************************************
// TLV structure
// ***************************************************
typedef struct _VS_TLV {
	VS_TAG			tag;			// tag identifying the value
	VS_LENGTH		length;			// length of value
	VS_VALUE		value;			// various generic values (see above)
} VS_TLV, *PVS_TLV;

#pragma pack(pop, tlv_types)

// ***************************************************
// Macros for building and tearing down TLVs
// ***************************************************

//
// VsSetTlvTag			Set tag in TLV
// VsSetTlvLength		Set value length in TLV
// VsSetTlvValue		Set value in TLV
//
#define VsSetTlvTag(pVsTlv, vsTag)				(((PVS_TLV)pVsTlv)->tag = vsTag)
#define VsSetTlvLength(pVsTlv, vsLen)			(((PVS_TLV)pVsTlv)->length = vsLen)
#define VsSetTlvValue(pVsTlv, pVsVal, vsLen)	(RtlCopyMemory(&((PVS_TLV)pVsTlv)->value, pVsVal, vsLen))

//
// VsGetTlvTag			Get tag in TLV
// VsGetTlvLength		Get value length in TLV
// VsGetTlvValue		Get value in TLV
//
#define VsGetTlvTag(pVsTlv)						(((PVS_TLV)pVsTlv)->tag)
#define VsGetTlvLength(pVsTlv)					(((PVS_TLV)pVsTlv)->length)
#define VsGetTlvValue(pVsTlv, pVsVal, vsLen)	(RtlCopyMemory(pVsVal, &((PVS_TLV)pVsTlv)->value, vsLen))


/****************************************************************************\
*
*   VsLocateNextTlv		Locate next TLV
*
\****************************************************************************/
PVS_TLV static __inline VsLocateNextTlv(PVS_TLV pVsTlv, PUINT8 bufEnd)
{
	UINT32		tlvLen;
	PUINT8		bufAddr;

	// Calculate length of current TLV
	tlvLen = (UINT32)  (sizeof(pVsTlv->tag)		+		// tag		size
						sizeof(pVsTlv->length)	+		// length	size
						pVsTlv->length);				// value	size

	// Set buffer pointer beyond current TLV
	bufAddr = (PUINT8) pVsTlv + tlvLen;

	// Reach end of buffer?
	if (bufAddr >= bufEnd)
		bufAddr = NULL;

	// Return next TLV pointer
	return (PVS_TLV) bufAddr;
}

/****************************************************************************\
*
*   ROUTINE: VsLocateTlvInBlock
*
* Retrieve a pointer to the nth instance of a TLV with the specified tag
*
* Parameters
*	Pointer to a buffer with parent TLV defining a TLV block
*	The tag of the TLV to search for.
*	The instance of the TLV to return.  (0 is the first)
*
* Returns:	Pointer to the TLV you were looking for, or NULL if one could 
*			not be found.
\****************************************************************************/
PVS_TLV static __inline VsLocateTlvInBlock (	PVS_TLV		pBlockTlv,		// Info block TLV
												UINT32		tag,			// tag to search for
												UINT32		instance)		// instance of tag
{
	PVS_TLV		pTargetTlv	= NULL;
	PVS_TLV		pTlv		= NULL;
	PUINT8		bufAddr		= NULL;
	PUINT8		bufEnd		= NULL;
	UINT32		nth			= instance;

	// Sanity
	if (pBlockTlv)
	{
		// Get the block TLV parameters 
		bufAddr	= (PUINT8) &pBlockTlv->value;
		bufEnd	= bufAddr + pBlockTlv->length;

		// Locate first TLV within the block
		pTlv = (PVS_TLV) bufAddr;

		// Have a TLV?
		while (pTlv)
		{
			// Correct tag?
			if (pTlv->tag == tag)
			{
				if (nth == 0)			// correct instance?
				{
					pTargetTlv = pTlv;	// yes, set target TLV addr
					break;				// found it, done
				}

				nth--;					// decrement instance
			}

			// Locate next TLV
			pTlv = VsLocateNextTlv(pTlv, bufEnd);
		}
	}

	return pTargetTlv;
}

/****************************************************************************\
*
*   VsGetTlvInteger		Retrieve an integer value from a TLV
*
* Returns:	An unsigned 64 bit integer to be cast to target
\****************************************************************************/
UINT64 static __inline VsGetTlvInteger(PVS_TLV pVsTlv)
{
	PVS_VALUE		pValue;
	UINT64			uInt64Value	= 0;
	INT64			 int64Value	= 0;

	// Sanity
	if (pVsTlv)
	{
		// Locate the value
		pValue = &pVsTlv->value;

		// Is it a signed integer?
		if ((pVsTlv->tag & VS_TAG_DATA_TYPE_MASK) == VS_TAG_DATA_TYPE_SIGNED_INT)
		{
			// Fetch signed integer value based on length
			switch (pVsTlv->length)
			{
			case sizeof(INT8):		int64Value = (INT64) pValue->vInt8;		break;
			case sizeof(INT16):		int64Value = (INT64) pValue->vInt16;	break;
			case sizeof(INT32):		int64Value = (INT64) pValue->vInt32;	break;
			case sizeof(INT64):		int64Value = (INT64) pValue->vInt64;	break;
			}

			// Cast to an unsigned 64 bit integer for return
			uInt64Value = (UINT64) int64Value;
		}
		else
		{
			// Fetch unsigned integer value based on length
			switch (pVsTlv->length)
			{
			case sizeof(UINT8):		uInt64Value = (UINT64) pValue->vUInt8;	break;
			case sizeof(UINT16):	uInt64Value = (UINT64) pValue->vUInt16;	break;
			case sizeof(UINT32):	uInt64Value = (UINT64) pValue->vUInt32;	break;
			case sizeof(UINT64):	uInt64Value = (UINT64) pValue->vUInt64;	break;
			}
		}
	}

	return uInt64Value;
}

/****************************************************************************\
*
*   VsAddTlv		Add a TLV to the supplied buffer
*
\****************************************************************************/
PVS_TLV static __inline VsAddTlv (PVS_TLV		pVsTlv, 
								  PUINT8		bufEnd, 
								  VS_TAG		vsTag, 
								  VS_LENGTH		vsLen, 
								  PVOID			pVsVal)
{
	UINT32		tlvLen;
	PUINT8		tlvEnd;

	// Sanity
	if (pVsTlv)
	{
		// Calculate length of proposed TLV
		tlvLen = (UINT32)  (sizeof(pVsTlv->tag)		+		// tag		size
							sizeof(pVsTlv->length)	+		// length	size
							vsLen);							// value	size

		// Locate end of proposed TLV
		tlvEnd = (PUINT8) pVsTlv + tlvLen;

		// Will this TLV fit in the buffer?
		if (tlvEnd < bufEnd)
		{
			// Set the TLV
			VsSetTlvTag		(pVsTlv, vsTag);
			VsSetTlvLength	(pVsTlv, vsLen);
			VsSetTlvValue	(pVsTlv, pVsVal, vsLen);

			// Get location of next TLV
			pVsTlv = (PVS_TLV) ((PUINT8)pVsTlv + tlvLen);
		}
		else
		{
			// TLV won't fit
			pVsTlv = NULL;
		}
	}

	return pVsTlv;
}	


#endif
