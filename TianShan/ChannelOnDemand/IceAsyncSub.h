

#ifndef _ICE_ASYNC_SUB_
#define _ICE_ASYNC_SUB_

#include "todas.h"


class TodasForCodTeardownCB : public com::izq::todas::integration::cod::AMI_TodasForCod_sessionTeardown
{
public:
	TodasForCodTeardownCB(const std::string& cltSession);

    virtual void ice_response(const ::com::izq::todas::integration::cod::SessionResultData& rd);
    virtual void ice_exception(const ::Ice::Exception& ex);

private:
	std::string		clientSessionId;
};




#endif