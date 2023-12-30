#include "AllocationRequest.h"
#include "AllocationOwnerImpl.h"
#include <map>
#include "TsEdgeResource.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

std::vector<TianShanIce::EdgeResource::AllocationPrx> AllAllocation;
ZQ::common::Mutex GlobleMutex;

#define READ_RES_FIELD(_VAR, _RESMAP, _RESTYPE, _RESFILED, _RESFIELDDATA) \
{ ::TianShanIce::SRM::Resource& res = _RESMAP[::TianShanIce::SRM::_RESTYPE]; \
	if (res.resourceData.end() != res.resourceData.find(#_RESFILED) && !res.resourceData[#_RESFILED].bRange && res.resourceData[#_RESFILED]._RESFIELDDATA.size() >0) \
	_VAR = res.resourceData[#_RESFILED]._RESFIELDDATA[0]; \
 else log(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMPHO, "unacceptable " #_RESTYPE " in session: " #_RESFILED "(range=%d, size=%d)"), res.resourceData[#_RESFILED].bRange, res.resourceData[#_RESFILED]._RESFIELDDATA.size()); \
}

int AllocationRequest::sgCount = 0;

AllocationRequest::AllocationRequest(ZQ::common::NativeThreadPool& _thrdPool,ZQ::common::Log& _log, TianShanIce::EdgeResource::EdgeResouceManagerPrx& _erm, TianShanIce::EdgeResource::AllocationOwnerPrx& _allocOwnerPrx)
: ThreadRequest(_thrdPool), log(_log), erm(_erm), allocOwnerPrx(_allocOwnerPrx)
{
}

AllocationRequest::~AllocationRequest()
{
}

int AllocationRequest::run(void)
{

	srand( (unsigned)time( NULL ) );	

	try
	{
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Start Time"));

		static const char* DeviceCandidate[] = { "QAM1", "QAM2", "QAM3", "QAM4", "QAM5",
			"QAM6", "QAM7", "QAM8", "QAM9", "QAM10", "QAM11",
			"QAM12", "QAM13", "QAM14", "QAM15" };

		static const int ServiceGroupCandidate[] = { 1000, 2000, 3000, 4000, 5000,
			6000, 7000, 8000, 9000, 10000, 11000,
			12000, 13000, 14000, 15000 };

		DWORD begintime = GetTickCount();

		::TianShanIce::SRM::ResourceMap resRequirement;
		TianShanIce::SRM::Resource resource;
		//edgeDeviceName
		::TianShanIce::Variant var;
		var.type = ::TianShanIce::vtStrings;
// 		int DeviceCount = rand()%6;
// 		if(DeviceCount < 3)
// 			DeviceCount = 3;
// 		int j = rand()%10;
// 		for(int i = 0; i < DeviceCount; i++)
// 		{
// 			var.strs.push_back(DeviceCandidate[j++]);
// 		}
		for(int i = 0; i < 15; i++)
		{
			var.strs.push_back(DeviceCandidate[i]);
		}
		resource.resourceData.insert(std::make_pair("edgeDeviceName",  var));
		resource.resourceData["edgeDeviceName"].bRange=false;

		resRequirement.insert(std::make_pair(TianShanIce::SRM::ResourceType::rtPhysicalChannel,resource));
		char ownerContextKey[20];
		snprintf(ownerContextKey, sizeof(ownerContextKey), "Owner");
		int TTL = 1000;
		TianShanIce::EdgeResource::AllocationPrx alloc = erm->createAllocation(resRequirement, 1000*60*10, allocOwnerPrx, ownerContextKey);

		ident = alloc->getId();
		TianShanIce::SRM::Resource res;

		//edgeDeviceName
// 		::TianShanIce::Variant var_edgeDeviceName;
// 		var_edgeDeviceName.type = ::TianShanIce::vtStrings;
// 		var_edgeDeviceName.strs.push_back("QAM");
// 		var_edgeDeviceName.strs.push_back("QAM1");
// 		var_edgeDeviceName.strs.push_back("QAM2");
// 		var_edgeDeviceName.strs.push_back("QAM3");
// 		var_edgeDeviceName.strs.push_back("QAM4");
// 		var_edgeDeviceName.strs.push_back("QAM5");
// 		var_edgeDeviceName.strs.push_back("QAM6");
// 		var_edgeDeviceName.strs.push_back("QAM7");
// 		var_edgeDeviceName.strs.push_back("QAM8");
// 		var_edgeDeviceName.strs.push_back("QAM9");
// 		var_edgeDeviceName.strs.push_back("QAM10");
// 		var_edgeDeviceName.strs.push_back("QAM11");
// 		var_edgeDeviceName.strs.push_back("QAM12");
// 		var_edgeDeviceName.strs.push_back("QAM13");
// 		var_edgeDeviceName.strs.push_back("QAM14");
// 		var_edgeDeviceName.strs.push_back("QAM15");
		res.resourceData.insert(std::make_pair("edgeDeviceName",  var));
		res.resourceData["edgeDeviceName"].bRange=false;

		alloc->addResource(TianShanIce::SRM::ResourceType::rtPhysicalChannel, res);

		res.resourceData.clear();


		///add serviceGroup:id
		::TianShanIce::Variant var_servicegroup;
		var_servicegroup.type = ::TianShanIce::vtInts;
		var_servicegroup.bRange = false;
		var_servicegroup.ints.push_back(ServiceGroupCandidate[sgCount%15]);
		sgCount++;
		res.resourceData.insert(std::make_pair("id",  var_servicegroup));
		alloc->addResource(TianShanIce::SRM::rtServiceGroup , res);
		res.resourceData.clear();


		//modulationFormat
		::TianShanIce::Variant var_modulationFormat;
		var_modulationFormat.type = ::TianShanIce::vtBin;
		var_modulationFormat.bin.push_back(0x08);
		res.resourceData.insert(std::make_pair("modulationFormat",  var_modulationFormat));
		res.resourceData["modulationFormat"].bRange=false;

		alloc->addResource(TianShanIce::SRM::ResourceType::rtAtscModulationMode, res);

		res.resourceData.clear();

		//bandwidth
		::TianShanIce::Variant var_bandwidth;
		var_bandwidth.type = ::TianShanIce::vtLongs;
		var_bandwidth.lints.push_back(3750000);
		res.resourceData.insert(std::make_pair("bandwidth",  var_bandwidth));
		res.resourceData["bandwidth"].bRange=false;

		alloc->addResource(TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth, res);

		res.resourceData.clear();

		//Id
// 		::TianShanIce::Variant var_Id;
// 		var_Id.type = ::TianShanIce::vtInts;
// 		var_Id.ints.push_back(rand());
// 		res.resourceData.insert(std::make_pair("Id",  var_Id));
// 		res.resourceData["Id"].bRange=false;
// 
// 		alloc->addResource(TianShanIce::SRM::ResourceType::rtMpegProgram, res);

		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "start provision");
		alloc->provision(0, true);
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "after provision");

		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "start serve");
		alloc->serve();
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"),"after serve");

		DWORD past = GetTickCount() - begintime;
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Allocation[%s] created cost %d milliseconds"), (alloc->getId()).c_str(), past);

		{
			//// add by huangli

			::Ice::Long channelId =-1;
			::Ice::Byte	FEC = -1;
			::Ice::Int symbolRate = -1;
			::Ice::Byte modulationFormat = -1;
			::Ice::Long PnId = -1;
			::std::string edgeDeviceIP = "";
			::std::string edgeDeviceMac = "";
			::std::string edgeDeviceGroup ="";
			::std::string edgeDeviceName ="";
			::Ice::Int destPort = -1;
			Ice::Long	bw2Alloc = 0;

			::TianShanIce::SRM::ResourceMap allocResourceMap = alloc->getResources();

			::TianShanIce::Variant var;

			READ_RES_FIELD(edgeDeviceIP, allocResourceMap,ResourceType::rtPhysicalChannel, edgeDeviceIP, strs);
			READ_RES_FIELD(edgeDeviceMac, allocResourceMap, ResourceType::rtPhysicalChannel, edgeDeviceMac, strs);
			READ_RES_FIELD(edgeDeviceGroup, allocResourceMap, ResourceType::rtPhysicalChannel, edgeDeviceGroup, strs);
			READ_RES_FIELD(edgeDeviceName, allocResourceMap,ResourceType::rtPhysicalChannel, edgeDeviceName, strs);
			READ_RES_FIELD(destPort, allocResourceMap, ResourceType::rtPhysicalChannel, destPort, ints);
			READ_RES_FIELD(FEC, allocResourceMap,ResourceType::rtAtscModulationMode, FEC, bin);
			READ_RES_FIELD(symbolRate, allocResourceMap, ResourceType::rtAtscModulationMode, symbolRate, ints);
			READ_RES_FIELD(channelId, allocResourceMap,ResourceType::rtPhysicalChannel, channelId, lints);
			READ_RES_FIELD(modulationFormat, allocResourceMap, ResourceType::rtAtscModulationMode, modulationFormat, bin);
			READ_RES_FIELD(PnId, allocResourceMap, ResourceType::rtMpegProgram, Id, lints);

			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "DeviceName[%s], ChannelId[%lld],Pn[%lld], DestPort[%d]"),
				edgeDeviceName.c_str(), channelId, PnId, destPort);

		}

		ZQ::common::MutexGuard mg(GlobleMutex);
		// store when it succeed
		AllAllocation.push_back(alloc);

		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Allocation[%s] stored, total %d Allocations"), (alloc->getId()).c_str(), AllAllocation.size());
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "ThreadPool [%d] active, [%d] total with [%d] pending request"), _pool.activeCount(), _pool.size(), _pool.pendingRequestSize());
	}
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
		log(ZQ::common::Log::L_ERROR, CLOGFMT(Alloc, "create Allocation[%s] caught:%s"), ident.c_str(), e.ice_name().c_str());
		return false;
	}
	return true;
}