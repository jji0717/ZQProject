// definition file for rtfPkt class
// implements transport stream packet handling
//

#ifndef _RTF_PKT_H
#define _RTF_PKT_H 1

// constants ****************************************************************************

// these masks are used to isolate single flags from the "flags" member variable
// packet description flags
#define RTF_PKT_PAYLOADABSENT		0x00000001
#define RTF_PKT_PAYLOADENCRYPTED	0x00000002
#define RTF_PKT_PAYLOADUNITSTART	0x00000004
#define RTF_PKT_DISCONTINUITY		0x00000008
#define RTF_PKT_ADAPTATIONPRESENT	0x00000010
#define RTF_PKT_PCRPRESENT			0x00000020
#define RTF_PKT_SEQSTARTPRESENT		0x00000040
#define RTF_PKT_GOPSTARTPRESENT		0x00000080
#define RTF_PKT_PICSTARTPRESENT		0x00000100
#define RTF_PKT_SEQHDRPRESENT		0x00000200
#define RTF_PKT_GOPHDRPRESENT		0x00000400
#define RTF_PKT_CODHDRPRESENT		0x00000800
#define RTF_PKT_SQXHDRPRESENT		0x00001000
#define RTF_PKT_PESHDRPRESENT		0x00002000
#define RTF_PKT_PROGRESSIVESEQ		0x00004000
#define RTF_PKT_NONZEROPAYLOAD		0x00008000
// packet state flags
#define RTF_PKT_ISDAMAGED			0x80000000

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPktGetStorageRequirement();

// constructor / destructor *************************************************************

RTF_RESULT rtfPktConstructor ( RTF_PKT_HANDLE *pHandle, RTF_HANDLE hParent );

RTF_RESULT rtfPktDestructor ( RTF_PKT_HANDLE handle );

// accessor methods *********************************************************************

// return the PID
RTF_RESULT rtfPktGetPID( RTF_PKT_HANDLE handle, unsigned short *pPid );

// return a pointer to the packet data
RTF_RESULT rtfPktGetStorage( RTF_PKT_HANDLE handle, unsigned char **ppStorage );

// return the handle of the buffer that contains this packet
RTF_RESULT rtfPktGetBuffer( RTF_PKT_HANDLE handle, RTF_BUF_HANDLE *phBuf );

// return the input packet number
RTF_RESULT rtfPktGetInpPktNumber( RTF_PKT_HANDLE handle, unsigned long *pInpPktNumber );

// return the mapped input packet number
RTF_RESULT rtfPktGetMapPktNumber( RTF_PKT_HANDLE handle, unsigned long *pMapPktNumber );

// set the output packet number (main file copy only)
RTF_RESULT rtfPktSetOutPktNumber( RTF_PKT_HANDLE handle, unsigned long outPktNumber );

// return the output packet number (main file copy only)
RTF_RESULT rtfPktGetOutPktNumber( RTF_PKT_HANDLE handle, unsigned long *pOutPktNumber );

// return the continuity counter of the packet
RTF_RESULT rtfPktGetCC( RTF_PKT_HANDLE handle, unsigned char *pCC );

// return some information about the packet
RTF_RESULT rtfPktGetInfo( RTF_PKT_HANDLE handle, unsigned long *pPacketNumber,
						  unsigned short *pPid, unsigned long *pFlags, unsigned char **ppStorage,
						  unsigned char *pPayloadOffset, unsigned char *pPayloadBytes,
						  unsigned char *pPesHeaderOffset, unsigned char *pPesHeaderBytes );

// get the flags from this packet
RTF_RESULT rtfPktGetFlags( RTF_PKT_HANDLE handle, unsigned long *pFlags );

// record the start-of-sequence offset
RTF_RESULT rtfPktSetSeqStartOffset( RTF_PKT_HANDLE handle, unsigned char offset, BOOL isVirtual );

// get the start-of-sequence offset
RTF_RESULT rtfPktGetSeqStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset, BOOL *pIsVirtual );

// set the start-of-group offset
RTF_RESULT rtfPktSetGopStartOffset( RTF_PKT_HANDLE handle, unsigned char offset, BOOL isVirtual );

// get the start-of-group offset
RTF_RESULT rtfPktGetGopStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset, BOOL *pIsVirtual );

// record the start-of-picture offset
RTF_RESULT rtfPktSetPicStartOffset( RTF_PKT_HANDLE handle, unsigned char offset );

// get the start-of-picture offset
RTF_RESULT rtfPktGetPicStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset );

// get the picture coding extension header info
RTF_RESULT rtfPktGetCodHdrOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset );

// set the picture coding extension header info
RTF_RESULT rtfPktSetCodHdrOffset( RTF_PKT_HANDLE handle, unsigned char offset );

// get the sequence extension header offset
RTF_RESULT rtfPktGetSqxHdrOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset );

// set the sequence extension header offset
RTF_RESULT rtfPktSetSqxHdrOffset( RTF_PKT_HANDLE handle, unsigned char offset );

// get the PES header offset from the packet (returns offset of 0 if not present)
RTF_RESULT rtfPktGetPesHdrInfo( RTF_PKT_HANDLE handle, unsigned char *pPayOffset,
							    unsigned char *pHdrOffset, unsigned char *pHdrBytes );

// get the value of the PCR in this packet
RTF_RESULT rtfPktGetPcrTimestamp( RTF_PKT_HANDLE handle, RTF_TIMESTAMP *pTimestamp );

// set the state of the progressive sequence flag bit
RTF_RESULT rtfPktSetProgressiveSeq( RTF_PKT_HANDLE handle, BOOL progressiveSeq );

// service methods **********************************************************************

// reset a packet descriptor
RTF_RESULT rtfPktReset( RTF_PKT_HANDLE handle, BOOL isClose );

// map the transport packet to the indicated contiguous storage
RTF_RESULT rtfPktMap( RTF_PKT_HANDLE handle, RTF_BUF_HANDLE hBuf,
					  unsigned char *pStorage, unsigned long packetNumber,
					  unsigned long mappedPacketNumber, BOOL *pFirstPcrAcquired );

// add a reference to the packet
RTF_RESULT rtfPktAddReference( RTF_PKT_HANDLE handle, RTF_HANDLE hRefHolder );

// remove a reference to the packet
RTF_RESULT rtfPktRemoveReference( RTF_PKT_HANDLE handle, RTF_HANDLE hRefHolder );

// get the reference count of a packet
RTF_RESULT rtfPktGetRefCount( RTF_PKT_HANDLE handle, unsigned long *pRefCount );

#ifdef DO_TRACKREFCOUNTS
// get the handle to the holder of a particular reference
RTF_RESULT rtfPktGetRefHandle( RTF_PKT_HANDLE handle, unsigned long index, RTF_HANDLE *phRefHandle );
#endif

// copy the payload of the packet to the output buffer - return the byte count
RTF_RESULT rtfPktCopyPayload( RTF_PKT_HANDLE handle, unsigned char *pDst );

// reset the transport packet to an idle state
RTF_RESULT rtfPktUnmap( RTF_PKT_HANDLE handle );

#ifdef _DEBUG
// dump out the contents of a packet
RTF_RESULT rtfPktDump( RTF_PKT_HANDLE handle );
#endif
#endif // #ifndef _RTF_PESPACKET_H
