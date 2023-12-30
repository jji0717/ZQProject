#include "Log.h"
#include "streamsession.h"
#include "ScLog.h"
#include "issapi.h"
#include <string>

using namespace ZQ::common;

int main()
{
	pGlog = new ScLog(L"E:\\bernieworkpath\\ZQProjs\\PlaylistAS\\pas.log", Log::L_DEBUG, 8*1024*1024);

	TYPEINST typeInst;
	typeInst.s.dwType = ITV_TYPE_PRIMARY_ZQ_MUSICPLAYLIST;
	typeInst.s.dwInst = 0x1;
//	CStreamSession TheSSMan = CStreamSession::Instance(typeInst, 0x80003);
	
	PlaylistSoapProxy TheProxy;
	try {
		TheProxy.Initialize(
			L"E:\\bernieworkpath\\ZQProjs\\PlaylistAS\\PlaylistSoapInterface.wsdl", 
			L"E:\\bernieworkpath\\ZQProjs\\PlaylistAS\\PlaylistSoapInterface.wsml", 
			L"PlaylistSoapInterfaceService", 
			L"PlaylistSoapInterface", 
			L"http://192.168.80.92:8080/telewest/services/PlaylistSoapInterface");
		
		glog(Log::L_DEBUG, L"Soap Proxy initialized");

		AELIST newAELIST= TheProxy.OnSetupSoapCall("07268499533","54:0B:D6:44:09:FA","88","00070001");
		glog(Log::L_DEBUG, L"Setup Call done, with AE count %d", newAELIST.AECount);
		PAELEMENT pAElist = (PAELEMENT)newAELIST.AELlist;
		for(int i=0; i<newAELIST.AECount; i++) {
			glog(Log::L_DEBUG, L"\tAEUid = %x",pAElist[i]);
		}
		TheProxy.releaseAEList(newAELIST);
		
		long lret = TheProxy.OnPlaySoapCall("07268499533","54:0B:D6:44:09:FA","88","00070001");
		glog(Log::L_DEBUG, L"Play Call done, with return value %d", lret);

		lret = TheProxy.OnTeardownSoapCall("07268499533","54:0B:D6:44:09:FA","88","00070001","Oh, my god!");
		glog(Log::L_DEBUG, L"Teardown Call done, with return value %d", lret);

	}
	catch( _com_error Error )
	{
		std::string Msg;
		Msg += "";
		Msg += Error.Description();
		glog(Log::L_ERROR, Msg.c_str());
		return 1;
	}

	return 0;
}
