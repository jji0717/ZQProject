#ifndef _Config_H_
#define _Config_H_

#ifndef _MAC_STDAFX_H_
#include "stdafx.h"  // standard MFC includes
#endif
#include "cfgpkg.h"

class CConfig : public CObject {

public:
   CConfig (WCHAR *pwszProductName, WCHAR *pwszServiceName);
   ~CConfig (void);

   const CConfig& operator=(const CConfig& rhs); // Assignment operator
   CConfig(const CConfig& rhs);                  // Copy constructor

   void GetInteger ( WCHAR *wszName, DWORD *pdwValue,  DWORD dwDefault );
   void GetString  ( WCHAR *wszName, WCHAR *pwszValue, WCHAR *pwszDefault, DWORD dwLen );

public:
    HANDLE m_hCfgSession;    // CfgPkg handle

    #ifdef _DEBUG
        public:
            virtual void AssertValid() const;
        protected:
            virtual void Dump(CDumpContext& dc) const;
    #endif

};
#endif // _Config_H_
