#ifndef __ZQTianShan_Application_MOD_ModSvcIce__
#define __ZQTianShan_Application_MOD_ModSvcIce__


////////////////////////////////////////////////////////////////////////////////
// How to compile this Ice definition file.
////////////////////////////////////////////////////////////////////////////////
/*
Commands
$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir . .\$(InputName).ice
$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir . --index "ZQTianShan::Application::MOD::Stream2Purchase,ZQTianShan::Application::MOD::ModPurchase,streamId" Stream2Purchase .\$(InputName).ice

Outputs
./$(InputName).h
./$(InputName).cpp
*/


#include <Ice/Identity.ice>

#include <TsApplication.ICE>
#include <TsStreamer.ICE>
#include <LAMFacade.ice>
#include <TianShanIce.ICE>
#include <Surf_Tianshan.ice>


module ZQTianShan
{

module Application
{

module MOD
{

///////////////////////////////////////////////////////////////////////////////
// representating an purchase in MOD application
// it's a relevant object just like a playlist in StreamSmith application.
///////////////////////////////////////////////////////////////////////////////
["freeze:write"]
class ModPurchase extends TianShanIce::Application::Purchase
{
	// ident, the unique-id for this object
	Ice::Identity							ident;

	// weiwoo, stores the server session's proxy
	TianShanIce::SRM::Session*				weiwoo;

	// stream, stores the stream's proxy
	TianShanIce::Streamer::Stream*			stream;
	string									streamId;

	// enableAuthorize, a flag indicates whether or not to be authorized on other component
	bool									enableAuthorize;

	string									clientSessionId;
	string									serverSessionId;

	// apppath, path of rtsp request
	string									appPath;
	
	//Authorize endpoint
	string									authEndpoint;
	
	//assetProps store PID and PAID
	//::TianShanIce::Properties               assetProps;
	
		// store privatdata
	::TianShanIce::ValueMap                  purPrivData;

	// bInService, indicates whether the purchase has been provisioned successfully
	// this data member will effect what actions will we take when the purchase's timeout
	// arrived, if not in service status we will detach the purchase.
	bool									bInService;

	// assetElements, stores the asset elements returned from lam. used when purchase render
	com::izq::am::facade::servicesForIce::AEInfo3Collection assetElements;
	
	::TianShanIce::StrValues                     volumesLists;
    
	// return the id of this object
	["cpp:const", "freeze:read"] string getId();

	["cpp:const", "freeze:read"] Ice::Identity getIdent();

	["cpp:const", "freeze:read"] TianShanIce::Streamer::Stream* getStream()
		throws TianShanIce::ServerError;

	["cpp:const", "freeze:read"] string getClientSessionId();
	["cpp:const", "freeze:read"] string getServerSessionId();

	["cpp:const", "freeze:read"] bool isInService();
	
                                 void sendStatusNotice();
    ["cpp:const", "freeze:read"] ::TianShanIce::ValueMap getPrivataData();
								 void setPrivateData(string key, ::TianShanIce::Variant variant);
								 void setPrivateData2(::TianShanIce::ValueMap privateData);


}; // class ModPurchase

}; // module MOD

}; // module Application

}; // module ZQTianShan

#endif // #define __ZQTianShan_Application_MOD_ModSvcIce__

