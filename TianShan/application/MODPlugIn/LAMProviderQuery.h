// LAMProviderQuery.h: interface for the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAMPROVIDERQUERY_H__CD3B3833_49F6_46E2_8552_3DE74440AAD2EE__INCLUDED_)
#define AFX_LAMPROVIDERQUERY_H__CD3B3833_49F6_46E2_8552_3DE74440AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace ZQTianShan {
	namespace Application{
		namespace MOD{

#define LAM_ProviderQuery_Name		"LookupPid"
			class LAMProviderQuery:  public Ice::LocalObject, 
				public ZQAPPMOD::IProviderQuery    
			{
			public:
				LAMProviderQuery(::Ice::CommunicatorPtr& _ic);
				virtual ~LAMProviderQuery();
			public:
				int getProviderId(ProviderInfo& pidInfo);
			public:
				::Ice::CommunicatorPtr& _iceComm;

			};
		}}}//end namespace

#endif // !defined(AFX_LAMPROVIDERQUERY_H__CD3B3833_49F6_46E2_8552_3DE74440AAD2EE__INCLUDED_)
