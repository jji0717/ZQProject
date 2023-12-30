// StreamEventSinkI.cpp: implementation of the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamEventSinkI.h"
#include "NGODEnv.h"
#include <HelperClass.h>
#include <TianShanIceHelper.h>

#define ANNOUNCELOG _pSsmNGODr2c1->_fileLog

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StreamEventSinkI::~StreamEventSinkI()
{
}

StreamEventSinkI::StreamEventSinkI(NGODEnv* pSsmNGODr2c1) : _pSsmNGODr2c1(pSsmNGODr2c1)
{
}

void StreamEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
}

void StreamEventSinkI::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic)const
{
	ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(End-of-Stream): [%s]"), proxy.c_str());

	try
	{
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "End-of-Stream";
		::std::vector<::Ice::Identity> idents;
		idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
		{
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "unknown streamID[%s], ignore the event"), uid.c_str());
			return;
		}

		//NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ctxData  NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
			return;

		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Sess(%s) start processing Event(End-of-Stream): [%s]"), NewContext.ident.name.c_str(), proxy.c_str());

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = NewContext.ident.name;
		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _pSsmNGODr2c1->_globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			pNewContextPrx->increaseAnnSeq();
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", NewContext.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		::std::string responseHead;
		responseHead = "ANNOUNCE " + NewContext.normalURL + " RTSP/1.0";

		std::string notice_str;
		notice_str = NGOD_ANNOUNCE_ENDOFSTREAM " \"" NGOD_ANNOUNCE_ENDOFSTREAM_STRING "\" " "event-date=";
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;
		
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;
		
		/*if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION_HEX];
		}*/

		const Ice::Context& ctx = ic.ctx;
		Ice::Context::const_iterator itEndTimeOffset = ctx.find("EndTimeOffset");
		if( itEndTimeOffset != ctx.end() )
		{
			// snprintf( szTemp , sizeof(szTemp) - 1, "%x" , lEndOffset );
			long lEndOffset = atol(itEndTimeOffset->second.c_str() );
			char szTemp[64];
			if(_ngodConfig._MessageFmt.rtspNptUsage>= 1)
			{
				int main = lEndOffset / 1000;
				int fraction = lEndOffset % 1000;
				snprintf( szTemp , sizeof(szTemp) - 1, "%d.%d" , main, fraction );
			}
			else
			{
				if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
					atoi(NewContext.prop["RequireC1"].c_str()) == NgodVerCode_C1_DecNpt)
				{
					int main = lEndOffset / 1000;
					int fraction = lEndOffset % 1000;
					snprintf( szTemp , sizeof(szTemp) - 1, "%d.%d" , main, fraction );
				}
				else
				{
					snprintf( szTemp , sizeof(szTemp) - 1, "%x" , lEndOffset );
				}
			}

			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "OnEndOfStream() read event parameters: %s"), szTemp);
			pos_str = szTemp;
		}
		notice_str += pos_str;

		// add by zjm to support session history
		if (_ngodConfig._sessionHistory.enableHistory > 0)
		{
			NGODr2c1::SessionEventRecord sessionEvent;
			sessionEvent.eventType = NGODr2c1::EndEvent;
			sessionEvent.eventTime = NgodUtilsClass::generatorISOTime();
			sessionEvent.NPT = "EOS"; 
			char buffer[256];
			sprintf_s(buffer, "%d", NewContext.setupInfos.size());
			sessionEvent.streamResourceID = buffer;

			sessionEvent.prop["reason"] = "SERVER";
			pNewContextPrx->addEventRecord(sessionEvent);
			pNewContextPrx->updateCtxProp("hasEndEvent", "true");
		}

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop); // server requests' smart pointer
		::std::string C1ConnId = pNewContextPrx->getCtxPropItem(C1CONNID);
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), C1ConnId);
		
		if (NULL == pSettop)
		{
			ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "session[%s] failed to create ANNOUNCE context to send"), NewContext.ident.name.c_str());
			return;
		}

		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		if (atoi(NewContext.prop["RequireC1"].c_str()) == NgodVerCode_C1)
		{
			pSettop->printHeader("Require" , "com.comcast.ngod.c1");
		}
		else
		{
			pSettop->printHeader("Require", "com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
		}

		pSettop->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] ANNOUNCE(" NGOD_ANNOUNCE_ENDOFSTREAM_STRING " ) has been posted"),  NewContext.ident.name.c_str());

		/*		
		std::string currentConnID;
		currentConnID = _pSsmNGODr2c1->getCurrentConnID(NewContext);

		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm); // server requests' smart pointer
		pOdrm = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), currentConnID);

		if (NULL != pOdrm)
		{
		pOdrm->printCmdLine(responseHead.c_str());
		pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pOdrm->printHeader("Require" , "com.comcast.ngod.r2");
		pOdrm->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request on connectID: [%s]"), NewContext.ident.name.c_str(), currentConnID.c_str());
*/
	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process End-of-Stream caught an exception, stream[%s]"), proxy.c_str());
	}
}

void StreamEventSinkI::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic)const
{
	ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(Beginning-of-Stream): [%s]"), proxy.c_str());

	try
	{
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "Begin-of-Stream";
		::std::vector<::Ice::Identity> idents;
		idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
		{
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "unknown streamID[%s], ignore the event"), uid.c_str());
			return;
		}

		//NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
			return;

		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Sess(%s), Event(Beginning-of-Stream): [%s]"), NewContext.ident.name.c_str(), proxy.c_str());

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = NewContext.ident.name;
		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _pSsmNGODr2c1->_globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			pNewContextPrx->increaseAnnSeq();
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", NewContext.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		::std::string responseHead;
		responseHead = "ANNOUNCE " + NewContext.normalURL + " RTSP/1.0";
		
		std::string notice_str;
		notice_str = NGOD_ANNOUNCE_BEGINOFSTREAM " \"" NGOD_ANNOUNCE_BEGINOFSTREAM_STRING "\" " "event-date=";
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;
		
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "OnBeginningOfStream : the latest C1 reqire is [%s]"), NewContext.prop["RequireC1"].c_str());
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION_HEX];
		}
		
		notice_str += pos_str;

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop);
		::std::string C1ConnId = pNewContextPrx->getCtxPropItem(C1CONNID);
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), C1ConnId);

		if (NULL == pSettop)
		{
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "session[%s] failed to create ANNOUNCE context to send"), NewContext.ident.name.c_str());
			return ;
		}

		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		if (atoi(NewContext.prop["RequireC1"].c_str()) == NgodVerCode_C1)
		{
			pSettop->printHeader("Require" , "com.comcast.ngod.c1");
		}
		else
		{
			pSettop->printHeader("Require", "com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
		}
		pSettop->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] ANNOUNCE(" NGOD_ANNOUNCE_BEGINOFSTREAM_STRING " ) has been posted"),  NewContext.ident.name.c_str());
		
			

/*
		// DO: get current session's connnectionID established between ODRM and stream server.
		std::string currentConnID;
		currentConnID = _pSsmNGODr2c1->getCurrentConnID(NewContext);

		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), currentConnID);

		if (NULL != pOdrm)
		{
		pOdrm->printCmdLine(responseHead.c_str());
		pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pOdrm->printHeader("Require" , "com.comcast.ngod.r2");
		pOdrm->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request on connectID: [%s]"), NewContext.ident.name.c_str(), currentConnID.c_str());
*/
	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process Beginning-of-Stream caught an exception, stream: [%s]"), proxy.c_str());
	}
}

void StreamEventSinkI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic)const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(Scale Changed): [%s] from [%f] to[%f]"), proxy.c_str() , prevSpeed , currentSpeed );
	if( _ngodConfig._announce._useTianShanAnnounceCodeScaleChanged <=0 )
		return;

	try
	{
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "Speed-Change";
		::std::vector<::Ice::Identity> idents;
		idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
		{
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "unknown streamID[%s], ignore the event"), uid.c_str());
			return;
		}

		//NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
			return;

		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Sess(%s), Event(Beginning-of-Stream): [%s]"), NewContext.ident.name.c_str(), proxy.c_str());

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = NewContext.ident.name;
		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _pSsmNGODr2c1->_globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			pNewContextPrx->increaseAnnSeq();
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", NewContext.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;

		::std::string responseHead;
		responseHead = "ANNOUNCE " + NewContext.normalURL + " RTSP/1.0";

		std::string notice_str;
		notice_str = NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE " \"" NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE_STRING "\" " "event-date=";
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;

		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Scale Changed : the latest C1 reqire is [%s]"), NewContext.prop["RequireC1"].c_str());
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION];
		}

		notice_str += pos_str;

		{
			char scaleBuf[32];
			sprintf( scaleBuf , " scale=%f ", currentSpeed);
			notice_str += scaleBuf;
		}

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop);
		::std::string C1ConnId = pNewContextPrx->getCtxPropItem(C1CONNID);
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), C1ConnId);

		if (NULL == pSettop)
		{
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "session[%s] failed to create ANNOUNCE context to send"), NewContext.ident.name.c_str());
			return;
		}
		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		if (atoi(NewContext.prop["RequireC1"].c_str()) == NgodVerCode_C1)
		{
			pSettop->printHeader("Require" , "com.comcast.ngod.c1");
		}
		else
		{
			pSettop->printHeader("Require", "com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
		}
		pSettop->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] ANNOUNCE(" NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE_STRING " ) has been posted"),  NewContext.ident.name.c_str());

	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process Scale-Change caught an exception, stream: [%s]"), proxy.c_str());
	}
	
}

void StreamEventSinkI::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState curState, const ::Ice::Current& ic/* = ::Ice::Current */)const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(State Changed): [%s]"), proxy.c_str());
	if( _ngodConfig._announce._useTianShanAnnounceCodeStateChanged <=0 )
		return;
	try
	{
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "State-Change";
		::std::vector<::Ice::Identity> idents;
		idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(uid, 1);
		if (idents.size() == 0)
		{
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "unknown streamID[%s], ignore the event"), uid.c_str());
			return;
		}

		//NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
			return;

		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Sess(%s), Event(State Changed): [%s]"), NewContext.ident.name.c_str(), proxy.c_str());

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = NewContext.ident.name;
		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _pSsmNGODr2c1->_globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			pNewContextPrx->increaseAnnSeq();
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", NewContext.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;

		::std::string responseHead;
		responseHead = "ANNOUNCE " + NewContext.normalURL + " RTSP/1.0";

		std::string notice_str;
		notice_str = NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE " \"" NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE_STRING "\" " " event-date=";
	
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ presentation_state=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;
		
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string state_str;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "State Changed : The latest C1 reqire is [%s]"), NewContext.prop["RequireC1"].c_str());
			//_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			switch (curState)
			{
			case TianShanIce::Streamer::stsSetup:
				state_str = " init";
				break;
			case TianShanIce::Streamer::stsStreaming:
				state_str = " play";
				break;
			case TianShanIce::Streamer::stsPause:
				state_str = " pause";
				break;
			case TianShanIce::Streamer::stsStop:
				state_str = " ready";
				break;
			default:
				state_str = "unknown";
				break;
			}
		}		
		notice_str += state_str;

		/*::TianShanIce::Streamer::StreamPrx streamPrx = NULL;*/
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "OnBeginningOfStream : the latest C1 reqire is [%s]"), NewContext.prop["RequireC1"].c_str());
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			pos_str = " npt="+inoutMap[MAP_KEY_STREAMPOSITION_HEX];
		}
		notice_str += pos_str;

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop);
		::std::string C1ConnId = pNewContextPrx->getCtxPropItem(C1CONNID);
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), C1ConnId);

		if (NULL == pSettop)
		{
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "session[%s] failed to create ANNOUNCE context to send"), NewContext.ident.name.c_str());
			return;
		}
		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		if (atoi(NewContext.prop["RequireC1"].c_str()) == NgodVerCode_C1)
		{
			pSettop->printHeader("Require" , "com.comcast.ngod.c1");
		}
		else
		{
			pSettop->printHeader("Require", "com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
		}
		pSettop->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] ANNOUNCE(" NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE_STRING " ) has been posted"),  NewContext.ident.name.c_str());

	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process State-Change caught an exception, stream: [%s]"), proxy.c_str());
	}

}

void StreamEventSinkI::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(Exit-of-Stream): [%s]"), proxy.c_str());
}
void StreamEventSinkI::OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current&) const
{
}


PlayListEventSinkI::PlayListEventSinkI(NGODEnv* pSsmNGODr2c1)
:_pSsmNGODr2c1(pSsmNGODr2c1)
{

}

PlayListEventSinkI::~PlayListEventSinkI()
{

}


typedef struct _ErrorDescArray 
{
	char*	errorCodeStr;
	char*	errorDescStr;
}ErrorDescArray;

//must sync with error code defined in TsStreamer.Ice
static ErrorDescArray errors[]=
{
	{"",""},
	{NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR,		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING },
	{NGOD_ANNOUNCE_ERROR_READING_CONTENT,		NGOD_ANNOUNCE_ERROR_READING_CONTENT_STRING },
	{NGOD_ANNOUNCE_DOWNSTREAM_FAILURE,			NGOD_ANNOUNCE_DOWNSTREAM_FAILURE_STRING },
	{NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT,	NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING	},
	{NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE,		NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING }
};

void PlayListEventSinkI::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic /* = ::Ice::Current */) const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PlayListEventSinkI, "Event(Item Step): [%s]"), proxy.c_str());
	
	int errorCode	= 0;
	std::string		errDesc;
	std::string		errorItemName;
	ZQTianShan::Util::getPropertyDataWithDefault( ItemProps, "ItemSkipErrorCode", 0 , errorCode );
	ZQTianShan::Util::getPropertyDataWithDefault( ItemProps, "ItemSkipErrorDescription", "" , errDesc );
	ZQTianShan::Util::getPropertyDataWithDefault( ItemProps, "ItemSkipErrorFileName", "" , errorItemName );
	
	// add Element Transition into session history
	// add by zjm to support session history
	if (_ngodConfig._sessionHistory.enableHistory > 0)
	{
		if (prevUserCtrlNum > 0 && currentUserCtrlNum > 0)
		{
			char buffer[256];
			NGODr2c1::SessionEventRecord sessionEvent;
			sessionEvent.eventTime = NgodUtilsClass::generatorISOTime();
			sessionEvent.eventType = NGODr2c1::Transition;
			
			std::string strValue;
			ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "oldNPT", "",  strValue);
			if (strValue.empty())
			{
				sessionEvent.NPT = "EOS";
			}
			else
			{
				int iOffset = atoi( strValue.c_str() );
				sprintf(buffer, "%d.%03d", iOffset/1000, iOffset%1000);
				sessionEvent.NPT =buffer;
			};

			ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "newNPT", "", strValue);
			if (strValue.empty())
			{
				sessionEvent.prop["newNPT"] = "BOS";
			}
			else
			{
				int iOffset = atoi( strValue.c_str() );
				sprintf(buffer, "%d.%03d", iOffset/1000, iOffset%1000);
				sessionEvent.prop["newNPT"] =buffer;
			};

			sprintf(buffer, "%d", prevUserCtrlNum);
			sessionEvent.streamResourceID = buffer;

			sprintf(buffer, "%d", currentUserCtrlNum);
			sessionEvent.prop["newStreamResourcesID"] = buffer;

			sessionEvent.prop["reason"] = "SERVER";

			ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "newState", "", strValue);
			if (strValue == "")
			{
				strValue = "PLAY";
			}
			else
			{
				int state = atoi(strValue.c_str());
				if (TianShanIce::Streamer::stsStreaming == state)
				{
					strValue = "PLAY";
				}
				else
				{
					strValue = "PAUSE";
				}
			}
			sessionEvent.prop["newState"] = strValue;

			ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "scale", "1.000000", strValue);
			sessionEvent.prop["scale"] = strValue;

			try
			{
				::std::vector<::Ice::Identity> idents;
				idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(playlistId, 1);
				if (idents.size() != 0)
				{
					INOUTMAP inoutMap;
					inoutMap[MAP_KEY_METHOD] = "Item Step";
					NGODr2c1::ctxData NewContext;
					NGODr2c1::ContextPrx pNewContextPrx = NULL;
					if (_pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
					{
						pNewContextPrx->addEventRecord(sessionEvent);
					}
				}
			}
			catch (...)
			{
			}		
		}
	}

	if( errorCode == 0 ||  errorCode >= (sizeof(errors)/sizeof(errors[0])) )
	{
		ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PlayListEventSinkI,"Don't send announce message to client"));
		return;
	}
	
	try
	{
		::std::vector<::Ice::Identity> idents;
		idents = _pSsmNGODr2c1->_pStreamIdx->findFirst(playlistId, 1);
		if (0 == idents.size())
		{
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PlayListEventSinkI, "unknown streamID[%s], ignore the event"), playlistId.c_str());
			return;
		}
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "Item Step";
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, NewContext, pNewContextPrx, inoutMap))
		{
			return;
		}
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(PlayListEventSinkI, "Sess(%s), Event(Item Step): [%s]"), NewContext.ident.name.c_str(), proxy.c_str());
		inoutMap[MAP_KEY_SESSION] = NewContext.ident.name;
		std::string sequence;
		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _pSsmNGODr2c1->_globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			pNewContextPrx->increaseAnnSeq();
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", NewContext.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		inoutMap[MAP_KEY_STREAMFULLID] = NewContext.streamFullID;

		::std::string responseHead = "ANNOUNCE " + NewContext.normalURL + " RTSP/1.0";
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str("");
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(PlayListEventSinkI, "Scale Changed : The latest C1 reqire is [%s]"), NewContext.prop["RequireC1"].c_str());
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap, atoi(NewContext.prop["RequireC1"].c_str()));
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION];
			size_t nPos = pos_str.find("-");
			if (nPos != std::string::npos)
			{
				pos_str = pos_str.substr(0, nPos);
			}
		}
		std::string notice_str = NgodUtilsClass::generatorNoticeString( errors[errorCode].errorCodeStr,	errors[errorCode].errorDescStr, pos_str);

		_pSsmNGODr2c1->_sentryLog(ZQ::common::Log::L_ERROR, CLOGFMT(PlayListEventSinkI, "session[%s] ANNOUNCE( %s ), Error: %s,file[%s]"), 
			NewContext.ident.name.c_str(),errors[errorCode].errorDescStr,
			errDesc.c_str(), errorItemName.c_str() );
		
		IServerRequest* pR2 = NULL;
		SmartServerRequest smtSettop(pR2);
		
		//::std::string r2ConnId = pNewContextPrx->getCtxPropItem(C1CONNID);
		//connectID
		::std::string r2ConnId = NewContext.connectID;

		pR2 = _pSsmNGODr2c1->_pSite->newServerRequest(NewContext.ident.name.c_str(), r2ConnId);

		if (NULL == pR2)
		{
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(PlayListEventSinkI, "session[%s] failed to create ANNOUNCE context to send"), NewContext.ident.name.c_str());
			return ;
		}
		pR2->printCmdLine(responseHead.c_str());
		pR2->printHeader(NGOD_HEADER_SESSION, (char*) NewContext.ident.name.c_str());
		pR2->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pR2->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) NewContext.onDemandID.c_str());
		pR2->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pR2->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		
		if (atoi(NewContext.prop["RequirerR2"].c_str()) == NgodVerCode_R2)
		{
			pR2->printHeader("Require" , "com.comcast.ngod.r2");
		}
		else
		{
			pR2->printHeader("Require", "com.comcast.ngod.r2,com.comcast.ngod.r2.decimal_npts");
		}

		pR2->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(PlayListEventSinkI, "session[%s] ANNOUNCE( %s ) has been posted"),  NewContext.ident.name.c_str(),errors[errorCode].errorDescStr );
	}
	catch (...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(PlayListEventSinkI, "process State-Change caught an exception, stream: [%s]"), proxy.c_str());
	}

}

void PlayListEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic /* = ::Ice::Current */)
{

}