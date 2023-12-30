
#include "BaseClass.h"
#include "NTFSTarget.h"
#include "VstrmFilesetTarget.h"


#define TargetFac			"TargetFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

BaseTarget* TargetFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName, TARGET_TYPE_NTFS))
		return new NTFSTarget();

	if (!stricmp(szName, TARGET_TYPE_VSTRMFS))
		return new VstrmFsTarget();
	
	return NULL;
}

}}