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

#pragma warning (disable : 4786)

#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>

#include "Asset_impl.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace AssetModule;


Asset_impl::Asset_impl( const char *                         name,
                        AssetModule::seacAsset_ptr           parentAsset,
                        MetadataModule::seacMetadataList_ptr mdRef)
{
    strcpy (m_name, name );
    m_parent = parentAsset;
    m_mdlist = mdRef;
}

void 
Asset_impl::setMd ( const char * szMdName, const char * szMdValue )
      throw ( CORBA::SystemException)
{
    m_mdlist -> setValue (szMdName, szMdValue);
}

void 
Asset_impl::resetMd ()
      throw ( CORBA::SystemException)
{
    m_mdlist -> resetValue ();
}

void 
Asset_impl::setParentMd ( const char * szMdName, const char * szMdValue )
      throw ( CORBA::SystemException)
{
    if (! CORBA::is_nil(m_parent))
    {
        m_parent -> setMd (szMdName, szMdValue);
    }
}

void 
Asset_impl::resetParentMd ()
      throw ( CORBA::SystemException)
{
    if (! CORBA::is_nil(m_parent))
    {
        m_parent -> resetMd ();
    }
}

AssetModule::AssetFactory_ptr 
Asset_impl::theAssetFactory ()
  throw ( CORBA::SystemException)
{
    return 0;
}

void 
Asset_impl::theAssetFactory ( AssetModule::AssetFactory_ptr theAssetFactory )
  throw ( CORBA::SystemException)
{
}

AssetModule::Asset::aContent_def * 
Asset_impl::aContent ()
    throw ( CORBA::SystemException )
{
    return 0;
}

void 
Asset_impl::aContent ( const AssetModule::Asset::aContent_def & aContent )
  throw( CORBA::SystemException )
{
}

MetadataModule::MetadataList_ptr 
Asset_impl::theMetadataList ()
  throw( CORBA::SystemException )
{
    return MetadataModule::MetadataList::_duplicate(m_mdlist);
}

void 
Asset_impl::theMetadataList ( MetadataModule::MetadataList_ptr theMetadataList )
  throw( CORBA::SystemException )
{
}

AssetModule::Asset::aChildAsset_def * 
Asset_impl::aChildAsset ()
  throw( CORBA::SystemException )
{
    return 0;
}

void 
Asset_impl::aChildAsset ( const AssetModule::Asset::aChildAsset_def & aChildAsset )
  throw( CORBA::SystemException )
{
}

AssetModule::Asset_ptr 
Asset_impl::theParentAsset ()
  throw( CORBA::SystemException )
{
    return AssetModule::Asset::_duplicate(m_parent);
}

void 
Asset_impl::theParentAsset (AssetModule::Asset_ptr theParentAsset )
  throw( CORBA::SystemException )
{
}

void 
Asset_impl::provisionAsset ( ServerModule::AdministrativeState theAdministrativeState )
  throw(
    CORBA::SystemException,
    ServerModule::UnspecifiedException,
    ServerModule::InvalidStateChange,
    ServerModule::ProvisioningFailed  )
{
}

void
Asset_impl::addAsset ( AssetModule::Asset_ptr a )
  throw(
    CORBA::SystemException,
    ServerModule::UnspecifiedException,
    ServerModule::ServantNotFound,
    ServerModule::ProvisioningFailed,
    AssetModule::AssetCycle            )
{
}

StreamModule::Stream_ptr 
Asset_impl::play (
    SessionModule::Session_ptr aSession,
    CORBA::Long aServiceGroup
  )
  throw( CORBA::SystemException,
         ServerModule::UnspecifiedException,
         ServerModule::OutOfService,
         ServerModule::ServantNotFound )
{
    return 0;
}

void
Asset_impl::getAssetProvisioning (
    CORBA::String_out name,
    MetadataModule::MetadataList_out theMetadataList,
    ContentModule::ContentList_out Contents,
    AssetModule::Asset_out theParentAsset,
    AssetModule::AssetList_out ChildAssets,
    ServerModule::OperationalState_out theOperationalState,
    ServerModule::AdministrativeState_out theAdministrativeState
  )
  throw( CORBA::SystemException,
         ServerModule::UnspecifiedException,
         ServerModule::ServantNotProvisioned )
{
    name = CORBA::string_dup (m_name);
    theMetadataList = MetadataModule::MetadataList::_duplicate(m_mdlist);
    Contents = new ContentModule::ContentList();
    theParentAsset = AssetModule::Asset::_duplicate(m_parent);
    ChildAssets = new AssetModule::AssetList();
    theParentAsset  = AssetModule::Asset::_duplicate(m_parent);
}

ContentModule::ContentList * 
Asset_impl::returnContents ()
  throw( CORBA::SystemException,
         ServerModule::UnspecifiedException )
{
    return 0;
}

ContentModule::Content_ptr 
Asset_impl::locateContent ( CORBA::Long serviceGroup )
  throw( CORBA::SystemException,
         ServerModule::UnspecifiedException,
         ServerModule::OutOfService  )
{
    return 0;
}

    
///////////////////
// from ServantBase
char* 
Asset_impl::name()
        throw(CORBA::SystemException)
{
	char * pName = CORBA::string_dup("SeaChangeSession");
	return pName;
}

void 
Asset_impl::provisioningGui(CORBA::String_out provisioningGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
Asset_impl::statusGui(CORBA::String_out statusGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
Asset_impl::setAdminState(ServerModule::AdministrativeState adminState)
        throw(ServerModule::InvalidStateChange,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

ServerModule::AdministrativeState 
Asset_impl::getAdminState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::as_InService;
}

ServerModule::OperationalState
Asset_impl::getOpState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::os_InService;
}

void 
Asset_impl::destroy()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void
Asset_impl::getCreateTime (CORBA::LongLong_out createTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
			
void
Asset_impl::getLastModifiedTime (CORBA::LongLong_out lastModifiedTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
