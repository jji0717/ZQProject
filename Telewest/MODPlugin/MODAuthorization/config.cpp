// #includes
#include <assert.h>

#include "Config.h"  // the header for this .cpp file's class is always first

// Windows debugging tools
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ModAuthReporter.h"
extern CModAuthReporter* g_pAuthReporter;

/////////////////////////////////////////////////////////////////////////////
// CConfig
CConfig::CConfig ( WCHAR *pwszProductName, WCHAR *pwszServiceName ) {

    DWORD dwNumValues;  // Number of values under the initial key

    // Setup Cfgpkg
    m_hCfgSession = CFG_INITEx ( pwszServiceName, &dwNumValues, pwszProductName );

    if ( m_hCfgSession == NULL )
        throw ( FALSE );
}

CConfig::~CConfig() {

    CFGSTATUS Status = CFG_TERM ( m_hCfgSession );

    if ( Status != CFG_SUCCESS )
        throw ( FALSE );
}

const CConfig& CConfig::operator=(const CConfig& rhs) {

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostics
#ifdef _DEBUG
void CConfig::AssertValid() const {

    CObject::AssertValid();
    // additional object validation goes here
}

void CConfig::Dump(CDumpContext& dc) const {

    CObject::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Implementation

/*
 * Procedure:   GetInteger
 *
 * Description: Looks up an integer value from the configuration database.
 *
 * Arguments:   pwszName
 *                  Name of the value to look up as it appears in configuration database
 *
 *              pdwValue
 *                  Place to put the looked up value
 *
 *              dwDefault
 *                  Default value to place in the variable if nothing is found.
 *
 * Returns:     None.
 *
 * Effects:     None
 */

void CConfig::GetInteger ( WCHAR *pwszName, DWORD *pdwValue, DWORD dwDefault ) {

    CFGSTATUS Status;
    DWORD     dwAttrType;
    BOOL      bReset;
    WCHAR     *pwszSlash;
    DWORD     dwNumValues;                      // Number of values under the key
    DWORD     dwBufSize  = sizeof(*pdwValue);   // Get the size of the output value

    // If a subkey was specified in the path then go there
    pwszSlash = wcsrchr ( pwszName, '\\' );
    if ( pwszSlash == NULL ) {

        bReset = FALSE;

        // Invoke configuration library to read the value
        Status = CFG_GET_VALUE ( m_hCfgSession, pwszName, (PBYTE) pdwValue, &dwBufSize, &dwAttrType );
    }
    else {

        bReset = TRUE;

        // Determine subkey to go to.
        WORD  wLength     = pwszSlash - pwszName;
        WCHAR *pwszSubkey = new WCHAR[wcslen(pwszName)];

        wcsncpy ( pwszSubkey, pwszName, wLength );
        pwszSubkey[wLength] = 0;

        // Go to that subkey
        Status = CFG_SUBKEY ( m_hCfgSession, pwszSubkey, &dwNumValues );
        delete pwszSubkey;

        if ( Status != CFG_SUCCESS ) {

            // Log fact that value was not present
            if ( g_pAuthReporter != NULL )
                g_pAuthReporter->ReportLog(REPORT_WARNING, L"Get Configuration [%s] = [%d] (Default)", pwszName, dwDefault );

            // Use default value
            *pdwValue = dwDefault;
            return;
        }

        // Invoke configuration library to read the value
        Status = CFG_GET_VALUE ( m_hCfgSession, ++pwszSlash, (PBYTE) pdwValue, &dwBufSize, &dwAttrType );
    }

    if ( Status != CFG_SUCCESS ) {

        // Log fact that value was not present
        if ( g_pAuthReporter != NULL )
            g_pAuthReporter->ReportLog( REPORT_WARNING, L"Get Configuration [%s] = [%d] (Default)", pwszName, dwDefault );

        // Use default value
        *pdwValue = dwDefault;
    }
    else {

        // Report returned value
        if ( g_pAuthReporter != NULL )
            g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Get Configuration [%s] = [%d]", pwszName, *pdwValue );
    }

    if ( bReset == TRUE ) 
        CFG_SUBKEY ( m_hCfgSession, NULL, &dwNumValues );

}

/*
 * Procedure:   GetString
 *
 * Description: Looks up a string value from the configuration database.
 *
 * Arguments:   pwszName
 *                  Name of the value to look up as it appears in configuration database
 *
 *              pwszValue
 *                  Place to put the looked up value
 *
 *              pwszDefault
 *                  Default value to place in the variable if nothing is found.
 *
 *              dwLen
 *                  Maximum length of the string
 *
 * Returns:     None.
 *
 * Effects:     None
 */

void CConfig::GetString ( WCHAR *pwszName, WCHAR *pwszValue, WCHAR *pwszDefault, DWORD dwLen ) {

    CFGSTATUS Status;
    DWORD     dwAttrType;
    BOOL      bReset;
    WCHAR     *pwszSlash;
    DWORD     dwNumValues;                      // Number of values under the key
    DWORD     dwBufSize = dwLen;

    // If a subkey was specified in the path then go there
    pwszSlash = wcsrchr ( pwszName, '\\' );
    if ( pwszSlash == NULL ) {

        bReset = FALSE;

        // Invoke configuration library to read the value
        Status = CFG_GET_VALUE ( m_hCfgSession, pwszName, (PBYTE) pwszValue, &dwBufSize, &dwAttrType );
    }
    else {

        bReset = TRUE;

        // Determine subkey to go to.
        WORD  wLength     = pwszSlash - pwszName;
        WCHAR *pwszSubkey = new WCHAR[wcslen(pwszName)];

        wcsncpy ( pwszSubkey, pwszName, wLength );
        pwszSubkey[wLength] = 0;

        // Go to that subkey
        Status = CFG_SUBKEY ( m_hCfgSession, pwszSubkey, &dwNumValues );
        delete pwszSubkey;

        if ( Status != CFG_SUCCESS ) {

            // Log fact that value was not present
            if ( g_pAuthReporter != NULL )
                g_pAuthReporter->ReportLog( REPORT_WARNING, L"Get Configuration [%s] = [%s] (Default)", pwszName, pwszDefault );

            // Use default value
            wcscpy ( pwszValue, pwszDefault );
            return;
        }

        // Invoke configuration library to read the value
        Status = CFG_GET_VALUE ( m_hCfgSession, ++pwszSlash, (PBYTE) pwszValue, &dwBufSize, &dwAttrType );
    }

    if ( Status != CFG_SUCCESS ) {

        // Log fact that value was not present
        if ( g_pAuthReporter != NULL )
            g_pAuthReporter->ReportLog( REPORT_WARNING, L"Get Configuration [%s] = [%s] (Default)", pwszName, pwszDefault );

        // Use default value
        wcscpy ( pwszValue, pwszDefault );
    }
    else {

        // Report returned value
        if ( g_pAuthReporter != NULL )
            g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Get Configuration [%s] = [%s]", pwszName, pwszValue );
    }

    if ( bReset == TRUE )
        CFG_SUBKEY ( m_hCfgSession, NULL, &dwNumValues );

}
