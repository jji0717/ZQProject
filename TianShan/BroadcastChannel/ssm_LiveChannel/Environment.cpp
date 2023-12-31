#include "./Environment.h"
#include "./RequestHandler.h"
#include "./SetupRequest.h"
#include "./PlayRequest.h"
#include "./PauseRequest.h"
#include "./TeardownRequest.h"
#include "./GetParamRequest.h"
#include "./OptionRequest.h"
#include "./DescribeRequest.h"
#include "./StreamEvent.h"
#include "./PlaylistEvent.h"
#include "./SessionView.h"
#include "./PingHandler.h"

#include "strHelper.h"
#include "FileSystemOp.h"
#include <TimeUtil.h>

extern "C"
{
#ifdef ZQ_OS_MSWIN
#include <io.h>
#else
#include <sys/stat.h>
#endif
};

#ifdef ZQ_OS_LINUX
extern "C" {
#include <sys/time.h>
#include <time.h>
#include <math.h>
}
#else
#	define atoll _atoi64
#endif

#define MODULE_NAME "ssm_LiveChannel"

ZQ::common::Log* s1log = NULL;
ZQ::common::Config::Loader<LiveChannel::LiveChannelConfig> _liveChannelConfig("");

namespace LiveChannel
{
//////////////////////////////////////////////////////////////////////////
// class SmartRequestHandler
//////////////////////////////////////////////////////////////////////////	
class SmartRequestHandler
{
public:
	SmartRequestHandler(RequestHandler* pRequestHandler);
	virtual ~SmartRequestHandler();
	
protected:
	RequestHandler* _pRequestHandler;
	
}; // class SmartFixupRequest

SmartRequestHandler::SmartRequestHandler(RequestHandler* pRequestHandler)
: _pRequestHandler(pRequestHandler)
{
}

SmartRequestHandler::~SmartRequestHandler()
{
	if (NULL != _pRequestHandler)
		delete _pRequestHandler;
	_pRequestHandler = NULL;
}

//////////////////////////////////////////////////////////////////////////
// implemetation of Environment
//////////////////////////////////////////////////////////////////////////	
const long JAN_1ST_1900 = 2415021;
uint32 CNtpTime::m_MsToNTP[1000] = 
{
	0x00000000, 0x00418937, 0x0083126f, 0x00c49ba6, 0x010624dd, 0x0147ae14,
		0x0189374c, 0x01cac083, 0x020c49ba, 0x024dd2f2, 0x028f5c29, 0x02d0e560,
		0x03126e98, 0x0353f7cf, 0x03958106, 0x03d70a3d, 0x04189375, 0x045a1cac,
		0x049ba5e3, 0x04dd2f1b, 0x051eb852, 0x05604189, 0x05a1cac1, 0x05e353f8,
		0x0624dd2f, 0x06666666, 0x06a7ef9e, 0x06e978d5, 0x072b020c, 0x076c8b44,
		0x07ae147b, 0x07ef9db2, 0x083126e9, 0x0872b021, 0x08b43958, 0x08f5c28f,
		0x09374bc7, 0x0978d4fe, 0x09ba5e35, 0x09fbe76d, 0x0a3d70a4, 0x0a7ef9db,
		0x0ac08312, 0x0b020c4a, 0x0b439581, 0x0b851eb8, 0x0bc6a7f0, 0x0c083127,
		0x0c49ba5e, 0x0c8b4396, 0x0ccccccd, 0x0d0e5604, 0x0d4fdf3b, 0x0d916873,
		0x0dd2f1aa, 0x0e147ae1, 0x0e560419, 0x0e978d50, 0x0ed91687, 0x0f1a9fbe,
		0x0f5c28f6, 0x0f9db22d, 0x0fdf3b64, 0x1020c49c, 0x10624dd3, 0x10a3d70a,
		0x10e56042, 0x1126e979, 0x116872b0, 0x11a9fbe7, 0x11eb851f, 0x122d0e56,
		0x126e978d, 0x12b020c5, 0x12f1a9fc, 0x13333333, 0x1374bc6a, 0x13b645a2,
		0x13f7ced9, 0x14395810, 0x147ae148, 0x14bc6a7f, 0x14fdf3b6, 0x153f7cee,
		0x15810625, 0x15c28f5c, 0x16041893, 0x1645a1cb, 0x16872b02, 0x16c8b439,
		0x170a3d71, 0x174bc6a8, 0x178d4fdf, 0x17ced917, 0x1810624e, 0x1851eb85,
		0x189374bc, 0x18d4fdf4, 0x1916872b, 0x19581062, 0x1999999a, 0x19db22d1,
		0x1a1cac08, 0x1a5e353f, 0x1a9fbe77, 0x1ae147ae, 0x1b22d0e5, 0x1b645a1d,
		0x1ba5e354, 0x1be76c8b, 0x1c28f5c3, 0x1c6a7efa, 0x1cac0831, 0x1ced9168,
		0x1d2f1aa0, 0x1d70a3d7, 0x1db22d0e, 0x1df3b646, 0x1e353f7d, 0x1e76c8b4,
		0x1eb851ec, 0x1ef9db23, 0x1f3b645a, 0x1f7ced91, 0x1fbe76c9, 0x20000000,
		0x20418937, 0x2083126f, 0x20c49ba6, 0x210624dd, 0x2147ae14, 0x2189374c,
		0x21cac083, 0x220c49ba, 0x224dd2f2, 0x228f5c29, 0x22d0e560, 0x23126e98,
		0x2353f7cf, 0x23958106, 0x23d70a3d, 0x24189375, 0x245a1cac, 0x249ba5e3,
		0x24dd2f1b, 0x251eb852, 0x25604189, 0x25a1cac1, 0x25e353f8, 0x2624dd2f,
		0x26666666, 0x26a7ef9e, 0x26e978d5, 0x272b020c, 0x276c8b44, 0x27ae147b,
		0x27ef9db2, 0x283126e9, 0x2872b021, 0x28b43958, 0x28f5c28f, 0x29374bc7,
		0x2978d4fe, 0x29ba5e35, 0x29fbe76d, 0x2a3d70a4, 0x2a7ef9db, 0x2ac08312,
		0x2b020c4a, 0x2b439581, 0x2b851eb8, 0x2bc6a7f0, 0x2c083127, 0x2c49ba5e,
		0x2c8b4396, 0x2ccccccd, 0x2d0e5604, 0x2d4fdf3b, 0x2d916873, 0x2dd2f1aa,
		0x2e147ae1, 0x2e560419, 0x2e978d50, 0x2ed91687, 0x2f1a9fbe, 0x2f5c28f6,
		0x2f9db22d, 0x2fdf3b64, 0x3020c49c, 0x30624dd3, 0x30a3d70a, 0x30e56042,
		0x3126e979, 0x316872b0, 0x31a9fbe7, 0x31eb851f, 0x322d0e56, 0x326e978d,
		0x32b020c5, 0x32f1a9fc, 0x33333333, 0x3374bc6a, 0x33b645a2, 0x33f7ced9,
		0x34395810, 0x347ae148, 0x34bc6a7f, 0x34fdf3b6, 0x353f7cee, 0x35810625,
		0x35c28f5c, 0x36041893, 0x3645a1cb, 0x36872b02, 0x36c8b439, 0x370a3d71,
		0x374bc6a8, 0x378d4fdf, 0x37ced917, 0x3810624e, 0x3851eb85, 0x389374bc,
		0x38d4fdf4, 0x3916872b, 0x39581062, 0x3999999a, 0x39db22d1, 0x3a1cac08,
		0x3a5e353f, 0x3a9fbe77, 0x3ae147ae, 0x3b22d0e5, 0x3b645a1d, 0x3ba5e354,
		0x3be76c8b, 0x3c28f5c3, 0x3c6a7efa, 0x3cac0831, 0x3ced9168, 0x3d2f1aa0,
		0x3d70a3d7, 0x3db22d0e, 0x3df3b646, 0x3e353f7d, 0x3e76c8b4, 0x3eb851ec,
		0x3ef9db23, 0x3f3b645a, 0x3f7ced91, 0x3fbe76c9, 0x40000000, 0x40418937,
		0x4083126f, 0x40c49ba6, 0x410624dd, 0x4147ae14, 0x4189374c, 0x41cac083,
		0x420c49ba, 0x424dd2f2, 0x428f5c29, 0x42d0e560, 0x43126e98, 0x4353f7cf,
		0x43958106, 0x43d70a3d, 0x44189375, 0x445a1cac, 0x449ba5e3, 0x44dd2f1b,
		0x451eb852, 0x45604189, 0x45a1cac1, 0x45e353f8, 0x4624dd2f, 0x46666666,
		0x46a7ef9e, 0x46e978d5, 0x472b020c, 0x476c8b44, 0x47ae147b, 0x47ef9db2,
		0x483126e9, 0x4872b021, 0x48b43958, 0x48f5c28f, 0x49374bc7, 0x4978d4fe,
		0x49ba5e35, 0x49fbe76d, 0x4a3d70a4, 0x4a7ef9db, 0x4ac08312, 0x4b020c4a,
		0x4b439581, 0x4b851eb8, 0x4bc6a7f0, 0x4c083127, 0x4c49ba5e, 0x4c8b4396,
		0x4ccccccd, 0x4d0e5604, 0x4d4fdf3b, 0x4d916873, 0x4dd2f1aa, 0x4e147ae1,
		0x4e560419, 0x4e978d50, 0x4ed91687, 0x4f1a9fbe, 0x4f5c28f6, 0x4f9db22d,
		0x4fdf3b64, 0x5020c49c, 0x50624dd3, 0x50a3d70a, 0x50e56042, 0x5126e979,
		0x516872b0, 0x51a9fbe7, 0x51eb851f, 0x522d0e56, 0x526e978d, 0x52b020c5,
		0x52f1a9fc, 0x53333333, 0x5374bc6a, 0x53b645a2, 0x53f7ced9, 0x54395810,
		0x547ae148, 0x54bc6a7f, 0x54fdf3b6, 0x553f7cee, 0x55810625, 0x55c28f5c,
		0x56041893, 0x5645a1cb, 0x56872b02, 0x56c8b439, 0x570a3d71, 0x574bc6a8,
		0x578d4fdf, 0x57ced917, 0x5810624e, 0x5851eb85, 0x589374bc, 0x58d4fdf4,
		0x5916872b, 0x59581062, 0x5999999a, 0x59db22d1, 0x5a1cac08, 0x5a5e353f,
		0x5a9fbe77, 0x5ae147ae, 0x5b22d0e5, 0x5b645a1d, 0x5ba5e354, 0x5be76c8b,
		0x5c28f5c3, 0x5c6a7efa, 0x5cac0831, 0x5ced9168, 0x5d2f1aa0, 0x5d70a3d7,
		0x5db22d0e, 0x5df3b646, 0x5e353f7d, 0x5e76c8b4, 0x5eb851ec, 0x5ef9db23,
		0x5f3b645a, 0x5f7ced91, 0x5fbe76c9, 0x60000000, 0x60418937, 0x6083126f,
		0x60c49ba6, 0x610624dd, 0x6147ae14, 0x6189374c, 0x61cac083, 0x620c49ba,
		0x624dd2f2, 0x628f5c29, 0x62d0e560, 0x63126e98, 0x6353f7cf, 0x63958106,
		0x63d70a3d, 0x64189375, 0x645a1cac, 0x649ba5e3, 0x64dd2f1b, 0x651eb852,
		0x65604189, 0x65a1cac1, 0x65e353f8, 0x6624dd2f, 0x66666666, 0x66a7ef9e,
		0x66e978d5, 0x672b020c, 0x676c8b44, 0x67ae147b, 0x67ef9db2, 0x683126e9,
		0x6872b021, 0x68b43958, 0x68f5c28f, 0x69374bc7, 0x6978d4fe, 0x69ba5e35,
		0x69fbe76d, 0x6a3d70a4, 0x6a7ef9db, 0x6ac08312, 0x6b020c4a, 0x6b439581,
		0x6b851eb8, 0x6bc6a7f0, 0x6c083127, 0x6c49ba5e, 0x6c8b4396, 0x6ccccccd,
		0x6d0e5604, 0x6d4fdf3b, 0x6d916873, 0x6dd2f1aa, 0x6e147ae1, 0x6e560419,
		0x6e978d50, 0x6ed91687, 0x6f1a9fbe, 0x6f5c28f6, 0x6f9db22d, 0x6fdf3b64,
		0x7020c49c, 0x70624dd3, 0x70a3d70a, 0x70e56042, 0x7126e979, 0x716872b0,
		0x71a9fbe7, 0x71eb851f, 0x722d0e56, 0x726e978d, 0x72b020c5, 0x72f1a9fc,
		0x73333333, 0x7374bc6a, 0x73b645a2, 0x73f7ced9, 0x74395810, 0x747ae148,
		0x74bc6a7f, 0x74fdf3b6, 0x753f7cee, 0x75810625, 0x75c28f5c, 0x76041893,
		0x7645a1cb, 0x76872b02, 0x76c8b439, 0x770a3d71, 0x774bc6a8, 0x778d4fdf,
		0x77ced917, 0x7810624e, 0x7851eb85, 0x789374bc, 0x78d4fdf4, 0x7916872b,
		0x79581062, 0x7999999a, 0x79db22d1, 0x7a1cac08, 0x7a5e353f, 0x7a9fbe77,
		0x7ae147ae, 0x7b22d0e5, 0x7b645a1d, 0x7ba5e354, 0x7be76c8b, 0x7c28f5c3,
		0x7c6a7efa, 0x7cac0831, 0x7ced9168, 0x7d2f1aa0, 0x7d70a3d7, 0x7db22d0e,
		0x7df3b646, 0x7e353f7d, 0x7e76c8b4, 0x7eb851ec, 0x7ef9db23, 0x7f3b645a,
		0x7f7ced91, 0x7fbe76c9, 0x80000000, 0x80418937, 0x8083126f, 0x80c49ba6,
		0x810624dd, 0x8147ae14, 0x8189374c, 0x81cac083, 0x820c49ba, 0x824dd2f2,
		0x828f5c29, 0x82d0e560, 0x83126e98, 0x8353f7cf, 0x83958106, 0x83d70a3d,
		0x84189375, 0x845a1cac, 0x849ba5e3, 0x84dd2f1b, 0x851eb852, 0x85604189,
		0x85a1cac1, 0x85e353f8, 0x8624dd2f, 0x86666666, 0x86a7ef9e, 0x86e978d5,
		0x872b020c, 0x876c8b44, 0x87ae147b, 0x87ef9db2, 0x883126e9, 0x8872b021,
		0x88b43958, 0x88f5c28f, 0x89374bc7, 0x8978d4fe, 0x89ba5e35, 0x89fbe76d,
		0x8a3d70a4, 0x8a7ef9db, 0x8ac08312, 0x8b020c4a, 0x8b439581, 0x8b851eb8,
		0x8bc6a7f0, 0x8c083127, 0x8c49ba5e, 0x8c8b4396, 0x8ccccccd, 0x8d0e5604,
		0x8d4fdf3b, 0x8d916873, 0x8dd2f1aa, 0x8e147ae1, 0x8e560419, 0x8e978d50,
		0x8ed91687, 0x8f1a9fbe, 0x8f5c28f6, 0x8f9db22d, 0x8fdf3b64, 0x9020c49c,
		0x90624dd3, 0x90a3d70a, 0x90e56042, 0x9126e979, 0x916872b0, 0x91a9fbe7,
		0x91eb851f, 0x922d0e56, 0x926e978d, 0x92b020c5, 0x92f1a9fc, 0x93333333,
		0x9374bc6a, 0x93b645a2, 0x93f7ced9, 0x94395810, 0x947ae148, 0x94bc6a7f,
		0x94fdf3b6, 0x953f7cee, 0x95810625, 0x95c28f5c, 0x96041893, 0x9645a1cb,
		0x96872b02, 0x96c8b439, 0x970a3d71, 0x974bc6a8, 0x978d4fdf, 0x97ced917,
		0x9810624e, 0x9851eb85, 0x989374bc, 0x98d4fdf4, 0x9916872b, 0x99581062,
		0x9999999a, 0x99db22d1, 0x9a1cac08, 0x9a5e353f, 0x9a9fbe77, 0x9ae147ae,
		0x9b22d0e5, 0x9b645a1d, 0x9ba5e354, 0x9be76c8b, 0x9c28f5c3, 0x9c6a7efa,
		0x9cac0831, 0x9ced9168, 0x9d2f1aa0, 0x9d70a3d7, 0x9db22d0e, 0x9df3b646,
		0x9e353f7d, 0x9e76c8b4, 0x9eb851ec, 0x9ef9db23, 0x9f3b645a, 0x9f7ced91,
		0x9fbe76c9, 0xa0000000, 0xa0418937, 0xa083126f, 0xa0c49ba6, 0xa10624dd,
		0xa147ae14, 0xa189374c, 0xa1cac083, 0xa20c49ba, 0xa24dd2f2, 0xa28f5c29,
		0xa2d0e560, 0xa3126e98, 0xa353f7cf, 0xa3958106, 0xa3d70a3d, 0xa4189375,
		0xa45a1cac, 0xa49ba5e3, 0xa4dd2f1b, 0xa51eb852, 0xa5604189, 0xa5a1cac1,
		0xa5e353f8, 0xa624dd2f, 0xa6666666, 0xa6a7ef9e, 0xa6e978d5, 0xa72b020c,
		0xa76c8b44, 0xa7ae147b, 0xa7ef9db2, 0xa83126e9, 0xa872b021, 0xa8b43958,
		0xa8f5c28f, 0xa9374bc7, 0xa978d4fe, 0xa9ba5e35, 0xa9fbe76d, 0xaa3d70a4,
		0xaa7ef9db, 0xaac08312, 0xab020c4a, 0xab439581, 0xab851eb8, 0xabc6a7f0,
		0xac083127, 0xac49ba5e, 0xac8b4396, 0xaccccccd, 0xad0e5604, 0xad4fdf3b,
		0xad916873, 0xadd2f1aa, 0xae147ae1, 0xae560419, 0xae978d50, 0xaed91687,
		0xaf1a9fbe, 0xaf5c28f6, 0xaf9db22d, 0xafdf3b64, 0xb020c49c, 0xb0624dd3,
		0xb0a3d70a, 0xb0e56042, 0xb126e979, 0xb16872b0, 0xb1a9fbe7, 0xb1eb851f,
		0xb22d0e56, 0xb26e978d, 0xb2b020c5, 0xb2f1a9fc, 0xb3333333, 0xb374bc6a,
		0xb3b645a2, 0xb3f7ced9, 0xb4395810, 0xb47ae148, 0xb4bc6a7f, 0xb4fdf3b6,
		0xb53f7cee, 0xb5810625, 0xb5c28f5c, 0xb6041893, 0xb645a1cb, 0xb6872b02,
		0xb6c8b439, 0xb70a3d71, 0xb74bc6a8, 0xb78d4fdf, 0xb7ced917, 0xb810624e,
		0xb851eb85, 0xb89374bc, 0xb8d4fdf4, 0xb916872b, 0xb9581062, 0xb999999a,
		0xb9db22d1, 0xba1cac08, 0xba5e353f, 0xba9fbe77, 0xbae147ae, 0xbb22d0e5,
		0xbb645a1d, 0xbba5e354, 0xbbe76c8b, 0xbc28f5c3, 0xbc6a7efa, 0xbcac0831,
		0xbced9168, 0xbd2f1aa0, 0xbd70a3d7, 0xbdb22d0e, 0xbdf3b646, 0xbe353f7d,
		0xbe76c8b4, 0xbeb851ec, 0xbef9db23, 0xbf3b645a, 0xbf7ced91, 0xbfbe76c9,
		0xc0000000, 0xc0418937, 0xc083126f, 0xc0c49ba6, 0xc10624dd, 0xc147ae14,
		0xc189374c, 0xc1cac083, 0xc20c49ba, 0xc24dd2f2, 0xc28f5c29, 0xc2d0e560,
		0xc3126e98, 0xc353f7cf, 0xc3958106, 0xc3d70a3d, 0xc4189375, 0xc45a1cac,
		0xc49ba5e3, 0xc4dd2f1b, 0xc51eb852, 0xc5604189, 0xc5a1cac1, 0xc5e353f8,
		0xc624dd2f, 0xc6666666, 0xc6a7ef9e, 0xc6e978d5, 0xc72b020c, 0xc76c8b44,
		0xc7ae147b, 0xc7ef9db2, 0xc83126e9, 0xc872b021, 0xc8b43958, 0xc8f5c28f,
		0xc9374bc7, 0xc978d4fe, 0xc9ba5e35, 0xc9fbe76d, 0xca3d70a4, 0xca7ef9db,
		0xcac08312, 0xcb020c4a, 0xcb439581, 0xcb851eb8, 0xcbc6a7f0, 0xcc083127,
		0xcc49ba5e, 0xcc8b4396, 0xcccccccd, 0xcd0e5604, 0xcd4fdf3b, 0xcd916873,
		0xcdd2f1aa, 0xce147ae1, 0xce560419, 0xce978d50, 0xced91687, 0xcf1a9fbe,
		0xcf5c28f6, 0xcf9db22d, 0xcfdf3b64, 0xd020c49c, 0xd0624dd3, 0xd0a3d70a,
		0xd0e56042, 0xd126e979, 0xd16872b0, 0xd1a9fbe7, 0xd1eb851f, 0xd22d0e56,
		0xd26e978d, 0xd2b020c5, 0xd2f1a9fc, 0xd3333333, 0xd374bc6a, 0xd3b645a2,
		0xd3f7ced9, 0xd4395810, 0xd47ae148, 0xd4bc6a7f, 0xd4fdf3b6, 0xd53f7cee,
		0xd5810625, 0xd5c28f5c, 0xd6041893, 0xd645a1cb, 0xd6872b02, 0xd6c8b439,
		0xd70a3d71, 0xd74bc6a8, 0xd78d4fdf, 0xd7ced917, 0xd810624e, 0xd851eb85,
		0xd89374bc, 0xd8d4fdf4, 0xd916872b, 0xd9581062, 0xd999999a, 0xd9db22d1,
		0xda1cac08, 0xda5e353f, 0xda9fbe77, 0xdae147ae, 0xdb22d0e5, 0xdb645a1d,
		0xdba5e354, 0xdbe76c8b, 0xdc28f5c3, 0xdc6a7efa, 0xdcac0831, 0xdced9168,
		0xdd2f1aa0, 0xdd70a3d7, 0xddb22d0e, 0xddf3b646, 0xde353f7d, 0xde76c8b4,
		0xdeb851ec, 0xdef9db23, 0xdf3b645a, 0xdf7ced91, 0xdfbe76c9, 0xe0000000,
		0xe0418937, 0xe083126f, 0xe0c49ba6, 0xe10624dd, 0xe147ae14, 0xe189374c,
		0xe1cac083, 0xe20c49ba, 0xe24dd2f2, 0xe28f5c29, 0xe2d0e560, 0xe3126e98,
		0xe353f7cf, 0xe3958106, 0xe3d70a3d, 0xe4189375, 0xe45a1cac, 0xe49ba5e3,
		0xe4dd2f1b, 0xe51eb852, 0xe5604189, 0xe5a1cac1, 0xe5e353f8, 0xe624dd2f,
		0xe6666666, 0xe6a7ef9e, 0xe6e978d5, 0xe72b020c, 0xe76c8b44, 0xe7ae147b,
		0xe7ef9db2, 0xe83126e9, 0xe872b021, 0xe8b43958, 0xe8f5c28f, 0xe9374bc7,
		0xe978d4fe, 0xe9ba5e35, 0xe9fbe76d, 0xea3d70a4, 0xea7ef9db, 0xeac08312,
		0xeb020c4a, 0xeb439581, 0xeb851eb8, 0xebc6a7f0, 0xec083127, 0xec49ba5e,
		0xec8b4396, 0xeccccccd, 0xed0e5604, 0xed4fdf3b, 0xed916873, 0xedd2f1aa,
		0xee147ae1, 0xee560419, 0xee978d50, 0xeed91687, 0xef1a9fbe, 0xef5c28f6,
		0xef9db22d, 0xefdf3b64, 0xf020c49c, 0xf0624dd3, 0xf0a3d70a, 0xf0e56042,
		0xf126e979, 0xf16872b0, 0xf1a9fbe7, 0xf1eb851f, 0xf22d0e56, 0xf26e978d,
		0xf2b020c5, 0xf2f1a9fc, 0xf3333333, 0xf374bc6a, 0xf3b645a2, 0xf3f7ced9,
		0xf4395810, 0xf47ae148, 0xf4bc6a7f, 0xf4fdf3b6, 0xf53f7cee, 0xf5810625,
		0xf5c28f5c, 0xf6041893, 0xf645a1cb, 0xf6872b02, 0xf6c8b439, 0xf70a3d71,
		0xf74bc6a8, 0xf78d4fdf, 0xf7ced917, 0xf810624e, 0xf851eb85, 0xf89374bc,
		0xf8d4fdf4, 0xf916872b, 0xf9581062, 0xf999999a, 0xf9db22d1, 0xfa1cac08,
		0xfa5e353f, 0xfa9fbe77, 0xfae147ae, 0xfb22d0e5, 0xfb645a1d, 0xfba5e354,
		0xfbe76c8b, 0xfc28f5c3, 0xfc6a7efa, 0xfcac0831, 0xfced9168, 0xfd2f1aa0,
		0xfd70a3d7, 0xfdb22d0e, 0xfdf3b646, 0xfe353f7d, 0xfe76c8b4, 0xfeb851ec,
		0xfef9db23, 0xff3b645a, 0xff7ced91, 0xffbe76c9
};


CNtpTime::CNtpTime()
{
}	

CNtpTime::CNtpTime(const SYS::TimeStamp& st)
{
	//Currently this function only operates correctly in 
	//the 1900 - 2036 primary epoch defined by NTP
	
	long JD = GetJulianDay(st.year, st.month, st.day);
	JD -= JAN_1ST_1900;
	
	//		ASSSET(JD >= 0); //NTP only supports dates greater than 1900
	uint64 Seconds = JD;
	Seconds = (Seconds * 24) + st.hour;
	Seconds = (Seconds * 60) + st.minute;
	Seconds = (Seconds * 60) + st.second;
	//		ASSSET(Seconds <= 0xFFFFFFFF); //NTP Only supports up to 2036
	m_Time = (Seconds << 32) + MsToNtpFraction(st.millisecond);
}

long CNtpTime::GetJulianDay(uint16 Year, uint16 Month, uint16 Day)
{
	long y = (long) Year;
	long m = (long) Month;
	long d = (long) Day;
	if (m > 2) 
		m = m - 3;
	else 
	{
		m = m + 9;
		y = y - 1;
	}
	long c = y / 100;
	long ya = y - 100 * c;
	long j = (146097L * c) / 4 + (1461L * ya) / 4 + (153L * m + 2) / 5 + d + 1721119L;
	return j;
}

uint32 CNtpTime::MsToNtpFraction(uint16 wMilliSeconds)
{
	//		ASSERT(wMilliSeconds < 1000);
	return m_MsToNTP[wMilliSeconds];
}

//////////////////////////////////////////////////////////////////////////
// implemetation of Environment
//////////////////////////////////////////////////////////////////////////	
Environment::Environment() : 
_pSite(NULL),
_pCommunicator(NULL), 
_pAdapter(NULL), 
_pFreezeConnection(NULL), 
_pContextEvictor(NULL), 
_pFactory(NULL), 
_pStreamIdx(NULL), 
_pWatchDog(NULL),
_streamEventSinker(0),
_mmib(_fileLog, 1000, 6),
_snmpSA(_fileLog, _mmib, 5000),
_eventDispatcher(NULL)
{
	_iceOverrideTimeout = -1;
	_lastEventReceivedTime = 0;
//	_sysLog.open(ZQ_PRODUCT_NAME, ZQ::common::Log::L_WARNING);
}

Environment::~Environment()
{
	if(_streamEventSinker)
	{
		delete _streamEventSinker;
		_streamEventSinker = 0;
	}
}

bool Environment::addCacheSess(const std::string sessionId,SessStatus sessStatus)
{
	ZQ::common::MutexGuard gd(_lkCacheSess);
	bool isSucceed = false;
	if (!sessionId.empty())
	{
		isSucceed = true;
		_sessStatusMap[sessionId] = sessStatus;
	}

	return isSucceed;
}

bool Environment::removeCachedSess(const std::string sessionId)
{
	ZQ::common::MutexGuard gd(_lkCacheSess);
	SessStatusMap::iterator iter = _sessStatusMap.find(sessionId);
	if(!sessionId.empty()
		&&iter == _sessStatusMap.end())
	{
		return false;
	}
	_sessStatusMap.erase(sessionId);
	return true;
}

bool Environment::findSession(const std::string sessionId)
{
	ZQ::common::MutexGuard gd(_lkCacheSess);
	SessStatusMap::iterator iter = _sessStatusMap.find(sessionId);
	if(iter == _sessStatusMap.end())
		return false;

	return true;
}

bool Environment::getSessStatus(const std::string sessionId, Environment::SessStatus& ssOut)
{
	ZQ::common::MutexGuard gd(_lkCacheSess);
	bool isSucceed = false;
	SessStatusMap::iterator iter = _sessStatusMap.find(sessionId);
	if (!sessionId.empty() && iter != _sessStatusMap.end())
	{
		isSucceed = true;
		ssOut = iter->second;
	}

	return isSucceed;
}

bool Environment::doInit(const char* cfgPath, IStreamSmithSite* pSite)
{
	_pSite = pSite;
	
	_cfgDir = NULL != cfgPath ? cfgPath : "";
	_cfgDir += FNSEPS MODULE_NAME ".xml";

	try
	{
		_fileLog.open((_logDir + MODULE_NAME ".log").c_str(), ZQ::common::Log::L_INFO);
		_sessLog.open((_logDir + MODULE_NAME ".sess.log").c_str(), ZQ::common::Log::L_INFO);
	}
	catch (ZQ::common::FileLogException&)
	{
//		_sysLog(ErrorLevel, "create log caught %s", ex.getString());
		return false;
	}

	s1log = &_fileLog;

	_liveChannelConfig.setLogger(s1log);
	
	if (false == loadConfig())
		return false;

	_fileLog.setFileSize(_liveChannelConfig._pluginLog._size);
	_fileLog.setFileCount(_liveChannelConfig._pluginLog._maxCount);
	_fileLog.setLevel(_liveChannelConfig._pluginLog._level);
	_fileLog.setBufferSize(_liveChannelConfig._pluginLog._bufferSize);
	
	if (false == initIceRuntime())
		return false;
	
	// session watch dog
	_pWatchDog = new WatchDog(*this);
	_pWatchDog->start();
	
	// connect service thread which used to connect Weiwoo service and IceStorm service
	_pConnectService = new ConnectService(*this);
	_pConnectService->start();
	
	if (false == openSafeStore())
		return false;
	
	try
	{
		SessionViewImplPtr pSessionView = new SessionViewImpl(*this);
		_pAdapter->ZQADAPTER_ADD(_pCommunicator, pSessionView, "SessionView");
		SSMLOG(NoticeLevel, CLOGFMT(Environment, "Register SessionView successfully"));
	}
	catch (const Ice::Exception& ex)
	{
		SSMLOG(WarningLevel, CLOGFMT(Environment, "Register SessionView caught %s"), ex.ice_name().c_str());
	}
	catch (...)
	{
		SSMLOG(WarningLevel, CLOGFMT(Environment, "Register SessionView caught exception"));
	}

//	_liveChannelConfig.snmpRegister("ssm_NGOD2");
	

	{
		//publish log and syntax file
		LiveChannelConfig::LogPublishs::const_iterator it  = _liveChannelConfig._logpublish.begin();
		for( ; it != _liveChannelConfig._logpublish.end() ; it ++ )
		{
			if(!_pAdapter->publishLogger( it->logPath.c_str() , it->syntaxPath.c_str() ,
				it->key.c_str()  , it->type.c_str() ) )
			{
				SSMLOG(ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] synax[%s] key[%s] type[%s]", 					
					it->logPath.c_str(), it->syntaxPath.c_str(), it->key.c_str() , it->type.c_str() );
			}
			else
			{
				SSMLOG(ZQ::common::Log::L_INFO, "Publish logger name[%s] synax[%s] key[%s] type[%s] successful", 					
					it->logPath.c_str(), it->syntaxPath.c_str() ,it->key.c_str( ) ,it->type.c_str() );				
			}
		}
	}

	regesterSnmp(pSite);

	//add cache interval for bug 17998
	_cacheInterval = _liveChannelConfig._rtspSession._timeout /3 *1000;
	if (_cacheInterval < GETPARA_MININTERVAL)
		_cacheInterval = GETPARA_MININTERVAL;
	if (_cacheInterval > _liveChannelConfig._rtspSession._timeout * 800)
		_cacheInterval = _liveChannelConfig._rtspSession._timeout * 800;

	int32	size = _liveChannelConfig._rtspSession._pingCacheSize;
	if(size < GETPARA_MINCACHESIZE)
		size = GETPARA_MINCACHESIZE;
	_sessStatusMap.resize(size);

	return true;
}

bool Environment::regesterSnmp(IStreamSmithSite* pSite)
{
    //_mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<Environment, uint32>("rtspLogS1",   *this, ZQ::SNMP::AsnType_Int32, &LiveChannel::Environment::snmp_getLogLevel_Main, &LiveChannel::Environment::snmp_setLogLevel_Main));
    //_mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<Environment, uint32>("rtspLogIcetrace",   *this, ZQ::SNMP::AsnType_Int32, &LiveChannel::Environment::snmp_getLogLevel_Ice, &LiveChannel::Environment::snmp_setLogLevel_Ice));

    //_snmpSA.start();
    return true;
}

void Environment::unregisterSnmp()
{
}

void Environment::doUninit()
{
	closeSafeStore();
	
	if (NULL != _pConnectService)
		delete _pConnectService; // destructor will call stop() implictly
	_pConnectService = NULL;
	
	if (NULL != _pWatchDog)
		delete _pWatchDog; // destructor will call stop() implictly
	_pWatchDog = NULL;
	
	uninitIceRuntime();
	unregisterSnmp();
}

RequestProcessResult Environment::doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	// validate input param
	if (NULL == pSite)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "site is null"));
		throw "site is null";
	}
	
	if (NULL == pReq)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "request is null"));
		throw "request is null";
	}
	
	IServerResponse* pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "response is null"));
		throw "response is null";
	}
	
	if (_liveChannelConfig._response._maxResponseTimeout >0)
	{
		int64 stampStart = atoll(pReq->getContext("SYS.RequestTime"));
		int64 stampNow = ZQ::common::now();
		if (stampStart >0 && stampStart + _liveChannelConfig._response._maxResponseTimeout < stampNow)
		{
			SSMLOG(ErrorLevel, CLOGFMT(Environment, "Req(%p:FixupRequest) expired after queue, created[%lld +%d] now[%lld]"),
				pReq, stampStart, _liveChannelConfig._response._maxResponseTimeout, stampNow);
			pResponse->printf_preheader("RTSP/1.0 503 Service Unavailable");
			pResponse->setHeader(HeaderTianShanNotice, "");
			pResponse->post();
			return RequestDone;
		}
	}

	
	assert( pReq != 0 );
	assert( pResponse != 0 );
	FixupRequest::Ptr pFixupRequest = NULL;
	switch (pReq->getVerb())
	{
	case RTSP_MTHD_SETUP:			pFixupRequest = new FixupSetup(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PLAY:			pFixupRequest = new FixupPlay(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PAUSE:			pFixupRequest = new FixupPause(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_TEARDOWN:		pFixupRequest = new FixupTeardown(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_GET_PARAMETER:	pFixupRequest = new FixupGetParam(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_DESCRIBE:		pFixupRequest = new FixupDescribe(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_OPTIONS:			pFixupRequest = new FixupOption(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PING:			pFixupRequest = new FixupPing(*this, pSite, pReq, pResponse); break;

	case RTSP_MTHD_RESPONSE:
		{
			char szStartline[512];szStartline[sizeof(szStartline)-1] = 0;
			pReq->getStartline(szStartline,sizeof(szStartline)-1 );
			char szSeq[512];
			szSeq[sizeof(szSeq)-1] = 0;
			uint16 seqLen = sizeof(szSeq)-1;
			pReq->getHeader("CSeq",szSeq,&seqLen);
			szSeq[seqLen] = 0;

			SSMLOG(InfoLevel,CLOGFMT(Environment,"doFixupRequest() got a rtsp response message, just skip it: sess[%s] seq[%s] startline[%s]"),
				pReq->getClientSessionId(), szSeq, szStartline);
			return RequestDone;
			break;
		}

	default: 
		pResponse->printf_preheader(ResponseMethodNotAllowed);
		pResponse->post();
		return RequestDone;
	}
	
	/*SmartRequestHandler smart(pFixupRequest);*/
	try 
	{
		if (false == pFixupRequest->process())
			return RequestUnrecognized; // request done, need not modify response

		return RequestPhaseDone; // fixup request has been processed, need not process in the same phase, be ready to processed in content handler
	}
	catch (...)
	{
		pFixupRequest->_bProcessSuccessfully = false;
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "Req(%s:FixupRequest) caught exception"),
			pFixupRequest->getReqIdent().c_str());
		pResponse->printf_preheader(ResponseUnexpectServerError);
		pFixupRequest->postResponse(); // fixup request error, send response directory
		return RequestDone; // request done, need not modify response
	}
}

RequestProcessResult Environment::doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	// validata input param
	if (NULL == pSite)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "site is null"));
		throw "site is null";
	}
	
	if (NULL == pReq)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "request is null"));
		throw "request is null";
	}
	
	IServerResponse* pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "response is null"));
		throw "response is null";
	}
	
	if (_liveChannelConfig._response._maxResponseTimeout >0)
	{
		int64 stampStart = atoi(pReq->getContext("SYS.RequestTime"));
		int64 stampNow = ZQ::common::now();
		if (stampStart >0 && stampStart + _liveChannelConfig._response._maxResponseTimeout < stampNow)
		{
			SSMLOG(ErrorLevel, CLOGFMT(Environment, "Req(%p:doContentHandler) expired after queue, created[%lld +%d] now[%lld]"),
				pReq, stampStart, _liveChannelConfig._response._maxResponseTimeout, stampNow);

			pResponse->printf_preheader("RTSP/1.0 503 Service Unavailable");
			pResponse->setHeader(HeaderTianShanNotice, "");
			pResponse->post();
			return RequestDone;
		}
	}

	ContentHandler::Ptr pContentHandler = NULL;
	RTSP_VerbCode verbCode = pReq->getVerb();
	switch (verbCode)
	{
	case RTSP_MTHD_SETUP:			pContentHandler = new HandleSetup(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PLAY:			pContentHandler = new HandlePlay(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PAUSE:			pContentHandler = new HandlePause(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_TEARDOWN:		pContentHandler = new HandleTeardown(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_GET_PARAMETER:	pContentHandler = new HandleGetParam(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_DESCRIBE:		pContentHandler = new HandleDescribe(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_OPTIONS:			pContentHandler = new HandleOption(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PING:			pContentHandler = new HandlePing(*this, pSite, pReq, pResponse); break;

	default: 
		pResponse->printf_preheader(ResponseMethodNotAllowed);
		pResponse->post();
		return RequestDone;
	}
	
	//SmartRequestHandler smart(pContentHandler);
	try 
	{
		bool ret = pContentHandler->process();
		switch(verbCode)
		{
		case RTSP_MTHD_SETUP:
			{
				//TODO: this line should go to event
				_sessLog(ZQ::common::Log::L_NOTICE, CLOGFMT(RTSPSession, "sess[%s] SETUP[%d]: %s"), pContentHandler->getSession().c_str(), pContentHandler->responseStatusCode(), pContentHandler->eventParamLine().c_str());

				if (false == ret)
				{
					// if setup failed, make sure the RtspProxy session to be destroyed
					if (_pSite->destroyClientSession(pContentHandler->getSession().c_str()))
						SSMLOG(InfoLevel, CLOGFMT(Environment, "RtspProxySessioin(%s) destroyed, reason: Setup Failed"), pContentHandler->getSession().c_str());
				}
			}
			break;

		case RTSP_MTHD_TEARDOWN:
			{
				//TODO: this line should go to event
				_sessLog(ZQ::common::Log::L_NOTICE, CLOGFMT(RTSPSession, "sess[%s] TEARDOWN[%d]: %s"), pContentHandler->getSession().c_str(), pContentHandler->responseStatusCode(), pContentHandler->eventParamLine().c_str());
			}
			break;
		}

		// whether request has been processed successfully, modify the response according the return value of needModifyResponse function.
		if (pContentHandler->getReturnType() == RequestHandler::RETURN_ASYNC)
			return RequestDone; // request has been processed absolutely, need not any other process			

		if (true == pContentHandler->needModifyResponse())
				return RequestProcessed;

		pContentHandler->postResponse(); // need not modify response, post directly
		return RequestDone; // request has been processed absolutely, need not any other process
	}
	catch (...)
	{
		// if setup failed, make sure the RtspProxy session to be destroyed
		if (RTSP_MTHD_SETUP == verbCode && _pSite->destroyClientSession(pContentHandler->getSession().c_str()))
			SSMLOG(InfoLevel, CLOGFMT(Environment, "RtspProxySessioin(%s) destroyed, reason: Setup Failed"), pContentHandler->getSession().c_str());
		
		pContentHandler->_bProcessSuccessfully = false;
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "session(%s), phase(%s), process(%s) caught exception"), pContentHandler->getSession().c_str(), pContentHandler->getPhase().c_str(), pContentHandler->getMethod().c_str());
		pResponse->printf_preheader(ResponseUnexpectServerError);
		pResponse->post();

		return RequestDone;
	}
}

RequestProcessResult Environment::doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	// validate input param
	if (NULL == pSite)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "site is null"));
		throw "site is null";
	}
	
	if (NULL == pReq)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "request is null"));
		throw "request is null";
	}
	
	IServerResponse* pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "response is null"));
		throw "response is null";
	}
	
	FixupResponse::Ptr pFixupResponse = NULL;
	switch (pReq->getVerb())
	{
	case RTSP_MTHD_SETUP:				
		pFixupResponse = new SetupResponse(*this, pSite, pReq, pResponse);
		{
			bool bShouldDestroySess = false;
			try 
			{
				if (!pFixupResponse->process())
					return RequestUnrecognized;

				if (!pFixupResponse->postResponse() )
				{//destroy the session newly created because we can't not send out response to client
					bShouldDestroySess = true;
				}
				
			}
			catch (...)
			{
				pFixupResponse->_bProcessSuccessfully = false;
				SSMLOG(ErrorLevel, CLOGFMT(Environment, "Req(%s:FixupResponse) caught exception"), pFixupResponse->getReqIdent().c_str());
				pResponse->printf_preheader(ResponseUnexpectServerError);
				if( pResponse->post() <= 0 )
				{//destroy the session newly created because we can't not send out response to client
					bShouldDestroySess = true;
				}
				
			}
			
			if( bShouldDestroySess )
			{//destroy the session because failed to send SETUP response to client
				//construct a teardown message to destroy the session
				//Hack
				
				char szSessionBuff[512] = {0};
				uint16 bufLen = sizeof(szSessionBuff);
				pResponse->getHeader("Session",szSessionBuff,&bufLen);
				
				SSMLOG(ErrorLevel,CLOGFMT(Environment,"failed to post for SETUP response for session[%s], destroy it"),szSessionBuff);
				
				IClientRequestWriter* pWriteReq = (IClientRequestWriter*)pReq;
				assert( pWriteReq != NULL );
				pWriteReq->setArgument( RTSP_MTHD_TEARDOWN,"rtsp://./?","RTSP/1.0");
				pWriteReq->setHeader("CSeq","999999999");				
				pWriteReq->setHeader("Session",szSessionBuff);
				HandleTeardown handler(*this,pSite,pWriteReq,pResponse);
				handler.process();
			}

			return RequestDone;
		}
		break;

	case RTSP_MTHD_PLAY:				pFixupResponse = new PlayResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PAUSE:				pFixupResponse = new PauseResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_TEARDOWN:			pFixupResponse = new TeardownResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_GET_PARAMETER:		pFixupResponse = new GetParamResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_DESCRIBE:			pFixupResponse = new DescribeResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_OPTIONS:				pFixupResponse = new OptionResponse(*this, pSite, pReq, pResponse); break;
	case RTSP_MTHD_PING:				pFixupResponse = new PingResponse(*this, pSite, pReq, pResponse); break;

	default: 
		pResponse->printf_preheader(ResponseMethodNotAllowed);
		pResponse->post();
		return RequestDone;
	}
	
	/*SmartRequestHandler smart(pFixupResponse);*/
	try 
	{
		pFixupResponse->process();
		pFixupResponse->postResponse(); // must post here
		return RequestDone;
	}
	catch (...)
	{
		pFixupResponse->_bProcessSuccessfully = false;
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "Req(%s:FixupResponse) caught exception")
			, pFixupResponse->getReqIdent().c_str());
		pResponse->printf_preheader(ResponseUnexpectServerError);
		pResponse->post();
		return RequestDone;
	}
}

void Environment::setLogDir(const char* logDir)
{
    if (logDir)
		_logDir = logDir;

	if (_logDir.empty())
		_logDir = ".";

	if (FNSEPC != _logDir[_logDir.length()-1])
		_logDir += FNSEPS;
}

bool Environment::loadConfig()
{
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "**************** (Load %s) ****************"), MODULE_NAME);
	if(!_liveChannelConfig.load(_cfgDir.c_str()))
    {
        SSMLOG(ZQ::common::Log::L_INFO, "load config %s failed", _cfgDir.c_str());
        return false;
    }

	if (_liveChannelConfig._rtspSession._timeout < 60)
		_liveChannelConfig._rtspSession._timeout = 60;

	if (_liveChannelConfig._rtspSession._monitorThreads < 3)
		_liveChannelConfig._rtspSession._monitorThreads = 3;

	if (_liveChannelConfig._response._maxFieldLen < 50)
		_liveChannelConfig._response._maxFieldLen = 50;

	if (_liveChannelConfig._response._maxResponseTimeout >0 && _liveChannelConfig._response._maxResponseTimeout < 1000)
		_liveChannelConfig._response._maxResponseTimeout = 1000;

	//check if there is Ice Override timeout
	std::vector<IceProperty::IcePropertyHolder>& iceProps = _liveChannelConfig._iceProps._propDatas;
	std::vector<IceProperty::IcePropertyHolder>::const_iterator itIceTimeOut = iceProps.begin() ;
	
	for( ; itIceTimeOut != iceProps.end() ; itIceTimeOut ++ )
	{
		if( itIceTimeOut->_name == "Ice.Override.Timeout" )
		{
			if( !itIceTimeOut->_value.empty() )
			{
				_iceOverrideTimeout = atoi( itIceTimeOut->_value.c_str() );
				SSMLOG(ZQ::common::Log::L_INFO,"get ice override timeout [%d]",_iceOverrideTimeout);
			}
		}
	}

	std::vector<float>& forwardSpeeds = _liveChannelConfig._fixedSpeedSet.forwardSpeeds;
	std::string strForwardSpeed = _liveChannelConfig._fixedSpeedSet.strForwardSpeeds;
	std::vector<std::string> temps;
	ZQ::common::stringHelper::SplitString(strForwardSpeed,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
	{
		forwardSpeeds.push_back((float)atof(it->c_str()));
		SSMLOG(ZQ::common::Log::L_INFO,"get a forward speed [%s]",it->c_str());
	}

	std::vector<float>& backwardSpeeds = _liveChannelConfig._fixedSpeedSet.backwardSpeeds;
	std::string strBackwardSpeed = _liveChannelConfig._fixedSpeedSet.strBackwardSpeeds;	
	ZQ::common::stringHelper::SplitString(strBackwardSpeed,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
	{
		backwardSpeeds.push_back((float)atof(it->c_str()));
		SSMLOG(ZQ::common::Log::L_INFO,"get a backward speed [%s]",it->c_str());
	}

	SSMLOG(ZQ::common::Log::L_INFO," [%s] FixedSpeedSet",_liveChannelConfig._fixedSpeedSet.enable ? "enable":"disable");
	return true;
}

bool Environment::initIceRuntime()
{
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do initIceRuntime()"));
	try
	{
		// DO: initialize properties for ice run-time
		int i=0;
		Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
		if (props)
		{
		std::vector<IcePropertyHolder>::iterator it;
		for (it = _liveChannelConfig._iceProps._propDatas.begin(); it != _liveChannelConfig._iceProps._propDatas.end(); it ++)
			props->setProperty(it->_name.c_str(), it->_value.c_str());
		}
		
		// DO: create communicator
		SSMLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Environment, "do create communicator"));
		Ice::InitializationData idt;
		idt.properties = props;

		std::string iceLogDir = _logDir + FNSEPS MODULE_NAME ".icetrace.log";
		if( _liveChannelConfig._iceTrace._enabled >= 1 )
		{
			try
			{
				_iceFileLog.open(iceLogDir.c_str(),
								_liveChannelConfig._iceTrace._level,	
								_liveChannelConfig._iceTrace._maxCount,
								_liveChannelConfig._iceTrace._size);
				_iceLogger = new TianShanIce::common::IceLogI(&_iceFileLog);
				idt.logger = _iceLogger;
			}
			catch( const ZQ::common::FileLogException&  )
			{
				SSMLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "failed to create ice trace file log"));
			}
		}

		_pCommunicator=Ice::initialize(idt);
		SSMLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "communicator created successfully"));
	}
	catch(const Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create communicator caught(%s)"), ex.ice_name().c_str());
		return false;
	}
	
	try
	{
		_pAdapter = ZQADAPTER_CREATE(_pCommunicator, "LocalAdapter", _liveChannelConfig._bind._endpoint.c_str(), SSMLOG);
		_pAdapter->activate();
	}
	catch(Ice::Exception& ex) 
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create adapter(%s) caught(%s)"), _liveChannelConfig._bind._endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave initIceRuntime()"));
	return true;
}
	
void Environment::uninitIceRuntime()
{
	if(_streamEventSinker)
	{
		_streamEventSinker->stop();
	}
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do uninitIceRuntime()"));
	try
	{
		if (_pAdapter)
		{
			SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do _pAdapter->deactivate()"));
			_pAdapter->deactivate();
			SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "_pAdapter->deactivate() successfully"));
		}
		_pAdapter = NULL;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "deactive adapters caught(%s)"), ex.ice_name().c_str());
	}
	
	try
	{
		if (_pCommunicator)
		{
			SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do _pCommunicator->deactivate()"));
			_pCommunicator->destroy();
			SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "_pCommunicator->deactivate() successfully"));
		}
		_pCommunicator = NULL;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "deactive adapters caught(%s)"), ex.ice_name().c_str());
	}
	
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave uninitIceRuntime()"));
}

void Environment::updateDbEnvConfig( const std::string& env, const std::string& key, const std::string& value )
{
    static std::string prefix = "Freeze.DbEnv.";

    std::string strProp = prefix+env+"."+key;
    Ice::PropertiesPtr props = _pCommunicator->getProperties();

    props->setProperty( strProp , value );
    SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(Environment,"updateDbEnvConfig() key[%s] value[%s]"),strProp.c_str() , value.c_str() );
}

void Environment::updateDbFileConfig( const std::string& env, const std::string& file ,const std::string& key, const std::string& value )
{
    static std::string prefix = "Freeze.Evictor.";

    std::string strProp = prefix+env+"."+file+"."+key;
    Ice::PropertiesPtr props = _pCommunicator->getProperties();

    props->setProperty( strProp , value );
    SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(Environment,"updateDbFileConfig() key[%s] value[%s]"), strProp.c_str() , value.c_str() );
}

bool Environment::openSafeStore()
{
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do openSafeStore()"));
    //use "runtimePath" as db path, namely, move db from "TianShan/data" to "TianShan/data/runtime"
    //bug#20384
    if (_liveChannelConfig._database._runtimePath.empty())
    {
        _liveChannelConfig._database._runtimePath = _liveChannelConfig._database._path;
    }

	_liveChannelConfig._database._runtimePath += FNSEPS MODULE_NAME;
	std::string pathStr(_liveChannelConfig._database._runtimePath);

    if(!FS::createDirectory(pathStr, true)) {
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "failed to create database path (%s)"), pathStr.c_str());
        return false;
    }	

	// preparing the DB_CONFIG file
	std::string dbConfFile = pathStr + "/DB_CONFIG";
	if ( -1 == access(dbConfFile.c_str(), 0))
	{
		SSMLOG(InfoLevel, CLOGFMT(Environment, "initializing %s"), dbConfFile.c_str());
		FILE* f = ::fopen(dbConfFile.c_str(), "w+");
		if (NULL != f)
		{
			::fprintf(f, "set_lk_max_locks %ld\n",   10000);
			::fprintf(f, "set_lk_max_objects %ld\n", 10000);
			::fprintf(f, "set_lk_max_lockers %ld\n", 10000);
			::fprintf(f, "set_cachesize 0 %ld 0\n",  16384*1024);
			::fclose(f);
		}
	}

	try
	{
		_pFactory = new Factory(*this);
		_pCommunicator->addObjectFactory(_pFactory, SessionContext::ice_staticId());
		_pFreezeConnection = Freeze::createConnection(_pCommunicator, pathStr);
	}
	catch (Freeze::DatabaseException& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create freeze connection caught(%s: %s)"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create freeze connection caught(%s)"), ex.ice_name().c_str());
		return false;
	}
	
	try
	{
		_pStreamIdx = new LiveChannel::StreamIdx(INDEXFILENAME(StreamIdx));
		::std::vector<Freeze::IndexPtr> indexs;
		indexs.push_back(_pStreamIdx);

        updateDbEnvConfig(pathStr,"DbPrivate","0");
        updateDbEnvConfig(pathStr ,"DbRecoverFatal",_liveChannelConfig._database._fatalRecover );
        updateDbEnvConfig(pathStr,"CheckpointPeriod",_liveChannelConfig._database._checkpointPeriod);
        updateDbFileConfig(pathStr,"SessionContexts","SaveSizeTrigger",_liveChannelConfig._database._saveSizeTrigger);
        updateDbFileConfig(pathStr,"SessionContexts","SavePeriod",_liveChannelConfig._database._savePeriod);

		ZQ::common::MutexGuard lk(_lockEvictor);

#if ICE_INT_VERSION / 100 >= 303
		_pContextEvictor = ::Freeze::createBackgroundSaveEvictor(_pAdapter, _liveChannelConfig._database._runtimePath.c_str(), "SessionContexts", 0, indexs);
#else
		_pContextEvictor = ::Freeze::createEvictor(_pAdapter, _liveChannelConfig._database._runtimePath.c_str(), "SessionContexts", 0, indexs);
#endif
		_pContextEvictor->setSize(_liveChannelConfig._rtspSession._cacheSize);
		_pAdapter->addServantLocator(_pContextEvictor, ServantType);
	}
	catch (Freeze::DatabaseException& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create freeze evictor caught(%s: %s)"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "create freeze evictor caught(%s)"), ex.ice_name().c_str());
		return false;
	}
	
	int nSessionNum = 0;
	::Freeze::EvictorIteratorPtr tItor = _pContextEvictor->getIterator("", 20000);
	while (tItor->hasNext())
	{
		int64 rn_value = (int64) _liveChannelConfig._rtspSession._timeout * 1000 + 60000 + (20 * nSessionNum++);
		Ice::Identity ident = tItor->next();
		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		TianShanIce::SRM::SessionPrx srvrSessPrx = NULL;
		if (true == openSessionCtx(ident, cltSessPrx, cltSessCtx)
			&& true == getWeiwooSessionPrx(srvrSessPrx, cltSessCtx.srvrSessPrxID)
			&& true == renewWeiwooSession(srvrSessPrx, cltSessCtx.srvrSessPrxID, rn_value))
		{
			_pSite->createClientSession(ident.name.c_str(), cltSessCtx.url.c_str());
		}
		//check the stream instance 
		bool bExist = true;
		{
			try
			{
				TianShanIce::Streamer::StreamPrx prx = 	TianShanIce::Streamer::StreamPrx::checkedCast(_pCommunicator->stringToProxy(cltSessCtx.streamPrxID) );
			}
			catch( const Ice::ObjectNotExistException&)
			{
				bExist = false;
			}
			catch(...)
			{
			}
		}

		if(bExist)
		{
			_pWatchDog->watch(ident.name, rn_value - 60000);
		}
		else
		{
			try
			{
				cltSessPrx->onTimer();
			}
			catch(...)
			{

			}
		}
		
	}
	
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave openSafeStore()"));
	return true;
}

void Environment::closeSafeStore()
{
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do closeSafeStore()"));
	if (_pFreezeConnection)
	{
		try
		{
			_pFreezeConnection->close();
		}
		catch (::Freeze::DatabaseException& ex)
		{
			SSMLOG(ErrorLevel, CLOGFMT(Environment, "close freeze connection caught(%s: %s)"), ex.ice_name().c_str(), ex.message.c_str());
		}
	}

	_pContextEvictor   = NULL; 
	_pFreezeConnection = NULL;
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave closeSafeStore()"));
}

bool Environment::connectIceStorm()
{
	SSMLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do connectIceStorm()"));
	try
	{
		_eventDispatcher = new StreamEventDispatcher(*this);
		_streamEventSinker = new StreamEventSinker( *this, _pAdapter );
		TianShanIce::Streamer::StreamEventSinkPtr _evtStream = new StreamEvent(*this, *_eventDispatcher);
		TianShanIce::Streamer::PlaylistEventSinkPtr _evtPlaylist = new PlaylistEvent(*this, *_eventDispatcher);
		TianShanIce::Properties qos;
		_streamEventSinker->addEventHandler(_evtStream);
		_streamEventSinker->addEventHandler(_evtPlaylist);		
		return _streamEventSinker->start(_liveChannelConfig._iceStorm._endpoint);
	}
	catch (TianShanIce::BaseException& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "connectIceStorm(%s) caught(%s: %s)")
			, _liveChannelConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "connectIceStorm(%s) caught(%s)"),
			_liveChannelConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str());
			return false;
	}

	return true;
}

bool Environment::getWeiwooSessionPrx(TianShanIce::SRM::SessionPrx& srvrSessPrx, const std::string& srvrSessPrxID)
{
	try
	{
		srvrSessPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(_pCommunicator->stringToProxy(srvrSessPrxID));
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "getWeiwooSessionPrx(%s) caught(%s)"),
			srvrSessPrxID.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool Environment::renewWeiwooSession(const TianShanIce::SRM::SessionPrx& srvrSessPrx, const std::string& srvrSessPrxID, const int64& rn_time)
{
	try
	{
		srvrSessPrx->renew(rn_time);
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "renewWeiwooSession(%s) caught(%s)"), srvrSessPrxID.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool Environment::getPurchasePrx(TianShanIce::Application::PurchasePrx& purchasePrx, const std::string& purchasePrxID)
{
	try
	{
		purchasePrx = TianShanIce::Application::PurchasePrx::uncheckedCast(_pCommunicator->stringToProxy(purchasePrxID));
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "getPurchasePrx(%s) caught(%s)")
			, purchasePrxID.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool Environment::getPlaylistPrx(TianShanIce::Streamer::PlaylistPrx& playlistPrx, const std::string& streamPrxID)
{
	try
	{
		playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_pCommunicator->stringToProxy(streamPrxID));
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "getPlaylistPrx(%s) caught(%s)"), streamPrxID.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool Environment::getStreamState(const TianShanIce::Streamer::PlaylistPrx& playlistPrx, const std::string& streamPrxID, TianShanIce::Streamer::StreamState& strmState, std::string& statDept)
{
	try
	{
		strmState = playlistPrx->getCurrentState();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[%s:%04d] %s caught by stream(%s).getCurrentState"),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), streamPrxID.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[SsmEnv:0300] caught by stream(%s).getCurrentState"), 
			ex.ice_name().c_str(), streamPrxID.c_str());
		return false;
	}
	
	switch(strmState)
	{
	case TianShanIce::Streamer::stsSetup: statDept = "init"; break;
	case TianShanIce::Streamer::stsStreaming: statDept = "play"; break;
	case TianShanIce::Streamer::stsPause: statDept = "pause"; break;
	case TianShanIce::Streamer::stsStop: statDept = "stop"; break;

	default: statDept = "unknown"; break;
	}
	
	return true;
}

bool Environment::getStreamPlayInfo(const TianShanIce::Streamer::PlaylistPrx& playlist, const std::string& streamPrxID, std::string& scale, Ice::Int& iCurrentPos, Ice::Int& iTotalPos)
{
	TianShanIce::ValueMap vMap;
	
	bool bInfoSucc = false;
	try
	{
		// 这里必须是infoStreamNptPos，因为我们要取的值是相对与整个playlist头部的offset
		bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoSTREAMNPTPOS, vMap);
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[SsmEnv:0400] caught by stream(%s).getInfo"), 
			ex.ice_name().c_str(), streamPrxID.c_str());
		return false;
	}
	
	if (!bInfoSucc)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "stream(%s).getInfo() failed[SsmEnv:0401]"), 
			streamPrxID.c_str());
		return false;
	}
	
	if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
		scale = vMap["scale"].strs[0];
	if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
		iCurrentPos = vMap["playposition"].ints[0];
	if (vMap.end() != vMap.find("totalplaytime") && vMap["totalplaytime"].type == TianShanIce::vtInts && vMap["totalplaytime"].ints.size() > 0)
		iTotalPos = vMap["totalplaytime"].ints[0];
	
	return true;
}

bool Environment::getPlaylistPlayInfo(const TianShanIce::Streamer::PlaylistPrx& playlist, const std::string& streamPrxID, std::string& scale, Ice::Int& ctrlNum, Ice::Int& offset)
{
	TianShanIce::ValueMap vMap;
	
	bool bInfoSucc = false;
	try
	{
		bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoPLAYPOSITION, vMap);
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[SsmEnv:0500] caught by stream(%s).getInfo"), 
			ex.ice_name().c_str(), streamPrxID.c_str());
		return false;
	}
	
	if (!bInfoSucc)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "stream(%s).getInfo() failed[SsmEnv:0501]"), 
			streamPrxID.c_str());
		return false;
	}
	
	if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
		scale = vMap["scale"].strs[0];
	if (vMap.end() != vMap.find("ctrlnumber") && vMap["ctrlnumber"].type == TianShanIce::vtInts && vMap["ctrlnumber"].ints.size() > 0)
		ctrlNum = vMap["ctrlnumber"].ints[0];
	if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
		offset = vMap["playposition"].ints[0];
	
	return true;
}

bool Environment::PlayInfo2UtcTime(const TianShanIce ::Application ::PurchasePrx& purchasePrx, const std::string& purchasePrxID, const ::Ice::Int& ctrlNum, const ::Ice::Int& offset, std::string& utcTime)
{
	STRINGVECTOR params;
	params.push_back("BcastPos");
	::TianShanIce::ValueMap inMap, outMap;
	::TianShanIce::Variant vrtUserCtrlNum, vrtOffset;
	vrtUserCtrlNum.type = TianShanIce::vtInts;
	vrtUserCtrlNum.bRange = false;
	vrtUserCtrlNum.ints.clear();
	vrtUserCtrlNum.ints.push_back(ctrlNum);
	vrtOffset.type = TianShanIce::vtInts;
	vrtOffset.bRange = false;
	vrtOffset.ints.clear();
	vrtOffset.ints.push_back(offset);
	inMap["UserCtrlNum"] = vrtUserCtrlNum;
	inMap["Offset"] = vrtOffset;
	
	try
	{
		purchasePrx->getParameters(params, inMap, outMap);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[%s:%04d] %s caught by purchase(%s).getParameters")
			, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), purchasePrxID.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[SsmEnv:0600] caught by purchase(%s).getParameters"), 
			ex.ice_name().c_str(), purchasePrxID.c_str());
		return false;
	}
	
	if (outMap.end() != outMap.find("BcastPos") && TianShanIce::vtStrings == outMap["BcastPos"].type && outMap["BcastPos"].strs.size() > 0)
		utcTime = outMap["BcastPos"].strs[0];
	
	return true;
}

// session context manager
bool Environment::openSessionCtx(const Ice::Identity& ident, SessionContextPrx& cltSessPrx, SessionData& cltSessCtx)
{
	SSMLOG(DebugLevel, CLOGFMT(Environment, "do openSessionCtx(%s)"), ident.name.c_str());
	try
	{
		cltSessPrx = SessionContextPrx::uncheckedCast(_pAdapter->createProxy(ident));
		cltSessCtx = cltSessPrx->getSessionData();
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ErrorLevel, CLOGFMT(Environment, "%s[SsmEnv:0700] caught in openSessionCtx()"), ex.ice_name().c_str());
		return false;
	}
	SSMLOG(InfoLevel, CLOGFMT(Environment, "openSessionCtx(%s) successfully"), ident.name.c_str());
	return true;
}

bool Environment::removeSessionCtx(const Ice::Identity& ident, const std::string& reason)
{	
	SSMLOG(DebugLevel, CLOGFMT(Environment, "do removeSessionCtx(%s), reason: %s"), ident.name.c_str(), reason.c_str());
	try
	{
		ZQ::common::MutexGuard lk(_lockEvictor);
		_pContextEvictor->remove(ident);
	}
	catch (Freeze::DatabaseException& ex)
	{
		SSMLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "%s[SsmEnv:0800] %s caught in removeSessionCtx()")
			, ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		SSMLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "%s[SsmEnv:0801] caught in removeSessionCtx()"), ex.ice_name().c_str());
		return false;
	}
	SSMLOG(InfoLevel, CLOGFMT(Environment, "removeSessionCtx(%s) successfully, reason: %s"), ident.name.c_str(), reason.c_str());
	return true;
}

StreamEventSinker::StreamEventSinker( Environment& env, ZQADAPTER_DECLTYPE objAdapter )
:_env(env),
mObjAdapter(objAdapter),
mbQuit( true ),
mSentinel(NULL)
{
}

StreamEventSinker::~StreamEventSinker(void)
{
	stop();
}

bool StreamEventSinker::addEventHandler( TianShanIce::Streamer::StreamEventSinkPtr handler )
{
	::Ice::ObjectPrx obj  = mObjAdapter->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}
	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = TianShanIce::Streamer::TopicOfStream;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::addEventHandler( TianShanIce::Streamer::PlaylistEventSinkPtr handler )
{
	::Ice::ObjectPrx obj  = mObjAdapter->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}

	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = TianShanIce::Streamer::TopicOfPlaylist;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::addEventHandler( TianShanIce::Events::GenericEventSinkPtr handler , const std::string& topicName  )
{
	::Ice::ObjectPrx obj  = mObjAdapter->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}

	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = topicName;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::start( const std::string& endpoint )
{
	if(endpoint.empty())
	{
		mEndpoint= "";
		return true;
	}

	if(endpoint.find(":") == std::string::npos )
	{
		mEndpoint = SERVICE_NAME_TopicManager":";
		mEndpoint = mEndpoint + endpoint;
	}
	else
	{
		mEndpoint = endpoint;
	}
	mbQuit = false;
	if( mSentinel )
	{
		mSentinel->stop();
		delete mSentinel;
	}
	mSentinel = new EventChannel::Sentinel(SSMLOG,mEndpoint,this);
	mSentinel->start();	
	return ZQ::common::NativeThread::start();
}

int StreamEventSinker::run()
{
	int64 interval = _liveChannelConfig._rtspSession._announceResubscribeAtIdle;
	if(interval < 10 * 60 * 1000 )
		interval = 10 * 60 * 1000;
	SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"event receiving status checker is running, interval[%lld]"), interval);
	while(!mbQuit)
	{
		mSem.timedWait(1000);
		if(mbQuit)	break;
		int64 delta = ZQ::common::now() - _env.getLastEventRecvTime();
		if( delta > interval )
		{
			SSMLOG(ZQ::common::Log::L_WARNING,CLOGFMT(StreamEventSinker,"no new event received for [%lld]ms , reconnect to event channel"),delta );
			try
			{
				subscribe();// force to reconnect to event channel
			}
			catch(...){}
		}
	}
	SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"event receiving status checker is stopped"));
	return 0;
}

void StreamEventSinker::stop()
{
	mbQuit = true;
	mSem.post();
	if( mSentinel)
	{
		mSentinel->stop();
		delete mSentinel;
		mSentinel = NULL;
	}
	unsubscribe();
	waitHandle(10000);
}

void StreamEventSinker::onConnectionEstablished()
{	
	subscribe();
}
void StreamEventSinker::reportBadConnection()
{
	SSMLOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinker,"connection to eventChannel is down, try to re-connect"));
}

void StreamEventSinker::unsubscribe( )
{
	std::vector< Subscriber >::iterator it = mSubscriber.begin();
	for( ; it != mSubscriber.end() ; it ++ )
	{
		try
		{
			if( it->topic)
			{
				it->topic->unsubscribe(it->handler);
			}			
		}
		catch( const Ice::Exception&)
		{
		}

		it->topic = NULL;
	}	
}

bool StreamEventSinker::subscribe( )
{
	static ZQ::common::Mutex locker;
	ZQ::common::MutexGuard gd(locker);//should get lock before connect to event channel

	{
		try
		{
			Ice::CommunicatorPtr ic = mObjAdapter->getCommunicator();
			mTopicManager = ::IceStorm::TopicManagerPrx::checkedCast( ic->stringToProxy(mEndpoint) );
			SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"connected to [%s] OK"), mEndpoint.c_str() );
		}
		catch( const Ice::Exception& ex )
		{
			SSMLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinker,"failed to connect to [%s] due to [%s]"), mEndpoint.c_str(),ex.ice_name().c_str() );
			return false;
		}

		if( mTopicManager == NULL )
			return false;
	}
	unsubscribe();
	std::vector< Subscriber >::iterator itSub = mSubscriber.begin();
	for( ; itSub != mSubscriber.end() ; itSub ++ )
	{
		try
		{
			itSub->topic = mTopicManager->retrieve( itSub->topicName );
		}
		catch( const ::IceStorm::NoSuchTopic& )
		{
			try
			{
				itSub->topic =  mTopicManager->create( itSub->topicName );
			}
			catch( const Ice::Exception& ex)
			{
				mTopicManager = NULL;
				SSMLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamEventSinker,"failed to create topic[%s] due to [%s]"),
					itSub->topicName.c_str() , ex.ice_name().c_str() );
				return false;
			}
		}
		catch( const Ice::Exception& ex )
		{
			mTopicManager = NULL;
			SSMLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamEventSinker,"failed to subscribe event due to [%s]"),ex.ice_name().c_str() );
			return false;
		}
		const ::TianShanIce::Properties qos;
#if ICE_INT_VERSION / 100 >= 303
		itSub->topic->subscribeAndGetPublisher(qos, itSub->handler );
#else
		itSub->topic->subscribe(qos, itSub->handler );
#endif
	}

	//update last event recv time
	_env.updateLastEventRecvTime( ZQ::common::now() );

	return true;
}

SmartServerRequest::SmartServerRequest(IServerRequest*& pServerRequest) : _pServerRequest(pServerRequest)
{
}

SmartServerRequest::~SmartServerRequest()
{
	if (NULL != _pServerRequest)
		_pServerRequest->release();
	_pServerRequest = NULL;
}

//event dispatch
EventSinkI::EventSinkI(Environment& env, StreamEventDispatcher& eventDispatcher)
:_env(env), mEventDispatcher(eventDispatcher)
{

}

EventSinkI::~EventSinkI()
{

}

void EventSinkI::sendEvent(const ::std::string& proxy, const ::std::string& uid, StreamEventType eventType, TianShanIce::Properties props, const ::Ice::Current ic)const
{
	//Environment& env, ZQ::common::NativeThreadPool& pool, const ::std::string& proxy, const ::std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties& extendProps)
	mEventDispatcher.pushEvent(proxy, uid, eventType, props, ic);
}


StreamEventDispatchRequest::StreamEventDispatchRequest(Environment& env, ZQ::common::NativeThreadPool& pool, const std::string& proxy, const std::string& uid, StreamEventType eventType, TianShanIce::Properties props, const ::Ice::Current& ic)
:_env(env),
_proxy(proxy),
_uid(uid),
_eventType(eventType),
_props(props),
_ic(ic),
_announce(new Announce()),
ZQ::common::ThreadRequest(pool)
{
}

StreamEventDispatchRequest::~StreamEventDispatchRequest()
{

}

int	StreamEventDispatchRequest::run()
{
	switch (_eventType)
	{
	case streamEventENDOFSTREAM:
		{
			_announce->postAnnounceEndofStream(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventBEGINOFSTREAM:
		{
			_announce->postAnnounceBeginningOfStream(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventSPEEDCHANGE:
		{
			_announce->postAnnounceSpeedChanged(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventSTATECHANGE:
		{
			_announce->postAnnounceStateChanged(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventITEMSTEP:
		{
			_announce->postAnnounceItemStepped(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventEXIT:
		{
			_announce->postAnnounceExit(_env, _proxy, _uid, _props, _ic);
		}
		break;
	case streamEventEXIT2:
		{
			_announce->postAnnounceExit2(_env, _proxy, _uid, _props, _ic);
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}

void StreamEventDispatchRequest::final(int retcode ,bool bCancelled)
{
	delete this;
}

StreamEventDispatcher::StreamEventDispatcher(Environment& env)
:_env(env)
{
	start();
}
StreamEventDispatcher::~StreamEventDispatcher()
{
	stop();
}

void StreamEventDispatcher::start()
{
	mPool.resize(10);
	SSMLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventDispatcher,"start StreamEventDispatcher with threadcount[%d]"), mPool.size());
}
void StreamEventDispatcher::stop( )
{
	mPool.stop();
}

void StreamEventDispatcher::pushEvent(const std::string& proxy, const std::string& uid, StreamEventType eventType, TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	StreamEventDispatchRequest* req = new StreamEventDispatchRequest(_env, mPool, proxy, uid, eventType, props, ic);
	req->start();
}


Announce::Announce()
{
}

Announce::~Announce()
{
}

void Announce::postAnnounceEndofStream(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());
	try
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s)"), proxy.c_str());

		std::vector<Ice::Identity> idents;
		idents = env._pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		if(SPEC_NGOD_S1 == cltSessCtx.requestType) // NGOD spec
		{
			std::string pos_str;
			ZQTianShan::Util::getPropertyDataWithDefault( props , "streamNptPosition" , "" , pos_str );
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " SC_ANNOUNCE_ENDOFSTREAM_STRING " event-date=%04d%02d%02dT%02d%02d%02d.%03dZ npt=%s",
				ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond, pos_str.c_str());
			pServerRequest->printHeader(HeaderNotice, szBuf);
			pServerRequest->printHeader(HeaderRequire, "com.comcast.ngod.c1");
			pServerRequest->printHeader(HeaderOnDemandSessionId, (char *)cltSessCtx.streamID.c_str());
		}
		else if (cltSessCtx.requestType == SPEC_NGOD_SeaChange) // SeaChange spec
		{
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " SC_ANNOUNCE_ENDOFSTREAM_STRING " %04d%02d%02dT%02d%02d%02dZ \"Normal End\"",
				ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			pServerRequest->printHeader(HeaderTianShanNotice, (char*) TS_ANNOUNCE_ENDOFSTREAM);
			TianShanIce::Properties::const_iterator it = cltSessCtx.props.find(SYS_PROP(primaryItemNPT));
			std::string tmp;
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				tmp = "BcastPos=";
			}
			else 
			{
				tmp = "npt=";
			}
			if (it!=cltSessCtx.props.end())
			{
				tmp = tmp + ";primaryItemNPT=" + it->second;
			}
			else
			{
				it = props.find(SYS_PROP(primaryItemNPT));
				if (it != props.end())
					tmp = tmp + ";primaryItemNPT=" + it->second;
			}
			pServerRequest->printHeader(HeaderTianShanNoticeParam, (char *)tmp.c_str());
		}

		pServerRequest->post();
		if( idents.size() >= 1)
			env.removeCachedSess(idents[0].name);//remove from cache  bug 17998
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s) caught unexpet exception"), proxy.c_str());
	}
}
void Announce::postAnnounceBeginningOfStream(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s)"), proxy.c_str());

		std::vector<Ice::Identity> idents;
		idents = env._pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		if(SPEC_NGOD_S1 == cltSessCtx.requestType) // NGOD spec
		{
			std::string pos_str;
			ZQTianShan::Util::getPropertyDataWithDefault( props , "streamNptPosition" , "" , pos_str );
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_BEGINOFSTREAM " Start-of-Stream Reached event-date=%04d%02d%02dT%02d%02d%02d.%03dZ npt=%s",
				ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond, pos_str.c_str());
			pServerRequest->printHeader(HeaderNotice, szBuf);
			pServerRequest->printHeader(HeaderRequire, "com.comcast.ngod.c1");
			pServerRequest->printHeader(HeaderOnDemandSessionId, (char *)cltSessCtx.streamID.c_str());
		}
		else if (cltSessCtx.requestType == SPEC_NGOD_SeaChange) // SeaChange spec
		{
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_BEGINOFSTREAM " " SC_ANNOUNCE_BEGINOFSTREAM_STRING " %04d%02d%02dT%02d%02d%02dZ",
				ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			std::string tmp = "";
			TianShanIce::Properties::const_iterator it = cltSessCtx.props.find(SYS_PROP(primaryItemNPT));
			if(it != cltSessCtx.props.end())
			{
				tmp = it->second;
			}
			else
			{
				it = props.find(SYS_PROP(primaryItemNPT));
				if(it != props.end())
					tmp = it->second;
			}
			TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
			TianShanIce::Application::PurchasePrx purchasePrx = NULL;
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				std::string scale, bcastPos;
				Ice::Int ctrlNum = 0, offset = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
					env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;primaryItemNPT=%s", bcastPos.c_str(), tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s", bcastPos.c_str());
				}
			}
			else 
			{
				std::string scale;
				Ice::Int curPos = 0, totalPos = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
					env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;primaryItemNPT=%s", curPos / 1000, curPos % 1000, tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d", curPos / 1000, curPos % 1000);
				}
			}
			pServerRequest->printHeader(HeaderTianShanNotice, (char*) TS_ANNOUNCE_BEGINOFSTREAM);
			pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);
		}
		pServerRequest->post();

		if( idents.size() >= 1)
			env.removeCachedSess(idents[0].name);
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s) caught unexpet exception"), proxy.c_str());
	}
}
void Announce::postAnnounceSpeedChanged(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s)"), proxy.c_str());

		//TODO: get speed
		::Ice::Float prevSpeed		= 0;
		::Ice::Float currentSpeed	= 0;
		ZQTianShan::Util::getPropertyDataWithDefault( props, "prevSpeed", 0 , prevSpeed );
		ZQTianShan::Util::getPropertyDataWithDefault( props, "currentSpeed", 0 , currentSpeed );

		std::vector<Ice::Identity> idents;
		idents = env._pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		std::map<std::string, std::string>::const_iterator icItor = ic.ctx.find(ICE_CONTEXT_EVENT_SEQUENCE);
		std::string curSeq = (ic.ctx.end() != icItor) ? icItor->second : "-1"; // because -1 is the initial value
		if (!cltSessPrx->canSendScaleChange(curSeq))
		{
			env._fileLog(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s), canSendScaleChange() return false, ignore this event"), proxy.c_str());
			return;
		}

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		if (cltSessCtx.requestType == SPEC_NGOD_SeaChange) // SeaChange spec
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "%f", currentSpeed);
			pServerRequest->printHeader(HeaderScale, szBuf);
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_SCALECHANGED " " SC_ANNOUNCE_SCALECHANGED_STRING " %04d%02d%02dT%02d%02d%02d.%03dZ",
				ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond);
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
			TianShanIce::Application::PurchasePrx purchasePrx = NULL;
			std::string tmp = "";
			TianShanIce::Properties::const_iterator it = cltSessCtx.props.find(SYS_PROP(primaryItemNPT));
			if(it != cltSessCtx.props.end())
			{
				tmp = it->second;
			}
			else
			{
				it = props.find(SYS_PROP(primaryItemNPT));
				if(it != props.end())
					tmp = it->second;
			}
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				std::string scale, bcastPos;
				Ice::Int ctrlNum = 0, offset = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
					env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;Scale=%f;primaryItemNPT=%s", 
						bcastPos.c_str(), currentSpeed, tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;Scale=%f", bcastPos.c_str(), currentSpeed);
				}
			}
			else 
			{
				std::string scale;
				Ice::Int curPos = 0, totalPos = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
					env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;Scale=%f;primaryItemNPT=%s",
						curPos / 1000, curPos % 1000, currentSpeed, tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;Scale=%f",
						curPos / 1000, curPos % 1000, currentSpeed);
				}
			}
			pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_SCALECHANGED);
			pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);

			if (SPEC_NGOD_S1 == cltSessCtx.requestType)// NGOD spec
			{
				SYS::TimeStamp ts;
				snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_SCALECHANGED " " SC_ANNOUNCE_SCALECHANGED_STRING " eventdate=%04d%02d%02dT%02d%02d%02d.%03dZ",
					ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond);
				pServerRequest->printHeader(HeaderNotice, szBuf);
				pServerRequest->printHeader(HeaderTianShanNotice, "Stream::0004 Scale Changed");
				pServerRequest->printHeader(HeaderOnDemandSessionId, (char *)cltSessCtx.streamID.c_str());
			}
		}
		pServerRequest->post();

		if( idents.size() >= 1)
			env.removeCachedSess(idents[0].name);

		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s) caught unexpet exception"), proxy.c_str());
	}
}

void Announce::postAnnounceStateChanged(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{

	int prevState;
	int currentState;
	ZQTianShan::Util::getPropertyDataWithDefault( props, "prevState", 0 , prevState );
	ZQTianShan::Util::getPropertyDataWithDefault( props, "currentState", 0 , currentState );

	env.updateLastEventRecvTime(ZQ::common::now());
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s)"), proxy.c_str());

		std::vector<Ice::Identity> idents;
		idents = env._pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		if (cltSessCtx.requestType == SPEC_NGOD_SeaChange) // SeaChange spec
			return; // not send

		// TianShan spec
		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		std::string stateDept;
		switch (currentState)
		{
		case 0: stateDept = "init"; break;		//TianShanIce::Streamer::stsSetup
		case 1: stateDept = "play"; break;		//TianShanIce::Streamer::stsStreaming
		case 2: stateDept = "pause"; break;		//TianShanIce::Streamer::stsPause
		case 3: stateDept = "stop"; break;		//TianShanIce::Streamer::stsStop
		}
		TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
		TianShanIce::Application::PurchasePrx purchasePrx = NULL;
		std::string tmp = "";
		TianShanIce::Properties::const_iterator it = cltSessCtx.props.find(SYS_PROP(primaryItemNPT));
		if(it != cltSessCtx.props.end())
		{
			tmp = it->second;
		}
		else
		{
			it = props.find(SYS_PROP(primaryItemNPT));
			if(it != props.end())
				tmp = it->second;
		}
		if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
		{
			std::string scale, bcastPos;
			Ice::Int ctrlNum = 0, offset = 0;
			if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
				env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
			if(!tmp.empty())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;presentation_state=%s;primaryItemNPT=%s"
					, bcastPos.c_str(), stateDept.c_str(), tmp.c_str());
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;presentation_state=%s"
					, bcastPos.c_str(), stateDept.c_str());
			}
		}
		else 
		{
			std::string scale;
			Ice::Int curPos = 0, totalPos = 0;
			if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
				env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);

			if(!tmp.empty())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;presentation_state=%s;primaryItemNPT=%s",
					curPos / 1000, curPos % 1000, stateDept.c_str(), tmp.c_str());
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;presentation_state=%s",
					curPos / 1000, curPos % 1000, stateDept.c_str());
			}
		}
		pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_STATECHANGED);
		pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);
		pServerRequest->post();

		if( idents.size() >= 1)
			env.removeCachedSess(idents[0].name);

		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s) sent"), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s) caught exception"), proxy.c_str());
	}
}
void Announce::postAnnounceItemStepped(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s)"), proxy.c_str());

		// if previous item is empty, just ignore the event
		TianShanIce::Properties tempMap = props;
		if (tempMap["prevItemName"].empty())
		{
			bool bSendItemStepped = false;
			std::vector<DefaultParamHolder>::iterator it;
			for (it = _liveChannelConfig._defaultParams._paramDatas.begin();
				it != _liveChannelConfig._defaultParams._paramDatas.end(); it ++)
			{
				if ((*it)._name == "SendItemStepped" && atoi((*it)._value.c_str()) != 0)
				{
					bSendItemStepped = true;
					break;
				}
			}
			if (!bSendItemStepped)
			{
				SSMLOG(InfoLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s) previous item name is empty and SendItemStepped is disabled, ignore event"), proxy.c_str());
				return;
			}
		}

		std::vector<Ice::Identity> idents;
		idents = env._pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		if (cltSessCtx.requestType == SPEC_NGOD_SeaChange) // SeaChange spec
		{
			std::string itemname = tempMap["prevItemName"];
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ITEMSTEPPED " " \
				SC_ANNOUNCE_ITEMSTEPPED_STRING " " "%04d%02d%02dT%02d%02d%02dZ" " " "\"%s\"" \
				, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, itemname.c_str());
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
			TianShanIce::Application::PurchasePrx purchasePrx = NULL;
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				std::string scale, bcastPos;
				Ice::Int ctrlNum = 0, offset = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
					env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
				if (tempMap.find("sys.primaryItemNPT") != tempMap.end())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;prev_item=%s;current_item=%s;primaryItemNPT=%s",
						bcastPos.c_str() , tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str(), tempMap["sys.primaryItemNPT"].c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;prev_item=%s;current_item=%s",
						bcastPos.c_str() , tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str());
				}
			}
			else 
			{
				std::string scale;
				Ice::Int curPos = 0, totalPos = 0;
				if (true == env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
					env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
				if (tempMap.find("sys.primaryItemNPT") != tempMap.end())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;prev_item=%s;current_item=%s;primaryItemNPT=%s",
						curPos / 1000, curPos % 1000, tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str(), tempMap["sys.primaryItemNPT"].c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;prev_item=%s;current_item=%s",
						curPos / 1000, curPos % 1000, tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str());
				}
			}
			pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_ITEMSTEPPED);				
			pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);
		}
		pServerRequest->post();

		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s) sent"), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s) caught exception"), proxy.c_str());
	}
}
void Announce::postAnnounceExit(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());

	std::vector<Ice::Identity> idents;
	idents = env._pStreamIdx->findFirst(uid, 1);
	if( idents.size() >= 1)
		env.removeCachedSess(idents[0].name);

	SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnExit(%s)"), proxy.c_str());
}
void Announce::postAnnounceExit2(Environment& env, const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)
{
	env.updateLastEventRecvTime(ZQ::common::now());

	std::vector<Ice::Identity> idents;
	idents = env._pStreamIdx->findFirst(uid, 1);
	if( idents.size() >= 1)
		env.removeCachedSess(idents[0].name);

	SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnExit2(%s)"), proxy.c_str());
}

} // end namespace LiveChannel

