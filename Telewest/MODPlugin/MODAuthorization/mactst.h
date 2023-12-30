#ifndef _MACTST_H
#define _MACTST_H

#ifndef _MAC_STDAFX_H_
#include "stdafx.h"  // standard MFC includes
#endif

#include "AppSite.h"

#include <assert.h>
#include <malloc.h>

#include "itv.h"
#include "itvMessages.h"
#include "Reporter.h"
#include "Config.h"
#include "mac_interfaces.h"

class CModAuth : public CObject {

protected:
    CModAuth( MACHANDLES *pHandles, MACCALLBACKS *pCallbacks );
    ~CModAuth();

public:
    // Global function for retrieving the ONE instance of this object
    static CModAuth* Instance(MACHANDLES *pHandles, MACCALLBACKS *pCallbacks );
	static CModAuth* Instance();
	static void FreeInstance();

    // Routines to initialize, process and uninitialize.
    ITVSTATUS Initialize();
    ITVSTATUS Uninitialize();
    ITVSTATUS Process (BYTE *pRequest, BOOL *pbResponse);
    ITVSTATUS Report  (BYTE *pRequest, BOOL *pbResponse);
	ITVSTATUS Update(STREAMID Sid, BOOL *pbResponse, 
						  DOUBLE fPrice, DWORD dwRentalTime, DWORD dwRentalType, DWORD dwAssetLength, DWORD dwOrigalRentalTime);

   // Return valid status
   inline BOOL IsValid() { return (m_bValid); }
   // Global functions for retrieving and setting THE management handle.
   inline HANDLE GetManHandle(void) { return (m_hManPkg); }
   inline void SetManHandle(HANDLE hManPkg) { m_hManPkg = hManPkg; }
   inline WCHAR *GetName(void) { return(m_pwszName); };

protected:

   inline void Encode (BYTE *pBuffer, WORD wBufSiz, WORD *pwPos, WORD wLth, BYTE *pVal)
   {
       // Ensure data will fit first
       SC_CASSERT ( (*pwPos + wLth) <= wBufSiz );

       memcpy(pBuffer + *pwPos, pVal, wLth);    // Encode value
       *pwPos += wLth;                          // Adjust position
   }

   inline void Encode (BYTE *pBuffer, WORD wBufSiz, WORD *pwPos, WORD wTag, WORD wLth, BYTE *pVal)
   {
       // Ensure data will fit first
       SC_CASSERT ( (*pwPos + wLth + (sizeof(WORD) * 2)) <= wBufSiz );

       *((WORD *)(pBuffer + *pwPos)) = wTag;    // Encode tag
       *pwPos += sizeof(WORD);                  // Adjust position

       *((WORD *)(pBuffer + *pwPos)) = wLth;    // Encode length
       *pwPos += sizeof(WORD);                  // Adjust position

       memcpy(pBuffer + *pwPos, pVal, wLth);    // Encode value
       *pwPos += wLth;                          // Adjust position
   }

   inline void Decode (BYTE *pBuffer, WORD *pwPos, WORD wLth, BYTE *pVal)
   {
        memcpy ( pVal, pBuffer + *pwPos, wLth );        // Decode value
        *pwPos += wLth;                                 // Adjust position
   }

   inline void Decode (BYTE *pBuffer, WORD *pwPos, WORD *pwTag, WORD *pwLth)
   {
        *pwTag = *((WORD *)(pBuffer + *pwPos));         // Decode tag
        *pwPos += sizeof(WORD);                         // Adjust position

        *pwLth = *((WORD *)(pBuffer + *pwPos));         // Decode length
        *pwPos += sizeof(WORD);                         // Adjust position
   }

private:

    CModAuth (const CModAuth&);            // Copy constructor not allowed.    Do not define.
    CModAuth& operator=(const CModAuth&);  // Assignment operator not allowed. Do not define.

private:
	// filter MOD_TERM_* and Reason code from DSMCC
	DWORD GetRealReasonCode(DWORD dwOrgReleaseCode);
	
	// Get AppID from TicketID
	char* GetAppIDFromTicketID(DWORD dwTicketID, char* szAppID);
protected:

    // This static contains a pointer to the ONE instance of this object.
    static CModAuth *m_instance;

    // Pointer to config object
    CConfig *m_pConfig;

    // Handle to manpkg object
    HANDLE     m_hManPkg;

    // Lock for protecting access to this objects members
    CRITICAL_SECTION m_Lock;

    // Unload routine
    MAC_UNLOAD_RTN m_pfnUnloadRtn;

    // Allocate routine for MAC_UPDATE_RTN
    MAC_ALLOC_RTN m_pfnAllocRtn;

    // Update routine
    MAC_UPDATE_RTN m_pfnUpdateRtn;

    // Indicate whether successfully constructed
    BOOL m_bValid;

    WCHAR *m_pwszName;

	BOOL m_bHasAppSiteCfg;
	CAppSiteDataManager m_appSites;
	
	DWORD m_dwPrefixLength;	// the prefix length of AppID in the ticketID
};

#endif // _MACTST_H
