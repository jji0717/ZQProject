// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: DSServantLocator.ice$
// Branch: $Name:  $
// Author: Huang Li
// Desc  : 
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataStream/DSServantLocator.ice $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-03-09 10:34 Li.huang

// ---------------------------------------------------------------------------

#ifndef FREEZE_DSSERVANTLOCATOR_ICE
#define FREEZE_DSSERVANTLOCATOR_ICE

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ServantLocator.ice>
#include <Ice/Identity.ice>
#include <Freeze/Exception.ice>

module Freeze
{
	class PingObject
	{
	};

local interface DSServantLocator extends Ice::ServantLocator
	{
		/**
			* Add a servant to this DSServantLocator.
			* @param servant The servant to add.
			* @param id The identity of the Ice object that is implemented by 
			* the servant.
			* @return A proxy that matches the given identity and this evictor's
			* object adapter.
			* @throws ::Ice::AlreadyRegisteredException Raised if the evictor already has
			* an object with this identity.
		**/
			Object* add(Object servant, Ice::Identity id);

		/**
			* Like [add], but with a facet. Calling <tt>add(servant, id)</tt>
			* is equivalent to calling [addFacet] with an empty facet.
			* @param servant The servant to add.
			* @param id The identity of the Ice object that is implemented by 
			* the servant.
			* @param facet The facet. An empty facet means the default
			* facet.
			* @return A proxy that matches the given identity and this evictor's
			* object adapter.

			* @throws ::Ice::AlreadyRegisteredException Raised if the evictor already has
			* an object with this identity.
			**/
			Object* addFacet(Object servant, Ice::Identity id, string facet);
		/**
		*
			* Permanently destroy an Ice object.
			* @param id The identity of the Ice object.
			* @return The removed servant.
			* @throws ::Ice::NotRegisteredException Raised if this identity was not 
			* registered with the evictor.
		**/
			Object remove(Ice::Identity id);

		/**
			* Like [remove], but with a facet. Calling <tt>remove(id)</tt> 
			* is equivalent to calling [removeFacet] with an empty facet.
			* @param id The identity of the Ice object.
			* @param facet The facet. An empty facet means the default
			* facet.
			* @return The removed servant.
			* @throws ::Ice::NotRegisteredException Raised if this identity was not 
			* registered with the evictor.
		**/
			Object removeFacet(Ice::Identity id, string facet);

		/**
			* Returns true if the given identity is managed by the evictor
			* with the default facet.
			* @return true if the identity is managed by the evictor, false
			* otherwise.
			* @throws DatabaseException Raised if a database failure occurred.
		**/
			bool hasObject(Ice::Identity id);

		/**
			* Like [hasObject], but with a facet. Calling <tt>hasObject(id)</tt>
			* is equivalent to calling [hasFacet] with an empty
			* facet.
			* @return true if the identity is managed by the evictor for the
			* given facet, false otherwise.
			* @throws DatabaseException Raised if a database failure occurred.
		**/
			bool hasFacet(Ice::Identity id, string facet);  
	};

};

#endif//FREEZE_DSSERVANTLOCATOR_ICE
