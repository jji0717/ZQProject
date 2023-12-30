
#include "PlaylistItemI.h"

#ifdef _DEBUG
	#include "adebugmem.h"
#endif





namespace ZQ
{
namespace StreamSmith
{

PlaylistItemI::PlaylistItemI ()
{

}
PlaylistItemI::~PlaylistItemI ()
{

}
void PlaylistItemI::destroy (const ::Ice::Current& /* = ::Ice::Current( */)
{
	RLock sync(*this);
}
::TianShanIce::Streamer::PlaylistItemAttr PlaylistItemI::getAttr(const ::Ice::Current&/* = ::Ice::Current()*/) const
{    
	RLock sync(*this);
	return attr;
}
void PlaylistItemI::updateAttr (const ::TianShanIce::Streamer::PlaylistItemAttr& pliAttr,
								const ::Ice::Current& /* = ::Ice::Current( */)
{
	WLock sync(*this);
	attr.PlaylistGuid=pliAttr.PlaylistGuid;
	attr.InternalCtrlNum=pliAttr.InternalCtrlNum;
	attr.itemSetupInfo.contentName=pliAttr.itemSetupInfo.contentName;
	attr.CtrlNumber=pliAttr.CtrlNumber;
	attr.itemSetupInfo.inTimeOffset=pliAttr.itemSetupInfo.inTimeOffset;
	attr.itemSetupInfo.outTimeOffset=pliAttr.itemSetupInfo.outTimeOffset;
	attr.itemSetupInfo.criticalStart=pliAttr.itemSetupInfo.criticalStart;
	attr.itemSetupInfo.spliceIn=pliAttr.itemSetupInfo.spliceIn;
	attr.itemSetupInfo.spliceOut=pliAttr.itemSetupInfo.spliceOut;
	attr.itemSetupInfo.forceNormal=pliAttr.itemSetupInfo.forceNormal;
	attr.vStrmSessID=pliAttr.vStrmSessID;	
	attr.SessState=pliAttr.SessState;	

}


}
}