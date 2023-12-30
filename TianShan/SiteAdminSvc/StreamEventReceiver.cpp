
#include <tianshandefines.h>
#include <time.h>
#include "StreamEventReceiver.h"

namespace ZQTianShan {
namespace Site {

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//StreamProgressSinkI::StreamProgressSinkI(EventSenderManager& ssMan):_ssMan(ssMan)
//{
//}
//void StreamProgressSinkI::OnProgress(const ::std::string& proxy, const ::std::string& uid,
//				::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalStep,
//				const ::std::string&, const ::Ice::Current& ic /*= ::Ice::Current()*/)const
//{
//	//marshal
//	
//}
//void StreamProgressSinkI::ping(::Ice::Long iL, const ::Ice::Current& ic/*= ::Ice::Current()*/)
//{
//}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


StreamEventSinkI ::StreamEventSinkI(EventSenderManager& ssMan):_ssMan(ssMan)
{
}
void StreamEventSinkI::ping(::Ice::Long iL, const ::Ice::Current& ic/* = ::Ice::Current( */)
{
}
void StreamEventSinkI::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic/* = ::Ice::Current( */) const
{
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Stream";
	msg.eventName	= "EndOfStream";
	msg.id			= 0001;

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

	msg.property["streamId"] = uid;
	//msg.property["stampLocal"] = FormatLocalTime(buf,sizeof(buf));

	_ssMan.PostEvent(msg);
	
}
void StreamEventSinkI::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic/* = ::Ice::Current( */) const
{
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Stream";
	msg.eventName	= "BeginningOfStream";
	msg.id			= 0002;

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);

	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

	msg.property["streamId"] = uid;
	//msg.property["stampLocal"] = FormatLocalTime(buf,sizeof(buf));

	_ssMan.PostEvent(msg);
}

const char* getStreamStateString(::TianShanIce::Streamer::StreamState st)
{
	switch(st) 
	{
	case TianShanIce::Streamer::stsSetup:
		return "0";
		break;
	case TianShanIce::Streamer::stsStreaming :
		return "1";
		break;
	case TianShanIce::Streamer::stsPause :
		return "2";
		break;
	case TianShanIce::Streamer::stsStop:
		return "3";
		break;
	default:
		return "Unkown State";
		break;
	}
}

void StreamEventSinkI::OnStateChanged(const ::std::string& proxy,
									  const ::std::string& uid, 
									  ::TianShanIce::Streamer::StreamState prevState, 
									  ::TianShanIce::Streamer::StreamState currentState, 
									  const TianShanIce::Properties& props, 
									  const ::Ice::Current& ic/* = ::Ice::Current( */) const
{
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Stream";
	msg.eventName	= "StreamStateChanged";
	msg.id			= 0003;

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);

	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";
	
	msg.property["streamId"]	= uid;
	//msg.property["stampLocal"]	= FormatLocalTime(buf,sizeof(buf));

	msg.property["previousState"] = getStreamStateString(prevState);
	msg.property["currentState"] = getStreamStateString(currentState);

	_ssMan.PostEvent(msg);
}
void StreamEventSinkI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props, const ::Ice::Current& ic/* = ::Ice::Current( */) const
{
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Stream";
	msg.id			= 0004;
	msg.eventName	= "StreamSpeedChanged";

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);

	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

	msg.property["streamId"] = uid;
	sprintf(buf,"%f",prevSpeed);
	msg.property["previousSpeed"] = buf;
	sprintf(buf,"%f",currentSpeed);
	msg.property["currentSpeed"] = buf;

	//msg.property["stampLocal"] = FormatLocalTime(buf,sizeof(buf));

	_ssMan.PostEvent(msg);
}
void StreamEventSinkI::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& /* = ::Ice::Current( */) const
{
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Stream";
	msg.id			= 0005;
	msg.eventName	= "StreamExit";

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
	
	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";
	
	msg.property["streamId"] = uid;	
	char	szBuf[256];
	ZeroMemory(szBuf,sizeof(szBuf));
	msg.property["exitCode"] = itoa(exitCode,szBuf,10);

	//msg.property["stampLocal"] = FormatLocalTime(buf,sizeof(buf));
	
	_ssMan.PostEvent(msg);
	
}

void StreamEventSinkI::OnExit2(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::TianShanIce::Properties& props, const ::Ice::Current&) const
{
#pragma message(__MSGLOC__"TODO: implement new interface OnExit2()")
}

PlaylistEventSinkI::PlaylistEventSinkI(EventSenderManager& ssMan):_ssMan(ssMan)
{	
	
}

void PlaylistEventSinkI::OnItemStepped(const ::std::string& proxy, const ::std::string& uid,
									   ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum,
									   const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& /* = ::Ice::Current( */) const
{
	//				pro.insert(std::make_pair<std::string,std::string>("prevItemName",strPrevFileName));
	//				pro.insert(std::make_pair<std::string,std::string>("currentItemName",strnextFileName));
	//				currentItemTimeOffset

	::TianShanIce::Properties ItemProp = ItemProps;
	char buf[256];
	MSGSTRUCT msg;
	msg.category	= "Playlist";
	msg.id			= 0001;
	msg.eventName	= "ItemStepped";

	ZeroMemory(buf,sizeof(buf));
	msg.timestamp	= SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
	
	ZeroMemory(buf,sizeof(buf));
	msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";
	
	msg.property["playlistId"] = uid;	
	char	szBuf[256];
	ZeroMemory(szBuf,sizeof(szBuf));
	msg.property["currentUserCtrlNum"]			= itoa(currentUserCtrlNum,szBuf,10);
	msg.property["prevUserCtrlNum"]				= itoa(prevUserCtrlNum,szBuf,10);
	msg.property["currentItem"]					= ItemProp["currentItemName"];
	msg.property["prevItem"]					= ItemProp["prevItemName"];
	msg.property["currentItemTimeOffset"]		= ItemProp["currentItemTimeOffset"];
	
	//msg.property["stampLocal"] = FormatLocalTime(buf,sizeof(buf));
	
	_ssMan.PostEvent(msg);
}
}}//namespace ZQTianShan::Site