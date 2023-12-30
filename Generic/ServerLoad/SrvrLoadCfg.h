#ifndef __SrvrLoadCfg_H__
#define __SrvrLoadCfg_H__
#include <ConfigHelper.h>
namespace SrvrLoad
{
	class SrvrLoadCfg
	{
	public:
		static void structure(ZQ::common::Config::Holder<SrvrLoadCfg> &holder)
		{
		}
	};
}
/*
#include "ConfigLoader.h"

// 实际上并没有使用此类，紧紧是为了能让ZQApplicationService能够编译通过，我会在Service::Oninit()中load配置
namespace SrvrLoad
{

class SrvrLoadCfg : public ZQ::common::ConfigLoader
{
public: 
	SrvrLoadCfg();
	virtual ~SrvrLoadCfg();

protected: 
	virtual ZQ::common::ConfigLoader::PConfigSchema getSchema();
	
}; // class SrvrLoadCfg

} // namespace SrvrLoad
*/
#endif // #define __SrvrLoadCfg_H__

