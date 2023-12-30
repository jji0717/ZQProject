// SopInfo.cpp: implementation of the SopInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SopInfo.h"
#include <Windows.h>
#include <XMLPreference.h>
#include <map>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SopInfo::SopInfo()
{
	_strRead = "";
	_strWrite = "";
	_strSopName = "SOP_Name";
}

SopInfo::~SopInfo()
{

}

void SopInfo::parse(int argc, char **argv)
{
	if(argc<3)
	{
		printf("please input command line!\n");
		printf("@parameter 1 input xml file path\n@parameter 2 output xml file path\n@parameter 3 SOP name,default is SOP_Name\n");
		return ;
	}
	else if(argc == 3)
	{
		_strRead = argv[1];
		_strWrite = argv[2];
	}
	else if(argc == 4)
	{
		_strRead = argv[1];
		_strWrite = argv[2];
		_strSopName = argv[3];
	}
	else
	{
		printf("please input right command line!\n");
		printf("@parameter 1 input xml file path\n@parameter 2 output xml file path\n@parameter 3 SOP name,default is SOP_Name\n");
		return ;
	}
	
	std::string xml = getSopInfo(_strRead.c_str(),_strSopName.c_str());
	if(xml.length() == 0)
	{
		printf("Not get SOP restriction infomation\n");
		return;
	}
	

	HANDLE hFile = CreateFileA(_strWrite.c_str(),GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwByte = 0;
		if(!WriteFile(hFile,xml.c_str(),xml.length(),&dwByte,NULL))
		{
			printf("save to %s failure!\n",_strWrite.c_str());
			DWORD err = GetLastError();
		}
		CloseHandle(hFile);
	}
	else
		printf("Open file %s failure!\n",_strWrite.c_str());
}

std::string SopInfo::getSopInfo(const char* pFile, const char* pSopName)
{
	std::map<::std::string,SOPSTRUCT>  sopMap;

	ZQ::common::ComInitializer in;
	ZQ::common::XMLPrefDoc xmlDoc(in);
	try{		
		if(!xmlDoc.open(pFile))
		{
			printf("Open xml file %s failed",pFile);
			return "";
		}
	}
	catch(...)
	{
		printf("Catch a exception when open %s file\n",pFile);
		return "";
	}
	
	ZQ::common::XMLPreference* preRoot = (ZQ::common::XMLPreference*)xmlDoc.root();
	if(!preRoot)
	{
		printf("Xml parse error,getRootPreference function failed");
		xmlDoc.close();
		return "";
	}
	//streamlink
	ZQ::common::XMLPreference* pStreamLinkPre = (ZQ::common::XMLPreference*)preRoot->firstChild("StreamLinks");
	if(!pStreamLinkPre)
	{
		printf("Parse storagelinks error,firstChild function failed");
		preRoot->free();
		xmlDoc.close();
		return ""; 
	}
	char name[20] = {0};
	char value[1024] = {0};
	for(ZQ::common::XMLPreference* pStreamLinkNode=(ZQ::common::XMLPreference*)pStreamLinkPre->firstChild(); pStreamLinkNode; pStreamLinkNode=(ZQ::common::XMLPreference*)pStreamLinkPre->nextChild())
	{
		int servgID = 0;
		std::string streamID = "";
		std::string sopValue = "";
		ZeroMemory(value,sizeof(value));
		streamID = pStreamLinkNode->get("streamId",value,"failure",sizeof(value));
		ZeroMemory(value,sizeof(value));
		servgID = atoi(pStreamLinkNode->get("servicegroupId",value,"failure",sizeof(value)));
		
		ZQ::common::XMLPreference* pPriv = (ZQ::common::XMLPreference*)pStreamLinkNode->firstChild();
		if(pPriv)
		{		
			for(ZQ::common::XMLPreference* pNode=(ZQ::common::XMLPreference*)pPriv->firstChild(); pNode; pNode=(ZQ::common::XMLPreference*)pPriv->nextChild())
			{
				ZeroMemory(value,sizeof(value));
				pNode->get("key",value,"failure",sizeof(value));
				if(stricmp(value,pSopName) == 0)
				{
					ZQ::common::XMLPreference* pvec=(ZQ::common::XMLPreference*)pNode->firstChild();
					if(pvec)
					{
						ZeroMemory(value,sizeof(value));
						sopValue = pvec->get("value",value,"failure",sizeof(value));
						pNode->free();
					}	break;
				}
				pNode->free();
			}
			pPriv->free();
		}
		
		if(sopValue.length() == 0)
		{
			pStreamLinkNode->free();
			continue;
		}
		bool bHas = false;
		std::map<std::string,SOPSTRUCT>::iterator sopIt = sopMap.find(sopValue);
		if(sopIt != sopMap.end())//the map has contained the SOP_Name
		{
			for(std::vector<int>::iterator sgIdIt=sopIt->second.servgID.begin(); sgIdIt<sopIt->second.servgID.end(); sgIdIt++)
			{
				if(servgID == *sgIdIt)
				{
					bHas = true;
					break;
				}
			}
			if(!bHas)
				sopIt->second.servgID.push_back(servgID);
			bHas = false;
			for(::std::vector<::std::string>::iterator streamIdIt=sopIt->second.streamID.begin(); streamIdIt<sopIt->second.streamID.end(); streamIdIt++)
			{
				if(streamID.compare(*streamIdIt) == 0)
				{
					bHas = true;
					break;
				}
			}
			if(!bHas)
				sopIt->second.streamID.push_back(streamID);
		}
		else
		{
			SOPSTRUCT sopStruct;
			sopStruct.servgID.push_back(servgID);
			sopStruct.streamID.push_back(streamID);
			::std::pair <::std::string,SOPSTRUCT> pair;
			pair = ::std::make_pair(sopValue,sopStruct);
			sopMap.insert(pair);
		}
		pStreamLinkNode->free();		
	}

	pStreamLinkPre->free();
	preRoot->free();
	xmlDoc.close();
	
	//parse to a xml string
	std::string sopxml = "<SOPRestriction>";
	int serID = 0;
	char chID[20] = {0}; 
	for(std::map<std::string,SOPSTRUCT>::iterator sopIt=sopMap.begin(); sopIt!=sopMap.end(); sopIt++ )
	{
		sopxml += "<sop name=\"" + sopIt->first + "\"";
		if(sopIt->second.servgID.size() > 1)
			sopxml += ">";
		else
		{
			ZeroMemory(chID,sizeof(chID));
			serID = sopIt->second.servgID.front();
			sprintf(chID,"%d",serID);
			sopxml += " serviceGroup=\"";
			sopxml += chID;
			sopxml += "\">";
		}
		for(std::vector<std::string>::iterator streamIdIt=sopIt->second.streamID.begin(); streamIdIt<sopIt->second.streamID.end(); streamIdIt++)
		{
			sopxml += "<streamer netId=\"" + *streamIdIt + "\" />";
		}
		sopxml += "</sop>";
	}

	sopxml += "</SOPRestriction>";


	return sopxml;
}