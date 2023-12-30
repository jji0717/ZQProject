
#include "BaseClass.h"
#include "RTFProc.h"

#define ProcessFac			"ProcessFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


BaseProcess* ProcessFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
#ifdef ZQ_OS_MSWIN
	if (!stricmp(szName, PROCESS_TYPE_RTF))
#else
	if (!strcasecmp(szName, PROCESS_TYPE_RTF))
#endif
		return new RTFProcess();
	
	return NULL;
}


}}

