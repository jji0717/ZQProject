
#include "stdafx.h"
#include "ObjectWrapper.h"
#include "PrivateSection.h"
static CObjectWrapper * pObjectWrapper = NULL;

CObjectWrapper::CObjectWrapper()
: m_bUpdateParaSet(false), m_bEncrypt(false)
{
	m_nPID = 0;
	InitializeCriticalSection( &m_csUpdateParaSet );
}

CObjectWrapper::~CObjectWrapper()
{	
	DeleteCriticalSection( &m_csUpdateParaSet );
}

bool CObjectWrapper::UpdateParaSet()
{
	EnterCriticalSection( &m_csUpdateParaSet );
	if( m_bUpdateParaSet )
	{
		m_ParaSet = m_tmpParaSet;
		m_bUpdateParaSet = false;
	}
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::SetWrapParaSet( DW_ParameterSet inPara )
{
	m_tmpParaSet = inPara;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetWrapParaSet( DW_ParameterSet * outPara )
{
	if( outPara == NULL )
		return false;

	*outPara = m_ParaSet;
	return true;
}

bool CObjectWrapper::SetChannelPID(long inPID)
{
	m_tmpParaSet.lChannelPID = inPID;
	m_nPID = (int)inPID;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetChannelPID(long * outPID)
{
	if( outPID == NULL )
		return false;

	*outPID = m_ParaSet.lChannelPID;
	return true;
}

bool CObjectWrapper::SetMode(BYTE byMode )
{
	m_tmpParaSet.byMode = byMode;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetMode(BYTE * byMode)
{
	if( byMode == NULL )
		return false;

	*byMode = m_ParaSet.byMode;
	return true;
}


bool CObjectWrapper::GetStreamCount( int * nCount )
{
	if( nCount == NULL )
		return false;

	*nCount = m_ParaSet.nStreamCount;
	return true;
}

bool CObjectWrapper::SetStreamCount( int nCount )
{
	m_tmpParaSet.nStreamCount = nCount;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::SetChannelTag(const char * inTag, BYTE inLength)
{
	if( inTag == NULL )
		return false;
	if( inLength > MAXTAGLENGTH )
		inLength = MAXTAGLENGTH;

	m_tmpParaSet.byTagLength = inLength;
	memset( m_tmpParaSet.sChannelTag, 0, sizeof(m_tmpParaSet.sChannelTag) );
	memcpy( m_tmpParaSet.sChannelTag, inTag, inLength );
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetChannelTag(char * outTag, BYTE * outLength)
{
	if( outTag == NULL )
		return false;
	if( outLength == NULL )
		return false;
	if( *outLength < m_ParaSet.byTagLength )
		return false;

	memset( outTag, 0, *outLength );
	*outLength = m_ParaSet.byTagLength;	
	memcpy( outTag, m_ParaSet.sChannelTag, m_ParaSet.byTagLength );
	return true;
}

bool CObjectWrapper::AddVersionNumber()
{
	m_tmpParaSet.byVersion ++;
	if( m_tmpParaSet.byVersion > 31 )
		m_tmpParaSet.byVersion = 1;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::ResetVersionNumber()
{
	m_tmpParaSet.byVersion = DEFAULTVERSION;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion)
{
	if( inVersion > 31 )
		return false;

	if( inObjectKey == NULL )
	{ // set the version of entire channel to new value
		EnterCriticalSection( &m_csUpdateParaSet );

		m_tmpParaSet.byVersion = inVersion;			
		m_bUpdateParaSet = true;

		LeaveCriticalSection( &m_csUpdateParaSet );
	}
	else
	{ // set the version of certain object.
		if( inObjectKeyLen > m_ParaSet.byObjectKeyLength )
			return false;
		
	}
	return true;
}

bool CObjectWrapper::GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion)
{
	if( outVersion == NULL )
		return false;

	if( inObjectKey == NULL )
	{
		*outVersion = m_ParaSet.byVersion;
	}
	else
	{
		if( inObjectKeyLen < m_ParaSet.byObjectKeyLength )
			return false;
	}
	return true;
}

bool CObjectWrapper::SetObjectKeyLength(BYTE inLength)
{
	m_tmpParaSet.byObjectKeyLength = inLength;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetObjectKeyLength(BYTE *outLength)
{
	if( outLength == NULL )
		return false;

	*outLength = m_ParaSet.byObjectKeyLength;
	return true;
}

bool CObjectWrapper::SetIndexDescLength(BYTE inLength)
{
	m_tmpParaSet.byIndexDescLength = inLength;
	EnterCriticalSection( &m_csUpdateParaSet );
	m_bUpdateParaSet = true;
	LeaveCriticalSection( &m_csUpdateParaSet );
	return true;
}

bool CObjectWrapper::GetIndexDescLength(BYTE *outLength)	
{
	if( outLength == NULL )
		return false;

	*outLength = m_ParaSet.byIndexDescLength;
	return true;
}

bool CObjectWrapper::SetEncrypt( BYTE inEncrypt )
{
	m_bEncrypt = inEncrypt;
	return true;
}

bool CObjectWrapper::GetEncrypt( BYTE * outEncrypt )
{
	if( outEncrypt == NULL )
		return false;

	*outEncrypt = m_bEncrypt;
	return true;
}

/*
bool CObjectWrapper::SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen )
{
	if( inObjectName == NULL )
		return false;
	if( inObjectKey == NULL )
		return false;
	if( inObjectKeyLen > m_ParaSet.byObjectKeyLength )
		return false;

	// use a queue to record the all objects that need to be set for object key field.
	// don't implement the method current because no a queue to save all object name.

	return true;
}

bool CObjectWrapper::GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen)
{
	if( inObjectName == NULL )
		return false;
	if( outObjectKey == NULL )
		return false;
	if( outObjectKeyLen == NULL )
		return false;
	if( *outObjectKeyLen < m_ParaSet.byObjectKeyLength )
		return false;

	// dynamic generate object key, so don't implement the method current.

	return true;
}*/

bool CObjectWrapper::SetMaxSizeOfSample(long inSize)
{
	m_tmpParaSet.lMaxSizeOfSample = inSize;
	return true;
}

bool CObjectWrapper::GetMaxSizeOfSample(long *outSize)
{
	if( outSize == NULL )
		return false;

	*outSize = m_ParaSet.lMaxSizeOfSample;
	return true;
}

bool CObjectWrapper::SetMinSizeOfSample(long inSize)
{
	m_tmpParaSet.lMinSizeOfSample = inSize;
	return true;
}

bool CObjectWrapper::GetMinSizeOfSample(long *outSize)
{
	if( outSize == NULL )
		return false;

	*outSize = m_ParaSet.lMinSizeOfSample;
	return true;
}

bool CObjectWrapper::WrapData(  BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel )
{
	//if( pInBuffer == NULL )
	//	return false;
	if( outSize == NULL )
		return false;

	BYTE byMode;
	GetMode( &byMode );
	// if mode equal with the WRAP_MODE_NE, use the PID generated before.
	// if mode not equal with the WRAP_MODE_NE, get the PID in m_ParaSet.
	if( !(byMode & WRAP_MODE_MULTISTREAM) )	
		GetChannelPID( &(pSubChannel.lChannelPID) );
	GetChannelTag( pSubChannel.sChannelTag, &(pSubChannel.byTagLength) );

	// if inSize > TABLEPAYLOADMAXLEN, the pInBuffer need to be chip to multiple table
	// that indicate the table id or table id extention need to be increased.
	if( inSize > TABLEPAYLOADMAXLEN )
	{
		if( !ChipTable( pInBuffer, TABLEPAYLOADMAXLEN, pOutBuffer, bufsize, outSize, pSubChannel ) )
			return false;

		// outSize indicate that the pOutBuffer has several bytes padded.
		WrapData( pInBuffer+TABLEPAYLOADMAXLEN, inSize-TABLEPAYLOADMAXLEN, pOutBuffer, bufsize, outSize, pSubChannel );
	}
	else
	{
		if( !ChipTable( pInBuffer, inSize, pOutBuffer, bufsize, outSize, pSubChannel ) )
			return false;
	}

	char    chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:After CObjectWrapper::WrapData, PID: %d",m_nPID,pSubChannel.lChannelPID);LogMyEvent(1,0,chLogMsg);

	return true;
}

bool CObjectWrapper::ChipTable( BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel )
{
	// When the same file is chiped into multiple tables, increase the lowest byte of pSubChannel.wTableIDExtension.
	WORD m_wTmpExt = 0;
	WORD m_wTableIDExt = 0;
	if( m_wTableIDExt != pSubChannel.wTableIDExtension )
	{
		m_wTableIDExt = pSubChannel.wTableIDExtension;
		m_wTmpExt = 0;
	}
	else
	{
		BYTE byMode;
		GetMode( &byMode );
		if( !(byMode & WRAP_MODE_TABLEIDFROMFILENAME) )
		{
			m_wTmpExt++;
			pSubChannel.wTableIDExtension += m_wTmpExt;
		}
	}
	char    chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:ChipTable() - table id: %d  table id extension: %d",m_nPID,pSubChannel.byTableID, pSubChannel.wTableIDExtension);LogMyEvent(1,0,chLogMsg);

	// generate object header segment based on the pSubChannel, under the last_section_number.
	ObjectHeader tmpObjHeader;
	memcpy( tmpObjHeader.sObjectTag, m_ParaSet.sChannelTag, m_ParaSet.byTagLength );
	tmpObjHeader.dwReserved[0] = DEFAULTINDEXDESCLENGTH;
	tmpObjHeader.nObjectKeyLength = m_ParaSet.byObjectKeyLength;
	memcpy( tmpObjHeader.sObjectKey, pSubChannel.sObjectKey, m_ParaSet.byObjectKeyLength );
	DWORD * pdwTmp = (DWORD *)tmpObjHeader.dwObjectContentLength;
	*pdwTmp	= htonl( inSize );

	BYTE * pTmpBuffer = new BYTE[inSize+sizeof(ObjectHeader)];
	memcpy( pTmpBuffer, &tmpObjHeader, sizeof(ObjectHeader) );
	if( NULL != pInBuffer )
		memcpy( pTmpBuffer+sizeof(ObjectHeader), pInBuffer, inSize );

	CPrivateSection ps(m_nPID);
	ps.InitSectionNumber( LONGSECTION, inSize+sizeof(ObjectHeader), pSubChannel );
	if( !ps.ChipSection( pTmpBuffer, inSize+sizeof(ObjectHeader), pOutBuffer, bufsize, outSize, pSubChannel ) )
	{
		delete [] pTmpBuffer;
		pTmpBuffer = NULL;
		return false;
	}

	delete [] pTmpBuffer;
	pTmpBuffer = NULL;
	return true;
}
