#ifndef __SrvrLoadSvc_H__
#define __SrvrLoadSvc_H__

#include <BaseZQServiceApplication.h>
#include "./SrvrLoadEnv.h"

namespace SrvrLoad
{

class SrvrLoadSvc : public ZQ::common::BaseZQServiceApplication
{
public: 
	SrvrLoadSvc();
	virtual ~SrvrLoadSvc();

protected: 
	virtual HRESULT OnInit();	
	virtual HRESULT OnStart();	
	virtual HRESULT OnStop();	
	virtual HRESULT OnUnInit();	

private: 
	SrvrLoadEnv _env;

}; // class SrvrLoadSvc

} // namespace SrvrLoad

#endif // #define __SrvrLoadSvc_H__

