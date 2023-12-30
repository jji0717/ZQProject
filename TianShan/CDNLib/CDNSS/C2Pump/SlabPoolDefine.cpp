#include "SlabPoolDefine.h"
#include "HttpFetcher.h"
#include "C2Request.h"

#ifdef ENABLE_SLABPOOL

SlabPool gSlabPool;

// class list
SLOTHOLDER_INIT(C2Streamer::HttpFetcher)
SLOTHOLDER_INIT(ZQ::StreamService::C2ReadFile)
SLOTHOLDER_INIT(ZQ::StreamService::GetRequest)
SLOTHOLDER_INIT(ZQ::StreamService::LocateRequest)
SLOTHOLDER_INIT(ZQ::StreamService::TransferDelete)
SLOTHOLDER_INIT(ZQ::StreamService::C2QueryIndex)

#endif//
