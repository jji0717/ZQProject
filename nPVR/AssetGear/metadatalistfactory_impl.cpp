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


////////////////////////////
// from MetatdataListFactory


// ------------------------------------------------------------------------- //
// ServantFactory operations
// ------------------------------------------------------------------------- //
ServerModule::ServantBase_ptr 
MetadataListFactory_impl::createServant(const char* name)
    throw(ServerModule::DuplicateServant,
          ServerModule::ServantCreateFailed,
          ServerModule::OutOfService,
          ServerModule::UnspecifiedException,
          CORBA::SystemException)
{
	return ServerModule::ServantBase::_nil();
}

void
MetadataListFactory_impl::list(CORBA::ULong how_many,
                  ServerModule::ServantBaseList_out bl,
                  ServerModule::ServantBaseIterator_out bi)
    throw(ServerModule::UnspecifiedException,
          CORBA::SystemException)
{
}

ServerModule::ServantBase_ptr
MetadataListFactory_impl::find(const char* name)
    throw(ServerModule::NameNotFound, 
          ServerModule::UnspecifiedException,
          CORBA::SystemException)
{
	return ServerModule::ServantBase::_nil();
}

void
MetadataListFactory_impl::removeServant(ServerModule::ServantBase_ptr s)
    throw(ServerModule::ServantNotFound,
          ServerModule::OutOfService,
          ServerModule::UnspecifiedException,
          CORBA::SystemException)
{
}

    
///////////////////
// from ServantBase

char* 
MetadataListFactory_impl::name()
        throw(CORBA::SystemException)
{
	char * pName = CORBA::string_dup("SeaChangeAssetFactory");
	return pName;
}

void 
MetadataListFactory_impl::provisioningGui(CORBA::String_out provisioningGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
MetadataListFactory_impl::statusGui(CORBA::String_out statusGui)
        throw(ServerModule::NoGuiProvisioned,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void 
MetadataListFactory_impl::setAdminState(ServerModule::AdministrativeState adminState)
        throw(ServerModule::InvalidStateChange,
              ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

ServerModule::AdministrativeState 
MetadataListFactory_impl::getAdminState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::as_InService;
}

ServerModule::OperationalState
MetadataListFactory_impl::getOpState()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
    return ServerModule::os_InService;
}

void 
MetadataListFactory_impl::destroy()
        throw(ServerModule::UnspecifiedException,
              CORBA::SystemException)
{
}

void
MetadataListFactory_impl::getCreateTime (CORBA::LongLong_out createTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
			
void
MetadataListFactory_impl::getLastModifiedTime (CORBA::LongLong_out lastModifiedTime)
        throw (ServerModule::UnspecifiedException, 
               ServerModule::Unimplemented,
               CORBA::SystemException)
{
}
