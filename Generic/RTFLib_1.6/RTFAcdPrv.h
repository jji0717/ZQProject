// private definition file for rtfAcd class
// Note: this info would normally be embedded in the class source file, but
// this class has sub-classes that need to know the parent class state structure
//

#ifndef RTF_ACD_PRV_H
#define RTF_ACD_PRV_H 1

// audio codec specific definition files ************************************************

#ifdef DO_ACD_AC3
#include "RTFAcdAC3.h"
#endif

#ifdef DO_ACD_MPEG2
#include "RTFAcdMPEG2.h"
#endif

// typedefs *****************************************************************************

// ACD state enumerator
typedef enum _RTF_ACDSTATE
{
	RTF_ACDSTATE_INVALID = 0,

	RTF_ACDSTATE_CLOSED,
	RTF_ACDSTATE_OPEN,

} RTF_ACDSTATE;

// ACD specific info union
typedef union _RTF_ACD_INFO
{
#ifdef DO_ACD_AC3
	RTF_ACD_INFO_AC3   ac3;
#endif
#ifdef DO_ACD_MPEG2
	RTF_ACD_INFO_MPEG2 mpeg2;
#endif

} RTF_ACD_INFO;

// ACD class state structure
typedef struct _RTF_ACD
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_ACDSTATE state;
	// session handle
	RTF_SES_HANDLE hSes;
	// audio specification
	RTF_ESTREAM_SPEC audioSpec;

	// audio codec specific info
	RTF_ACD_INFO acdInfo;

} RTF_ACD;

#endif // #ifndef RTF_ACD_PRV_H
