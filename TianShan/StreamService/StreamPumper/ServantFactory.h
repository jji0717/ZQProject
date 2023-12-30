
#ifndef _ZQ_StreamService_servant_factory_header_file_h__
#define _ZQ_StreamService_servant_factory_header_file_h__

#include "SsServiceImpl.h"

namespace ZQ
{
namespace StreamService
{

class SsEnvironment;

class ServantFactory : public Ice::ObjectFactory
{
public:
	ServantFactory(SsEnvironment* environment ,  SsServiceImpl& svc );
	virtual ~ServantFactory( );

	Ice::ObjectPtr create(const std::string& strID);

	void destroy();

private:
	SsEnvironment*		env;
	SsServiceImpl&		svcImpl;
};

}
}

#endif//_ZQ_StreamService_servant_factory_header_file_h__

