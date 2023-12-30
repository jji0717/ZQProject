// DodStoragePho.cpp: implementation of the DodStoragePho class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DodStoragePho.h"

DodStoragePho::DodStoragePho(ZQTsAcPath::IPHOManager& mgr):
	IStorageLinkPHO(mgr), _phoManager(mgr)
{

}

DodStoragePho::~DodStoragePho()
{

}

/// Implementaions of IPathHelperObject
bool DodStoragePho::getSchema(const char* type, 
	TianShanIce::PDSchema& schema)
{
	return true;
}

void DodStoragePho::validateConfiguration(const char* type, 
	const char* identStr, 
	::TianShanIce::ValueMap& configPD)
{

}

ZQTsAcPath::IStorageLinkPHO::NarrowResult DodStoragePho::doNarrow(
	const TsTrans::PathTicketPtr& ticket, 
	const ZQTsAcPath::SessCtx& sessCtx)
{
	return ZQTsAcPath::IStorageLinkPHO::NR_Narrowed;
}

void DodStoragePho::doCommit(const TsTrans::PathTicketPtr& ticket,
		const ZQTsAcPath::SessCtx& sessCtx)
{

}

void DodStoragePho::doFreeResources(const TsTrans::PathTicketPtr& ticket)
{
	
}

/// Implementations of IStorageLinkPHO
Ice::Int DodStoragePho::doEvaluation(
			ZQTsAcPath::LinkInfo& linkInfo, const ZQTsAcPath::SessCtx& sessCtx,
		    TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost)
{
	return 0;
}

