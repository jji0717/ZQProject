// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --output-dir . ./$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::EdgeResource::PhoAllocationOfChannel,TianShanIce::EdgeResource::PhoAllocation, sessKey" PhoAllocationOfChannel --output-dir . ./$(InputName).ice

#ifndef __ZQ_TianShanIce_EdgeRM_PHO_ICE__
#define __ZQ_TianShanIce_EdgeRM_PHO_ICE__

#include <TsEdgeResource.ICE>

module TianShanIce
{

module EdgeResource
{

// PHO evictor class
["freeze:write"] 
class PhoAllocation
{
	Ice::Identity ident;/// <class identity

	string sessKey;/// <SharedLinkStr#sessId
	
	string sessionGroup;/// <session group id
	
	Allocation* alloc;/// <Allocation identity id

	///AllocationOwner* allocOwner;
	
	["cpp:const", "freeze:read"] Ice::Identity getIdent();
	
	["cpp:const", "freeze:read"] Allocation* getAllocation();
	
	["cpp:const", "freeze:read"] string getSessionGroup();
	
	["cpp:const", "freeze:read"] string getSessKey();

};

};
};

#endif // __ZQ_TianShanIce_EdgeRM_PHO_ICE__
