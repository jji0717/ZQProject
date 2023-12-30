#ifndef __ZQ_SNMP_H__
#define __ZQ_SNMP_H__

enum ZQSNMP_STATUS
{
    ZQSNMP_E_NOERROR,
    ZQSNMP_E_FAILURE,
    ZQSNMP_E_NOSUCHNAME,
    ZQSNMP_E_BADVALUE,
    ZQSNMP_E_READONLY
};
//////////////////////////////////////////////////////////////////////////
//ZQ service's MIB definition
//          1.3.6.1.4.1(enterprise oid prefix)
//                ____|__
//             ...  |   ...
//                22839(ZQ Interactive)
//          ________|____
//         ...  |       ...(future extension)
//              4 (TianShanComponents)
//             _|__________
//             |       |
//             1       ...
//  (internal service branch)
//        ______|_______
//      ...       |
//           (service id)
//    ____________|______________
//   |                  |       ...(future extension)
//   1(shell process)   2(application process)
//   |                  |
//(variable table)   (variable table)


//////////////////////////////////////////////////////////////////////////
//variable table definition

//
//                         ...(service process)
//      ____________________|_________
//     |           |          |      ...(future extension)
//     1(value)    2(name)    3(access)
//    ...        __|____     ...
//              |  |  | ...
//(instance id) 1  2  3


//////////////////////////////////////////////////////////////////////////
//conceptual table illustration
//    (column)     value       name       access
//                  (1)        (2)         (3)
//(row)            ____________________________
//instance id: 1 |item1.1     item2.1     item3.1
//             2 |item1.2     item2.2     item3.2
//             3 |...         ...         ...
//            ...

//////////////////////////////////////////////////////////////////////////

#define ZQSNMP_OID_ZQ           22839
#define ZQSNMP_OID_TianShan     4
#define ZQSNMP_OID_Service      1

#define ZQSNMP_OID_LEN_SVCPREFIX        9 // 1.3.6.1.4.1.22839.4.1
#define ZQSNMP_OID_LEN_SVCINSTANCE      (ZQSNMP_OID_LEN_SVCPREFIX + 1)
#define ZQSNMP_OID_LEN_SVCPROCESS       (ZQSNMP_OID_LEN_SVCINSTANCE + 1)
#define ZQSNMP_OID_LEN_VARINFOTYPE      (ZQSNMP_OID_LEN_SVCPROCESS + 1)
#define ZQSNMP_OID_LEN_VARINSTANCE      (ZQSNMP_OID_LEN_VARINFOTYPE + 1)

#define ZQSNMP_OID_LEN_MIN              ZQSNMP_OID_LEN_SVCPREFIX
#define ZQSNMP_OID_LEN_MAX              ZQSNMP_OID_LEN_VARINSTANCE

#define ZQSNMP_OID_IDX_SVCINSTANCE      (ZQSNMP_OID_LEN_SVCINSTANCE - 1)
#define ZQSNMP_OID_IDX_SVCPROCESS       (ZQSNMP_OID_IDX_SVCINSTANCE + 1)
#define ZQSNMP_OID_IDX_VARINFOTYPE      (ZQSNMP_OID_IDX_SVCPROCESS + 1)
#define ZQSNMP_OID_IDX_VARINSTANCE      (ZQSNMP_OID_IDX_VARINFOTYPE + 1)

//service's process
#define ZQSNMP_OID_SVCPROCESS_SVCSHELL      1
#define ZQSNMP_OID_SVCPROCESS_SVCAPP        2
#define ZQSNMP_OID_SVCPROCESS_MIN           1 //ZQSNMP_OID_SVCPROCESS_SVCSHELL 
#define ZQSNMP_OID_SVCPROCESS_MAX           2 //ZQSNMP_OID_SVCPROCESS_SVCAPP
//variable information table column
#define ZQSNMP_OID_VARINFOTYPE_VALUE        1
#define ZQSNMP_OID_VARINFOTYPE_NAME         2
#define ZQSNMP_OID_VARINFOTYPE_ACCESS       3
#define ZQSNMP_OID_VARINFOTYPE_MIN          1 //ZQSNMP_OID_VARINFOTYPE_VALUE
#define ZQSNMP_OID_VARINFOTYPE_MAX          3 //ZQSNMP_OID_VARINFOTYPE_ACCESS
//variable information table row
#define ZQSNMP_OID_VARINSTANCE_MIN          1

//variable information type
#define ZQSNMP_VARINFOTYPE_VALUE    ZQSNMP_OID_VARINFOTYPE_VALUE
#define ZQSNMP_VARINFOTYPE_NAME     ZQSNMP_OID_VARINFOTYPE_NAME
#define ZQSNMP_VARINFOTYPE_ACCESS   ZQSNMP_OID_VARINFOTYPE_ACCESS

//variable type
#define ZQSNMP_VARTYPE_INT32        0
#define ZQSNMP_VARTYPE_CSTRING      1
#define ZQSNMP_VARTYPE_STDSTRING    2
#define ZQSNMP_VARTYPE_UINT32       3
#define ZQSNMP_VARTYPE_STRING       ZQSNMP_VARTYPE_CSTRING
//variable access types
#define ZQSNMP_ACCESS_READWRITE     0
#define ZQSNMP_ACCESS_READONLY      1

//snmp pdu type
#define ZQSNMP_PDU_GET     0x00
#define ZQSNMP_PDU_GETNEXT 0x01
#define ZQSNMP_PDU_SET     0x02
#define ZQSNMP_PDU_UNKNOWN 0xff

//////////////////////////////////////////////////////////////////////////
//message
#define ZQSNMP_MSG_LEN_MAX 1024
//named pipe
#define ZQSNMP_NP_NAMEPREFIX    "\\\\.\\pipe\\ZQSnmp\\TianShan\\Service\\"
#define ZQSNMP_NP_BUFSIZE       1280

#define ZQSNMP_NP_MAXINSTANCE   1

//callback
typedef void (*ZQSNMP_CALLBACK)(const char *szVarName);

// format of the variable name: [#GROUP#]name
#define ZQSNMP_VARNAME_PREFIX(group) "#" group "#"
#define ZQSNMP_VARNAME(group, name) (ZQSNMP_VARNAME_PREFIX(group) name)

#endif
