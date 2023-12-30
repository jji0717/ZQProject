
#include <sstream>
#include "HttpSession.h"
#include <TimeUtil.h>


HttpSessionFactory::HttpSessionFactory( )
:mTotalBytes(0)
{
}

HttpSessionFactory::~HttpSessionFactory()
{
}

SimpleHttpSessionPtr HttpSessionFactory::createSession()
{
	return new SimpleHttpSession(mRecorder,*this);
}

void HttpSessionFactory::registerEventHandler( SimpleSessionEvent startEvent , SimpleSessionEvent completeEvent )
{
	mSessionCompleteEvent	= completeEvent;
	mSessionStartEvent		= startEvent;
}

void HttpSessionFactory::onSessionComplete( SimpleHttpSessionPtr sess )
{
	bool bSignal = false;
	{
		ZQ::common::MutexGuard gd(mMutex);
		mSessions.erase( sess );
		bSignal = mSessions.size() <= 0;
	}
	if( bSignal && mSessionCompleteEvent )
	{
		mSessionCompleteEvent();		
	}

}

void HttpSessionFactory::onSessionCreated( SimpleHttpSessionPtr sess )
{
	bool bSignal = false;
	{
		ZQ::common::MutexGuard  gd(mMutex);
		bSignal = mSessions.size() == 0 ;
		mSessions.insert( sess );		
	}
	if( bSignal && mSessionStartEvent )
	{
		mSessionStartEvent();
	}
}

SessionDataRecorder& HttpSessionFactory::getRecorder( )
{
	return mRecorder;
}

uint64 HttpSessionFactory::getTotalBytes( ) const
{
	uint64 ret = 0;
	{
		ZQ::common::MutexGuard gd(mMutex);
		ret = mTotalBytes;
	}
	return ret;
}

void HttpSessionFactory::resetTotalBytes( )
{
	ZQ::common::MutexGuard gd(mMutex);
	mTotalBytes = 0;
}

void HttpSessionFactory::updateTotalByte( uint64 byteInc )
{
	ZQ::common::MutexGuard gd(mMutex);
	mTotalBytes += byteInc;
}

//////////////////////////////////////////////////////////////////////////
///

SimpleHttpSession::SimpleHttpSession( SessionDataRecorder& recorder , HttpSessionFactory& fac)
:mbFirstData(true),
mRecorder(recorder),
mDataRecorder(NULL),
mSessFac(fac)
{
}
SimpleHttpSession::~SimpleHttpSession()
{

}

void SimpleHttpSession::onCreated( ZQ::DataPostHouse::IDataCommunicatorPtr comm )
{
	mComm = comm;
	mSessFac.onSessionCreated( this );
}

void SimpleHttpSession::onHeadersComplete( HttpMessagePtr msg )
{
	mMsg = msg;
	if( msg )
	{
		int retCode = atoi(msg->getUrl().c_str());
		if( retCode / 100 != 2 )
		{
			if(mComm)
				mComm->close();
		}
	}
}


void SimpleHttpSession::onBadBodyContent( const char* data , size_t size )
{

}

bool SimpleHttpSession::onBodyContent( const char* data , size_t size ,bool bComplete )
{
	HttpSessionDataRecordInfo r;
	r.time	= ZQ::common::now();
	r.size	= size;
	if( mbFirstData )
	{
		std::ostringstream tag;tag<<(mMsg->getUrl());
		tag<<"=="<<mComm->getCommunicatorId();
		ZQ::DataPostHouse::SharedObjectPtr userData = mComm->getUserData();
		if( userData )
		{
			tag.str("");
			HttpSessionUserDataPtr sessUserData = HttpSessionUserDataPtr::dynamicCast(userData);
			if( sessUserData)
			{
				tag << sessUserData->getTag();
			}			
		}
		//mDataRecorder = mRecorder.createDataRecord( tag.str() , r.time );
		mbFirstData = false;
	}	
	
	if( r.size > 0   )
	{
		if(mDataRecorder)
			mDataRecorder->put( r );
		mSessFac.updateTotalByte( r.size );
	}

	if( bComplete )
	{
		onMessageComplete();
	}
	return true;
}

void SimpleHttpSession::onMessageComplete( )
{
	if(mComm)
	{
		mSessFac.onSessionComplete( this );
		mComm->close();
	}
	mComm = NULL;
}

