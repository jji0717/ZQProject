#ifndef _ZQ_STREAMSMITH_CHECK_CONTENT_H__
#define _ZQ_STREAMSMITH_CHECK_CONTENT_H__

#include <StreamSmithConfig.h>

#include <nativethread.h>

#include <ice/Ice.h>
#include <iceutil/iceutil.h>
#include <tsstorage.h>

#include <IndexFileParser.h>

#include "ContentStore.h"

#define STREAMSMITH_FILE_FLAG_NPVR	(1<<0)

namespace ZQ{
namespace StreamSmith{

class CheckContent 
{
public:
	CheckContent( Ice::CommunicatorPtr& ic , 
				Ice::ObjectPrx objPrx = NULL ,
				VHANDLE vstrmHandle = NULL,
				ZQ::IdxParser::IdxParserEnv* env = NULL);
	~CheckContent( );
public:
	
	bool						setContentStoreProxy( Ice::ObjectPrx  objPrx  );	
	
	
	bool						GetItemType(const std::string& strItem,
											std::string& strFullName,
											const std::string& PlaylistID,
											ULONG* fileFlag = NULL );
	bool						GetItemAttribute(const std::string& strItem ,
												long& lPlayTime , 
												long& lBitRate,
												long& lTotalTime,
												bool& bPWE,
												const std::string& PlaylistID,
												bool bCuein = false,
												bool bCueout = false,
												int inTimeoffset =0,
												int outTimeoffset = 0);
	
	bool						getItemTypeFromRemote( const std::string& strItem,
														std::string& strFullName,
														const std::string& PlaylistID,
														ULONG* fileFlag = NULL ,
														bool bLocalContent = true );
	bool						GetItemAttributeFromRemote(const std::string& strItem ,
														long& lPlayTime , 
														long& lBitRate,
														bool& bPWE,
														const std::string& PlaylistID,
														bool bLocalContent );

	std::string					getLastError( ) const
	{
		return mLastError;
	}

protected:

	TianShanIce::Storage::UnivContentPrx	getContentProxy( const std::string& itemfullname );

	bool									updateContentAttribute( const std::string& strFullContentName ,const ZQ::IdxParser::IndexData& data , const std::string& PlaylistID);

private:

	/*ZQ::IdxzVvxParser							_localVvxParser;*/
	
	std::string								_currentContentStoreEndpoint;
		
	TianShanIce::Storage::ContentStorePrx	_ctntStorageServicePrx;
	Ice::CommunicatorPtr&					_icPtr;
	bool									_bQuit;
	HANDLE									_hEvent;
	bool									_bPrimaryEndpoint;

	std::string								_strHDot264Driver;

	ZQ::IdxParser::IdxParserEnv				*mIdxParserEnv;
	bool									mbOwnEnv;

	std::string								mLastError;
	bool									mbEdgeServer;

};

}}//namespace ZQ::StreamSmith
#endif//_ZQ_STREAMSMITH_CHECK_CONTENT_H__
