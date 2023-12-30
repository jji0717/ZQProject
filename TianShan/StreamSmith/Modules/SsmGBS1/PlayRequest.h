#ifndef __TianShanS1_PlayRequest_H__
#define __TianShanS1_PlayRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupPlay : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupPlay> Ptr;
		FixupPlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupPlay();
		virtual bool process();

	};

	class HandlePlay : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandlePlay> Ptr;
		HandlePlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandlePlay();
		virtual bool process();
		bool setRangePrefix(const std::string& rngPf);

		bool prepareResponseParameter( const ::TianShanIce::Streamer::StreamInfo& info );
		void prepareResponseParameter( std::string& scale, Ice::Int& currentPos, Ice::Int& totalPos );

		void rollbackSpeed( );

	protected:

		bool fixSpeed( float& requestSpeed );


		

	protected:
		friend class playExAsync;
		friend class playAsyncResponse;
		friend class playItemAsync;

		std::string _requestRange;
		std::string _requestScale;
		std::string _returnRange;
		std::string _returnScale;

	};

	class PlayResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<PlayResponse> Ptr;
		PlayResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~PlayResponse();
		virtual bool process();

	};

} // end namespace TianShanS1

#endif // __TianShanS1_PlayRequest_H__

