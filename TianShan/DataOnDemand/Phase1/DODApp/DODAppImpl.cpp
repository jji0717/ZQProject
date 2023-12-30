#include "stdafx.h"
#include <DODAppImpl.h>
#include <string.h>
#include "DataPublisherImpl.h"
#include "DestinationImpl.h"
#include "FolderChannelImpl.h"
#include "MessageChannelImpl.h"

//////////////////////////////////////////////////////////////////////////
using ZQ::common::Log;


::Ice::Identity 
DataOnDemand::createDestinationIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = "Destination";
	ident.name = name;
	return ident;
}

::Ice::Identity 
DataOnDemand::createChannelIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = "Channel";
	ident.name = name;
	return ident;
}

//////////////////////////////////////////////////////////////////////////

Ice::ObjectPtr 
DataOnDemand::DODAppFactory::create(const std::string& type)
{
	if (type == "::DataOnDemand::DataPublisherEx") {

		return new ::DataOnDemand::DataPublisherImpl;

	} else if (type == "::DataOnDemand::DestinationEx") {

		return new ::DataOnDemand::DestinationImpl;

	} else if (type == "::DataOnDemand::FolderChannelEx") {

		return new ::DataOnDemand::FolderChannelImpl;

	} else if (type == "::DataOnDemand::MessageChannelEx") {

		return new ::DataOnDemand::MessageChannelImpl;

	}

	assert(false);
	return NULL;
}
