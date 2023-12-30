#ifndef _data_type_convert_from_vstrm_to_native_header_file_h__
#define _data_type_convert_from_vstrm_to_native_header_file_h__

#include <ZQ_common_conf.h>

#ifndef ZQ_OS_MSWIN
#ifndef _VSTRM_DATA_TYPE_DEFINED
typedef int8   CHAR , *PCHAR;
typedef int8   BOOLEAN;
typedef uint64 UQUADWORD, *PUQUADWORD,ULONG64;
typedef int64  QUADWORD, *PQUADWORD;
#endif //_VSTRM_DATA_TYPE_DEFINED

typedef uint8 UCHAR, *PUCHAR; 
typedef uint16 USHORT ,*PUSHORT;
typedef int16  SHORT , *PSHORT;
typedef uint32 ULONG, *PULONG, CLONG, *PCLONG;
typedef int32  LONG,*PLONG;
#ifndef _CTF_DATA_TYPE_DEFINED
typedef uint64 ULONGLONG;
typedef uint64 UINT64;
typedef int64  LONGLONG,LONG64;
#endif
typedef long   VSTATUS;
typedef void*  HANDLE;
typedef const char* LPCTSTR;

typedef struct _SPEED_IND
{
	SHORT	numerator;
	USHORT	denominator;
}SPEED_IND,*PSPEED_IND;

#endif

#endif//_data_type_convert_from_vstrm_to_native_header_file_h__
