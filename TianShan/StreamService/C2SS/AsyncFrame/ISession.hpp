#ifndef __I_SESSION_HPP__
#define __I_SESSION_HPP__

#include <list>
#include <map>

#include "common_async.hpp"

class IManaged
{
public:
	virtual int  add(IManaged* )    = 0;
	virtual int  remove(IManaged* ) = 0;
	virtual int  count(IManaged* )  = 0;
	virtual IManaged*  lookupByGuid(std::string& guid) = 0;
	virtual IManaged*  lookupBySock(int sock) = 0;
};

class SessionObject: public IManaged
{
public:
	virtual int  add(IManaged* )   ;
	virtual int  remove(IManaged* );
	virtual int  count(IManaged* ) ;
	virtual IManaged*  lookupByGuid(std::string& guid);
	virtual IManaged*  lookupBySock(int sock);
private:

};

class CompositeSessionObject: public IManaged
{
public:
	virtual int  add(IManaged* )   ;
	virtual int  remove(IManaged* );
	virtual int  count(IManaged* ) ;
	virtual IManaged*  lookupByGuid(std::string& guid);
	virtual IManaged*  lookupBySock(int sock);

protected:
	std::map<int, IManaged*> _commMap;
};

#ifdef DEBUG
#include <iostream>
class HelloSession : public IManaged
{
public:
	int  add(IManaged* )
	{
		std::cout<<"hello world"<<std::endl;
	}

	int  remove(IManaged* )	
	{
		std::cout<<"hello world"<<std::endl;
	}

	int  count(IManaged* )
	{
		std::cout<<"hello world"<<std::endl;
	}

	IManaged*  lookupByGuid(std::string& guid)
	{
		std::cout<<"hello world"<<std::endl;
	}

	IManaged*  lookupBySock(int sock)
	{
		std::cout<<"hello world"<<std::endl;
	}
};

#endif //DEBUG

#endif//__I_SESSION_HPP__