
#ifndef _cdn_streamservice_commandheader_file_h__
#define _cdn_streamservice_commandheader_file_h__

#include <SsStreamBase.h>
#include <SsServiceImpl.h>
#include <ContentStore.h>
#include "CdnEnv.h"

namespace ZQ
{
namespace StreamService
{


class CdnSsCommand
{
public:
	CdnSsCommand(CdnSsEnvironment * environment , SsServiceImpl& ss , SsContext& contextKey);
	virtual ~CdnSsCommand( );

public:
	int32					doLoad( const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,
									std::string& sessId );


	int32					doCommit(	IN const PlaylistItemSetupInfos& items  ,
										INOUT TianShanIce::SRM::ResourceMap& crResource);

	int32					doValidateItem( INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info );

	int32					doDestroy( const std::string& sessionId ,const std::string& clientTransfer );

	StreamParams			getStreamInfo( ) const;

	void					doClearResource( const TianShanIce::ValueMap& playlistParams );

protected:
#ifndef ZQ_CDN_UMG
	std::string				getFileNameWithoutMountPath( TianShanIce::Storage::UnivContentPrx& ctntPrx , const std::string& key, int64* bitrate = 0 );
	std::string				getFileNameWithoutMountPath( const std::string& filepath );

	std::string				getFilePathNameFromContentName( const std::string& name);

	bool					findShadowIndexContent(  TianShanIce::Storage::UnivContentPrx contentPrx,
													const std::string& upstreamUrl,
													const std::string& name, 
													const std::string& pid, const std::string& paid ,
													const std::string& subtype );
#endif
private:

	SsServiceImpl&				ss;
	SsContext&					mStreamCtx;
	StreamParams				mStreamInfo;
	CdnSsEnvironment*			env;
	CdnStreamerManager&			mStreamerManager;
};

#define SHADOW_INDEX_CONTENT_NOT_FOUND	"sys.shadow.index.not.found"
#define REQUEST_IS_MAINFILE  "sys.request.mainfile"
#define REQUEST_ASSET_INDEXFILE_PATH "sys.request.asset.indexpathname"
#define REQUEST_ASSET_ISPWE "sys.request.asset.pwe"

}}

#endif//_cdn_streamservice_commandheader_file_h__
