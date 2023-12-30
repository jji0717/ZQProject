


#include "BaseClass.h"
#include "PushSource.h"
#include "NTFSSource.h"
#include "NTFSFileSetSource.h"
#include "FTPSource.h"
#include "FTPFilesetSource.h"

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

	if (!stricmp(szName,SOURCE_TYPE_NTFS_FILESET))
		return new NTFSFileSetSource();

	if (!stricmp(szName, SOURCE_TYPE_FTP))
		return new FTPIOSource();
	
	if (!stricmp(szName, SOURCE_TYPE_FTPFileset))
		return new FTPFilesetSource();


	return NULL;
}


}}