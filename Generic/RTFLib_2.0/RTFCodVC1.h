// definition file for rtfCod class H.264 video codec
//

#ifndef _RTF_COD_VC1_H
#define _RTF_COD_VC1_H 1

// forward reference pointer to parent class state structure
typedef struct _RTF_COD *P_RTF_COD;

// VC1 start code type enumerator
typedef enum _RTF_STARTCODE_TYPE_VC1
{
	RTF_STARTCODE_TYPE_VC1_INVALID = 0,

} RTF_STARTCODE_TYPE_VC1;

// VC1 codec specific info structure
typedef struct _RTF_CODEC_INFO_VC1
{
	unsigned long bar;

} RTF_CODEC_INFO_VC1;

// VC-1 codec specific functions (only used in RTFCod.c)

// process an VC-1 start code
RTF_RESULT rtfCodVC1ProcessStartCode( P_RTF_COD pCod, unsigned long code );

// build the VC-1 no change frames
void rtfCodVC1BuildNCFrames( P_RTF_COD pCod );

#endif // #ifndef _RTF_COD_VC1_H
