
#include "ZQ_common_conf.h"
#include "RTIRawTargetFac.h"

namespace ZQTianShan 
{
	namespace ContentProvision
	{

		RTIRawTargetFac::RTIRawTargetFac(FileIoFactory* pFactory)
			:_pFileIoFac(pFactory)
		{
		}
		BaseTarget* RTIRawTargetFac::create(const char* szName)
		{
			if (!stricmp(szName, TARGET_TYPE_RTIRAW))
				return new RTIRawTarget(_pFileIoFac);
			return NULL;
		}

	}}//namespace
