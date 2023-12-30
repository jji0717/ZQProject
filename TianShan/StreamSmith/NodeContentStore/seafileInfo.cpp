
#define USE_C_OFFSETOF
#include <ZQ_common_conf.h>
#include "seafileInfo.h"

namespace ZQ
{
namespace StreamSmith
{

SfuInformation::SfuInformation(  ZQ::common::Log& l )
:driverHandle(INVALID_HANDLE_VALUE),
logger(l)
{
	pDriverInfo = NULL;
}
SfuInformation::~SfuInformation( )
{
	closeDriver();
}
void SfuInformation::closeDriver()
{
	if( driverHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( driverHandle );
		driverHandle = INVALID_HANDLE_VALUE;
	}
	if( pDriverInfo )
	{
		delete pDriverInfo;
		pDriverInfo = NULL;
	}
}
bool SfuInformation::openDriver( )
{
	closeDriver( );
	driverHandle = CreateFile (SEAFILE_DRIVER_DEVICE_NAME,
									GENERIC_WRITE | GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
	if( driverHandle == INVALID_HANDLE_VALUE )
	{
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"can't open driver [%s] and last error is [%u]"),
					SEAFILE_DRIVER_DEVICE_NAME,GetLastError() );
		return false;
	}
	return true;
}

bool SfuInformation::retrieveDriverInformation( )
{
	if( driverHandle == INVALID_HANDLE_VALUE )
	{
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"no available driver handle, can't get driver information"));
		return false;
	}
	if(!pDriverInfo )
	{
		pDriverInfo = new SF_DRIVER_INFORMATION;		
		//if pDriverInfo is NOT NULL
		//indicate that the driver information has been retrieved already
		DWORD		amountRead = 0;
		if (!DeviceIoControl (	driverHandle,
			(DWORD) IOCTL_SF_DRIVER_QUERY,
			NULL, 0, pDriverInfo, 
			sizeof(SF_DRIVER_INFORMATION),
			&amountRead, 
			NULL))
		{			
			logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"failed to query driver information and error[%u]"),
				GetLastError());
			return false;
		}
	}

	return true;
}

bool SfuInformation::getVolumeInfo( SF_VOLUME_INFORMATION& volInfo , const char* volumeName )
{
	DWORD tempReaded;

	//open device
	HANDLE hVolDevice = CreateFile (volumeName, 
									GENERIC_WRITE | GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
	if( hVolDevice == INVALID_HANDLE_VALUE )
	{
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"can't open device[%s] with error [%u]"),
			volumeName , GetLastError() );
		return false;
	}
	//query volume information

	if (!DeviceIoControl ( hVolDevice,
		(DWORD) IOCTL_SF_VOLUME_QUERY, 
		NULL, 0, &volInfo, 
		sizeof(SF_VOLUME_INFORMATION), 
		&tempReaded, NULL))
	{
		CloseHandle(hVolDevice);
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"failed to query volume[%s]'s information and error[%u]"),
			volumeName , GetLastError() );
		return false;
	}

	CloseHandle(hVolDevice);
	return true;
}

bool SfuInformation::retrieveVolumeInformation( sfVolumeInfos& volInfos , const char* volumeName )
{
	if( driverHandle == INVALID_HANDLE_VALUE && !openDriver() )
	{
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"no available driver handle, can't get driver information"));
		return false;
	}
	if( !pDriverInfo && !retrieveDriverInformation() )
	{
		logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"can't get driver information"));
		return false;
	}
	volInfos.clear();

	for ( int i = 0 ;i < (int)pDriverInfo->volumeCount ; i ++ )
	{
		char szDeviceName[256];
		snprintf(szDeviceName,sizeof(szDeviceName),"\\\\.\\%s%d",SEAFILE_BASE_VOLUME_NAME, pDriverInfo->volumeTable[i] );
		SF_VOLUME_INFORMATION	volInfo;
		if( !getVolumeInfo( volInfo , szDeviceName ) )
		{
			logger(ZQ::common::Log::L_ERROR,CLOGFMT(SfuInformation,"can't get volume [%s] 's information"),	szDeviceName );
			return false;
		}
		if( volumeName )
		{
			if( strncmp(volumeName,(const char*)(volInfo.volumeLabel) , strlen((const char*)(volInfo.volumeLabel)) ) == 0 )
			{
				volInfos.push_back( volInfo );
			}
		}
		else
		{
			volInfos.push_back( volInfo );
		}
	}		

	return true;
}


}}//namespace ZQ::StreamSmith

