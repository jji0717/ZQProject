#include "ISession.hpp"

int  CompositeSessionObject::add(IManaged* add)
{
	return true;
}

int  CompositeSessionObject::remove(IManaged* rm)
{
	return true;
};

int  CompositeSessionObject::count(IManaged* )
{
	return NULL;
}

IManaged*  CompositeSessionObject::lookupByGuid(std::string& guid)
{
	return NULL;
}

IManaged*  CompositeSessionObject::lookupBySock(int sock)
{
	return NULL;
}