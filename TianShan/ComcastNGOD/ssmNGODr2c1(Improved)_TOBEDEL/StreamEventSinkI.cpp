// StreamEventSinkI.cpp: implementation of the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamEventSinkI.h"
#include "ssmNGODr2c1.h"

#define ANNOUNCELOG _pSsmNGODr2c1->_fileLog

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StreamEventSinkI::~StreamEventSinkI()
{
}

StreamEventSinkI::StreamEventSinkI(ssmNGODr2c1* pSsmNGODr2c1) : _pSsmNGODr2c1(pSsmNGODr2c1)
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
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "streamID: [%s] not found"), uid.c_str());
			return;
		}

		NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, pNewContext, pNewContextPrx, inoutMap))
			return;

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = pNewContext->ident.name;
		if (true == _pSsmNGODr2c1->_config._useGlobalSeq)
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
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", pNewContext->announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		::std::string responseHead;
		responseHead = "ANNOUNCE " + pNewContext->resourceURL + " RTSP/1.0";

		std::string notice_str;
		notice_str = NGOD_ANNOUNCE_ENDOFSTREAM " \"" NGOD_ANNOUNCE_ENDOFSTREAM_STRING "\" " "eventdate=";
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;
		
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = pNewContext->streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap);
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION];
		}
		
		notice_str += pos_str;

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop); // server requests' smart pointer
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(pNewContext->ident.name.c_str());
		
		if (NULL != pSettop)
		{
		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) pNewContext->ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pNewContext->onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pSettop->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request"), pNewContext->ident.name.c_str());
		
		std::string currentConnID;
		currentConnID = _pSsmNGODr2c1->getCurrentConnID(pNewContext);

		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm); // server requests' smart pointer
		pOdrm = _pSsmNGODr2c1->_pSite->newServerRequest(pNewContext->ident.name.c_str(), currentConnID);

		if (NULL != pOdrm)
		{
		pOdrm->printCmdLine(responseHead.c_str());
		pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) pNewContext->ident.name.c_str());
		pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pNewContext->onDemandID.c_str());
		pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pOdrm->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request on connectID: [%s]"), pNewContext->ident.name.c_str(), currentConnID.c_str());
	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process End-of-Stream caught an exception, stream: [%s]"), proxy.c_str());
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
			ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "streamID: [%s] not found"), uid.c_str());
			return;
		}

		NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*_pSsmNGODr2c1);
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (false == _pSsmNGODr2c1->openContext(idents[0].name, pNewContext, pNewContextPrx, inoutMap))
			return;

		std::string sequence;

		inoutMap[MAP_KEY_SESSION] = pNewContext->ident.name;
		if (true == _pSsmNGODr2c1->_config._useGlobalSeq)
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
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", pNewContext->announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		::std::string responseHead;
		responseHead = "ANNOUNCE " + pNewContext->resourceURL + " RTSP/1.0";
		
		std::string notice_str;
		notice_str = NGOD_ANNOUNCE_BEGINOFSTREAM " \"" NGOD_ANNOUNCE_BEGINOFSTREAM_STRING "\" " "eventdate=";
		SYSTEMTIME time;
		GetLocalTime(&time);
		char t[50];
		memset(t, 0, 50);
		snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
		notice_str += t;
		
		::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
		std::string pos_str;
		inoutMap[MAP_KEY_STREAMFULLID] = pNewContext->streamFullID;
		if (true == _pSsmNGODr2c1->getStream(streamPrx, inoutMap))
		{
			_pSsmNGODr2c1->getPositionAndScale(streamPrx, inoutMap);
			pos_str = inoutMap[MAP_KEY_STREAMPOSITION];
		}
		
		notice_str += pos_str;

		IServerRequest* pSettop = NULL;
		SmartServerRequest smtSettop(pSettop);
		pSettop = _pSsmNGODr2c1->_pSite->newServerRequest(pNewContext->ident.name.c_str());

		if (NULL != pSettop)
		{
		pSettop->printCmdLine(responseHead.c_str());
		pSettop->printHeader(NGOD_HEADER_SESSION, (char*) pNewContext->ident.name.c_str());
		pSettop->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pSettop->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pNewContext->onDemandID.c_str());
		pSettop->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pSettop->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pSettop->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request"), pNewContext->ident.name.c_str());
		
		// DO: get current session's connnectionID established between ODRM and stream server.
		std::string currentConnID;
		currentConnID = _pSsmNGODr2c1->getCurrentConnID(pNewContext);

		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSsmNGODr2c1->_pSite->newServerRequest(pNewContext->ident.name.c_str(), currentConnID);

		if (NULL != pOdrm)
		{
		pOdrm->printCmdLine(responseHead.c_str());
		pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) pNewContext->ident.name.c_str());
		pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _pSsmNGODr2c1->_serverHeader.c_str());
		pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pNewContext->onDemandID.c_str());
		pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
		pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
		pOdrm->post();
		}
		else 
			ANNOUNCELOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "[failed] session: [%s] create request on connectID: [%s]"), pNewContext->ident.name.c_str(), currentConnID.c_str());
	}
	catch(...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "process Beginning-of-Stream caught an exception, stream: [%s]"), proxy.c_str());
	}
}

void StreamEventSinkI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic)const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(Scale Changed): [%s]"), proxy.c_str());
}

void StreamEventSinkI::OnStateChanged(const ::std::string& proxy, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const ::Ice::Current& /* = ::Ice::Current */)const
{
#ifdef _DEBUG

#endif
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(State Changed): [%s]"), proxy.c_str());
}

void StreamEventSinkI::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "Event(Exit-of-Stream): [%s]"), proxy.c_str());
}