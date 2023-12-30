

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Mar 02 17:05:18 2007
 */
/* Compiler settings for D:\ZQProjs\Generic\ColorBar\Source\ColorBar.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ColorBar_h__
#define __ColorBar_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IColorBarControl_FWD_DEFINED__
#define __IColorBarControl_FWD_DEFINED__
typedef interface IColorBarControl IColorBarControl;
#endif 	/* __IColorBarControl_FWD_DEFINED__ */


#ifndef ___IColorBarControlEvents_FWD_DEFINED__
#define ___IColorBarControlEvents_FWD_DEFINED__
typedef interface _IColorBarControlEvents _IColorBarControlEvents;
#endif 	/* ___IColorBarControlEvents_FWD_DEFINED__ */


#ifndef __ColorBarControl_FWD_DEFINED__
#define __ColorBarControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class ColorBarControl ColorBarControl;
#else
typedef struct ColorBarControl ColorBarControl;
#endif /* __cplusplus */

#endif 	/* __ColorBarControl_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IColorBarControl_INTERFACE_DEFINED__
#define __IColorBarControl_INTERFACE_DEFINED__

/* interface IColorBarControl */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IColorBarControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AF10C27B-775E-4BFD-9772-14806BC780A9")
    IColorBarControl : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FillColor( 
            /* [in] */ COLORREF crColor,
            /* [in] */ double dStartPos,
            /* [in] */ double dEndPos,
            /* [in] */ BSTR bstrName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowRange( 
            /* [in] */ double dStartPos,
            /* [in] */ double dEndPos) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateCursor( 
            /* [in] */ COLORREF crColor,
            /* [retval][out] */ int *pID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteCursor( 
            /* [in] */ int pID) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DrawLine( 
            /* [in] */ int iID,
            /* [in] */ double dCurPos) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeletePaint( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IColorBarControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColorBarControl * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColorBarControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColorBarControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IColorBarControl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IColorBarControl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IColorBarControl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IColorBarControl * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FillColor )( 
            IColorBarControl * This,
            /* [in] */ COLORREF crColor,
            /* [in] */ double dStartPos,
            /* [in] */ double dEndPos,
            /* [in] */ BSTR bstrName);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShowRange )( 
            IColorBarControl * This,
            /* [in] */ double dStartPos,
            /* [in] */ double dEndPos);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateCursor )( 
            IColorBarControl * This,
            /* [in] */ COLORREF crColor,
            /* [retval][out] */ int *pID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeleteCursor )( 
            IColorBarControl * This,
            /* [in] */ int pID);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DrawLine )( 
            IColorBarControl * This,
            /* [in] */ int iID,
            /* [in] */ double dCurPos);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeletePaint )( 
            IColorBarControl * This);
        
        END_INTERFACE
    } IColorBarControlVtbl;

    interface IColorBarControl
    {
        CONST_VTBL struct IColorBarControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColorBarControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColorBarControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColorBarControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColorBarControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IColorBarControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IColorBarControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IColorBarControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IColorBarControl_FillColor(This,crColor,dStartPos,dEndPos,bstrName)	\
    (This)->lpVtbl -> FillColor(This,crColor,dStartPos,dEndPos,bstrName)

#define IColorBarControl_ShowRange(This,dStartPos,dEndPos)	\
    (This)->lpVtbl -> ShowRange(This,dStartPos,dEndPos)

#define IColorBarControl_CreateCursor(This,crColor,pID)	\
    (This)->lpVtbl -> CreateCursor(This,crColor,pID)

#define IColorBarControl_DeleteCursor(This,pID)	\
    (This)->lpVtbl -> DeleteCursor(This,pID)

#define IColorBarControl_DrawLine(This,iID,dCurPos)	\
    (This)->lpVtbl -> DrawLine(This,iID,dCurPos)

#define IColorBarControl_DeletePaint(This)	\
    (This)->lpVtbl -> DeletePaint(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_FillColor_Proxy( 
    IColorBarControl * This,
    /* [in] */ COLORREF crColor,
    /* [in] */ double dStartPos,
    /* [in] */ double dEndPos,
    /* [in] */ BSTR bstrName);


void __RPC_STUB IColorBarControl_FillColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_ShowRange_Proxy( 
    IColorBarControl * This,
    /* [in] */ double dStartPos,
    /* [in] */ double dEndPos);


void __RPC_STUB IColorBarControl_ShowRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_CreateCursor_Proxy( 
    IColorBarControl * This,
    /* [in] */ COLORREF crColor,
    /* [retval][out] */ int *pID);


void __RPC_STUB IColorBarControl_CreateCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_DeleteCursor_Proxy( 
    IColorBarControl * This,
    /* [in] */ int pID);


void __RPC_STUB IColorBarControl_DeleteCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_DrawLine_Proxy( 
    IColorBarControl * This,
    /* [in] */ int iID,
    /* [in] */ double dCurPos);


void __RPC_STUB IColorBarControl_DrawLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColorBarControl_DeletePaint_Proxy( 
    IColorBarControl * This);


void __RPC_STUB IColorBarControl_DeletePaint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IColorBarControl_INTERFACE_DEFINED__ */



#ifndef __COLORBARLib_LIBRARY_DEFINED__
#define __COLORBARLib_LIBRARY_DEFINED__

/* library COLORBARLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_COLORBARLib;

#ifndef ___IColorBarControlEvents_DISPINTERFACE_DEFINED__
#define ___IColorBarControlEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IColorBarControlEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IColorBarControlEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("6BF0D3EF-37BB-4637-95BF-BEA86A69F1E4")
    _IColorBarControlEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IColorBarControlEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IColorBarControlEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IColorBarControlEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IColorBarControlEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IColorBarControlEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IColorBarControlEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IColorBarControlEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IColorBarControlEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IColorBarControlEventsVtbl;

    interface _IColorBarControlEvents
    {
        CONST_VTBL struct _IColorBarControlEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IColorBarControlEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IColorBarControlEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IColorBarControlEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IColorBarControlEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IColorBarControlEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IColorBarControlEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IColorBarControlEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IColorBarControlEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_ColorBarControl;

#ifdef __cplusplus

class DECLSPEC_UUID("6CC8D260-B070-4428-98FC-C3F412802DCB")
ColorBarControl;
#endif
#endif /* __COLORBARLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


