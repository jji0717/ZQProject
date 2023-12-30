// Copyright (c) 2001 by
// SeaChange International, Inc., Maynard, Mass.
// All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of SeaChange International Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from SeaChange International Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without  notice and
// should not be construed as a commitment by SeaChange International Inc.
// 
// SeaChange  assumes  no  responsibility  for the use or reliability of its
// software on equipment which is not supplied by SeaChange.
// 
// RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
// Government is subject  to  restrictions  as  set  forth  in  Subparagraph
// (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.


#ifndef __Asset_impl_h___
#define __Asset_impl_h___

#include "isa.h"


#include <string>
#include <sstream>
#include <map>
#include <list>

#include "AssetComponentS.h"
#include "seacContentComponentS.h"



namespace AssetModule
{

class Asset_impl : public POA_AssetModule::seacAsset,
                   public PortableServer::RefCountServantBase
{
public:
    //static PortableServer::POA_var          _Asset_poa;

private:
	//static PortableServer::Current_var		_current;

    char                                      m_name [256];
    AssetModule::seacAsset_ptr                m_parent;
    MetadataModule::seacMetadataList_var      m_mdlist;

public:

    Asset_impl( const char *                         name,
                AssetModule::seacAsset_ptr           parentAsset,
                MetadataModule::seacMetadataList_ptr mdRef);

	/////////////////
    // From seacAsset
    virtual void setMd ( const char * szMdName, const char * szMdValue )
      throw ( CORBA::SystemException);

    virtual void resetMd ()
      throw ( CORBA::SystemException);

    virtual void setParentMd ( const char * szMdName, const char * szMdValue )
      throw ( CORBA::SystemException);

    virtual void resetParentMd ()
      throw ( CORBA::SystemException);

    /////////////
	// From Asset
    virtual AssetModule::AssetFactory_ptr theAssetFactory ()
      throw ( CORBA::SystemException);
    
    virtual void theAssetFactory ( AssetModule::AssetFactory_ptr theAssetFactory )
      throw ( CORBA::SystemException);
    
    virtual AssetModule::Asset::aContent_def * aContent ()
        throw ( CORBA::SystemException );
    
    virtual void aContent ( const AssetModule::Asset::aContent_def & aContent )
      throw( CORBA::SystemException );
    
    virtual MetadataModule::MetadataList_ptr theMetadataList ()
      throw( CORBA::SystemException );
    
    virtual void theMetadataList ( MetadataModule::MetadataList_ptr theMetadataList )
      throw( CORBA::SystemException );
    
    virtual AssetModule::Asset::aChildAsset_def * aChildAsset ()
      throw( CORBA::SystemException );
    
    virtual void aChildAsset ( const AssetModule::Asset::aChildAsset_def & aChildAsset )
      throw( CORBA::SystemException );
    
    virtual AssetModule::Asset_ptr theParentAsset ()
      throw( CORBA::SystemException );
    
    virtual void theParentAsset (AssetModule::Asset_ptr theParentAsset )
      throw( CORBA::SystemException );
    
    virtual void provisionAsset ( ServerModule::AdministrativeState theAdministrativeState )
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::InvalidStateChange,
        ServerModule::ProvisioningFailed
      );
    
    virtual void addAsset ( AssetModule::Asset_ptr a )
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::ServantNotFound,
        ServerModule::ProvisioningFailed,
        AssetModule::AssetCycle
      );
    
    virtual StreamModule::Stream_ptr play (
        SessionModule::Session_ptr aSession,
        CORBA::Long aServiceGroup
      )
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::OutOfService,
        ServerModule::ServantNotFound
      );
    
    virtual void getAssetProvisioning (
        CORBA::String_out name,
        MetadataModule::MetadataList_out theMetadataList,
        ContentModule::ContentList_out Contents,
        AssetModule::Asset_out theParentAsset,
        AssetModule::AssetList_out ChildAssets,
        ServerModule::OperationalState_out theOperationalState,
        ServerModule::AdministrativeState_out theAdministrativeState
      )
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::ServantNotProvisioned
      );
    
    virtual ContentModule::ContentList * returnContents ()
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException
      );
    
    virtual ContentModule::Content_ptr locateContent (
        CORBA::Long serviceGroup
      )
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::OutOfService
      );

    virtual void logPurchase (
        const char * purchaseName,
        const char * streamName
      )
      throw (
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::OutOfService,
        ServerModule::ServantNotFound,
        ServerModule::Unimplemented
        ) {};

#if ISA_MAJOR_VERSION >= 1 && ISA_MINOR_VERSION >=5
    virtual void removeAsset(AssetModule::Asset_ptr a)
      throw(CORBA::SystemException,
			ServerModule::UnspecifiedException,
			ServerModule::ServantNotFound,
			ServerModule::ProvisioningFailed,
			AssetModule::AssetCycle)
	{
	}
#endif // ISA_IDL_1.5


    
    ///////////////////
    // from ServantBase
    virtual char* name()
        throw(CORBA::SystemException) ;

    virtual void provisioningGui(CORBA::String_out provisioningGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void statusGui(CORBA::String_out statusGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void setAdminState(ServerModule::AdministrativeState adminState)
        throw(ServerModule::InvalidStateChange,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual ServerModule::AdministrativeState getAdminState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual ServerModule::OperationalState getOpState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void destroy()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void getCreateTime (CORBA::LongLong_out createTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException);
			
    virtual void getLastModifiedTime (CORBA::LongLong_out lastModifiedTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException);

}; // class Asset_impl


class AssetFactory_impl : public POA_AssetModule::AssetFactory,
					      public PortableServer::RefCountServantBase
{
public:
//    static PortableServer::POA_var          _session_poa;

private:
//	static PortableServer::Current_var		_current;
    

public:

	////////////////////
	// From AssetFactory
    virtual AssetModule::AssetFactory::anAsset_def * anAsset ()
        throw ( CORBA::SystemException );
    
    virtual void anAsset ( const AssetModule::AssetFactory::anAsset_def & anAsset)
        throw ( CORBA::SystemException );


    //////////////////////
    // from ServantFactory
    virtual ServerModule::ServantBase_ptr createServant(const char* name)
        throw(ServerModule::DuplicateServant,
              ServerModule::ServantCreateFailed,
              ServerModule::OutOfService,
              ServerModule::UnspecifiedException,
              CORBA::SystemException);

    virtual void list(CORBA::ULong how_many,
                      ServerModule::ServantBaseList_out bl,
                      ServerModule::ServantBaseIterator_out bi)
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException);

    virtual ServerModule::ServantBase_ptr find(const char* name)
        throw(ServerModule::NameNotFound,
              ServerModule::UnspecifiedException,
              CORBA::SystemException);

    virtual void removeServant(ServerModule::ServantBase_ptr s)
        throw(ServerModule::ServantNotFound,
              ServerModule::OutOfService,
              ServerModule::UnspecifiedException,
              CORBA::SystemException);

    
    ///////////////////
    // from ServantBase
    virtual char* name()
        throw(CORBA::SystemException) ;

    virtual void provisioningGui(CORBA::String_out provisioningGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void statusGui(CORBA::String_out statusGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void setAdminState(ServerModule::AdministrativeState adminState)
        throw(ServerModule::InvalidStateChange,
              ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual ServerModule::AdministrativeState getAdminState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual ServerModule::OperationalState getOpState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void destroy()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException) ;

    virtual void getCreateTime (CORBA::LongLong_out createTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException);
			
    virtual void getLastModifiedTime (CORBA::LongLong_out lastModifiedTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException);

}; // class AssetFactory_impl


}; // namespace AssetModule


#endif
