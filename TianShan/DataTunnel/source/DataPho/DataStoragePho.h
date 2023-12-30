// DodStoragePho.h: interface for the DodStoragePho class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DODSTORAGEPHO_H__57D5A291_B459_4A88_AA2B_2E177AE78022__INCLUDED_)
#define AFX_DODSTORAGEPHO_H__57D5A291_B459_4A88_AA2B_2E177AE78022__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define	DOD_STORAGEPHO_NAME		"DodStoragePho"

class DodStoragePho: public ::Ice::LocalObject, 
	public ZQTsAcPath::IStorageLinkPHO
{
public:
	DodStoragePho(ZQTsAcPath::IPHOManager& mgr);
	virtual ~DodStoragePho();

	/// Implementaions of IPathHelperObject
	virtual bool getSchema(const char* type, 
		TianShanIce::PDSchema& schema);

	virtual void validateConfiguration(const char* type, 
		const char* identStr, 
		::TianShanIce::ValueMap& configPD);

	virtual NarrowResult doNarrow(
		const TsTrans::PathTicketPtr& ticket, 
		const ZQTsAcPath::SessCtx& sessCtx);


	virtual void doFreeResources(const TsTrans::PathTicketPtr& ticket);

	/// Implementations of IStorageLinkPHO
	virtual Ice::Int doEvaluation(ZQTsAcPath::LinkInfo& linkInfo, const ZQTsAcPath::SessCtx& sessCtx,
		TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost);


	virtual void doCommit(const TsTrans::PathTicketPtr& ticket,
		const ZQTsAcPath::SessCtx& sessCtx) ;

protected:
	ZQTsAcPath::IPHOManager&	_phoManager;
};

#endif // !defined(AFX_DODSTORAGEPHO_H__57D5A291_B459_4A88_AA2B_2E177AE78022__INCLUDED_)
