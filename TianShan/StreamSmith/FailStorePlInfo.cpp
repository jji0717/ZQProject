
#include <ZQ_common_conf.h>
#include "FailStorePlInfo.h"
#include <Log.h>
#include <playlist.h>
#include <TianShanDefines.h>
#include <io.h>

#ifdef _DEBUG
	#include "adebugmem.h"
#endif



namespace ZQ
{
namespace StreamSmith
{
#ifdef	_ICE_INTERFACE_SUPPORT

#define ICEDBFMT(x)	"[IceDB] "##x
using namespace ZQ::common;

FailOverPlaylistInfoFactory::FailOverPlaylistInfoFactory()
{
}
FailOverPlaylistInfoFactory::~FailOverPlaylistInfoFactory()
{
}

Ice::ObjectPtr FailOverPlaylistInfoFactory::create(const std::string& strID)
{
	if(strID==TianShanIce::Streamer::InternalPlaylistEx::ice_staticId ())
		return new PlaylistExI();	
	return NULL;
}
void FailOverPlaylistInfoFactory::destroy()
{
	//delete this;
}

FailOverPlItemInfoFactory::FailOverPlItemInfoFactory()
{
}
FailOverPlItemInfoFactory::~FailOverPlItemInfoFactory()
{
}

Ice::ObjectPtr FailOverPlItemInfoFactory::create(const std::string& strID)
{
	if(strID==TianShanIce::Streamer::PlaylistItem::ice_staticId ())
		return new PlaylistItemI();	
	return NULL;
}
void FailOverPlItemInfoFactory::destroy()
{
	//delete this;
}


//////////////////////////////////////////////////////////////////////////



#define TRY_BEGIN()	try {
#define TRY_END(_PROMPT)	}	catch(const Ice::Exception& ex) \
{ std::ostringstream s; s << _PROMPT;ex.ice_print(s);glog(Log::L_ERROR,"%s",s.str().c_str());}\
catch(...){ std::ostringstream s; s << _PROMPT;glog(Log::L_ERROR,"%s Unexpect error , DB FILE MAY BE CORRUPT",s.str().c_str());return false;}


//////////////////////////////////////////////////////////////////////////
PlaylistInfoStore::PlaylistInfoStore (PlaylistManager& plMan,::Ice::CommunicatorPtr ConnPtr)
					:m_pCommunicatorPtr(ConnPtr),m_plMan(plMan)
{
/*	m_pPlaylistContent=NULL;*/
	m_pFactoryPL=new FailOverPlaylistInfoFactory;
	m_pFactoryPLI=new FailOverPlItemInfoFactory;
	try
	{
		m_pCommunicatorPtr->addObjectFactory(m_pFactoryPL,TianShanIce::Streamer::InternalPlaylistEx::ice_staticId ());
		
		m_pCommunicatorPtr->addObjectFactory(m_pFactoryPLI,TianShanIce::Streamer::PlaylistItem::ice_staticId ());
	}
	catch (...) 
	{
	}
}
PlaylistInfoStore::PlaylistInfoStore(ZQADAPTER_DECLTYPE adapter,PlaylistManager& plMan)
					:m_plMan(plMan)
{
//	m_pPlaylistContent=NULL;
	m_pCommunicatorPtr=adapter->getCommunicator();
	m_pAdpter=adapter;
	//m_pFactory=new FailOverPlInfoFactory;
	m_pFactoryPL=new FailOverPlaylistInfoFactory;
	m_pFactoryPLI=new FailOverPlItemInfoFactory;
	try
	{
		m_pCommunicatorPtr->addObjectFactory(m_pFactoryPL,TianShanIce::Streamer::InternalPlaylistEx::ice_staticId ());
		
		m_pCommunicatorPtr->addObjectFactory(m_pFactoryPLI,TianShanIce::Streamer::PlaylistItem::ice_staticId ());
	}
	catch (...) 
	{
	}
}
PlaylistInfoStore::~PlaylistInfoStore ()
{
	try
	{		
//		m_pCommunicatorPtr->removeObjectFactory(TianShanIce::Streamer::InternalPlaylistEx::ice_staticId ());
//		m_pCommunicatorPtr->removeObjectFactory(TianShanIce::Streamer::PlaylistItem::ice_staticId ());
	}
	catch (...) 
	{
	}
//	m_pFactoryPL=NULL;
//	m_pFactoryPLI=NULL;
//	if(!m_pPlaylistContent)
//	{
//		delete m_pPlaylistContent;
//		m_pPlaylistContent=NULL;
//	}
	m_pPlaylistItemContent = NULL;
	m_pPlaylistContentEvictor = NULL;
	m_pPlaylistItemContentEvictor = NULL;
}
Ice::Long			systemUpTime( )
{
#ifdef ZQ_OS_MSWIN
	LARGE_INTEGER freq ;
	QueryPerformanceFrequency( &freq );
	LARGE_INTEGER counter ;
	QueryPerformanceCounter(&counter);
	Ice::Long lUpTime = counter.QuadPart / (freq.QuadPart/1000);
	
	return  (ZQTianShan::now() - lUpTime);
	
#else
#error "not implement"
#endif
}

void PlaylistInfoStore::updateDbHealth( bool bHealth )
{
	std::string stampFilePath = mDbPath + "StreamSmith.stamp";
	FILE* fStamp = fopen( stampFilePath.c_str() , "r+b");
	if(!fStamp)
	{
		fStamp = fopen( stampFilePath.c_str() , "w+b");
		if(!fStamp)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(updateDbHealth,"can't open %s ") ,stampFilePath.c_str());
			return ;
		}
	}

// 	if( fread( &mDbhealthInfo , 1, sizeof(info), fStamp ) != sizeof(mDbhealthInfo) )
// 	{//no record 
// 		//do nothing , treat it as new created file		
// 	}
// 	else
	{		
		fseek( fStamp ,0 , SEEK_SET );		
		mDbhealthInfo.dbRecordNA = bHealth ? 0 : 1 ; //set health to 0 , other wise , set to 1
		fwrite( &mDbhealthInfo , 1, sizeof(mDbhealthInfo) , fStamp );
	}
	fclose(fStamp);
	
}
bool PlaylistInfoStore::verifyDb( const std::string& dbPath )
{
	std::string stampFilePath = dbPath + "StreamSmith.stamp";
	FILE* fStamp = fopen( stampFilePath.c_str() , "r+b");
	if(!fStamp)
	{
		fStamp = fopen( stampFilePath.c_str() , "w+b");
		if(!fStamp)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(verifyDb,"can't open %s ") ,stampFilePath.c_str());
			return true;
		}
	}

	if( fread( &mDbhealthInfo , 1, sizeof(mDbhealthInfo), fStamp ) != sizeof(mDbhealthInfo) )
	{//no record 
		//do nothing , treat it as new created file		
	}
	else
	{
		Ice::Long currentStart	= ZQTianShan::now();
		Ice::Long sysUpTime		= systemUpTime( );

		
		Ice::Long interval = gStreamSmithConfig.lDbHealththreshold;
		interval = interval < 300000 ? 300000 : interval;

		if( (mDbhealthInfo.lastRestore < sysUpTime) || (mDbhealthInfo.lastStart > ( mDbhealthInfo.lastRestore  + interval)) )
		{
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(verifyDb,"lastRestore[%lld] lastStartup[%lld] systemUp[%lld] checkThreshold[%lld], clean db files"),
				mDbhealthInfo.lastRestore,
				mDbhealthInfo.lastStart,
				sysUpTime,
				interval );

			cleanDbFiles( dbPath );
		}
		if( mDbhealthInfo.dbRecordNA >= 1)
		{//db record not valid
			glog(ZQ::common::Log::L_WARNING,CLOGFMT(verifyDb,"db data not valid, clean db files"));
			cleanDbFiles( dbPath );
		}
		fseek( fStamp ,0 , SEEK_SET );		
		mDbhealthInfo.lastStart		= ZQTianShan::now();
		mDbhealthInfo.dbRecordNA	= 0;
		fwrite( &mDbhealthInfo , 1, sizeof(mDbhealthInfo) , fStamp );
	}
	fclose(fStamp);
	return true;
}

void deleteDbFile( const std::string& dbFile , const std::string& folder )
{
	WIN32_FIND_DATA wfd;
	HANDLE hFile = FindFirstFileA( dbFile.c_str() , &wfd );
	if( INVALID_HANDLE_VALUE != hFile )
	{
		bool bFind = true;
		do
		{
			if( !DeleteFileA( (folder + wfd.cFileName).c_str() ) )
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(cleanDbFiles,"failed to delete [%s] and error code[%u]"),
					(folder + wfd.cFileName).c_str(),
					GetLastError() );
			}
			bFind = FindNextFileA( hFile , &wfd );
		}while( bFind );

		FindClose(hFile);
	}
}

void PlaylistInfoStore::cleanDbFiles( const std::string& dbPath )
{	
	deleteDbFile( (dbPath + "__catalog") , dbPath );
	deleteDbFile( (dbPath + "ItemCatalog") , dbPath );
	deleteDbFile( (dbPath + "log.*") , dbPath );
	deleteDbFile( (dbPath + "Playlist") , dbPath );
}

bool PlaylistInfoStore::confirmDB( const std::string& dbPath  )
{
	std::string stampFilePath = dbPath + "StreamSmith.stamp";
	
	FILE* fStamp = fopen( stampFilePath.c_str() , "r+b");
	if(!fStamp)
	{
		fStamp = fopen( stampFilePath.c_str() , "w+b");
		if(!fStamp)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(confirmDB,"can't open %s ") ,stampFilePath.c_str());
			return true;
		}
	}
	
	mDbhealthInfo.lastRestore = mDbhealthInfo.lastStart = ZQTianShan::now();
	
	fwrite( &mDbhealthInfo , 1, sizeof(mDbhealthInfo) , fStamp );

	fclose(fStamp);
	return true;
}

bool PlaylistInfoStore::Init (const std::string dbPath)
{
	
	std::string	db=dbPath;
	if(db.empty ())
	{
		db="c:\\db\\";
	}
	if ( db.at(db.length()-1) != '\\' ) 
	{
		db+=FNSEPS;
	}
	{
		db+="streamsmith"FNSEPS;
	}

	::CreateDirectoryA(db.c_str(), NULL);

	verifyDb( db );
	mDbPath = db;
	
	{
#define MAX_CONTENTS 100000
#define DEFAULT_CACHE_SIZE (16*1024*1024)
		::std::string dbConfFile = db + FNSEPS + "DB_CONFIG";
		if ( -1 == ::access(dbConfFile.c_str(), 0))
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistInfoStore, "initializing %s"), dbConfFile.c_str());
			FILE* f = ::fopen(dbConfFile.c_str(), "w+");
			if (NULL != f)
			{
				::fprintf(f, "set_lk_max_locks %d\n",   MAX_CONTENTS);
				::fprintf(f, "set_lk_max_objects %d\n", MAX_CONTENTS);
				::fprintf(f, "set_lk_max_lockers %d\n", MAX_CONTENTS);
				::fprintf(f, "set_cachesize 0 %d 0\n",	DEFAULT_CACHE_SIZE);
				::fclose(f);
			}
		}
	
	}

	TRY_BEGIN();
		Ice::PropertiesPtr proper = m_pAdpter->getCommunicator()->getProperties();
		if ( strlen( gStreamSmithConfig.szIceEnvCheckPointPeriod ) > 0 )
		{
			std::string strCheckPointPeriod = "Freeze.DbEnv." + db ;
			strCheckPointPeriod = strCheckPointPeriod + ".CheckpointPeriod";
			proper->setProperty( strCheckPointPeriod,  gStreamSmithConfig.szIceEnvCheckPointPeriod  );
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strCheckPointPeriod.c_str() ,
				gStreamSmithConfig.szIceEnvCheckPointPeriod);
		}
		if ( strlen( gStreamSmithConfig.szIceEnvDbRecoverFatal ) ) 
		{
			std::string strDBRecoverFatal = "Freeze.DbEnv." + db ;
			strDBRecoverFatal = strDBRecoverFatal + ".DbRecoverFatal";
			proper->setProperty( strDBRecoverFatal, gStreamSmithConfig.szIceEnvDbRecoverFatal );
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strDBRecoverFatal.c_str() ,
				gStreamSmithConfig.szIceEnvDbRecoverFatal);
		}
		
		if ( strlen( gStreamSmithConfig.szFreezeItemSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + db;
			strSessionSavePeriod = strSessionSavePeriod + "."  + PLI_CATALOG +".SavePeriod";
			proper->setProperty( strSessionSavePeriod , gStreamSmithConfig.szFreezeItemSavePeriod );
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strSessionSavePeriod.c_str() ,
				gStreamSmithConfig.szFreezeItemSavePeriod);
		}
		if ( strlen(gStreamSmithConfig.szFreezeItemSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + db ;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + PLI_CATALOG + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger , gStreamSmithConfig.szFreezeItemSaveSizeTrigger);
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strSessSaveSizeTrigger.c_str() ,
				gStreamSmithConfig.szFreezeItemSaveSizeTrigger);
		}

		if ( strlen( gStreamSmithConfig.szFreezePlaylistSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + db;
			strSessionSavePeriod = strSessionSavePeriod + "."  + PL_CATALOG +".SavePeriod";
			proper->setProperty( strSessionSavePeriod , gStreamSmithConfig.szFreezePlaylistSavePeriod );
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strSessionSavePeriod.c_str() ,
				gStreamSmithConfig.szFreezePlaylistSavePeriod);
		}
		if ( strlen(gStreamSmithConfig.szFreezePlaylistSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + db ;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + PL_CATALOG + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger , gStreamSmithConfig.szFreezePlaylistSaveSizeTrigger);
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strSessSaveSizeTrigger.c_str() ,
				gStreamSmithConfig.szFreezePlaylistSaveSizeTrigger);
		}

		//set db fatal recover
		{
			std::string   strFatalRecover = "Freeze.DbEnv." + db;
			strFatalRecover = strFatalRecover + ".DbRecoverFatal";
			proper->setProperty(strFatalRecover,"1");
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strFatalRecover.c_str() , "1");
		}
		{
			std::string strDbPrivate = "Freeze.DbEnv." + db;
			strDbPrivate = strDbPrivate + ".DbPrivate";
			proper->setProperty(strDbPrivate,"0");
			glog(ZQ::common::Log::L_INFO,ICEDBFMT("set ice freeze property [%s--->%s]"),
				strDbPrivate.c_str() , "0");
		}
	
		m_pPlaylistItemContent=		new TianShanIce::Streamer::IndexItemToPlaylist("PlaylistItemAttr");
		//PLI_CATALOG
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back (m_pPlaylistItemContent);			
			
			try
			{
				
#if ICE_INT_VERSION / 100 >= 303			
			m_pPlaylistItemContentEvictor=Freeze::createBackgroundSaveEvictor (m_pAdpter,db,PLI_CATALOG,0,indices);
#else
			m_pPlaylistItemContentEvictor=Freeze::createEvictor (m_pAdpter,db,PLI_CATALOG,0,indices);
#endif
			}
			catch(...)
			{
				glog(Log::L_ERROR,ICEDBFMT("Unexpect error when create playlist item evictor"));
			}
			m_pAdpter->addServantLocator (m_pPlaylistItemContentEvictor,PLI_CATALOG);		
			m_pPlaylistItemContentEvictor->setSize (gStreamSmithConfig.lEvictorItemSize>100?
															gStreamSmithConfig.lEvictorItemSize:100);
		}
		
		//////////////////////////////////////////////////////////////////////////		
		{
			try
			{
				
#if ICE_INT_VERSION / 100 >= 303			
			m_pPlaylistContentEvictor=Freeze::createBackgroundSaveEvictor(m_pAdpter,db,PL_CATALOG);
#else
			m_pPlaylistContentEvictor=Freeze::createEvictor(m_pAdpter,db,PL_CATALOG);
#endif
			}
			catch(...)
			{
				glog(Log::L_ERROR,ICEDBFMT("Unexpect error when create playlist evictor"));
			}
			m_pAdpter->addServantLocator(m_pPlaylistContentEvictor,PL_CATALOG);
			m_pPlaylistContentEvictor->setSize(gStreamSmithConfig.lEvictorPlaylistSize>100?
															gStreamSmithConfig.lEvictorPlaylistSize:100);
		}

	TRY_END("bool PlaylistInfoStore::Init ()##");
	mDbPath = db;

	return true;
}
bool PlaylistInfoStore::SetupPlaylistInfo ( PLAYLISTRECONSTRUCTCALLBACK plCallback, 
											PLAYLISTITEMRECONSTRUCTCALLBACK pliCallback,
											void* pData)
{
	if(NULL==plCallback)
	{
		return false;
	}
	if(NULL==pliCallback)
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	//Read play list guid and attribute
	TianShanIce::Streamer::PlaylistDict::iterator it;
	TianShanIce::Streamer::PlaylistAttr	attr;
	TRY_BEGIN();
		//glog(Log::L_DEBUG,"bool PlaylistInfoStore::SetupPlaylistInfo ()##Total %d playlist was found",m_pPlaylistContent->size());
		typedef std::vector<Ice::Identity>	RESULT;
		RESULT	r;
		::Freeze::EvictorIteratorPtr	itPtr;
		itPtr=m_pPlaylistContentEvictor->getIterator("",1024);		
		r.clear();
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				r.push_back(itPtr->next());
			}
			for(RESULT::iterator it=r.begin();it!=r.end();it++)
			{
				//m_pPlaylistContentEvictor->createprox(*it);
				TianShanIce::Streamer::InternalPlaylistExPrx	prx=TianShanIce::Streamer::InternalPlaylistExPrx::uncheckedCast(m_pAdpter->createProxy(*it));
				if(prx)
				{
					attr=prx->getAttr();
					(*plCallback)(attr.Guid,attr,pData);
					ReadPliAttr(attr.Guid,pliCallback,pData);
				}
			}
		}			

	TRY_END("bool PlaylistInfoStore::SetupPlaylistInfo()");
	
	confirmDB( mDbPath );

	return true;
}
bool PlaylistInfoStore::ReadPliAttr (const std::string guid,PLAYLISTITEMRECONSTRUCTCALLBACK pliCallback,void* pData)
{
	typedef std::vector<Ice::Identity>	RESULT;
	TRY_BEGIN();
		RESULT	r=m_pPlaylistItemContent->find(guid);
		RESULT::iterator it;
		TianShanIce::Streamer::PlaylistItemPrx	prx;
		VECPlaylistItemAttr	vecAttr;
		vecAttr.clear();
		glog(Log::L_DEBUG,
					ICEDBFMT("void PlaylistInfoStore::ReadPliAttr()##total %d playlist item was found with GUID=%s"),
					r.size(),guid.c_str());
		TianShanIce::Streamer::PlaylistItemAttr	attr;
		for(it=r.begin ();it!=r.end ();it++)
		{
			std::string	strTest;
			//strTest=Freeze::IdentityToString
			prx=TianShanIce::Streamer::PlaylistItemPrx::uncheckedCast (m_pAdpter->createProxy(*it));
			if(!prx)
				continue;			
			attr=prx->getAttr ();
			//////////////////////////////////////////////////////////////////////////
			std::string strUUID=m_pCommunicatorPtr->proxyToString(prx);
			glog(Log::L_DEBUG,ICEDBFMT("Get a Playlist item with index=%d and vstrmSessID=%d and UUID=%s"),attr.InternalCtrlNum,attr.vStrmSessID,strUUID.c_str());
			//////////////////////////////////////////////////////////////////////////
			
			vecAttr.push_back(attr);
		}
		(*pliCallback)(guid,vecAttr,pData);
	TRY_END("void PlaylistInfoStore::ReadPliAttr ()##");
	return true;
}
bool PlaylistInfoStore::ClearPlaylistAttr(const std::string& strGuid)
{	
	if(strGuid.empty())
		return false;
	
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_plMutex,__MSGLOC__);
//		Freeze::TransactionHolder holder(m_pConnectionPtr);	
		
		Ice::Identity id;
		id.category=PL_CATALOG;
		id.name=strGuid;
		if(m_pPlaylistContentEvictor->hasObject(id))
		{

			//glog(ZQ::common::Log::L_DEBUG,ICEDBFMT("playlist record category=%s name=%s is cleared"),id.category.c_str(),id.name.c_str());
//			m_pPlaylistContentEvictor->destroyObject(id);
			:: Ice :: ObjectPtr objPtr= m_pPlaylistContentEvictor->remove(id);
			objPtr=NULL;
		}
		
//		holder.commit();
	TRY_END("bool PlaylistInfoStore::ClearPlaylistAttr()##");
	return true;
}
bool PlaylistInfoStore::ClearPlaylistItemAttr(const std::string& strGuid)
{
	if(strGuid.empty())
		return false;	
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_pliMutex,__MSGLOC__);
//		Freeze::TransactionHolder holder(m_pConnectionPtr);
		//Delete play list item attribute first if found		
		typedef std::vector<Ice::Identity>	RESULT;
		RESULT	r=m_pPlaylistItemContent->find(strGuid);
		for(RESULT::iterator itClear=r.begin();itClear!=r.end();itClear++)
		{
//			glog(	ZQ::common::Log::L_DEBUG,
//					ICEDBFMT("playlistItem record with playlist=%s category=%s name=%s is cleared"),
//					strGuid.c_str(),itClear->category.c_str(),itClear->name.c_str());
			
//			m_pPlaylistItemContentEvictor->destroyObject(*itClear);	
			:: Ice :: ObjectPtr objPtr= m_pPlaylistItemContentEvictor->remove(*itClear);
			objPtr=NULL;
		}
//		holder.commit();
	TRY_END("bool PlaylistInfoStore::ClearPlaylistItemAttr()##");
	return true;		 
}
bool PlaylistInfoStore::ClearAll()
{
	TRY_BEGIN()
//		ZQ::common::MutexGuard gdpl(m_plMutex,__MSGLOC__);
//		ZQ::common::MutexGuard gdpli(m_pliMutex,__MSGLOC__);
//		Freeze::TransactionHolder holder(m_pConnectionPtr);

//		m_pPlaylistContent->clear();		
		
		typedef std::vector<Ice::Identity>	RESULT;
		::Freeze::EvictorIteratorPtr itPtr=m_pPlaylistItemContentEvictor->getIterator("",1024);
		RESULT r;
		r.clear();
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				r.push_back(itPtr->next());
			}
			for(RESULT::iterator it=r.begin();it!=r.end();it++)
			{
				Ice::ObjectPtr objPtr=	 m_pPlaylistItemContentEvictor->remove(*it);
				objPtr = NULL;
			}
		}		
		//////////////////////////////////////////////////////////////////////////
		itPtr=m_pPlaylistContentEvictor->getIterator("",1024);		
		r.clear();
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				r.push_back(itPtr->next());
			}
			for(RESULT::iterator it=r.begin();it!=r.end();it++)
			{
//				m_pPlaylistContentEvictor->destroyObject(*it);
				:: Ice :: ObjectPtr objPtr=m_pPlaylistContentEvictor->remove(*it);
				objPtr=NULL;				
			}
		}		
//		holder.commit();
	TRY_END("bool PlaylistInfoStore::ClearAll()##");
	return true;
}

bool PlaylistInfoStore::AddPlaylistAttr(const std::string& strGuid, 
										   const TianShanIce::Streamer::PlaylistAttr& plAttr)
{
	if(strGuid.empty())
		return false;	
	TRY_BEGIN()
		::Ice::Identity id;
		id.category=PL_CATALOG;
		id.name=strGuid;
		TianShanIce::Streamer::InternalPlaylistExPtr pEx=new ZQ::StreamSmith::PlaylistExI(id,strGuid,plAttr,m_pAdpter);
		
		
		DWORD dwStart = GetTickCount();
		Ice::ObjectPrx prx= m_pPlaylistContentEvictor->add(pEx,id);
		glog(ZQ::common::Log::L_DEBUG,ICEDBFMT("Add playlist [%s] into db use time count =[%u]"),
			id.name.c_str(),GetTickCount()-dwStart);

	
	
	TRY_END("bool PlaylistInfoStore::AddPlaylistAttr() ");
	return true;
}
bool PlaylistInfoStore::UpdatePlaylistAttr(const std::string& strGuid, 
										   const TianShanIce::Streamer::PlaylistAttr& plAttr)
{
	if(strGuid.empty())
		return false;	
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_plMutex,__MSGLOC__);
//		Freeze::TransactionHolder holder(m_pConnectionPtr);

		::Ice::Identity	id;
		id.category=PL_CATALOG;
		id.name=strGuid;
		DWORD	dwStart = GetTickCount();
		if(!m_pPlaylistContentEvictor->hasObject(id))		
		{
			glog(ZQ::common::Log::L_DEBUG,ICEDBFMT("Check if playlist [%s] exist use time count =[%u]"),
				id.name.c_str(),GetTickCount()-dwStart);
			::Ice::Identity id;
			id.category=PL_CATALOG;
			id.name=strGuid;
			TianShanIce::Streamer::InternalPlaylistExPtr pEx=new ZQ::StreamSmith::PlaylistExI(id,strGuid,plAttr,m_pAdpter);


			dwStart = GetTickCount();
			Ice::ObjectPrx prx= m_pPlaylistContentEvictor->add(pEx,id);
			glog(ZQ::common::Log::L_DEBUG,ICEDBFMT("Add playlist [%s] into db use time count =[%u]"),
				id.name.c_str(),GetTickCount()-dwStart);

		}
		else
		{			
			TianShanIce::Streamer::InternalPlaylistExPrx prx=TianShanIce::Streamer::InternalPlaylistExPrx::checkedCast(m_pAdpter->createProxy(id));
			if(prx!=NULL)
			{
				prx->updateAttr(plAttr);
			}
		}

//		holder.commit ();
	TRY_END("bool PlaylistInfoStore::UpdatePlaylistAttr() ");
	return true;
}
bool PlaylistInfoStore::UpdatePlaylistItemAttr (const std::string& strGuid,  VECPlaylistItemAttr& pliAttr)
{
	if(strGuid.empty ())
		return false;	
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_pliMutex,__MSGLOC__);
//		Freeze::TransactionHolder holder(m_pConnectionPtr);


		//Delete play list item attribute first if found
		typedef std::vector<Ice::Identity>	RESULT;
		RESULT	r=m_pPlaylistItemContent->find(strGuid);
		for(RESULT::iterator itClear=r.begin();itClear!=r.end();itClear++)
		{
			Ice::ObjectPtr objPtr = m_pPlaylistItemContentEvictor->remove(*itClear);
			objPtr = NULL;
//			glog(ZQ::common::Log::L_DEBUG,
//				ICEDBFMT("playlistitem record with playlist=%s catagory=%s name=%s is cleared "),
//					strGuid.c_str(),itClear->category.c_str(),itClear->name.c_str());

		}
		
		VECPlaylistItemAttr::iterator it;
		for(it=pliAttr.begin ();it!=pliAttr.end ();it++)
		{
			TianShanIce::Streamer::PlaylistItemPtr pItem=new  ZQ::StreamSmith::PlaylistItemI;
			pItem->guid=strGuid;
			pItem->updateAttr (*it);			
			Ice::Identity	id;
			id.category=PLI_CATALOG;
			id.name=IceUtil::generateUUID();
			
			//////////////////////////////////////////////////////////////////////////
			//TianShanIce::Streamer::PlaylistItemAttr attr=pItem->getAttr();			
			//glog(Log::L_DEBUG,"store a play list item index =%d  and vstrmSessID=%d and UUID=%s",attr.InternalCtrlNum,attr.vStrmSessID,id.name.c_str());
			//////////////////////////////////////////////////////////////////////////
			
			m_pPlaylistItemContentEvictor->add (pItem,id);	
//			glog(ZQ::common::Log::L_DEBUG,
//						ICEDBFMT("add new playlistItem record with playlist=%s catagory=%s name=%s"),
//						strGuid.c_str(),id.category.c_str(),id.name.c_str());
		}

//		holder.commit ();
	TRY_END("bool PlaylistInfoStore::UpdatePlaylistItemAttr() ");
	return true;
}
int	PlaylistInfoStore::GetPlaylistItemSize()
{
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_pliMutex,__MSGLOC__);

		::Freeze::EvictorIteratorPtr itPtr = m_pPlaylistItemContentEvictor->getIterator("",1024);
		int i=0;
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				i++;
				itPtr->next();
			}
			return i;
		}			
		return 0;
	TRY_END("bool PlaylistInfoStore::GetPlaylistItemSize() ");
	return -1;
}
int	PlaylistInfoStore::GetPlaylistSize()
{
	TRY_BEGIN()
//		ZQ::common::MutexGuard gd(m_plMutex,__MSGLOC__);
		::Freeze::EvictorIteratorPtr itPtr=m_pPlaylistItemContentEvictor->getIterator("",1024);
		int i=0;
		if(itPtr)
		{
			while (itPtr->hasNext()) 
			{
				i++;
				itPtr->next();
			}
			return i;
		}			
		return 0;
	TRY_END("bool PlaylistInfoStore::GetPlaylistSize() ");
	return -1;
}

#endif//_ICE_INTERFACE_SUPPORT

}}//namespace
