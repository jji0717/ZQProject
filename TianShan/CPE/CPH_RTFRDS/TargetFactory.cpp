
#include "BaseClass.h"
#include "NTFSTarget.h"
#include "VStrmTarget.h"
#include "VstrmFilesetTarget.h"
#include "NTFSFsTar.h"

#define TargetFac			"TargetFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

BaseTarget* TargetFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName, TARGET_TYPE_NTFS))
		return new NTFSTarget();

	if (!stricmp(szName, TARGET_TYPE_VSTRM))
		return new VstrmTarget();

	if (!stricmp(szName, TARGET_TYPE_VSTRMFS))
		return new VstrmFsTarget();

	if (!stricmp(szName, TARGET_TYPE_NTFSFS))
		return new NtfsFsTarget();
	
	return NULL;
}

}}