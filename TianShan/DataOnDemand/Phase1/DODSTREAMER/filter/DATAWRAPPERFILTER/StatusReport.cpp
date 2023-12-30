

#include "Stdafx.h"
#include "StatusReport.h"

CStatusReport * CStatusReport::s_pStatusReport = NULL;

bool CStatusReport::GetState( long * outState )
{
	*outState = m_lState;
	return true;
}

bool CStatusReport::GetLastError( long * outError )
{
	*outError = m_lError;
	return true;
}

bool CStatusReport::GetErrorMsg( char * outMsg, BYTE * outLength )
{
	if( !outMsg )
		return false;

	strcpy( outMsg, m_szMsg );

	return true;
}

void CStatusReport::SetState( long inState )
{
	m_lState = inState;
	return;
}

void CStatusReport::SetLastError( long inError )
{
	m_lError = inError;
	return;
}

void CStatusReport::SetErrorMsg( LPCTSTR lpMsg )
{
	if( !lpMsg )
		return;

	strcpy( m_szMsg, lpMsg );
	return;
}
