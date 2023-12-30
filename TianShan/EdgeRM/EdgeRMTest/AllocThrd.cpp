#include "AllocThrd.h"
#include "AllocationOwnerImpl.h"
#include <map>
#include "TsEdgeResource.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

AllocThrd::AllocThrd(ZQ::common::Log& _log, TianShanIce::EdgeResource::EdgeResouceManagerPrx& _erm, TianShanIce::EdgeResource::AllocationOwnerPrx& _allocOwnerPrx, int _allocCount, DWORD _sleepTime) : log(_log), erm(_erm), allocOwnerPrx(_allocOwnerPrx), allocCount(_allocCount), sleepTime(_sleepTime)
{
}

AllocThrd::~AllocThrd()
{

	for(std::vector<TianShanIce::EdgeResource::AllocationPrx>::iterator it = allocationPrxs.begin(); it != allocationPrxs.end(); it++)
	{
		try
		{
			(*it)->destroy();
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			continue;
		}
		catch(const Ice::Exception& e) 
		{
			return;
		}
		catch(...)
		{
			return;
		}
	}
}

int AllocThrd::run(void)
{
	if(allocCount < 1)
	{
		return 0;
	}
	srand( (unsigned)time( NULL ) );

	bool bCreate = true;

	try
	{
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Start Time, [%d] Allocation to create"), allocCount);
		for(int i = 1; i <= allocCount; i++)
		{
			::TianShanIce::SRM::ResourceMap resRequirement;
			TianShanIce::SRM::Resource resource;
			//edgeDeviceName
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtStrings;
			var.strs.push_back("QAM");
			var.strs.push_back("QAM1");
			var.strs.push_back("QAM2");
			var.strs.push_back("QAM3");
			var.strs.push_back("QAM4");
			var.strs.push_back("QAM5");
			var.strs.push_back("QAM6");
			var.strs.push_back("QAM7");
			var.strs.push_back("QAM8");
			var.strs.push_back("QAM9");
			var.strs.push_back("QAM10");
			var.strs.push_back("QAM11");
			var.strs.push_back("QAM12");
			var.strs.push_back("QAM13");
			var.strs.push_back("QAM14");
			var.strs.push_back("QAM15");
			resource.resourceData.insert(std::make_pair("edgeDeviceName",  var));
			resource.resourceData["edgeDeviceName"].bRange=false;

			resRequirement.insert(std::make_pair(TianShanIce::SRM::ResourceType::rtPhysicalChannel,resource));
			char ownerContextKey[20];
			snprintf(ownerContextKey, sizeof(ownerContextKey), "Owner%d", i);
			int TTL = 1000;
			TianShanIce::EdgeResource::AllocationPrx alloc = erm->createAllocation(resRequirement, 1000*60*10, allocOwnerPrx, ownerContextKey);

			TianShanIce::SRM::Resource res;

			//channelid
// 			::TianShanIce::Variant var_channel;
// 			var_channel.type = ::TianShanIce::vtStrings;
// 			var_channel.strs.push_back("1");
// 			res.resourceData.insert(std::make_pair("channelId",  var_channel));
// 			res.resourceData["channelId"].bRange=false;

			//edgeDeviceName
			::TianShanIce::Variant var_edgeDeviceName;
			var_edgeDeviceName.type = ::TianShanIce::vtStrings;
			var_edgeDeviceName.strs.push_back("QAM");
			var_edgeDeviceName.strs.push_back("QAM1");
			var_edgeDeviceName.strs.push_back("QAM2");
			var_edgeDeviceName.strs.push_back("QAM3");
			var_edgeDeviceName.strs.push_back("QAM4");
			var_edgeDeviceName.strs.push_back("QAM5");
			var_edgeDeviceName.strs.push_back("QAM6");
			var_edgeDeviceName.strs.push_back("QAM7");
			var_edgeDeviceName.strs.push_back("QAM8");
			var_edgeDeviceName.strs.push_back("QAM9");
			var_edgeDeviceName.strs.push_back("QAM10");
			var_edgeDeviceName.strs.push_back("QAM11");
			var_edgeDeviceName.strs.push_back("QAM12");
			var_edgeDeviceName.strs.push_back("QAM13");
			var_edgeDeviceName.strs.push_back("QAM14");
			var_edgeDeviceName.strs.push_back("QAM15");
			res.resourceData.insert(std::make_pair("edgeDeviceName",  var_edgeDeviceName));
			res.resourceData["edgeDeviceName"].bRange=false;

			//edgeDeviceGroup
// 			::TianShanIce::Variant var_edgeDeviceGroup;
// 			var_edgeDeviceGroup.type = ::TianShanIce::vtStrings;
// 			var_edgeDeviceGroup.strs.push_back("SEAC.BOS");
// 			res.resourceData.insert(std::make_pair("edgeDeviceGroup",  var_edgeDeviceGroup));
// 			res.resourceData["edgeDeviceGroup"].bRange=false;

			alloc->addResource(TianShanIce::SRM::ResourceType::rtPhysicalChannel, res);

			res.resourceData.clear();

			//transmissionSystem
// 			::TianShanIce::Variant var_transmissionSystem;
// 			var_transmissionSystem.type = ::TianShanIce::vtInts;
// 			var_transmissionSystem.ints.push_back(0x01);
// 			res.resourceData.insert(std::make_pair("transmissionSystem",  var_transmissionSystem));
// 			res.resourceData["transmissionSystem"].bRange=false;

			//modulationFormat
			::TianShanIce::Variant var_modulationFormat;
			var_modulationFormat.type = ::TianShanIce::vtInts;
			var_modulationFormat.ints.push_back(0x07);
			res.resourceData.insert(std::make_pair("modulationFormat",  var_modulationFormat));
			res.resourceData["modulationFormat"].bRange=false;

			//modulationMode
// 			::TianShanIce::Variant var_modulationMode;
// 			var_modulationMode.type = ::TianShanIce::vtInts;
// 			var_modulationMode.ints.push_back(0x09);
// 			res.resourceData.insert(std::make_pair("modulationMode",  var_modulationMode));
// 			res.resourceData["modulationMode"].bRange=false;

			//FEC
// 			::TianShanIce::Variant var_FEC;
// 			var_FEC.type = ::TianShanIce::vtInts;
// 			var_FEC.ints.push_back(0x01);
// 			res.resourceData.insert(std::make_pair("FEC",  var_FEC));
// 			res.resourceData["FEC"].bRange=false;

			alloc->addResource(TianShanIce::SRM::ResourceType::rtAtscModulationMode, res);

			res.resourceData.clear();

			//bandwidth
			::TianShanIce::Variant var_bandwidth;
			var_bandwidth.type = ::TianShanIce::vtLongs;
			var_bandwidth.lints.push_back(3750000);
			res.resourceData.insert(std::make_pair("bandwidth",  var_bandwidth));
			res.resourceData["bandwidth"].bRange=false;

			//tsid
// 			::TianShanIce::Variant var_tsid;
// 			var_tsid.type = ::TianShanIce::vtInts;
// 			var_tsid.ints.push_back(i);
// 			res.resourceData.insert(std::make_pair("tsid",  var_tsid));
// 			res.resourceData["tsid"].bRange=false;

			alloc->addResource(TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth, res);

			res.resourceData.clear();

			//Id
			::TianShanIce::Variant var_Id;
			var_Id.type = ::TianShanIce::vtInts;
			var_Id.ints.push_back(rand());
			res.resourceData.insert(std::make_pair("Id",  var_Id));
			res.resourceData["Id"].bRange=false;

			alloc->addResource(TianShanIce::SRM::ResourceType::rtMpegProgram, res);

			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "start provision");
			alloc->provision(0, true);
			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "after provision");

			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"), "start serve");
			alloc->serve();
			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "%s"),"after serve");

			allocationPrxs.push_back(alloc); // store when it succeed

			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Allocation[%s] stored"), (alloc->getId()).c_str());
		}

		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "Stop Time, [%d] Allocation succeed"), allocationPrxs.size());
	}
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "create allocation caught:%s"), e.ice_name().c_str());
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "[%d] Allocation succeed"), allocationPrxs.size());
// 		try
// 		{
// 			for(std::vector<TianShanIce::EdgeResource::AllocationPrx>::iterator it = allocationPrxs.begin(); it != allocationPrxs.end(); it++)
// 			{
//  				(*it)->destroy();
// 			}
// 		}
// 		catch(const Ice::Exception& e)
// 		{
// 			log(ZQ::common::Log::L_DEBUG, CLOGFMT(Alloc, "delete allocation caught:%s"), e.ice_name().c_str());
// 			return 0;
// 		}
		return 0;
	}

	return 1;
}
