
#ifndef _ZQ_StreamService_StreamSmith_bandwidth_manager_Header_file__
#define _ZQ_StreamService_StreamSmith_bandwidth_manager_Header_file__


#include <NativeThread.h>
#include "StreamSmithEnv.h"

extern "C"
{
	#include <vstrmuser.h>
};

namespace ZQ
{
namespace StreamService
{

class VstrmBandwidthManager : public ZQ::common::NativeThread
{
public:
	VstrmBandwidthManager( StreamSmithEnv* environment );
	virtual ~VstrmBandwidthManager( );

public:
	
	virtual bool	start();

	void			stop( );

protected:
	
	int				run( );	


	void			clearResource( );

	void			compareStatus( );

private:
	StreamSmithEnv*						env;
	bool								mbQuit;
	VHANDLE								mVstrmHandle;
	
	VSTRM_BANDWIDTH_INQUIRY_INFO_EX2	mBwDataOld;
	VSTRM_BANDWIDTH_INQUIRY_INFO_EX2	mBwDataNew;

};

}}

#endif//_ZQ_StreamService_StreamSmith_bandwidth_manager_Header_file__
