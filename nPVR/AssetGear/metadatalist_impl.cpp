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

#include "metadata_impl.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace MetadataModule;


MetadataList_impl::MetadataList_impl(char ** ppMd, int num) 
{
    for (int i=0; i < num; ++i)
    {
        _mdvalues[ ppMd[i*2] ]  = ppMd[i*2+1];
    }
}

////////////////////////
// From seacMetadatalist

void
MetadataList_impl::setValue (const char * name, const char * value)
throw (CORBA::SystemException)
{
    _mdvalues [name] = value;
}

void
MetadataList_impl::resetValue ()
throw (CORBA::SystemException)
{
    _mdvalues.clear();
}

////////////////////
// From MetadataList

MetadataModule::MetadataList::aMetadata_def * 
MetadataList_impl::aMetadata ()
  throw ( CORBA::SystemException )
{
    return 0;
}

void 
MetadataList_impl::aMetadata (
    const MetadataModule::MetadataList::aMetadata_def & aMetadata
  )
  throw( CORBA::SystemException )
{
}

char * 
MetadataList_impl::getValue ( const char * name )
  throw(
    CORBA::SystemException,
    ServerModule::OutOfService,
    ServerModule::NameNotFound,
    ServerModule::UnspecifiedException
  )
{
    MdValueList::iterator itr;
    
    string mdname;
    string mdvalue;
    for (itr = _mdvalues.begin(); itr != _mdvalues.end(); ++itr)
    {
        mdname  = itr -> first;
        mdvalue = itr -> second;
    }

    itr = _mdvalues.find(name);
    if (itr == _mdvalues.end())
        throw ServerModule::NameNotFound();

    char * value = CORBA::string_dup(itr->second.c_str());
    return value;
}

void 
MetadataList_impl::unstreamMetadataList ()
  throw(
    CORBA::SystemException,
    ServerModule::OutOfService,
    ServerModule::UnspecifiedException
  )
{
}

void 
MetadataList_impl::provision ()
  throw(
    CORBA::SystemException,
    ServerModule::UnspecifiedException,
    ServerModule::InvalidStateChange,
    ServerModule::ProvisioningFailed
  )
{
}

void 
MetadataList_impl::getProvisioning ()
  throw(
    CORBA::SystemException,
    ServerModule::UnspecifiedException,
    ServerModule::ServantNotProvisioned
  )
{
}

    
///////////////////
// from ServantBase
char* 
MetadataList_impl::name()
        throw(CORBA::SystemException)
{
	char * pName = CORBA::string_dup("SeaChangeSession");
	return pName;
}

void 
MetadataList_impl::provisioningGui(CORBA::String_out provisioningGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
MetadataList_impl::statusGui(CORBA::String_out statusGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
MetadataList_impl::setAdminState(ServerModule::AdministrativeState adminState)
        throw(ServerModule::InvalidStateChange,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

ServerModule::AdministrativeState 
MetadataList_impl::getAdminState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::as_InService;
}

ServerModule::OperationalState
MetadataList_impl::getOpState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::os_InService;
}

void 
MetadataList_impl::destroy()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void
MetadataList_impl::getCreateTime (CORBA::LongLong_out createTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
			
void
MetadataList_impl::getLastModifiedTime (CORBA::LongLong_out lastModifiedTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
