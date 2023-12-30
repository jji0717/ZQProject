// LAMGetPlayList2.h: interface for the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAMGETPLAYLIST3_H__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
#define AFX_LAMGETPLAYLIST3_H__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace ZQTianShan {
namespace Application{
namespace MOD{

#define LAM_PlayList3_Name		"GetPlayListFromLAM3"
class LAMPlayListQuery3:  public ::Ice::LocalObject, 
	public ZQTianShan::Application::MOD::IPlayListQuery    
{
public:
	LAMPlayListQuery3(::Ice::CommunicatorPtr& _ic);
	virtual ~LAMPlayListQuery3();
public:
	int getPlayList(PlayListInfo& plinfo, AEReturnData& aedata);
private:
	int GetPlayListFromILAMFacade(PlayListInfo& plinfo, AEReturnData& aedata);
//	int GetPlayListFromISurfForTianshan(PlayListInfo& plinfo, AEReturnData& aedata);
public:
	::Ice::CommunicatorPtr& _iceComm;

};
}}}//end namespace

#endif // !defined(AFX_LAMGETPLAYLIST3_H__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
