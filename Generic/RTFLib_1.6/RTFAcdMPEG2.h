// definition file for rtfAcd class MPEG2 audio codec
//

#ifndef _RTF_ACD_MPEG2_H
#define _RTF_ACD_MPEG2_H 1

// forward reference pointer to parent class state structure
typedef struct _RTF_ACD *P_RTF_ACD;

// MPEG2 audio codec specific info structure
typedef struct _RTF_ACD_INFO_MPEG2
{
	unsigned long bar;

} RTF_ACD_INFO_MPEG2;

// MPEG2 audio codec specific functions (only used in RTFAcd.c)

// reset the MPEG2-specific state info
void rtfAcdMPEG2Reset( P_RTF_ACD pAcd );

// retreive the MPEG2 silence frame info
RTF_RESULT rtfAcdMPEG2GetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes );

#endif // #ifndef _RTF_ACD_MPEG2_H
