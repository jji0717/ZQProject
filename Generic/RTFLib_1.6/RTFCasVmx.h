// definition file for rtfCas class VMX (VeriMatrix) subclass
//

#ifndef _RTF_CAS_VMX_H
#define _RTF_CAS_VMX_H 1

// constants

#define RTF_CAS_VMX_MAXBITSTRINGBYTES	256
#define RTF_CAS_VMX_SAFEPKTDENSITY		128

// typedefs

typedef enum _RTF_CAS_FLAVOR_VMX
{
	RTF_CAS_FLAVOR_VMX_INVALID = 0,

	RTF_CAS_FLAVOR_VMX_PARALLEL_47,
	RTF_CAS_FLAVOR_VMX_SERIAL_E,
	RTF_CAS_FLAVOR_VMX_SERIAL_F,

	RTF_CAS_FLAVOR_VMX_LIMIT

} RTF_CAS_FLAVOR_VMX;

// VMX CAS specific info structure
typedef struct _RTF_CAS_INFO_VMX
{
	BOOL signatureLocked;
	RTF_CAS_FLAVOR_VMX flavor;
	int syncCount;
	int caPktCount;
	int totalPktCount;
	int caPktDensity;
	int nextInsertBitIndex;
	int pktsSinceLastInsert;
	int bitStringBits;
	unsigned char bitString[ RTF_CAS_VMX_MAXBITSTRINGBYTES ];
	unsigned char casPkt[ TRANSPORT_PACKET_BYTES ];

} RTF_CAS_INFO_VMX;

// VMX CAS specific functions (only used in RTFCas.c)

// reset the VMX-specific portion of the CAS state structure
void rtfCasVmxReset( P_RTF_CAS pCas );

// process an input packet for VMX-specific CAS information
RTF_RESULT rtfCasVmxProcessInputPkt( P_RTF_CAS pCas, RTF_PKT_HANDLE hPkt );

// process a sequence and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq );

// process a group and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop );

// process a pioture and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic );

// process a packet and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut );

#endif // #ifndef _RTF_CAS_VMX_H
