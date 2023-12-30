


#include "BaseClass.h"
#include "FTPSource.h"
#include "CIFSSource.h"
#include "McastCapSrc.h"
//#include "C2PullSrc.h"
#include "AquaSource.h"
#include "RTIRawSource.h"

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
	if(!stricmp(szName, SOURCE_TYPE_MCASTCAPSRC))
		return new McastCapSource();
	if(!stricmp(szName, SOURCE_TYPE_AQUA))
		return new AquaIOSource();
	if(!stricmp(szName, SOURCE_TYPE_RTIRAW))
		return new RTIRawSource();

	return NULL;
}


}}

