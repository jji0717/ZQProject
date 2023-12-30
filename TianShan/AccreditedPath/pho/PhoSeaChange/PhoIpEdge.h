
#ifndef __weiwoo_plugin_pho_seachange_ipedge_header_file_h__
#define __weiwoo_plugin_pho_seachange_ipedge_header_file_h__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "../../common/IPathHelperObj.h"

#include <Locks.h>

#define MLOG	mLogger

namespace ZQTianShan {
namespace AccreditedPath {

#define STRMLINK_TYPE_IPEDGE_IP					"SeaChange.IpEdge.IP"
#define STRMLINK_TYPE_IPEDGE_DVBC				"SeaChange.IpEdge.DVBC"
#define STRMLINK_TYPE_IPEDGE_DVBCSHARELINK		"SeaChange.IpEdge.DVBC.ShareLink"
#define STRMLINK_TYPE_IPEDGE_IPSHARELINK		"SeaChange.IpEdge.IP.ShareLink"


class PhoIpEdge : public IStreamLinkPHO
{
public:
	PhoIpEdge( IPHOManager& mgr , ZQ::common::Log& logger );
	virtual ~PhoIpEdge(void);
public:
	
	virtual bool			getSchema( const char* type, ::TianShanIce::PDSchema& schema );

	virtual void			validateConfiguration( const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD);

	virtual Ice::Int		doEvaluation( LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost );

	virtual NarrowResult	doNarrow(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	virtual void			doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx);

	virtual void			doFreeResources(const ::TianShanIce::Transport::PathTicketPtr& ticket);

private:

	typedef struct strmlinkIpEdgeAttr 
	{
		std::string				linkId;
		
		std::string				sourceIp;	//streaming source ip
		std::string				destMac;	//destination mac address

		Ice::Int				portUpperBond;
		Ice::Int				portLowerBond;

		Ice::Long				totalBW;
		Ice::Int				totalStreams;

		
		Ice::Long				usedBW;
		Ice::Int				usedStreams;

		std::list<Ice::Int>		availPorts;		
	};

	ZQ::common::Log&			mLogger;

};

}}//namespace ZQTianShan::AccreditedPath

#endif//__weiwoo_plugin_pho_seachange_ipedge_header_file_h__
