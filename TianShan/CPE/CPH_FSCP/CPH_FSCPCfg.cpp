
#include "CPH_FSCPCfg.h"

ZQ::common::Config::Loader<FscpConfig> _gCPHCfg("CPH_FSCP.xml");

// using namespace ZQ::common;
// 
// CPHFSCPCfg 		_gCPHCfg;
// 
 FscpConfig::FscpConfig()
 {
	 memset(szCacheDir, 0, sizeof(szCacheDir));
	 mediaSampleSize=64*1024;		//64K for vstrm
	 enableProgEvent = 1;
	 enableStreamEvent = 1;
	 enablePacingTrace = 0;	
	 bandwidthLimitRate = 110;
	 streamReqSecs = 5;				//5 seconds
 
	maxSessionNum = 30;
	maxBandwidthBps = 100000000;	//100mbp

	// for test
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	enableTestNTFS = 0;			//if 1 then do not write to vstrm but write to NTFS

 }
// 
// CPHFSCPCfg::~CPHFSCPCfg()
// {
// 
// }
// 
// ConfigLoader::ConfigSchemaItem* CPHFSCPCfg::getSchema()
// {
// 	static ConfigSchemaItem entry[] = 
// 	{
// 		{"CPH/FSCP/MediaSample",
// 			"size",		&mediaSampleSize,	sizeof(mediaSampleSize),
// 			true,		ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/Bandwidth",
// 		"maxLimitRate",			&bandwidthLimitRate,
// 		sizeof(bandwidthLimitRate),true,ConfigSchemaItem::TYPE_INTEGER},
// 			
// 		{"CPH/FSCP/Pacing",
// 			"cacheDir",			&szCacheDir,
// 		sizeof(szCacheDir),true,ConfigSchemaItem::TYPE_STRING},
// 
// 		{"CPH/FSCP/Pacing",
// 		"enableMD5",			&enableMD5,
// 		sizeof(enableMD5),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/Pacing",
// 		"enableTrace",			&enablePacingTrace,
// 		sizeof(enablePacingTrace),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/WaitTime",
// 		"interval",			&waittime,
// 		sizeof(waittime),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 
// 		{"CPH/FSCP/Event/Progress",
// 		"enable",			&enableProgEvent,
// 		sizeof(enableProgEvent),true,ConfigSchemaItem::TYPE_INTEGER},
// 			
// 		{"CPH/FSCP/Event/Streamable",
// 		"enable",			&enableStreamEvent,
// 		sizeof(enableStreamEvent),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/Event/Streamable",
// 		"requiredSeconds",			&streamReqSecs,
// 		sizeof(streamReqSecs),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/RTF",
// 		"maxSessionNum",			&maxSessionNum,
// 		sizeof(maxSessionNum),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/RTF",
// 		"maxInputBufferBytes",			&maxInputBufferBytes,
// 		sizeof(maxInputBufferBytes),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/RTF",
// 		"maxInputBuffersPerSession",			&maxInputBuffersPerSession,
// 		sizeof(maxInputBuffersPerSession),true,ConfigSchemaItem::TYPE_INTEGER},
// 
// 		{"CPH/FSCP/RTF",
// 		"maxCodingError",			&maxCodingError,
// 		sizeof(maxCodingError),true,ConfigSchemaItem::TYPE_INTEGER},
// 		
// 		{"CPH/FSCP/Test/NTFS",
// 		"enable",			&enableTestNTFS,
// 		sizeof(enableTestNTFS),true,ConfigSchemaItem::TYPE_INTEGER},
// 		
// 		{"CPH/FSCP/Test/NTFS",
// 		"outputDir",			&szNTFSOutputDir,
// 		sizeof(szNTFSOutputDir),true,ConfigSchemaItem::TYPE_STRING},
// 
// 		{NULL, NULL, NULL, 0, true, ConfigSchemaItem::TYPE_STRING}
// 		
// 	};
// 	return entry;	
// }