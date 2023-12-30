#ifndef _RESOURCE_MANAGER_H__
#define _RESOURCE_MANAGER_H__
#include <string>
#include <vector>
#include <map>
#include "XMLPreferenceEx.h"
#include <Locks.h>
#include <Guid.h>



using namespace ZQ::common;
namespace ZQ
{
namespace StreamSmith	
{
class ResourceMan
{
public:
	ResourceMan();
	~ResourceMan();
public:
	typedef struct _tagResourceAlloc 
	{		
		std::string			_QamIP;
		std::string			_QamMac;
		ULONG				_Frequency;
		ULONG				_Port;
		UINT				_ProgramNumber;
		ULONG				_QamMode;
		ULONG				_GroupID;
		ULONG				_BandWidth;
		ULONG				_ChannelID;
		_tagResourceAlloc(){_Frequency=_Port=_ProgramNumber=_QamMode=_GroupID=_BandWidth=_ChannelID=0;}
	}ResourceAlloc;

	/// Request a new QAM resource
	bool		ParseConfig(char *confPath);
	
	/// request a resource with frequency and program number
	bool		GetQamResource(ULONG groupID,ULONG needBW,ResourceAlloc& rOut,ZQ::common::Guid& uidOut);
	
	bool		GetSpigotIDsFromResource(int serviceGroupID,int MaxBitRate,std::vector<int>& SpigotIDs);
	///
	bool		FreeQamResource(ZQ::common::Guid& guid);
protected:
	bool		ParseServiceGroup(XMLPreferenceEx* pPref);
	bool		ParseQam(XMLPreferenceEx* pPref);
	bool		ParseChannel(XMLPreferenceEx* pPref,int id);
	
private:
	typedef struct _tagServiceQamResource 
	{
		ULONG				_GroupID;
		std::string			_QamIP;		
		ULONG				_Frequency;
		ULONG				_BasePort;
		ULONG				_PortStep;
		ULONG				_BasePN;
		ULONG				_PNCount;
		ULONG				_QamMode;
		ULONG				_GroupBandWidth;	//bandwidth of a qam assign to a service group
		ULONG				_QamSymbolRate;
		ULONG				_ChannelID;
		std::string			_MacAddress;		//qam mac address
		std::vector<int>	_LinkSpigotsIDs;	//link with spigots id
	}ServiceQamResource;

	typedef	struct _tagPhysicalQamResource 
	{		
		std::string			_QamIP;
		ULONG				_Frequency;
		ULONG				_TotalAvailableBW;
	}PyhisicalQamResource;
	typedef	std::vector<ServiceQamResource>			VecServiceQamResource;
	typedef	std::vector<PyhisicalQamResource>		VecPhysicalQamResource;
	
	typedef std::map<ZQ::common::Guid,ResourceAlloc>  MapResourceAlloc;
protected:
	ResourceMan::VecPhysicalQamResource::iterator				FindPhysicalQam(std::string& strQamIP,ULONG frequency);
	bool				PNIsOK(const std::string& strQamIP,ULONG uPN);
private:
	VecServiceQamResource	_serviceQamStack;
	VecPhysicalQamResource	_physicalQamStack;
	MapResourceAlloc		_AllocResourceMap;
	
	ULONG				_ServiceGroupdID;
	ULONG				_QamMode;
	ULONG				_QamSymbolRate;
	std::string			_QamIP;
	std::vector<int>	_LinkedSpigotIDs;
	std::string			_strMacAddress;
	Mutex				_Mutex;
};
};
};

#endif//_RESOURCE_MANAGER_H__