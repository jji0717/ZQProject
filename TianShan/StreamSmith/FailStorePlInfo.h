#ifndef _ZQ_STREAMSMITH_PL_FAILOVER_H__
#define	_ZQ_STREAMSMITH_PL_FAILOVER_H__


#include "itemtoplaylist.h"
#include "PlaylistDict.h"
#include "PlaylistExI.h"
#include "playlistitemI.h"
#include "PlaylistInternalUse.h"
#include <locks.h>
#include <string>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
#include <StreamSmithModuleEx.h>

#ifdef _DEBUG
#  pragma comment(lib, "Iced")
#  pragma comment(lib, "IceUtild")
#  pragma comment(lib, "freezed")
#else
#  pragma comment(lib, "Ice")
#  pragma comment(lib, "IceUtil")
#  pragma comment(lib, "freeze")
#endif //_DEBUG


namespace ZQ
{
namespace StreamSmith
{

typedef struct _DbVerifyInfo 
{
	Ice::Long			lastStart;
	Ice::Long			lastRestore;
	Ice::Long			dbRecordNA;
	_DbVerifyInfo( )
	{
		lastStart		=	0;
		lastRestore		=	0;
		dbRecordNA		=	0;
	}
}DbVerifyInfo ;

class FailOverPlaylistInfoFactory:public Ice::ObjectFactory
{
public:
	FailOverPlaylistInfoFactory();
	~FailOverPlaylistInfoFactory();	
public:
	Ice::ObjectPtr create(const std::string& strID);
	
	void destroy();
private:
	
};

class FailOverPlItemInfoFactory:public Ice::ObjectFactory
{
public:
	FailOverPlItemInfoFactory();
	~FailOverPlItemInfoFactory();	
public:
	Ice::ObjectPtr create(const std::string& strID);
	
	void destroy();
	
};



typedef IceUtil::Handle<FailOverPlaylistInfoFactory> FailOverPlaylistInfoFactoryPtr;
typedef IceUtil::Handle<FailOverPlItemInfoFactory> FailOverPlItemInfoFactoryPtr;


typedef std::vector<TianShanIce::Streamer::PlaylistItemAttr>	VECPlaylistItemAttr;

typedef	void (*PLAYLISTRECONSTRUCTCALLBACK)(const std::string& guidStr , 
											const TianShanIce::Streamer::PlaylistAttr& plAttr,void* pData);
typedef void (*PLAYLISTITEMRECONSTRUCTCALLBACK)(const std::string& guidStr ,
												VECPlaylistItemAttr& vecPliAttr,void* pData);
class PlaylistManager;
class PlaylistInfoStore
{
public:
	PlaylistInfoStore(PlaylistManager& plMan,::Ice::CommunicatorPtr ConnPtr);
	PlaylistInfoStore(ZQADAPTER_DECLTYPE adapter,PlaylistManager& plMan);
	~PlaylistInfoStore();	
public:
	bool				Init(const std::string dbPath);

	bool				SetupPlaylistInfo(	PLAYLISTRECONSTRUCTCALLBACK plCallback,
											PLAYLISTITEMRECONSTRUCTCALLBACK pliCallback,
											void* pData);

	bool				UpdatePlaylistAttr(const std::string& strGuid,
											const TianShanIce::Streamer::PlaylistAttr& plAttr);
	bool				AddPlaylistAttr(const std::string& strGuid,
											const TianShanIce::Streamer::PlaylistAttr& plAttr);
	bool				UpdatePlaylistItemAttr(const std::string& strGuid,
											 VECPlaylistItemAttr& pliAttr);
	bool				ClearPlaylistAttr(const std::string& strGuid);
	bool				ClearPlaylistItemAttr(const std::string& strGuid);
	bool				ClearAll();
	int					GetPlaylistItemSize();
	int					GetPlaylistSize();
	
	void				updateDbHealth( bool bHealth );

protected:


	bool				ReadPliAttr(const std::string guid,PLAYLISTITEMRECONSTRUCTCALLBACK pliCallback,void* pData);

	bool				verifyDb( const std::string& dbPath );

	bool				confirmDB( const std::string& dbPath  );

	void				cleanDbFiles( const std::string& dbPath );



	

private:
	ZQADAPTER_DECLTYPE										m_pAdpter;
	::Ice::CommunicatorPtr									m_pCommunicatorPtr;
	
	FailOverPlaylistInfoFactoryPtr							m_pFactoryPL;
	FailOverPlItemInfoFactoryPtr							m_pFactoryPLI;
	
	//TianShanIce::Streamer::PlaylistDict*					m_pPlaylistContent;
	Freeze::EvictorPtr										m_pPlaylistContentEvictor;
	TianShanIce::Streamer::IndexItemToPlaylistPtr			m_pPlaylistItemContent;
	Freeze::EvictorPtr										m_pPlaylistItemContentEvictor;
	
	PlaylistManager&										m_plMan;

	std::string												mDbPath;
	DbVerifyInfo											mDbhealthInfo;
};


}
}

#endif//_ZQ_STREAMSMITH_PL_FAILOVER_H__