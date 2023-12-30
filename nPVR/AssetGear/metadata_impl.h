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


#ifndef __Metadata_impl_h___
#define __Metadata_impl_h___

#include "isa.h"


#include <string>
#include <sstream>
#include <map>

#include "AssetComponentS.h"
#include "seacContentComponentS.h"

typedef std::map<std::string, std::string> MdValueList;


namespace MetadataModule
{

class MetadataList_impl : public POA_MetadataModule::seacMetadataList,
                          public PortableServer::RefCountServantBase
{
public:
    static PortableServer::POA_var          _Metadata_poa;

private:
	static PortableServer::Current_var		_current;
    
    MdValueList                             _mdvalues;

public:

	MetadataList_impl(char ** ppMd, int num);

    ////////////////////////
    // From seacMetadataList
    virtual void setValue (const char * name, const char * value)
        throw (CORBA::SystemException);
	
    virtual void resetValue ()
        throw (CORBA::SystemException);

    ////////////////////
	// From MetadataList

    virtual MetadataModule::MetadataList::aMetadata_def * aMetadata ()
      throw ( CORBA::SystemException );
    
    virtual void aMetadata (
        const MetadataModule::MetadataList::aMetadata_def & aMetadata
      )
      throw( CORBA::SystemException );
    
    virtual char * getValue ( const char * name )
      throw(
        CORBA::SystemException,
        ServerModule::OutOfService,
        ServerModule::NameNotFound,
        ServerModule::UnspecifiedException
      );
    
    virtual void unstreamMetadataList ()
      throw(
        CORBA::SystemException,
        ServerModule::OutOfService,
        ServerModule::UnspecifiedException
      );
    
    virtual void provision ()
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::InvalidStateChange,
        ServerModule::ProvisioningFailed
      );
    
    virtual void getProvisioning ()
      throw(
        CORBA::SystemException,
        ServerModule::UnspecifiedException,
        ServerModule::ServantNotProvisioned
      );

    void getMetadataSet (
        MetadataModule::MetadataSet_out theMetadataSet
      )
      throw (
        CORBA::SystemException,
        ServerModule::ServantNotProvisioned,
        ServerModule::Unimplemented,
        ServerModule::UnspecifiedException
        ) {};

    void setMetadataSet (
        const MetadataModule::MetadataSet & theMetadataSet
      )
      throw (
        CORBA::SystemException,
        ServerModule::ProvisioningFailed,
        ServerModule::Unimplemented,
        ServerModule::UnspecifiedException
        ) {};
    
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

}; // class MetadataList_impl


class MetadataListFactory_impl : public POA_MetadataModule::MetadataListFactory,
                                 public PortableServer::RefCountServantBase
{
public:
//    static PortableServer::POA_var          _session_poa;

private:
//	static PortableServer::Current_var		_current;
    

public:

	///////////////////////////
	// From MetadataListFactory

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

}; // class MetadataListFactory_impl


}; // namespace MetadataModule


#endif
