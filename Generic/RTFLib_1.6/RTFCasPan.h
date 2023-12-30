// definition file for rtfCas class PAN (Panasonic) subclass
//

#ifndef _RTF_CAS_PAN_H
#define _RTF_CAS_PAN_H 1

// PAN CAS specific info structure
typedef struct _RTF_CAS_INFO_PAN
{
	unsigned long foo;	// placeholder

} RTF_CAS_INFO_PAN;

// PAN CAS specific functions (only used in RTFCas.c)

// reset the PAN-specific portion of the CAS state structure
void rtfCasPanReset( P_RTF_CAS pCas );

// process an input packet for PAN-specific CAS information
RTF_RESULT rtfCasPanProcessInputPkt( P_RTF_CAS pCas, RTF_PKT_HANDLE hPkt );

// process a sequence and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq );

// process a group and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop );

// process a pioture and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic );

// process a packet and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut );

#endif // #ifndef _RTF_CAS_PAN_H
