// ZQSnmpUtil.cpp: implementation of the ZQSnmpUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZQSnmpUtil.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////
//asn encode/decode utility class declaration
class ZQAsnUtil  
{
	ZQAsnUtil();
	~ZQAsnUtil();
public:
    static bool fail(int retval);
	static BYTE checksum(const void *stream, int len);
	static bool check(const void *stream, int len);

    typedef std::ostringstream BUFFER_T;
    static int encodeOID(const AsnObjectIdentifier *pValue, BUFFER_T *pBuffer);
    static int decodeOID(const void *stream, AsnObjectIdentifier *pValue);

    static int encodeAny(const AsnAny *pAnyValue, BUFFER_T *pBuffer);
    static int decodeAny(const void *stream, AsnAny *pAnyValue);

private:
    static int encodeINT32(const AsnInteger32 *pValue, BUFFER_T *pBuffer);
    static int decodeINT32(const void *stream, AsnInteger32 *pValue);

    static int encodeUINT32(const AsnUnsigned32 *pValue, BUFFER_T *pBuffer);
    static int decodeUINT32(const void *stream, AsnUnsigned32 *pValue);

    static int encodeOCTETS(const AsnOctetString *pValue, BUFFER_T *pBuffer);
    static int decodeOCTETS(const void *stream, AsnOctetString *pValue);

    static int encodeCounter64(const AsnCounter64 *pValue, BUFFER_T *pBuffer);
    static int decodeCounter64(const void *stream, AsnCounter64 *pValue);
};

//////////////////////////////////////////////////////////////////////////
//ZQSnmpUtil implementation
//
const UINT ZQSnmpUtil::m_ZQServiceOIDPrefix[ZQSNMP_OID_LEN_SVCPREFIX] = {1, 3, 6, 1, 4, 1, 22839, 4, 1};
//encode rule:
//checksum(BYTE)-pdutype(BYTE)-errstat(AsnIngeger32)-varbind(OID-AsnAny)
bool ZQSnmpUtil::UpdateMsg(void *stream, int len, AsnInteger32 errstat)
{
    int nValidMsgMin = sizeof(BYTE) + sizeof(BYTE) + sizeof(AsnInteger32);
    if(NULL == stream || len < nValidMsgMin)
        return false;

    if(!ZQAsnUtil::check(stream, len))
        return false;

    BYTE *data = (BYTE *)stream;

    memcpy(data + 2, &errstat, sizeof(AsnInteger32));
    data[0] = ZQAsnUtil::checksum(data + 1, len - 1);

    return true;
}
bool ZQSnmpUtil::EncodeMsg(BYTE pdutype, AsnInteger32 errstat, const SnmpVarBind *pVb, std::string *pMsg)
{
	if(NULL == pVb || NULL == pMsg)
		return false;

    ZQAsnUtil::BUFFER_T buffer;
	buffer.put(0);//checksum
	buffer.put(pdutype);
	buffer.write((const char*)(&errstat), sizeof(AsnInteger32));

    int len = ZQAsnUtil::encodeOID(&(pVb->name), &buffer);
	if(ZQAsnUtil::fail(len))
		return false;

    len = ZQAsnUtil::encodeAny(&(pVb->value), &buffer);
	if(ZQAsnUtil::fail(len))
		return false;

	*pMsg = buffer.str();
	//update checksum
    pMsg->at(0) = ZQAsnUtil::checksum(pMsg->data(), pMsg->size());
	return true;
}
bool ZQSnmpUtil::DecodeMsg(const void *stream, int len, BYTE *pPdutype, AsnInteger32 *pErrstat, SnmpVarBind *pVb)
{
	if(NULL == stream || len < 1 || NULL == pPdutype || NULL == pErrstat || NULL == pVb)
		return false;

	if(!ZQAsnUtil::check(stream, len))
		return false;

	BYTE *data = (BYTE *)stream;
	++data;//skip the checksum field
	--len;

	BYTE pdutype = *data++;
    if((--len) < 0)
        return false;

	AsnInteger32 errstat;
    memcpy(&errstat, data, sizeof(AsnInteger32));
    if((len -= sizeof(AsnInteger32)) < 0)
        return false;
	data += sizeof(AsnInteger32);

	AsnObjectName name;
    int retval = ZQAsnUtil::decodeOID(data, &name);
	if(ZQAsnUtil::fail(retval) || (len -= retval) < 0)
		return false;
	data += retval;

	AsnObjectSyntax value;
    retval = ZQAsnUtil::decodeAny(data, &value);
	if(ZQAsnUtil::fail(retval) || (len -= retval) < 0)
	{
		SnmpUtilOidFree(&name);//cleanup
		return false;
	}

	if(0 != len)
	{
		//cleanup
		SnmpUtilOidFree(&name);
		SnmpUtilAsnAnyFree(&value);
		return false;
	}

	*pPdutype = pdutype;
	*pErrstat = errstat;
	pVb->name = name;
	pVb->value = value;

	return true;
}

ZQSNMP_STATUS ZQSnmpUtil::AnyToMem(DWORD type, const AsnAny *pValue, void *address)
{
    if(NULL == pValue || NULL == address)
        return ZQSNMP_E_FAILURE;

    switch(type)
    {
    case ZQSNMP_VARTYPE_INT32:
        if(ASN_INTEGER32 == pValue->asnType)
            *((AsnInteger32 *)address) = pValue->asnValue.number;
        else
            return ZQSNMP_E_BADVALUE;
        break;
    case ZQSNMP_VARTYPE_UINT32:
        if(ASN_UNSIGNED32 == pValue->asnType)
            *((AsnUnsigned32 *)address) = pValue->asnValue.unsigned32;
        else
            return ZQSNMP_E_BADVALUE;
        break;
    case ZQSNMP_VARTYPE_STRING: //C-style string
        if(ASN_OCTETSTRING == pValue->asnType)
        {
            BYTE *data = pValue->asnValue.string.stream;
            UINT len = pValue->asnValue.string.length;
            if(len)
            {
                if(data)
                    memcpy(address, data, len);
                else
                    return ZQSNMP_E_BADVALUE;
            }
            *((BYTE *)address + len) = 0;
        }
        else
            return ZQSNMP_E_BADVALUE;
        break;
    default: //unsupported type
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS ZQSnmpUtil::MemToAny(DWORD type, const void *address, AsnAny *pValue)
{
    //DON'T change any field in pValue except return ZQSNMP_E_NOERROR
    if(NULL == pValue || NULL == address)
        return ZQSNMP_E_FAILURE;

    switch(type)
    {
    case ZQSNMP_VARTYPE_INT32:
        pValue->asnValue.number = *((const AsnInteger32 *)address); 
        pValue->asnType = ASN_INTEGER32;
        break;
    case ZQSNMP_VARTYPE_UINT32:
        pValue->asnValue.unsigned32 = *((const AsnUnsigned32 *)address); 
        pValue->asnType = ASN_UNSIGNED32;
        break;
    case ZQSNMP_VARTYPE_STRING:
        {
            UINT len = strlen((const char*)address);
            if(len) //len > 0
            {
                BYTE *data = (BYTE *)SnmpUtilMemAlloc(len);
                if(NULL == data)
                    return ZQSNMP_E_FAILURE;
                memcpy(data, address, len);
                pValue->asnValue.string.stream = data;
                pValue->asnValue.string.length = len;
                pValue->asnValue.string.dynamic = TRUE;
            }
            else //len == 0
            {
                pValue->asnValue.string.stream = NULL;
                pValue->asnValue.string.length = 0;
                pValue->asnValue.string.dynamic = FALSE;
            }
            pValue->asnType = ASN_OCTETSTRING;
        }
        break;
    default:
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}

bool ZQSnmpUtil::ParseOid(const AsnObjectIdentifier *pOid, UINT *pSvcInstanceID, UINT *pSvcProcess, UINT *pVarInfoType, UINT *pVarInstanceID)
{
    if(NULL == pOid)
        return false;

    if(pOid->idLength < ZQSNMP_OID_LEN_SVCPREFIX)
        return false;

    int cmpResult = memcmp(m_ZQServiceOIDPrefix, pOid->ids, sizeof(UINT) * ZQSNMP_OID_LEN_SVCPREFIX);
    if(0 != cmpResult)
        return false;

    if(pSvcInstanceID)
    {
        *pSvcInstanceID = 0;
        if(pOid->idLength >= ZQSNMP_OID_LEN_SVCINSTANCE)
        {
            *pSvcInstanceID = pOid->ids[ZQSNMP_OID_IDX_SVCINSTANCE];
        }
    }
    if(pSvcProcess)
    {
        *pSvcProcess = 0;
        if(pOid->idLength >= ZQSNMP_OID_LEN_SVCPROCESS)
        {
            *pSvcProcess = pOid->ids[ZQSNMP_OID_IDX_SVCPROCESS];
        }
    }
    if(pVarInfoType)
    {
        *pVarInfoType = 0;        
        if(pOid->idLength >= ZQSNMP_OID_LEN_VARINFOTYPE)
        {
            *pVarInfoType = pOid->ids[ZQSNMP_OID_IDX_VARINFOTYPE];
        }
    }
    if(pVarInstanceID)
    {
        *pVarInstanceID = 0;
        if(pOid->idLength >= ZQSNMP_OID_LEN_VARINSTANCE)
        {
            *pVarInstanceID = pOid->ids[ZQSNMP_OID_IDX_VARINSTANCE];
        }
    }

    return true;
}

bool ZQSnmpUtil::CreateOid(AsnObjectIdentifier *pOid, UINT svcInstId, UINT svcProcess, const UINT *pVarInfoType, const UINT *pVarInstId)
{
    if(NULL == pOid)
        return false;

    UINT oidData[ZQSNMP_OID_LEN_VARINSTANCE] = {0};
    memcpy(oidData, m_ZQServiceOIDPrefix, sizeof(UINT) * ZQSNMP_OID_LEN_SVCPREFIX);
    oidData[ZQSNMP_OID_IDX_SVCINSTANCE] = svcInstId;
    oidData[ZQSNMP_OID_IDX_SVCPROCESS] = svcProcess;
    UINT len = ZQSNMP_OID_LEN_SVCPROCESS;

    if(pVarInfoType)
    {
        oidData[ZQSNMP_OID_IDX_VARINFOTYPE] = *pVarInfoType;
        len = ZQSNMP_OID_LEN_VARINFOTYPE;
        if(pVarInstId)
        {
            oidData[ZQSNMP_OID_IDX_VARINSTANCE] = *pVarInstId;
            len = ZQSNMP_OID_LEN_VARINSTANCE;
        }
    }
    else if(pVarInstId)
    {
        //bad input
        return false;
    }

    AsnObjectIdentifier oid = {len, oidData};
    return SnmpUtilOidCpy(pOid, &oid) ? true : false;
}

std::string ZQSnmpUtil::GetPipeName(UINT svcId, UINT svcProcess)
{
    std::string pipename = ZQSNMP_NP_NAMEPREFIX;
    char buf[12] = {0};
    pipename += ultoa(svcId, buf, 10);
    pipename += ".";
    pipename += ultoa(svcProcess, buf, 10);

    return pipename;
}

std::string ZQSnmpUtil::GetPduType(BYTE pdutype)
{
    switch(pdutype)
    {
    case SNMP_PDU_GET:
        return "GetRequest-PDU";
    case SNMP_PDU_GETNEXT:
        return "GetNextRequest-PDU";
    case SNMP_PDU_SET:
        return "SetRequest-PDU";
    case SNMP_PDU_RESPONSE:
        return "GetResponse-PDU";
    default:
        return "Unsupported PDU";
    }
}
std::string ZQSnmpUtil::GetErrorStatus(AsnInteger32 errstat)
{
    switch(errstat)
    {
    case SNMP_ERRORSTATUS_NOERROR:
        return "noError";
    case SNMP_ERRORSTATUS_TOOBIG:
        return "tooBig";
    case SNMP_ERRORSTATUS_NOSUCHNAME:
        return "noSuchName";
    case SNMP_ERRORSTATUS_BADVALUE:
        return "badValue";
    case SNMP_ERRORSTATUS_READONLY:
        return "readOnly";
    case SNMP_ERRORSTATUS_GENERR:
        return "genErr";
    default:
        return "unknown error status";
    }
}

std::string ZQSnmpUtil::GetVariableType(DWORD type)
{
    switch(type)
    {
    case ZQSNMP_VARTYPE_INT32:
        return "Integer32";
    case ZQSNMP_VARTYPE_STRING:
        return "String";
    case ZQSNMP_VARTYPE_UINT32:
        return "Unsigned32";
    default:
        return "Unknown Type";
    }
}
//////////////////////////////////////////////////////////////////////////
//asn encode/decode utility class implementation
//////////////////////////////////////////////////////////////////////
//encode rule
//Type(BYTE)-Length(AsnUnsigned32)-Value
#define ASNENCODE_LEN_TYPE      1   //sizeof(BYTE)
#define ASNENCODE_LEN_LENGTH    4   //sizeof(AsnUnsigned32)
#define ASNENCODE_E_FAILURE     -1
//////////////////////////////////////////////////////////////////////


bool ZQAsnUtil::fail(int retval)
{
    return (retval < (int)ASNENCODE_LEN_LENGTH);
}

BYTE ZQAsnUtil::checksum(const void *stream, int len)
{
	if(NULL == stream || len < 0)
		return 0;

	BYTE *data = (BYTE *)stream;
	BYTE checksum = 0;
	for(int i = 0; i < len; ++i)
	{
		checksum ^= data[i];
	}

	return checksum;
}
bool ZQAsnUtil::check(const void *stream, int len)
{
	if(NULL == stream || len < 1)//at least 1 byte
		return false;
	BYTE *data = (BYTE *)stream;

	bool valid = ((*data) == checksum(data + 1, len - 1));
	return valid;
}
int ZQAsnUtil::encodeINT32(const AsnInteger32 *pValue, BUFFER_T *pBuffer)
{
    if(NULL == pValue || NULL == pBuffer)
        return ASNENCODE_E_FAILURE;

    AsnUnsigned32 len = sizeof(AsnInteger32);//32-bits integer
    pBuffer->write((const char *)(&len), ASNENCODE_LEN_LENGTH);
    pBuffer->write((const char *)(pValue), len);

    return (ASNENCODE_LEN_LENGTH + len);
}

int ZQAsnUtil::decodeINT32(const void *stream, AsnInteger32 *pValue)
{
    if(NULL == stream || NULL == pValue)
        return ASNENCODE_E_FAILURE;

    const BYTE *data = (const BYTE *)stream;

    AsnUnsigned32 len;
    memcpy(&len, data, ASNENCODE_LEN_LENGTH);
    data += ASNENCODE_LEN_LENGTH;

    if(sizeof(AsnInteger32) != len)
        return ASNENCODE_E_FAILURE;

    memcpy(pValue, data, len);

    return (ASNENCODE_LEN_LENGTH + len);
}

int ZQAsnUtil::encodeUINT32(const AsnUnsigned32 *pValue, BUFFER_T *pBuffer)
{
    return encodeINT32((const AsnInteger32 *)pValue, pBuffer);
}
int ZQAsnUtil::decodeUINT32(const void *stream, AsnUnsigned32 *pValue)
{
    return decodeINT32(stream, (AsnInteger32 *)pValue);
}

int ZQAsnUtil::encodeOCTETS(const AsnOctetString *pValue, BUFFER_T *pBuffer)
{
    if(NULL == pValue || NULL == pBuffer)
        return ASNENCODE_E_FAILURE;

    AsnUnsigned32 len = pValue->length;
    BYTE *data = pValue->stream;
    if(0 != len && NULL == data)
        return ASNENCODE_E_FAILURE;//invalid octet string

    pBuffer->write((const char *)(&len), ASNENCODE_LEN_LENGTH);
    if(0 != len)
        pBuffer->write((const char *)(data), len);

    return (ASNENCODE_LEN_LENGTH + len);
}

int ZQAsnUtil::decodeOCTETS(const void *stream, AsnOctetString *pValue)
{
    if(NULL == stream || NULL == pValue)
        return ASNENCODE_E_FAILURE;

    const BYTE *data = (const BYTE *)stream;

    AsnUnsigned32 len;
    memcpy(&len, data, ASNENCODE_LEN_LENGTH);
    data += ASNENCODE_LEN_LENGTH;

    //allocate memory and copy value;
    if(len) //len > 0
    {
        BYTE *pBuffer = (BYTE *)SnmpUtilMemAlloc(len);
        if(pBuffer)
            memcpy(pBuffer, data, len);
        else
            return ASNENCODE_E_FAILURE;

        pValue->stream = pBuffer;
        pValue->length = len;
        pValue->dynamic = TRUE;
    }
    else //len == 0
    {
        pValue->stream = NULL;
        pValue->length = 0;
        pValue->dynamic = FALSE;
    }

    return (ASNENCODE_LEN_LENGTH + len);
}

int ZQAsnUtil::encodeOID(const AsnObjectIdentifier *pValue, BUFFER_T *pBuffer)
{
    if(NULL == pValue || NULL == pBuffer)
        return ASNENCODE_E_FAILURE;

    AsnOctetString dummyValue = {0};
    dummyValue.stream = (BYTE *)(pValue->ids);
    dummyValue.length = sizeof(UINT) * (pValue->idLength);
    return encodeOCTETS(&dummyValue, pBuffer);
}
int ZQAsnUtil::decodeOID(const void *stream, AsnObjectIdentifier *pValue)
{
    if(NULL == stream || NULL == pValue)
        return ASNENCODE_E_FAILURE;

    AsnOctetString dummyValue = {0};
    int len = decodeOCTETS(stream, &dummyValue);
    if(fail(len))
        return len;

    if((dummyValue.length) % sizeof(UINT))
    {
        SnmpUtilOctetsFree(&dummyValue);
        return ASNENCODE_E_FAILURE;
    }
    pValue->ids = (UINT *)(dummyValue.stream);
    pValue->idLength = (dummyValue.length) / sizeof(UINT);

    return len;
}

int ZQAsnUtil::encodeAny(const AsnAny *pAnyValue, BUFFER_T *pBuffer)
{
    if(NULL == pAnyValue || NULL == pBuffer)
        return ASNENCODE_E_FAILURE;

    pBuffer->put(pAnyValue->asnType);
    int len = 0;
    switch(pAnyValue->asnType)
    {
    case ASN_NULL:
        {
            //no value field, use a null string instead
            AsnOctetString nullStr = {0};
            len = encodeOCTETS(&nullStr, pBuffer);
        }
        break;
    case ASN_INTEGER32:
        len = encodeINT32(&(pAnyValue->asnValue.number), pBuffer);
        break;
    case ASN_UNSIGNED32:
        len = encodeUINT32(&(pAnyValue->asnValue.unsigned32), pBuffer);
        break;
    case ASN_COUNTER64:
        len = encodeCounter64(&(pAnyValue->asnValue.counter64), pBuffer);
        break;
    case ASN_OCTETSTRING:
        len = encodeOCTETS(&(pAnyValue->asnValue.string), pBuffer);
        break;
    case ASN_BITS:
        len = encodeOCTETS(&(pAnyValue->asnValue.bits), pBuffer);
        break;
    case ASN_OBJECTIDENTIFIER:
        len = encodeOID(&(pAnyValue->asnValue.object), pBuffer);
        break;
    case ASN_SEQUENCE:
        len = encodeOCTETS(&(pAnyValue->asnValue.sequence), pBuffer);
        break;
    case ASN_IPADDRESS:
        len = encodeOCTETS(&(pAnyValue->asnValue.address), pBuffer);
        break;
    case ASN_COUNTER32:
        len = encodeUINT32(&(pAnyValue->asnValue.counter), pBuffer);
        break;
    case ASN_GAUGE32:
        len = encodeUINT32(&(pAnyValue->asnValue.gauge), pBuffer);
        break;
    case ASN_TIMETICKS:
        len = encodeUINT32(&(pAnyValue->asnValue.ticks), pBuffer);
        break;
    case ASN_OPAQUE:
        len = encodeOCTETS(&(pAnyValue->asnValue.arbitrary), pBuffer);
        break;
    default:
        return ASNENCODE_E_FAILURE;//unsuport type
    };
    if(fail(len))
    {
        pBuffer->seekp(-1, std::ios::cur);//discard 1 byte: the type
        return ASNENCODE_E_FAILURE;
    }
    return (ASNENCODE_LEN_TYPE + len);
}
int ZQAsnUtil::decodeAny(const void *stream, AsnAny *pAnyValue)
{
    if(NULL == stream || NULL == pAnyValue)
        return ASNENCODE_E_FAILURE;

    const BYTE *data = (const BYTE *)stream;

    BYTE type = *data++;
    int len = 0;
    switch(type)
    {
    case ASN_NULL:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.string));
        break;
    case ASN_INTEGER32:
        len = decodeINT32(data, &(pAnyValue->asnValue.number));
        break;
    case ASN_COUNTER64:
        len = decodeCounter64(data, &(pAnyValue->asnValue.counter64));
        break;
    case ASN_UNSIGNED32:
        len = decodeUINT32(data, &(pAnyValue->asnValue.unsigned32));
        break;
    case ASN_OCTETSTRING:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.string));
        break;
    case ASN_BITS:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.bits));
        break;
    case ASN_OBJECTIDENTIFIER:
        len = decodeOID(data, &(pAnyValue->asnValue.object));
        break;
    case ASN_SEQUENCE:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.sequence));
        break;
    case ASN_IPADDRESS:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.address));
        break;
    case ASN_COUNTER32:
        len = decodeUINT32(data, &(pAnyValue->asnValue.counter));
        break;
    case ASN_GAUGE32:
        len = decodeUINT32(data, &(pAnyValue->asnValue.gauge));
        break;
    case ASN_TIMETICKS:
        len = decodeUINT32(data, &(pAnyValue->asnValue.ticks));
        break;
    case ASN_OPAQUE:
        len = decodeOCTETS(data, &(pAnyValue->asnValue.arbitrary));
        break;
    default:
        return ASNENCODE_E_FAILURE;//unsuport type
    };
    if(fail(len))
        return ASNENCODE_E_FAILURE;

    pAnyValue->asnType = type;
    return (ASNENCODE_LEN_TYPE + len);
}

int ZQAsnUtil::encodeCounter64(const AsnCounter64 *pValue, BUFFER_T *pBuffer)
{
    if(NULL == pValue || NULL == pBuffer)
        return ASNENCODE_E_FAILURE;

    AsnUnsigned32 len = sizeof(AsnCounter64);//64-bits integer
    pBuffer->write((const char *)(&len), ASNENCODE_LEN_LENGTH);
    pBuffer->write((const char *)(pValue), len);

    return (ASNENCODE_LEN_LENGTH + len);
}
int ZQAsnUtil::decodeCounter64(const void *stream, AsnCounter64 *pValue)
{
    if(NULL == stream || NULL == pValue)
        return ASNENCODE_E_FAILURE;

    const BYTE *data = (const BYTE *)stream;

    AsnUnsigned32 len;
    memcpy(&len, data, ASNENCODE_LEN_LENGTH);
    data += ASNENCODE_LEN_LENGTH;

    if(sizeof(AsnCounter64) != len)
        return ASNENCODE_E_FAILURE;

    memcpy(pValue, data, len);

    return (ASNENCODE_LEN_LENGTH + len);
}
