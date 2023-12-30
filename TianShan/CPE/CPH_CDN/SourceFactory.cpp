


#include "BaseClass.h"
#include "FTPSource.h"
#include "CIFSSource.h"
#include "C2PullSrc.h"

#define SourceFac			"SourceFac"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

BaseSource* SourceFactory::Create(const char* szName, ZQ::common::NativeThreadPool* pPool)
{
#ifdef ZQ_OS_MSWIN
	if (!stricmp(szName, SOURCE_TYPE_FTP))
#else
	if (!strcasecmp(szName, SOURCE_TYPE_FTP))
#endif
		return new FTPIOSource();

	if (!stricmp(szName, SOURCE_TYPE_CIFS))
		return new CIFSIOSource();
	if(!stricmp(szName, SOURCE_TYPE_C2PULL))
		return new C2PullSource();

	return NULL;
}


}}

