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

// ʵ���ϲ�û��ʹ�ô��࣬������Ϊ������ZQApplicationService�ܹ�����ͨ�����һ���Service::Oninit()��load����
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

