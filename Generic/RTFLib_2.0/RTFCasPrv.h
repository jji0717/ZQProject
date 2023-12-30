// private definition file for rtfCas class
// Note: this info would normally be embedded in the class source file
// but the CAS class is unique in that has CAS-specific sub-classes
// that need to know the parent class state structure
//

#ifndef RTF_CAS_PRV_H
#define RTF_CAS_PRV_H 1

// forward reference pointer to parent class state structure ****************************

typedef struct _RTF_CAS *P_RTF_CAS;

// codec specific definition files ******************************************************

#ifdef DO_CAS_VMX
#include "RTFCasVmx.h"
#endif

#ifdef DO_CAS_PAN
#include "RTFCasPan.h"
#endif

// typedefs *****************************************************************************

// CAS specific info
typedef struct _RTF_CAS_INFO
{
#ifdef DO_CAS_VMX
	RTF_CAS_INFO_VMX vmx;
#endif
#ifdef DO_CAS_PAN
	RTF_CAS_INFO_PAN pan;
#endif
} RTF_CAS_INFO;

// CAS class state structure
typedef struct _RTF_CAS
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_CASSTATE state;
	RTF_SES_HANDLE hSes;
	unsigned long inputPktCount;
	unsigned long cryptoPktCount;

	// CAS subclass-specific info
	RTF_CAS_TYPE casType;
	RTF_CAS_INFO casInfo;

} RTF_CAS;

#endif // #ifndef RTF_CAS_PRV_H
