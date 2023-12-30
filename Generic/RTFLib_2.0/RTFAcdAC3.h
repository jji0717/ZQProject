// definition file for rtfAcd class AC3 audio codec
//

#ifndef _RTF_ACD_AC3_H
#define _RTF_ACD_AC3_H 1

// forward reference pointer to parent class state structure
typedef struct _RTF_ACD *P_RTF_ACD;

// AC3 audio codec specific info structure
typedef struct _RTF_ACD_INFO_AC3
{
	unsigned long bar;

} RTF_ACD_INFO_AC3;

// AC3 audio codec specific functions (only used in RTFAcd.c)

// reset the AC3-specific state info
void rtfAcdAC3Reset( P_RTF_ACD pAcd );

// retreive the AC3 silence frame info
RTF_RESULT rtfAcdAC3GetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes );

#endif // #ifndef _RTF_ACD_AC3_H
