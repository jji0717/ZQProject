#pragma warning(disable:4786)
#include <Exception.h>
#include "ResourceManager.h"
#include "Log.h"


using namespace ZQ::common;
namespace ZQ{
namespace StreamSmith{		
	
template<class classType>
class auto_free
{
public:
	auto_free(classType _p)
	{
		_ptr=_p;
	}
	~auto_free()
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
	}	
public:
	classType& operator->()
	{
		return _ptr;
	}
	classType& operator=(const classType t)
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
		_ptr=t;
		return _ptr;
	}
	operator classType()
	{
		return _ptr;
	}
	bool operator==(classType& t)
	{
		if(t==_ptr)
			return true;
		else
			return false;
	}
	bool operator!()
	{
		return !_ptr;
	}
public:
	classType	_ptr;
};

ResourceMan::ResourceMan()
{
	_ServiceGroupdID=0;
	_QamMode=0;
	_QamSymbolRate=0;
}
ResourceMan::~ResourceMan()
{
}
bool ResourceMan::ParseConfig(char *confPath)
{
	try
	{
		//ComInitializer init;	
		XMLPreferenceDocumentEx Doc;
		//Get Qam configuration root
		if(!Doc.open(confPath))
		{
			glog(Log::L_ERROR,"Can't open configuration file %s ",confPath);
			return false;
		}
		auto_free<XMLPreferenceEx*> DocRoot=(XMLPreferenceEx*)Doc.getRootPreference();
		auto_free<XMLPreferenceEx*> QamRoot=DocRoot->firstChild("DeliveryPath");
		if(!QamRoot)
		{
			glog(Log::L_ERROR,"Can't get node DeliveryPath in Config file %s",confPath);
			return false;
		}
		auto_free<XMLPreferenceEx*> pPref=NULL;
		for(pPref=QamRoot->firstChild("ServiceGroup"); pPref; pPref=QamRoot->nextChild() )
		{
			if( !ParseServiceGroup(pPref) )
			{
				glog(Log::L_ERROR,"bool ResourceMan::ParseConfig()##ParseServiceGroup fail");
				return false;
			}
		}
		return true;
	}
	catch (ZQ::common::Exception e) 
	{
		glog(Log::L_ERROR,"Error was throw out when parse QAM config and desc = %s",e.getString());
		return false;
	}
}
void aEatWhite(std::string& str)
{	
	int iPos=0;
	int iLastPos=0;
	int iSize=str.size();
	if(iSize<=0)
		return;
	for (;iPos<iSize;iPos++) 
	{
		if(!(str[iPos]==' ' || str[iPos]=='\t'))
			break;
	}
	iLastPos=iSize-1;
	for(;iLastPos>=0;iLastPos--)
	{
		if(!(str[iLastPos]==' ' || str[iLastPos]=='\t'))
			break;
	}

	str=str.substr(iPos,iLastPos-iPos+1);
	return;
}
void SplitStringIntoVector(std::string& str,std::vector<int>& vecSpigotsIDs)
{
	vecSpigotsIDs.clear();
	if(str.empty() || str.length()<=0)
	{
		glog(Log::L_DEBUG,"no string content when split string into vector");
		return ;
	}
	char* spigotsID=(char*)str.c_str();
		
	int iStrLen=strlen(spigotsID);
	int iLastPos=-1;
	int iPos=0;
	char szBuf[128];
	for(iPos=0;iPos<iStrLen;iPos++)
	{
		if(spigotsID[iPos]!=';')
			continue;
		if(iLastPos==iPos)
			continue;
		if(iPos-iLastPos>0)
		{
			ZeroMemory(szBuf,sizeof(szBuf));
			strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
			std::string strTemp=szBuf;
			aEatWhite(strTemp);
			if(!strTemp.empty())
			{
				int iTemp=atoi(strTemp.c_str());
				glog(Log::L_DEBUG,"ParseResourceManager()##get a new spigot board number %d with string=%s",iTemp,szBuf);
				vecSpigotsIDs.push_back(iTemp);
			}
		}
		iLastPos=iPos;
	}
	if((iPos)-iLastPos>0)
	{
		ZeroMemory(szBuf,sizeof(szBuf));
		strncpy(szBuf,&spigotsID[iLastPos+1],iPos-iLastPos-1);
		std::string strTemp=szBuf;
		aEatWhite(strTemp);
		if(!strTemp.empty())
		{
			int iTemp=atoi(strTemp.c_str());
			glog(Log::L_DEBUG,"ParseResourceManager()##get a new spigot board number %d with string=%s",iTemp,szBuf);
			vecSpigotsIDs.push_back(iTemp);
		}			
	}
}
bool ResourceMan::ParseServiceGroup(XMLPreferenceEx* pPref)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"ResourceMan::ParseServiceGroup()## NULL pPref Pass IN");
		return false;
	}
	//Get service group ID
	char	szBuf[128];
	ZeroMemory(szBuf,sizeof(szBuf));
	pPref->get("id",szBuf,"",sizeof(szBuf)-1);
	_ServiceGroupdID=atoi(szBuf);
	
//	ZeroMemory(szBuf,sizeof(szBuf));
//	pPref->get("linkedSpigots",szBuf,"",sizeof(szBuf)-1);


	
	//Parse Qam
	auto_free<XMLPreferenceEx*> deft=NULL;
	for(deft=pPref->firstChild("Qam");deft;deft=pPref->nextChild())
	{
		if(!ParseQam(deft))
		{
			glog(Log::L_ERROR,"bool ResourceMan::ParseServiceGroup()##ParseQam fail");
			return false;
		}
	}
	return true;
}
bool ResourceMan::ParseQam(XMLPreferenceEx* pPref)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool ResourceMan::ParseQam()## NULL pPref pass in");
		return false;
	}
	_strMacAddress="";
	char	szBuf[128];
	ZeroMemory(szBuf,sizeof(szBuf));
	//Get qam IP
	pPref->get("IP",szBuf,"",sizeof(szBuf)-1);
	_QamIP=szBuf;
	//Get Qam mode
	pPref->get("mode",szBuf,"",sizeof(szBuf)-1);
	_QamMode=atoi(szBuf);
	//Get qam symbol rate
	pPref->get("symbolRate",szBuf,"",sizeof(szBuf)-1);
	_QamSymbolRate=atoi(szBuf);	
	//get qam mac address
	pPref->get("mac",szBuf,"",sizeof(szBuf)-1);
	_strMacAddress=szBuf;


	//Parse Channel
	auto_free<XMLPreferenceEx*> deft=NULL;
	int ChennelID=0;
	for(deft=pPref->firstChild("Channel");deft;deft=pPref->nextChild())
	{
		if(!ParseChannel(deft,ChennelID++))
		{
			glog(Log::L_ERROR,"bool ResourceMan::ParseQam()##Parse Channel fail");
			return false;
		}
	}
	return true;
}
bool ResourceMan::ParseChannel(XMLPreferenceEx* pPref,int id)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool ResourceMan::ParseChannel()##NULL pPref pass in");
		return false;
	}
	char	szBuf[128];
	ZeroMemory(szBuf,sizeof(szBuf));

	ServiceQamResource	sqr;
	//ZeroMemory(&sqr,sizeof(sqr));
	sqr._QamIP=_QamIP.c_str();
	sqr._QamMode=_QamMode;
	sqr._QamSymbolRate=_QamSymbolRate;
	sqr._GroupID=_ServiceGroupdID;
	sqr._MacAddress=_strMacAddress;
	

	//clear the variation
	_LinkedSpigotIDs.clear();
	//_strMacAddress="";

	pPref->get("frequency",szBuf,"",sizeof(szBuf)-1);
	sqr._Frequency=atoi(szBuf);

	pPref->get("baseport",szBuf,"",sizeof(szBuf)-1);
	sqr._BasePort=atoi(szBuf);

	pPref->get("basePN",szBuf,"",sizeof(szBuf)-1);
	sqr._BasePN=atoi(szBuf);

	pPref->get("PNCount",szBuf,"",sizeof(szBuf)-1);
	sqr._PNCount=atoi(szBuf);

	pPref->get("bandwidth",szBuf,"",sizeof(szBuf)-1);
	sqr._GroupBandWidth=atoi(szBuf);

	pPref->get("spigot",szBuf,"",sizeof(szBuf)-1);
	std::string strSpigots=szBuf;
	SplitStringIntoVector(strSpigots,_LinkedSpigotIDs);
	sqr._LinkSpigotsIDs=_LinkedSpigotIDs;
	

	pPref->get("portStep",szBuf,"",sizeof(szBuf)-1);
	sqr._PortStep=atoi(szBuf);
	if(sqr._PortStep<1)
		sqr._PortStep=1;


	pPref->get("totalBandWidth",szBuf,"",sizeof(szBuf)-1);
	
	bool	bFound=false;
	VecPhysicalQamResource::iterator	it;
	for(it=_physicalQamStack.begin();it!=_physicalQamStack.end();it++)
	{
		if(it->_QamIP==sqr._QamIP && it->_Frequency==sqr._Frequency)
		{
			bFound=true;
			break;
		}
	}
	if(!bFound)
	{
		PyhisicalQamResource pQr; 
		pQr._Frequency=sqr._Frequency;
		pQr._QamIP=sqr._QamIP;
		pQr._TotalAvailableBW=atoi(szBuf);
		_physicalQamStack.push_back(pQr);
	}
	sqr._ChannelID=id;
	_serviceQamStack.push_back(sqr);
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool ResourceMan::GetSpigotIDsFromResource(int serviceGroupID,int MaxBitRate,std::vector<int>& SpigotIDs)
{
	for(int i=0;i<(int)_serviceQamStack.size();i++)
	{
		if(serviceGroupID==(int)_serviceQamStack[i]._GroupID)
		{
			SpigotIDs=_serviceQamStack[i]._LinkSpigotsIDs;
			return true;
		}
	}
	return false;
}

ResourceMan::VecPhysicalQamResource::iterator ResourceMan::FindPhysicalQam(std::string& strQamIP,ULONG frequency)
{
	VecPhysicalQamResource::iterator it;
	for(it=_physicalQamStack.begin();it!=_physicalQamStack.end();it++)
	{
		if(strQamIP==it->_QamIP&&frequency==it->_Frequency)
		{			
			return it;			
		}
	}
	return _physicalQamStack.end();
}
bool ResourceMan::PNIsOK(const std::string& strQamIP,ULONG uPN)
{
	MapResourceAlloc::iterator it;
	for(it=_AllocResourceMap.begin();it!=_AllocResourceMap.end();it++)
	{
		if(it->second._ProgramNumber==uPN && it->second._QamIP==strQamIP)
			return false;
	}
	return true;
}
bool ResourceMan::GetQamResource(ULONG groupID,ULONG needBW,ResourceAlloc& rOut,ZQ::common::Guid& uidOut)
{
	VecServiceQamResource::iterator it;
	VecPhysicalQamResource::iterator itPhysical;
	MutexGuard gd(_Mutex);
	for(it=_serviceQamStack.begin();it!=_serviceQamStack.end();it++)
	{
		if(it->_GroupID==groupID)
		{
			if(it->_GroupBandWidth>=needBW)
			{
				itPhysical=FindPhysicalQam(it->_QamIP,it->_Frequency);
				if(itPhysical==_physicalQamStack.end())
					continue;

				//if(itPhysical->_TotalAvailableBW>=needBW)
				{//alloc this resource
					rOut._Frequency=it->_Frequency;
					for(unsigned int i=it->_BasePN;i<=it->_BasePN+it->_PNCount;i+=1)
					{
						//check the program number is ok or not!
						if(PNIsOK(it->_QamIP,i))
						{							
							rOut._ProgramNumber=i;
							//caculate the udp port
							rOut._Port=it->_BasePort+(i-it->_BasePN)*it->_PortStep;
							rOut._QamIP=it->_QamIP;
							rOut._QamMode=it->_QamMode;
							rOut._GroupID=groupID;
							rOut._BandWidth=needBW;
							rOut._QamMac=it->_MacAddress;
							rOut._ChannelID=it->_ChannelID;
							(it->_GroupBandWidth)-=needBW;
							//(itPhysical->_TotalAvailableBW)-=needBW;
							uidOut.create();
							_AllocResourceMap.insert(std::make_pair<ZQ::common::Guid,ResourceAlloc>(uidOut,rOut));
							return true;
						}
					}
				}
			}
		}
	}
	//if no available resource
	glog(Log::L_DEBUG,"No available resource for NodeGroup %u and needBW=%u(kbps)",groupID,needBW);
	glog(Log::L_DEBUG,"There are %d allocated resource",_AllocResourceMap.size());
	char szBuf[128];
	
	MapResourceAlloc::const_iterator itAlloc=_AllocResourceMap.begin();
	for(;itAlloc!=_AllocResourceMap.end();itAlloc++)
	{
		itAlloc->first.toString(szBuf,sizeof(szBuf));
		glog(Log::L_DEBUG,"%s with NodeGroup(%d) ip(%s) port(%u) frequency(%u) programNumber(%d) bandwidth(%u(kbps))",
							szBuf,itAlloc->second._GroupID,itAlloc->second._QamIP.c_str(),itAlloc->second._Port,
							itAlloc->second._Frequency,itAlloc->second._ProgramNumber,itAlloc->second._BandWidth);
	}
	return false;
}
bool ResourceMan::FreeQamResource(ZQ::common::Guid& guid)
{
	MutexGuard gd(_Mutex);
	MapResourceAlloc::iterator itAllocMap;
	itAllocMap=_AllocResourceMap.find(guid);
	if(itAllocMap==_AllocResourceMap.end())
		return true;
	//free bandwidth
	VecServiceQamResource::iterator		itServiceQam;
	VecPhysicalQamResource::iterator	itPhysicalQam;
	for(itServiceQam=_serviceQamStack.begin();itServiceQam!=_serviceQamStack.end();itServiceQam++)
	{
		if(itServiceQam->_GroupID==itAllocMap->second._GroupID
			&&itAllocMap->second._QamIP==itServiceQam->_QamIP
			&&itAllocMap->second._Frequency==itServiceQam->_Frequency)
		{
			itServiceQam->_GroupBandWidth+=itAllocMap->second._BandWidth;
			break;
		}
	}
	for(itPhysicalQam=_physicalQamStack.begin();itPhysicalQam!=_physicalQamStack.end();itPhysicalQam++)
	{
		if(itAllocMap->second._QamIP==itPhysicalQam->_QamIP
			&&itAllocMap->second._Frequency==itPhysicalQam->_Frequency)
		{
			itPhysicalQam->_TotalAvailableBW+=itAllocMap->second._BandWidth;
			break;
		}
	}
	
	_AllocResourceMap.erase(itAllocMap);

	char szBuf[128];
	ZeroMemory(szBuf,sizeof(szBuf));
	guid.toString(szBuf,sizeof(szBuf)-1);
	glog(Log::L_DEBUG,"free resource with guid=%s ",szBuf);
	return true;
}

};
};