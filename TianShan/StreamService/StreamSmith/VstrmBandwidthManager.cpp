
#include "VstrmBandwidthManager.h"

namespace ZQ
{
namespace StreamService
{

VstrmBandwidthManager::VstrmBandwidthManager(StreamSmithEnv* environment)
:mVstrmHandle(0),
mbQuit(false),
env(environment)
{
	memset( &mBwDataOld , 0 ,sizeof(mBwDataOld));
	memset( &mBwDataNew , 0 ,sizeof(mBwDataNew));
}

VstrmBandwidthManager::~VstrmBandwidthManager( )
{
	clearResource( );
}
void VstrmBandwidthManager::clearResource()
{
	if( mVstrmHandle != NULL )
	{
		VstrmClassCloseEx( mVstrmHandle );
		mVstrmHandle = NULL;
	}
}
bool VstrmBandwidthManager::start( )
{
	if( VstrmClassOpenEx(&mVstrmHandle) != VSTRM_SUCCESS )
	{
		SESSLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VstrmBandwidthManager,"failed to open vstrm class"));
		return false;
	}
	mBwDataOld.NodeId	= 0xFFFFFFFF;
	mBwDataOld.Version	= 4;
	VstrmClassInquireBandwidthEx2( mVstrmHandle , &mBwDataOld , sizeof(mBwDataOld) );

	return ZQ::common::NativeThread::start();
}

void VstrmBandwidthManager::stop( )
{
	mbQuit = true;
	waitHandle(5000);
	clearResource( );
}

void VstrmBandwidthManager::compareStatus( )
{
	
	VSTRM_BANDWIDTH_INQUIRY_BLOCK_V4& blockOld = mBwDataOld.Data.v4;
	VSTRM_BANDWIDTH_INQUIRY_BLOCK_V4& blockNew = mBwDataNew.Data.v4;

	//compare file bandwidth usage
	if ( memcmp( &blockOld.File , &blockNew.File , sizeof(blockNew.File) ) != 0 )
	{
		VSTRM_BANDWIDTH_POOL_EX3& poolNew = blockNew.File.Pool[kVSTRM_BANDWIDTH_POOLTYPE_CURRENT];
		VSTRM_BANDWIDTH_POOL_EX3& poolOld = blockOld.File.Pool[kVSTRM_BANDWIDTH_POOLTYPE_CURRENT];

		if( memcmp( &poolNew , &poolOld ,sizeof(poolOld) ) != 0 )
		{

		}
	}
	
}

int		VstrmBandwidthManager::run( )
{
	while( !mbQuit )
	{
		mBwDataNew.NodeId	= 0xFFFFFFFF;
		mBwDataNew.Version	= 4;
		 VSTATUS vret = VstrmClassInquireBandwidthEx2( mVstrmHandle , &mBwDataNew , sizeof(mBwDataNew) );
		 if( vret != VSTRM_SUCCESS )
		 {
			 char szErrorBuf[1024] = {0};
			 VstrmClassGetErrorText( mVstrmHandle , vret , szErrorBuf, sizeof(szErrorBuf) -1 );
			 SESSLOG(ZQ::common::Log::L_ERROR,
				 CLOGFMT(VstrmBandwidthManager,"failed to inquire bandwidth status [%s]"),
					 szErrorBuf);
		 }
		 else
		 {
			 compareStatus( );
			 memcpy(&mBwDataNew , &mBwDataOld , sizeof( mBwDataOld) );
		 }
		ZQ::common::delay(66);
		
	}
	return 0;
}

}}


