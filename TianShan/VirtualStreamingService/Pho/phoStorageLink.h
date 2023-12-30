
#ifndef _pho_vss_storage_link_header_file_h__
#define _pho_vss_storage_link_header_file_h__


#include <vector>
#include <string>
#include <list>
#include <map>

#include <Locks.h>
#include "IPathHelperObj.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define STORLINK_TYPE_C2TRANSFER          "SeaChange.NSS.C2Transfer"
#define STORLINK_TYPE_C2TRANSFER_STANDBY  "XOR-Media.NSS.C2Transfer$Standby"
#define STORLINK_TYPE_C2OVERAQUA		  "XOR-Media.NSS.c2overAqua"

	class NssC2Transfer : public IStorageLinkPHO , public Ice::LocalObject
{
public:
	NssC2Transfer( IPHOManager& mgr );
	virtual ~NssC2Transfer();

public:
	
	virtual bool				getSchema( const char* type, ::TianShanIce::PDSchema& schema );
	virtual void				validateConfiguration( const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD );
	virtual NarrowResult		doNarrow( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx );
	virtual void				doFreeResources( const ::TianShanIce::Transport::PathTicketPtr& ticket );
	virtual Ice::Int			doEvaluation( LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost );
	virtual void				doCommit( const ::TianShanIce::Transport::PathTicketPtr& ticket, const SessCtx& sessCtx );

protected:
	typedef struct _StorageLinkAttr 
	{
		std::string				linkId;
		Ice::Long				maxBW;
		Ice::Long				usedBW;
		_StorageLinkAttr()
		{
			maxBW		= 0;
			usedBW		= 0;
		}
	}StorageLinkAttr;
	typedef std::map<std::string, StorageLinkAttr> StorageLinkS;
	//                linkId       attr

	typedef struct _TicketAttr
	{
		std::string				ticketId;
		Ice::Long				usedBW;
		_TicketAttr()
		{
			usedBW		= 0;
		}
	}TicketAttr;
	typedef std::map<std::string , TicketAttr> TicketS;
	//               ticketId      ticketAttr
	void						validateC2TransStorConf( const char* identStr, ::TianShanIce::ValueMap& configPD  );
	void						filteroutUsedBWForC2TransferConf( const char* identStr , StorageLinkAttr& attr );
	Ice::Int					evaluateC2Trans( LinkInfo& linkInfo, const SessCtx& sessCtx, TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost  );

private:
	

	

	StorageLinkS				mStorLinks;

	
	TicketS						mTickets;

	ZQ::common::Mutex			mMutex;

private:

	IPHOManager&		mPhoManager;

};

}}

#endif//_pho_vss_storage_link_header_file_h__
