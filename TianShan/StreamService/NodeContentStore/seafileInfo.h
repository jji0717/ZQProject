#ifndef __zq_steamsmith_vstrm_contentstore_seafile_information_header_file_h__
#define __zq_steamsmith_vstrm_contentstore_seafile_information_header_file_h__

#include <ZQ_common_conf.h>
#include <WinIoCtl.h>
extern "C"
{
#ifdef offsetof
	#undef offsetof
#endif
#include <vstrmuser.h>
#include <SeaFileApi.h>
}
#include <vector>
#include <log.h>

namespace ZQ
{
namespace StreamSmith
{

///
///query Sea file information
/// This class is only available under windows 
///
class SfuInformation
{
public:
	SfuInformation( ZQ::common::Log& l );
	virtual ~SfuInformation( );

public:	

	typedef std::vector<SF_VOLUME_INFORMATION> sfVolumeInfos;
	

	bool		retrieveDriverInformation( );
	bool		retrieveVolumeInformation( sfVolumeInfos& volInfos , const char* volumeName = NULL );

protected:

	bool		openDriver( );

	void		closeDriver( );

	bool		getVolumeInfo( SF_VOLUME_INFORMATION& info , const char* volumeName );

	

private:
	HANDLE					driverHandle;
	ZQ::common::Log&		logger;

	SF_DRIVER_INFORMATION*	pDriverInfo;
	
};
}}//namespace ZQ::StreamSmith


#endif//__zq_steamsmith_vstrm_contentstore_seafile_information_header_file_h__
