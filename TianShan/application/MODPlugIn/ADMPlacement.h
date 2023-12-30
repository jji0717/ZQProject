// ADMPlacement.h: interface for the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADSPLACEMENT_H__CD3B3444_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
#define AFX_ADSPLACEMENT_H__CD3B3444_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "StdAfx.h"
namespace ZQTianShan {
	namespace Application{
		namespace MOD{

#define LAM_ADMPLACEMENT_Name		"AdReplacementRequest"
			class ADMPlacement:  public ::Ice::LocalObject, 
				public ZQAPPMOD::IAdsReplacement   
			{
			public:
				ADMPlacement(::Ice::CommunicatorPtr& _ic);
				virtual ~ADMPlacement();
			public:
				int getAdsReplacement(AdsInfo& adsinfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);
			public:
				::Ice::CommunicatorPtr& _iceComm;
			};
		}}}//end namespace

#endif // !defined(AFX_ADSPLACEMENT_H__CD3B3444_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)

