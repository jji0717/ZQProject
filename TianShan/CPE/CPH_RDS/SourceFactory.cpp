


#include "BaseClass.h"
#include "PushSource.h"
#include "NTFSSource.h"


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

	return NULL;
}


}}