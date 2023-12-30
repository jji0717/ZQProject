
#ifndef _ZQ_STREAMSMITH_EMBEDED_CONTENT_STORE_BRIDGE_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_EMBEDED_CONTENT_STORE_BRIDGE_HEADER_FILE_H__

#include <ContentImpl.h>
#include <vstrmuser.h>

namespace ZQ
{
namespace StreamSmith
{



class NCSBridge  
{
public:

	static Ice::ObjectPrx	StartContentStore(  ZQADAPTER_DECLTYPE& objAdapter , const std::string confFolder ,ZQ::common::Log& log ,VHANDLE vHandle);

	static bool				mountContentStore( );

	static bool				StopContentStore (  ZQ::common::Log& log  );

	static VHANDLE			getVstrmHandle( );

public:

	static ::ZQTianShan::ContentStore::ContentStoreImpl::Ptr embedContentStore;
	static VHANDLE		_vstrmHandle;
};


}}//namespace ZQ::StreamSmith

#endif//_ZQ_STREAMSMITH_EMBEDED_CONTENT_STORE_BRIDGE_HEADER_FILE_H__
