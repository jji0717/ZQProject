
#include "CPH_NasCopyCfg.h"

ZQ::common::Config::Loader<NasCopyConfig> _gCPHCfg("CPH_NasCopy.xml");


NasCopyConfig::NasCopyConfig()
{
	//maxSessionNum = 30;

	enableProgEvent = 1;
	enableStreamEvent = 1;
	//maxBandwidthKBps = 100*1024;
	streamReqSecs = 5;

	// for vstrm bandwidth management
	vstrmBwClientId = 0;
	bDisableBitrateLimit = 0;	//just for test, 0 as default
}
