
#ifndef _ZQ_StreamService_StreamFactory_h__
#define _ZQ_StreamService_StreamFactory_h__

#include "SsServiceImpl.h"

namespace ZQ
{
namespace StreamService
{

class SsEnvironment;

class StreamFactory : public Ice::ObjectFactory
{
public:
	StreamFactory(SsEnvironment* environment ,  SsServiceImpl& svc );
	virtual ~StreamFactory( );

	Ice::ObjectPtr create(const std::string& strID);

	void destroy();

private:
	SsEnvironment*		env;
	SsServiceImpl&		svcImpl;
};

}
}

#endif //_ZQ_StreamService_StreamFactory_h__

