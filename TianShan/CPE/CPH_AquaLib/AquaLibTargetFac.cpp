
#include "ZQ_common_conf.h"
#include "AquaLibTargetFac.h"

namespace ZQTianShan 
{
	namespace ContentProvision
	{

		AquaLibTargetFac::AquaLibTargetFac(FileIoFactory* pFactory)
			:_pFileIoFac(pFactory)
		{
		}
		BaseTarget* AquaLibTargetFac::create(const char* szName)
		{
			if (!stricmp(szName, TARGET_TYPE_AQUAFILESET))
				return new AquaFilesetTarget(_pFileIoFac);
			else if(!stricmp(szName, TARGET_TYPE_RTIRAW))
				return new RTIRawTarget(_pFileIoFac);
			return NULL;
		}

	}}//namespace
