// definition file for rtfVcd class VC-1 video codec
//

#ifndef _RTF_VCD_VC1_H
#define _RTF_VCD_VC1_H 1

// VC1 start code type enumerator
typedef enum _RTF_STARTCODE_TYPE_VC1
{
	RTF_STARTCODE_TYPE_VC1_INVALID = 0,

} RTF_STARTCODE_TYPE_VC1;

// VC1 codec specific info structure
typedef struct _RTF_VCD_INFO_VC1
{
	unsigned long bar;

} RTF_VCD_INFO_VC1;

// VC-1 codec specific functions (only used in RTFVcd.c)

// process an VC-1 start code
RTF_RESULT rtfVcdVC1ProcessStartCode( P_RTF_VCD pVcd, unsigned long code );

// build the VC-1 no change frames
void rtfVcdVC1BuildNCFrames( P_RTF_VCD pVcd );

#endif // #ifndef _RTF_VCD_VC1_H
