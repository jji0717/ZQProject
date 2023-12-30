// implementation file for rtfCas class VMX (VeriMatrix) sub-class
// provides VMX-specific functions
//

#include "RTFPrv.h"

#ifdef DO_CAS_VMX

#include "RTFCas.h"
#include "RTFCasPrv.h"

// look up tables ***********************************************************************

// note: the order is reversed to make comparison easier
const static unsigned char vmxSyncPatE[] =
{ 0xEE, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xEE };

// note: the order is reversed to make comparison easier
const static unsigned char vmxSyncPatF[] =
{ 0xFF, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xFF };

// VMX specific private functions *******************************************************

static void matchSyncPat( RTF_CAS *pCas, const unsigned char *pSyncPat,
						  const int syncPatBytes, RTF_CAS_FLAVOR_VMX flavor )
{
	RTF_CAS_INFO_VMX *pInfo = &pCas->casInfo.vmx;
	int i;

	// is the bit String long enough to contain the indicated sync pattern?
	if( pInfo->bitStringBits >= ( syncPatBytes << 3 ) )
	{
		// yes - does the bit string hold the indicated sync pattern?
		for( i=0; i<syncPatBytes; ++i )
		{
			if( pInfo->bitString[ i ] != pSyncPat[ i ] )
			{
				break;
			}
		}
		if( i >= syncPatBytes )
		{
			// yes - verimatrix sync pattern recognized
			pCas->casType = RTF_CAS_TYPE_VMX;
			pInfo->flavor = flavor;
			// set the state to ready
			pCas->state = RTF_CASSTATE_READY;
			// NOTE: we don't have the whole bit string yet - but we have
			// the preamble, and input packets are coming in much faster
			// than output packets are going out, so we should have the
			// rest of the bit string before it is needed. Set the CA pkt
			// density to a safe value until it can be calculated
			pInfo->caPktDensity = RTF_CAS_VMX_SAFEPKTDENSITY;
			// bump the sync count -  is this the second sync pattern?
			if( pInfo->syncCount++ != 0 )
			{
				// yes - bitString holds complete signature
				pCas->state = RTF_CASSTATE_LOCKED;
				// compute the average CA packet density
				pInfo->caPktDensity = ( pInfo->totalPktCount + ( pInfo->caPktCount>>1 ) ) / pInfo->caPktCount;
			}
		} // if( i >= syncPatBytes )
	} // if( pInfo->bitStringBits >= ( sizeof( vmxSyncPatE ) << 3 )
}

// VMX CAS specific public functions ****************************************************

// reset the VMX-specific portion of the CAS state structure
void rtfCasVmxReset( P_RTF_CAS pCas )
{
	RTF_CAS_INFO_VMX *pInfo;

	pInfo = &( pCas->casInfo.vmx );
	pInfo->flavor = RTF_CAS_FLAVOR_VMX_INVALID;
	pInfo->signatureLocked = FALSE;
	pInfo->syncCount = 0;
	pInfo->bitStringBits = 0;
	pInfo->nextInsertBitIndex = 0;
	pInfo->pktsSinceLastInsert = 0;
	pInfo->caPktCount = 0;
	pInfo->totalPktCount = 0;
	pInfo->caPktDensity = RTF_CAS_VMX_SAFEPKTDENSITY;
	memset( (void *)pInfo->bitString, 0, sizeof( pInfo->bitString ) );
	// set up a prototype VeriMatrix CA packet
	memset( (void *)pInfo->casPkt, 0, sizeof( pInfo->casPkt ) );
	pInfo->casPkt[ 0 ] = 0x47;
	pInfo->casPkt[ 1 ] = 0x1F;
	pInfo->casPkt[ 2 ] = 0xFF;
	pInfo->casPkt[ 3 ] = 0x10;
}

// process an input packet for VMX-specific CAS information
RTF_RESULT rtfCasVmxProcessInputPkt( P_RTF_CAS pCas, RTF_PKT_HANDLE hPkt )
{
	RTF_FNAME( "rtfCasVmxProcessInputPkt" );
	RTF_OBASE( pCas );
	RTF_RESULT result = RTF_PASS;
	RTF_CAS_INFO_VMX *pInfo;
	unsigned char *pStorage;
	unsigned char *pBitString;
	int i, bytes, found;
	unsigned short pid;
	unsigned char carry, ucTemp;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		pInfo = &( pCas->casInfo.vmx );
		// bump the total packet count
		++pInfo->totalPktCount;
		// bump the inter CA packet count
		++pInfo->pktsSinceLastInsert;
		// any need to look at more input packets?
		if( pInfo->signatureLocked != FALSE )
		{
			// no - escape.
			break;
		}
		// get the PID of this packet
		result = rtfPktGetPID( hPkt, &pid );
		RTF_CHK_RESULT;
		// is this the stuffing pid?
		if( pid == TRANSPORT_PAD_PID )
		{
			// yes. get the storage pointer from the packet
			result = rtfPktGetStorage( hPkt, &pStorage );
			RTF_CHK_RESULT;
			// a Verimatrix inserted packet will be a stuffing PID packet
			// it will have no adaptation field. and the payload will either
			// start with four bytes of 0x47, or else will consist of all zeros, 
			// except for the last byte, which will be 0 or 1
			found = 0;
			if( ( pStorage[ 4 ] == 0x47 ) &&
				( pStorage[ 5 ] == 0x47 ) &&
				( pStorage[ 6 ] == 0x47 ) &&
				( pStorage[ 7 ] == 0x47 ) )
			{
				// Verimatrix sync pattern recognized
				pCas->casType = RTF_CAS_TYPE_VMX;
				pInfo->flavor = RTF_CAS_FLAVOR_VMX_PARALLEL_47;
				// set the state to ready
				pCas->state = RTF_CASSTATE_READY;
				// NOTE: we have the ECM packet, but we don't know the density yet
				pInfo->caPktDensity = RTF_CAS_VMX_SAFEPKTDENSITY;
				// bump the sync count -  is this the second sync pattern?
				if( pInfo->syncCount++ != 0 )
				{
					// yes - compute the average CA packet density
					pCas->state = RTF_CASSTATE_LOCKED;
					pInfo->caPktDensity = ( pInfo->totalPktCount + ( pInfo->caPktCount>>1 ) ) / pInfo->caPktCount;
				}
				else
				{
					// no - record the prototype ECM packet in the cas packet buffer
					memcpy( pInfo->casPkt, pStorage, TRANSPORT_PACKET_BYTES );
					pInfo->signatureLocked = TRUE;
				}
			}
			else
			{
				pStorage += 4;
				for( i=4; i<TRANSPORT_PACKET_BYTES-1; ++i )
				{
					if( *pStorage++ != 0 )
					{
						break;
					}
				}
				// all zeros?
				if( i >= TRANSPORT_PACKET_BYTES-1 )
				{
					// is the final byte 0x00 or 0x01?
					if( ( *pStorage & 0xFE ) == 0 )
					{
						// yes - shift the final bit into the bit string
						carry = *pStorage;
						pBitString = pInfo->bitString;
						bytes = ( ++pInfo->bitStringBits + 7 ) >> 3;
						for( i=0; i<bytes; ++i )
						{
							ucTemp = *pBitString;
							*pBitString++ = ( ucTemp << 1 ) | carry;
							carry = ucTemp >> 7;
						}
						// check to see if the bit string holds the verimatrix E sync pattern
						matchSyncPat( pCas, vmxSyncPatE, sizeof(vmxSyncPatE), RTF_CAS_FLAVOR_VMX_SERIAL_E );
						// check to see if the bit string holds the verimatrix F sync pattern
						matchSyncPat( pCas, vmxSyncPatF, sizeof(vmxSyncPatF), RTF_CAS_FLAVOR_VMX_SERIAL_F );
					} // if( ( *pStorage & 0xFE ) == 0 )
				} // if( i >= TRANSPORT_PACKET_BYTES-1 )
				// yes - bump the CA packet count
				++pInfo->caPktCount;
			} // 
		} // if( pid == TRANSPORT_PAD_PID )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a sequence and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq )
{
	// nothing to do here
	return RTF_PASS;
}

// process a group and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	// nothing to do here
	return RTF_PASS;
}

// process a pioture and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic )
{
	RTF_FNAME( "rtfCasVmxProcessPic" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_CAS_INFO_VMX *pInfo;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		pInfo = &( pCas->casInfo.vmx );
		// set the inter-insertion packet count so that
		// an insertion will occur on the next packet
		pInfo->pktsSinceLastInsert = pInfo->caPktDensity;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a packet and insert any required VMX-specific CAS-related packets
RTF_RESULT rtfCasVmxProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut )
{
	RTF_FNAME( "rtfCasVmxProcessPkt" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_CAS_INFO_VMX *pInfo;
	unsigned char lastByte;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		pInfo = &( pCas->casInfo.vmx );
		// do we have a signature pattern lock?
		if( pInfo->signatureLocked != FALSE )
		{
			// yes - is it time to insert a CA packet?
			if( pInfo->pktsSinceLastInsert >= pInfo->caPktDensity )
			{
				// yes - update the prototype cas packet according to flavor
				switch( pInfo->flavor )
				{
				case RTF_CAS_FLAVOR_VMX_PARALLEL_47:
					// increment the embedded counter
					if( ++pInfo->casPkt[ 180 ] == 0 )
						++pInfo->casPkt[ 179 ];
					break;
				case RTF_CAS_FLAVOR_VMX_SERIAL_E:
				case RTF_CAS_FLAVOR_VMX_SERIAL_F:
					// get the next bit of the CA bit string to be output
					lastByte = pInfo->bitString[ pInfo->nextInsertBitIndex >> 3 ];
					lastByte >>= ( pInfo->nextInsertBitIndex & 0x07 );
					lastByte &= 0x01;
					// embed it in the CA packet buffer
					pInfo->casPkt[ TRANSPORT_PACKET_BYTES - 1 ] = lastByte;
					break;
				default:
					RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "VMX CAS flavor not recognized (%d)\n", pInfo->flavor );
				}
				RTF_CHK_RESULT_LOOP;
				// queue the cas packet to the indicated output
				result = rtfOutQueueData( hOut, pInfo->casPkt, TRANSPORT_PACKET_BYTES );
				// reset the inter-insertion byte count
				pInfo->pktsSinceLastInsert = 0;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // #ifdef DO_CAS_VMX
