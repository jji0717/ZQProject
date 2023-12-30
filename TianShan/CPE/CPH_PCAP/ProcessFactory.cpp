
#include "BaseClass.h"
#include "RTFProc.h"

#define ProcessFac			"ProcessFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


BaseProcess* ProcessFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
	if (!stricmp(szName, PROCESS_TYPE_RTF))
		return new RTFProcess();
	
	return NULL;
}


}}

