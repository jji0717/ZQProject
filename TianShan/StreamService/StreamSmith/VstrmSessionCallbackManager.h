
#ifndef _zq_StreamService_Vstrm_Session_call_back_Manager_header_file_h__
#define _zq_StreamService_Vstrm_Session_call_back_Manager_header_file_h__


#include <map>
#include <Locks.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

namespace ZQ
{
namespace StreamService
{

class VstrmSessionCallbackManager
{
public:
	VstrmSessionCallbackManager( );
	virtual ~VstrmSessionCallbackManager( );

public:
	
	///register playlist and return callbackManage ID
	uint32				registerPlaylist( const std::string& plId );

	///unregister with callbackManage ID
	void				unregisterPlaylist( const uint32 id );
	void				unregisterPlaylist( const std::string& plId );

	std::string			getPlId( const uint32 id ) const;

private:
	typedef std::map< uint32 , std::string>		STREAM2PLMAP;
	typedef std::map<std::string , uint32>		PL2STREAMMAP;
	
	ZQ::common::Mutex		mMapMutex;	
	STREAM2PLMAP			mId2PlMap;
	PL2STREAMMAP			mPl2IdMap;


	uint32					mCallBackId;

};

}}

#endif//_zq_StreamService_Vstrm_Session_call_back_Manager_header_file_h__
