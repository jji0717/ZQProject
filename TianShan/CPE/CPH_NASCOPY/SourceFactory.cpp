


#include "BaseClass.h"
#include "NasCopySource .h"

#define SourceFac			"SourceFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

BaseSource* SourceFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName,SOURCE_TYPE_NASCOPY))
		return new NasCopySource();

	return NULL;
}


}}