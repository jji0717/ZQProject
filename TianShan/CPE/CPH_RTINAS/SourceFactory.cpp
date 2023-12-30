


#include "BaseClass.h"
#include "PushSource.h"
#include "NTFSSource.h"
#include "McastCapSrc.h"

#define SourceFac			"SourceFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

		BaseSource* SourceFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName, SOURCE_TYPE_PUSHSRC))
		return new PushSource();

	if (!stricmp(szName, SOURCE_TYPE_NTFSSRC))
		return new NTFSIOSource();

	if (!stricmp(szName, SOURCE_TYPE_MCASTCAPSRC))
		return new McastCapSource();

	return NULL;
}


}}