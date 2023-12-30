// DodStreamPho.h: interface for the DodStreamPho class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DODSTREAMPHO_H__0DA73E38_3CA3_4626_A182_4C3FE7DC7591__INCLUDED_)
#define AFX_DODSTREAMPHO_H__0DA73E38_3CA3_4626_A182_4C3FE7DC7591__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	DOD_STREAMPHO_NAME		"DodStreamPho"

class DodStreamPho: public ::Ice::LocalObject, 
	public ZQTsAcPath::IStreamLinkPHO
{
public:
	DodStreamPho(ZQTsAcPath::IPHOManager& mgr);
	virtual ~DodStreamPho();

		/// Implementaions of IPathHelperObject
	virtual bool getSchema(const char* type, 
		TianShanIce::PDSchema& schema);

	virtual void validateConfiguration(const char* type, 
		const char* identStr, 
		::TianShanIce::ValueMap& configPD);

	virtual NarrowResult doNarrow(
		const TsTrans::PathTicketPtr& ticket, 
		const ZQTsAcPath::SessCtx& sess);

	virtual void doFreeResources(const TsTrans::PathTicketPtr& ticket);

	/// Implementations of IStorageLinkPHO
	Ice::Int doEvaluation(ZQTsAcPath::LinkInfo& linkInfo, const ZQTsAcPath::SessCtx& sessCtx,
		TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost);

	virtual void doCommit(const TsTrans::PathTicketPtr& ticket, 
		const ZQTsAcPath::SessCtx& sessCtx);


protected:
	ZQTsAcPath::IPHOManager&		_phoManager;

	struct StrmLinkInfo {
		std::string		strmLinkId;
		int				bandWidth;
		int				remainder;		
		std::string		network;
	};

	typedef std::map<std::string, StrmLinkInfo>	StrmLinkInfoMap;

	StrmLinkInfoMap					_strmLinkInfos;
};

#endif // !defined(AFX_DODSTREAMPHO_H__0DA73E38_3CA3_4626_A182_4C3FE7DC7591__INCLUDED_)
