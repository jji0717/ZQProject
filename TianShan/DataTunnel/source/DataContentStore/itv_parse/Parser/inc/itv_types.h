
#ifndef ITV_TYPES_INC
#define	ITV_TYPES_INC

/* define for different platformn */
#define ITV_BYTE char
#define ITV_WORD short
#define ITV_DWORD  long
#define ITV_PDWORD  long* 

#define ITV_UBYTE unsigned char
#define ITV_UWORD unsigned short
#define ITV_UDWORD unsigned long
#define ITV_FLOAT float
#define ITV_DOUBLE double
#define ITV_TIME_T time_t

#define ITV_PID  ITV_DWORD  

typedef int    ITV_CLASS_ID;
typedef int    ITV_BOOL;
typedef struct tm ITV_TIMEDATE;

#ifndef ITV_SUCCEED
	#define ITV_SUCCEED   0
#endif

#ifndef ITV_FAILED
	#define ITV_FAILED    -1
#endif

#ifndef ITV_SUCCEED
	#define ITV_SUCCESS 0
#endif

#ifndef ITV_ERROR
	#define ITV_ERROR -1
#endif

#ifndef ITV_TRUE
	#define ITV_TRUE 1
#endif

#ifndef ITV_FALSE
	#define ITV_FALSE 0
#endif

#ifndef TRUE
	#define TRUE	  1
#endif
#ifndef FALSE
	#define FALSE     0
#endif

#define ITV_INT_NULL -1
#define ITV_FLOAT_NULL -1

#define ITV_WORD_TO_DWORD(a)	a

#endif
/*ITV_TYPES_INC  */


