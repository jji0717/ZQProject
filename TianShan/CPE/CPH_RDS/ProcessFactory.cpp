
#include "BaseClass.h"
#include "TrickImportUser.h"

#define ProcessFac			"ProcessFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


BaseProcess* ProcessFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName, PROCESS_TYPE_TRICKGEN))
		return new CTrickImportUser();
	
	return NULL;
}

}}