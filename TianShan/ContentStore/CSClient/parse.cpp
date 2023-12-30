#include "Parse.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <iostream>

using namespace std;


#define RSDFLAG(_FLAG) (1 << ::TianShanIce::Storage::_FLAG)


Parse::Parse(void)
{
	_contentStorePrx = NULL;
	_contentPrx = NULL;
	_volumePrx = NULL;
	_commun = NULL;
	int i = 0;
	_commun = Ice::initialize(i, NULL);
}

Parse::~Parse(void)
{
	if(_commun)
	{
		_commun->destroy();
		_commun = NULL;
	}
}

void Parse::usage()
{
	cout<<"help            get help information"<<endl
		<<"connect<endpoint>                               connect ContentStore server with endpoint,the endpoint like:ContentStore:tcp -h 192.168.81.114 -p 10400"<<endl
		<<"quit            quit the CSClient"<<endl
		<<endl


		<<"type            get content store type"<<endl
		<<"netid           get content store netid"<<endl
		<<"valid           check if the content store is functional"<<endl
		<<"openvolume<volume name>                         open volume with voluem name"<<endl
		<<"listvolume<from><be Include virtual volume>     list volumes,parameter from is list from this volume,if it is \"*\",\"/\" or \"NULL\",is list from root,\"1\" is include virtual volume"<<endl
		<<"opencontentbyname<full name>                    open content by full name"<<endl
		<<"cachelevel       get content store cach level"<<endl
		<<"streamservice   get content store stream services infomation"<<endl
		<<"event<event type><file name>[<params key=value>]                         sent event to contentstore,type is 'create','delete','modify','rename','security',parmas format is 'key=value'"<<endl
		<<"provisionevent<event type><storeNetId><volume name><content name><properties>                                  a provision event,event type is one of [start, stop, progress, streamable],property form:key=value"<<endl
		<<endl		


		<<"populate        update the content attribute"<<endl
		<<"dirty           the content is dirty or not"<<endl
		<<"restore<stampLastFileWrite>                     restore the record during service restart"<<endl
		<<"filepath        get content main file path name"<<endl
		<<"filemodify      content file have modified"<<endl
		<<"filerename<new file name>                       file change to new file name"<<endl
//		<<"inuse           content in use or not"<<endl
		<<"enterstate<target state>                        enter state the target state is,target state can one of [ notprovision, provision, streamable, inservice, outservice, clean]"<<endl
		<<"checkresident<residentialstatus to test>       test the content residentialtatues,residentialstatus can one or some of [residential, read, write, absence, corrupt, directory ]"<<endl

		<<"store           get contnetstore of this content"<<endl
		<<"volume          get volume of this content in"<<endl
		<<"name            get content name"<<endl
		<<"metadata        get additional data"<<endl
		<<"setmetadata<key><value>                         set additional data it is key value pairs"<<endl
		<<"state           get content state"<<endl
		<<"provisioned     get content is provisioned or not"<<endl
		<<"provisiontime   get provision time stamp"<<endl
		<<"destroy[bool mandatory]                         destroy the content,if parameter is \"1\"will destroy mandatory or not"<<endl
		<<"localtype       get content local type"<<endl
		<<"subtype         get content sub type"<<endl
		<<"framerate       get content framerate"<<endl
		<<"resolution      get content resolutin,return the pixel of the horizontal and vertical"<<endl
		<<"filesize        get content mainfile size in bytes"<<endl
		<<"supportsize     get the support file size in bytes"<<endl
		<<"playtime        get content play time"<<endl
		<<"bitrate         get content bitrate"<<endl
		<<"md5checksum     get content MD5 checksum"<<endl
		<<"trickspeed      get supported trick speeds of the content"<<endl
		<<"exporturl<transferProtocol><transferBitrate>                             get content URL with transferProtocol and transferBitrate"<<endl
		<<"sourceurl       get content source URL"<<endl
		<<"provision<sourceUrl><sourceContentType><bOverwrite><startTimeUTC><stopTimeUTC><maxTransferBitrate>             provision the content from sourceurl with other parameter"<<endl
		<<"provisionpassive<sourceContentType><bOverwrite><startTimeUTC><stopTimeUTC><maxTransferBitrate>                 passive provision the content"<<endl
		<<"cancelprovision     cancel the content provision"<<endl
		<<endl

		
		<<"volumename      get volume name"<<endl
		<<"mountpath       get this volume mount path"<<endl
		<<"volumeinfo      get volume infomation"<<endl
		<<"syncfilesystem  sync file system"<<endl
		<<"listall<condition>                              get all content with condition"<<endl
		<<"list<metadata names><startName><maxCount>       get content list with condition metadata names ,startName and maxCount,metadate names -- the name of returning meta data of each content,end the metadata name input NULL; "
					                                    <<"startName -- the content from this start Name,if NULL is start the beging; maxCount -- the maximu count content the list return,0 not the limitation"<<endl
		<<"opencontent<name><type><notExistCreate>         open the content with the content name and type,if notExistCreate is \"1\" then create if the content is not exist"<<endl
		<<"capacity        get free capacity and toatal capacity"<<endl
		<<"subvolume<subvolume name><be create if not exist><quote space MB>        get subvolume of this volume"<<endl
		<<"parent          get parent volume"<<endl
		<<"destroyvolume   destroy this volume"<<endl
		<<endl;
}

bool Parse::parseCom(std::string& strCom)
{
	if(strCom.length() == 0)
	{
		usage();
		return false;
	}

	const char* pCom = strCom.c_str();
	char* pSpa = strchr((char*)strCom.c_str(), ' ');
	if(pSpa)
	{
		*pSpa = '\0'; 
	}

	if(stricmp(pCom,"help") == 0)
		usage();
	else if(stricmp(pCom,"connect") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;

		std::string strEndpoint = pCom;
		
		connectSer(strEndpoint);
	}
	else if(stricmp(pCom,"netid") == 0)
		cout<<netId()<<endl;
	else if(stricmp(pCom,"type") == 0)
		cout<<type()<<endl;
	else if(stricmp(pCom,"valid") == 0)
	{
		if(valid())
			cout<<"true"<<endl;
		else
			cout<<"false"<<endl;
	}
	else if(stricmp(pCom,"openvolume") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;

		std::string strName;
		if(stricmp(pCom,"null") != 0)
			strName = pCom;
		openVolume(strName);		
	}
	else if(stricmp(pCom,"listvolume") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';

		std::string strFrom;
		if(stricmp(pCom,"null") != 0)
			strFrom = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		
		bool bInclude = true;
		if(stricmp(pCom,"0") == 0)
			bInclude = false;

		listVolumes(strFrom,bInclude);

	}
	else if(stricmp(pCom,"opencontentbyname") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;

		openContentByName(pCom);

	}
	else if(stricmp(pCom,"cachelevel") == 0)
		cacheLevel();
	else if(stricmp(pCom,"streamservice") == 0)
		streamServices();
	else if(stricmp(pCom,"listall") == 0)
	{
		if(pSpa != NULL)
			pCom = pSpa+1;
		else
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		while(*pCom == ' ')
			pCom++;
		std::string strCondition = pCom;
		listAll(strCondition);
	}
	else if(stricmp(pCom,"list") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		//get metadata names
		TianShanIce::StrValues metadataS;
		do
		{
			pCom = pSpa+1;
			while(*pCom == ' ')
				pCom++;
			pSpa = strchr((char*)pCom,' ');
			if(pSpa == NULL)
			{
				cout<<"The command must input right parameter"<<endl;
				return false;
			}
			*pSpa = '\0';
			
			if(stricmp(pCom,"null") == 0)
				break;
			else
				metadataS.push_back(pCom);
		}while(1);
		
		//get startName
		std::string strName;
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		if(stricmp(pCom,"null") != 0)
			strName = pCom;

		//get maxCount
		int max = 0;
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;

		max = atoi(pCom);
		listContent(metadataS,strName,max);
		
	}
	else if(stricmp(pCom,"event") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strType = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		std::string strCon;
		std::map<std::string,std::string> params;
		if(pSpa == NULL)//it has not params
		{
			strCon = pCom;	
			event(strType,strCon,params);
		}
		else
		{
			*pSpa = '\0';
			strCon = pCom;
			std::string strP;
			while(pSpa)
			{
				pCom = pSpa+1;
				while(*pCom == ' ')
					pCom++;
				pSpa = strchr((char*)pCom,' ');
				if(pSpa != NULL)	
					*pSpa = '\0';					
		
				strP = pCom;
				size_t index = strP.find("=",0);
				if(index == std::string::npos)
				{
					cout<<"event params format is invalidate"<<endl;
					return false;
				}
				params.insert(std::pair<std::string,std::string>(strP.substr(0,index),strP.substr(index+1,strP.length()-index-1)));

			}
			event(strType,strCon,params);
		}

	}
	else if(stricmp(pCom,"provisionevent") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strType = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strNetId;
		if(stricmp(pCom,"null") != 0)
			strNetId = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strVol = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		std::string strCon;
		std::map<std::string,std::string> params;
		if(pSpa == NULL)//it has not params
		{
			strCon = pCom;	
			provisionEvent(strType, strNetId, strVol,strCon,params);
		}
		else
		{
			*pSpa = '\0';
			strCon = pCom;
			std::string strP;
			while(pSpa)
			{
				pCom = pSpa+1;
				while(*pCom == ' ')
					pCom++;
				pSpa = strchr((char*)pCom,' ');
				if(pSpa != NULL)	
					*pSpa = '\0';					
		
				strP = pCom;
				size_t index = strP.find("=",0);
				if(index == std::string::npos)
				{
					cout<<"event params format is invalidate"<<endl;
					return false;
				}
				params.insert(std::pair<std::string,std::string>(strP.substr(0,index),strP.substr(index+1,strP.length()-index-1)));

			}
			provisionEvent(strType, strNetId, strVol, strCon, params);
		}

	}

	else if(stricmp(pCom,"opencontent") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		//find name
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strName = pCom;

		//find type
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strType = pCom;

		//find bool value if not exist create
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		bool bCreate = false;
		if(stricmp(pCom,"1") == 0)
			bCreate = true;
		else
			bCreate = false;

		if(openContent(strName, strType,bCreate) == NULL)
			cout<<"opencontent failed"<<endl;

	}
	//content command
	else if(stricmp(pCom,"populate") == 0)
		populate();
	else if(stricmp(pCom,"dirty") == 0)
		dirty();

	else if(stricmp(pCom,"inuse") == 0)
		isInUse();
	else if(stricmp(pCom,"enterstate") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		std::string strTarget = pCom;
		TianShanIce::Storage::ContentState cState;
		if(stricmp(strTarget.c_str(),"notprovision") == 0)
			cState = TianShanIce::Storage::csNotProvisioned;
		else if(stricmp(strTarget.c_str(),"provision") == 0)
			cState = TianShanIce::Storage::csProvisioning;
		else if(stricmp(strTarget.c_str(),"streamable") == 0)
			cState = TianShanIce::Storage::csProvisioningStreamable;
		else if(stricmp(strTarget.c_str(),"inservice") == 0)
			cState = TianShanIce::Storage::csInService;
		else if(stricmp(strTarget.c_str(),"outservice") == 0)
			cState = TianShanIce::Storage::csOutService;
		else if(stricmp(strTarget.c_str(),"clean") == 0)
			cState = TianShanIce::Storage::csCleaning;
		else 
		{
			 cout<<"unknown target state"<<endl;
			return false;
		}

		enterState(cState);
	}

	else if(stricmp(pCom,"checkresident") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		uint64 flags = 0;
		while(pSpa != NULL)
		{
			pCom = pSpa+1;
			while(*pCom == ' ')
				pCom++;

			pSpa = strchr((char*)pCom,' ');
			if(pSpa != NULL)
				*pSpa = '\0';
			if(stricmp(pCom,"residential") == 0)
				flags |= RSDFLAG(frfResidential);
			else if(stricmp(pCom,"read") == 0)
				flags |= RSDFLAG(frfReading);
			else if(stricmp(pCom,"write") == 0)
				flags |= RSDFLAG(frfWriting);
			else if(stricmp(pCom,"absence") == 0)
				flags |= RSDFLAG(frfAbsence);
			else if(stricmp(pCom,"corrupt") == 0)
				flags |= RSDFLAG(frfCorrupt);
			else if(stricmp(pCom,"directory") == 0)
				flags |= RSDFLAG(frfDirectory);
			else
				;
		}
		checkResidentialStatus(flags);
	}

	else if(stricmp(pCom,"restore") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		std::string stampLastFileWrite = pCom;
		restore(stampLastFileWrite);
	}
	else if(stricmp(pCom, "filepath") == 0)
		filePath();
	else if(stricmp(pCom, "name") == 0)
		getName();
	else if(stricmp(pCom,"metadata") == 0)
		metaData();
	else if(stricmp(pCom,"setmetadata") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		//get key
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strKey = pCom;

		//get value
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		std::string strVal = pCom;

		setMetaData(strKey, strVal);
	}
	else if(stricmp(pCom,"state") == 0)
		getState();
	else if(stricmp(pCom,"provisioned") == 0)
		provisioned();

	else if(stricmp(pCom,"provisiontime") == 0)
		provisionTime();
	else if(stricmp(pCom,"destroy") == 0)
	{
		if(pSpa)
		{
			pCom = pSpa+1;
			while(*pCom == ' ')
				pCom++;
			bool bMandatory = false;
			if(stricmp(pCom,"1") == 0)
				bMandatory = true;
			destroy2(bMandatory);
		}
		else
			destroy();
	}
	else if(stricmp(pCom,"store") == 0)
		getStore();
	else if(stricmp(pCom,"volume") == 0)
		getVolume();

	else if(stricmp(pCom, "localtype") == 0)
		localType();
	else if(stricmp(pCom, "subtype") == 0)
		subType();
	else if(stricmp(pCom,"framerate") == 0)
		frameRate();
	else if(stricmp(pCom,"resolution") == 0)
	{
		int horP = 0;
		int verP = 0;
		resolution(horP,verP);
	}
	else if(stricmp(pCom,"filesize") == 0)
		fileSize();
	else if(stricmp(pCom,"supportsize") == 0)
		supportFileSize();
	else if(stricmp(pCom,"playtime") == 0)
		playTime();
	else if(stricmp(pCom,"bitrate") == 0)
		bitRate();
	else if(stricmp(pCom,"md5checksum") == 0)
		md5Checksum();
	else if(stricmp(pCom,"trickspeed") == 0)
		trickSpeed();
	else if(stricmp(pCom,"exporturl") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';

		std::string strPro = pCom;
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;

		std::string strBitrate = pCom;
		int nTransferBitrate = atoi(strBitrate.c_str());
		int nTTL =0;
		TianShanIce::Properties pros;
		exportURL(strPro, nTransferBitrate, nTTL, pros);
	}
	else if(stricmp(pCom,"sourceurl") == 0)
		sourceURL();
	else if(stricmp(pCom,"provision") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strURL = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strType = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		bool bOver = false;
		if(stricmp(pCom,"1") == 0)
			bOver = true;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strStart = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strStop = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		int nMax = atoi(pCom);

		provision(strURL, strType,bOver,strStart,strStop,nMax);
	}
	else if(stricmp(pCom,"provisionpassive") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strType = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		bool bOver = false;
		if(stricmp(pCom,"1") == 0)
			bOver = true;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strStart = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strStop = pCom;

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		int nMax = atoi(pCom);
		
		provisionPassive(strType,bOver,strStart,strStop,nMax);
	}
	else if(stricmp(pCom,"cancelprovision") == 0)
		cancelProvision();
	

	else if(stricmp(pCom,"volumename") == 0)
		getVolumeName();
	else if(stricmp(pCom,"mountpath") == 0)
		mountPath();
	else if(stricmp(pCom,"volumeinfo") == 0)
		getInfo();
	else if(stricmp(pCom,"syncfilesystem") == 0)
		syncFileSystem();
	else if(stricmp(pCom,"capacity") == 0)
	{
		Ice::Long freeMB,totalMB;
		capacity(freeMB,totalMB);
	}
	else if(stricmp(pCom,"subvolume") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		std::string strSub = pCom;
		
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		pSpa = strchr((char*)pCom,' ');
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}
		*pSpa = '\0';
		bool bCreate = true;
		if(stricmp(pCom,"0") == 0)
			bCreate = false;
		
		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		long space = 0;
		space = atoi(pCom);

		openSubVolume(strSub,bCreate,space);
	}
	else if(stricmp(pCom,"parent") == 0)
		parent();
	else if(stricmp(pCom,"destroyvolume") == 0)
		destroyVolume();

	else if(stricmp(pCom,"filemodify") == 0)
		fileModify();
	else if(stricmp(pCom,"filerename") == 0)
	{
		if(pSpa == NULL)
		{
			cout<<"The command must input right parameter"<<endl;
			return false;
		}

		pCom = pSpa+1;
		while(*pCom == ' ')
			pCom++;
		
		std::string strNewName = pCom;
		fileRename(strNewName);
	}
	
	else
	{
		cout<<"Do not know the command"<<endl;
		cout<<"Please input help for more information"<<endl;
		return false;
	}	

	return true;
}

bool Parse::connectSer(std::string& strEndpoint)
{
	if(_commun == NULL)
	{
		cout<<"communicator is null"<<endl;
		return false;
	}

	try
	{
		_contentStorePrx = TianShanIce::Storage::ContentStorePrx::checkedCast(_commun->stringToProxy(strEndpoint.c_str()));
		if(_contentStorePrx == NULL)
		{
			cout<<"connect server "<<strEndpoint<<" failed"<<endl;
			return false;
		}

		_volumePrx = TianShanIce::Storage::VolumePrx::checkedCast(_contentStorePrx);
		if(_volumePrx == NULL)
			cout<<"volume proxy is NULL"<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"Connect server catch a ice exception "<<ex.ice_name()<<endl;
		return false;
	}
	catch(...)
	{
		cout<<"Connect server catch an unknown exception"<<endl;
		return false;
	}

	return true;
}

std::string Parse::netId()
{
	std::string strId;
	try
	{
		strId = _contentStorePrx->getNetId();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"NetId catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"NetId catch an unknown exception "<<endl;
		return "";
	}
	return strId;
}

std::string Parse::type()
{
	std::string strType;
	try
	{
		strType = _contentStorePrx->type();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"type catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"type catch an unknown exception "<<endl;
		return "";
	}
	return strType;
}

bool Parse::valid()
{
	bool bvalid = false;
	try
	{
		bvalid = _contentStorePrx->isValid();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"valid catch a exception "<<ex.ice_name()<<endl;
		return false;
	}
	catch(...)
	{
		cout<<"valid catch an unknown exception "<<endl;
		return false;
	}

	return bvalid;
}

TianShanIce::Storage::VolumePrx Parse::openVolume(std::string& volName)
{
	try
	{
		_volumePrx = _contentStorePrx->openVolume(volName);
		if(_volumePrx == NULL)
			cout<<"not open volume "<<volName<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"open volume catch a exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"open volume catch an unknown exception"<<endl;
		return NULL;
	}

	return _volumePrx;
}

std::vector<TianShanIce::Storage::VolumeInfo> Parse::listVolumes(std::string strFrom, bool includeVirtual)
{
	std::vector<TianShanIce::Storage::VolumeInfo> infos;
	try
	{
		infos = _contentStorePrx->listVolumes(strFrom, includeVirtual);

		std::vector<TianShanIce::Storage::VolumeInfo>::iterator it;
		for(it = infos.begin(); it != infos.end(); it++)
		{
			cout<<"name:"<<it->name<<"  virtual:";
			if(it->isVirtual)
				cout<<"true"<<endl;
			else
				cout<<"false"<<endl;
			cout<<"propertiess:"<<endl;
			std::map<std::string,std::string>::iterator itMD;
			for(itMD = it->metaData.begin(); itMD != it->metaData.end(); itMD++)
			{
				cout<<itMD->first<<": "<<itMD->second<<endl;
			}
			cout<<endl;
		}
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"list volumes catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"list volumes catch an unknown exception"<<endl;
	}
	
	return infos;
}

TianShanIce::Storage::ContentPrx Parse::openContentByName(std::string strFullName)
{
	try
	{
		_contentPrx = TianShanIce::Storage::UnivContentPrx::checkedCast(_contentStorePrx->openContentByFullname(strFullName));
		if(_contentPrx == NULL)
			cout<<"open content by full name failed,return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"open content by full name catch exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"open content by full name catch an unknown exception"<<endl;
		return NULL;
	}

	return _contentPrx;
}


TianShanIce::Storage::ContentInfos Parse::listContent(TianShanIce::StrValues& metaDataNames, std::string& startName, int& maxCount)
{
	TianShanIce::Storage::ContentInfos contentV;
	try
	{
		contentV = _volumePrx->listContents(metaDataNames,startName,maxCount);
		TianShanIce::Storage::ContentInfos::iterator it = contentV.begin();
		TianShanIce::Storage::ContentState state;
		for(;it < contentV.end(); it++)
		{
			cout<<"name: "<<(*it).name<<"\tfullname: "<<(*it).fullname<<"\tstate: ";
			state = (*it).state;
			switch(state)
			{
				case TianShanIce::Storage::csNotProvisioned:cout<<"csNotProvisioned";
					break;

				case TianShanIce::Storage::csProvisioning:cout<<"csProvisioning";
					break;

				case TianShanIce::Storage::csProvisioningStreamable:cout<<"csProvisioningStreamable";
					break;

				case TianShanIce::Storage::csInService:cout<<"csInService";
					break;

				case TianShanIce::Storage::csOutService:cout<<"csOutService";
					break;

				case TianShanIce::Storage::csCleaning:cout<<"csCleaning";
					break;

				default:cout<<"unknown content state";
					break;;		
			}
			::TianShanIce::Properties::iterator itp = (*it).metaData.begin();
			cout<<"\tmetadata: ";
			for(; itp != (*it).metaData.end(); itp++)
				cout<<itp->first<<" = "<<itp->second<<" "; 
			cout<<endl;
		}
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"list catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"list catch an unknown exception "<<endl;
	}
	return contentV;
}

void Parse::event(std::string& strEventType,std::string& strContentName,std::map<std::string,std::string> params)
{
	::TianShanIce::Storage::FileEvent eventType;
	if(stricmp(strEventType.c_str(),"create") == 0)
		eventType = TianShanIce::Storage::fseFileCreated;
	else if(stricmp(strEventType.c_str(),"delete") == 0)
		eventType = ::TianShanIce::Storage::fseFileDeleted;
	else if(stricmp(strEventType.c_str(),"rename") == 0)
		eventType = ::TianShanIce::Storage::fseFileRenamed;
	else if(stricmp(strEventType.c_str(),"modify") == 0)
		eventType = ::TianShanIce::Storage::fseFileModified;
	else if(stricmp(strEventType.c_str(),"security") == 0)
		eventType = ::TianShanIce::Storage::fseSecurity;
	else
	{
		cout<<"event do not know event type: "<<strEventType<<endl;
		return;
	}

	TianShanIce::Properties props = params;

	try
	{
		TianShanIce::Storage::ContentStoreExPrx::checkedCast(_contentStorePrx)->OnFileEvent(eventType,strContentName,props);
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"event catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"event content catch an unknown exception "<<endl;
	}

}

TianShanIce::Storage::ContentPrx Parse::openContent(const std::string& strName,const std::string& strContentType, bool createIfNotExist)
{
	try
	{
		_contentPrx = TianShanIce::Storage::UnivContentPrx::checkedCast(_volumePrx->openContent(strName,strContentType,createIfNotExist));
		if(_contentPrx == NULL)
			cout<<"open content failed,return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"open content catch a exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"open content catch an unknown exception "<<endl;
		return NULL;
	}

	return _contentPrx;

}

bool Parse::dirty()
{
	bool bDirty = false;
	try
	{
		bDirty = _contentPrx->isDirty();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"dirty catch a exception "<<ex.ice_name()<<endl;
		return false;
	}
	catch(...)
	{
		cout<<"dirty catch an unknown exception "<<endl;
		return false;
	}

	if(bDirty)
		cout<<"true"<<endl;
	else
		cout<<"false"<<endl;

	return bDirty;
	
}

void Parse::populate()
{
	try
	{
		_contentPrx->populateAttrsFromFilesystem();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"populate catch a exception "<<ex.ice_name()<<endl;
		return;
	}
	catch(...)
	{
		cout<<"populate catch an unknown exception "<<endl;
		return;
	}
}

void Parse::restore(std::string& stampLastFileWrite)
{
	try
	{
		_contentPrx->OnRestore(stampLastFileWrite);
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"restore catch a exception "<<ex.ice_name()<<endl;
		return;
	}
	catch(...)
	{
		cout<<"restore catch an unknown exception "<<endl;
		return;
	}
}

std::string Parse::filePath()
{
	std::string strPath;
	try
	{
		strPath = _contentPrx->getMainFilePathname();
		cout<<"mainFilePathName: "<<strPath<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"file path catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"file path catch an unknown exception "<<endl;
	}

	return strPath;
}

TianShanIce::Storage::ContentStorePrx Parse::getStore()
{
	try
	{
		_contentStorePrx = _contentPrx->getStore();
		if(_contentStorePrx == NULL)
			cout<<"get store faile,return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"get store catch a exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"get store catch an unknown exception "<<endl;
		return NULL;
	}

	return _contentStorePrx;
}

TianShanIce::Storage::VolumePrx	Parse::getVolume()
{
	try
	{
		_volumePrx = _contentPrx->theVolume();
		if(_volumePrx == NULL)
			cout<<"get volume failed, return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"get volume catch a exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"get volume catch an unknown exception "<<endl;
		return NULL;
	}

	return _volumePrx;
}

std::string Parse::getName()
{
	std::string strName;
	try
	{
		strName = _contentPrx->getName();
		cout<<strName<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"name catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"name catch an unknown exception "<<endl;
		return "";
	}
	return strName;
}

bool Parse::metaData()
{
	try
	{
		TianShanIce::Properties props;
		props = _contentPrx->getMetaData();
		TianShanIce::Properties::iterator it;
		for(it = props.begin(); it != props.end(); it++)
		{
			cout<<it->first<<" = "<<it->second<<endl;
		}
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"metadata catch a exception "<<ex.ice_name()<<endl;
		return false;
	}
	catch(...)
	{
		cout<<"metadata catch an unknown exception "<<endl;
		return false;
	}

	return true;
}

void Parse::setMetaData(std::string& strKey, std::string strValue)
{
	try
	{
		_contentPrx->setUserMetaData(strKey, strValue);
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"setmetadata catch a exception "<<ex.ice_name()<<endl;
		return ;
	}
	catch(...)
	{
		cout<<"setmetadate catch an unknown exception "<<endl;
		return ;
	}
}

std::string Parse::getState()
{
	try
	{
		TianShanIce::Storage::ContentState state;
		state = _contentPrx->getState();
		switch(state)
		{
			case TianShanIce::Storage::csNotProvisioned:cout<<"csNotProvisioned"<<endl;
				return "csNotProvisioned";

			case TianShanIce::Storage::csProvisioning:cout<<"csProvisioning"<<endl;
				return "csProvisioning";

			case TianShanIce::Storage::csProvisioningStreamable:cout<<"csProvisioningStreamable"<<endl;
				return "csProvisioningStreamable";

			case TianShanIce::Storage::csInService:cout<<"csInService"<<endl;
				return "csInService";

			case TianShanIce::Storage::csOutService:cout<<"csOutService"<<endl;
				return "csOutService";

			case TianShanIce::Storage::csCleaning:cout<<"csCleaning"<<endl;
				return "csCleaning";

			default:cout<<"unknown content state"<<endl;
				return "";		
		}

	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"setmetadata catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"setmetadate catch an unknown exception "<<endl;
		return "";
	}
}

bool Parse::provisioned()
{
	bool bPro = false;
	try
	{
		bPro =_contentPrx->isProvisioned();
		if(bPro)
			cout<<"true"<<endl;
		else
			cout<<"false"<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"provisioned catch a exception "<<ex.ice_name()<<endl;
		return false;
	}
	catch(...)
	{
		cout<<"provisioned catch an unknown exception "<<endl;
		return false;
	}

	return bPro;
}

void Parse::destroy()
{
	try
	{
		_contentPrx->destroy();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"destroy catch a exception "<<ex.ice_name()<<endl;
		return ;
	}
	catch(...)
	{
		cout<<"destroy catch an unknown exception "<<endl;
		return ;
	}
}

void Parse::destroy2(bool mandatory)
{
	try
	{
		_contentPrx->destroy2(mandatory);
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"destroy catch a exception "<<ex.ice_name()<<endl;
		return ;
	}
	catch(...)
	{
		cout<<"destroy catch an unknown exception "<<endl;
		return ;
	}
}

std::string Parse::localType()
{
	std::string strType;
	try
	{
		strType = _contentPrx->getLocaltype();
		cout<<strType<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"localtype catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"localtype catch an unknown exception "<<endl;
		return "";
	}
	return strType;
}

std::string Parse::subType()
{
	std::string strType;
	try
	{
		strType = _contentPrx->getSubtype();
		cout<<strType<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"subtype catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"subtype catch an unknown exception "<<endl;
		return "";
	}
	return strType;
}

Ice::Float Parse::frameRate()
{
	Ice::Float frate = 0.0;
	try
	{
		frate = _contentPrx->getFramerate();
		cout<<frate<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"framerate catch a exception "<<ex.ice_name()<<endl;
		return 0.0;
	}
	catch(...)
	{
		cout<<"framerate catch an unknown exception "<<endl;
		return 0.0;
	}
	return frate;
}

void Parse::resolution(int& horizontalPixel, int& verticalPixel)
{
	try
	{
		_contentPrx->getResolution(horizontalPixel,verticalPixel);
		cout<<"horizontal: "<<horizontalPixel<<"  vertical: "<<verticalPixel<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"resolution catch a exception "<<ex.ice_name()<<endl;
		return;
	}
	catch(...)
	{
		cout<<"resolution catch an unknown exception "<<endl;
		return;
	}
}

long Parse::fileSize()
{
	Ice::Long lsize = 0;
	try
	{
		lsize = _contentPrx->getFilesize();
		cout<<lsize<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"filesize catch a exception "<<ex.ice_name()<<endl;
		return 0;
	}
	catch(...)
	{
		cout<<"filesize catch an unknown exception "<<endl;
		return 0;
	}
	return (long)lsize;
}

long Parse::supportFileSize()
{
	Ice::Long lsize = 0;
	try
	{
		lsize = _contentPrx->getSupportFileSize();
		cout<<lsize<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"supportfilesize catch a exception "<<ex.ice_name()<<endl;
		return 0;
	}
	catch(...)
	{
		cout<<"supportfilesize catch an unknown exception "<<endl;
		return 0;
	}
	return (long)lsize;
}

long Parse::playTime()
{
	Ice::Long lsize = 0;
	try
	{
		lsize = _contentPrx->getPlayTime();
		cout<<lsize<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"playtime catch a exception "<<ex.ice_name()<<endl;
		return 0;
	}
	catch(...)
	{
		cout<<"playtime catch an unknown exception "<<endl;
		return 0;
	}
	return (long)lsize;

}

int Parse::bitRate()
{
	Ice::Int rate = 0;
	try
	{
		rate = _contentPrx->getBitRate();
		cout<<rate<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"bitrate catch a exception "<<ex.ice_name()<<endl;
		return 0;
	}
	catch(...)
	{
		cout<<"bitrate catch an unknown exception "<<endl;
		return 0;
	}
	return (int)rate;
}

std::string Parse::md5Checksum()
{
	std::string strChe;
	try
	{
		strChe = _contentPrx->getMD5Checksum();
		cout<<strChe<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"md5checksum catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"md5checksum catch an unknown exception "<<endl;
		return "";
	}
	return strChe;
}

TianShanIce::Storage::TrickSpeedCollection Parse::trickSpeed()
{
	TianShanIce::Storage::TrickSpeedCollection trickS;
	try
	{
		trickS = _contentPrx->getTrickSpeedCollection();
		TianShanIce::Storage::TrickSpeedCollection::iterator it;
		for(it = trickS.begin(); it < trickS.end(); it++)
			cout<<(*it)<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"md5checksum catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"md5checksum catch an unknown exception "<<endl;
	}

	return trickS;
}

std::string Parse::exportURL(std::string& transferProtocol, int transferBitrate, int& ttl, TianShanIce::Properties& params)
{
	std::string strURL;
	int permittedBitrate;
	try
	{
		strURL = _contentPrx->getExportURL(transferProtocol, transferBitrate, ttl, params);
		permittedBitrate = atoi(params[::TianShanIce::Storage::expTransferBitrate].c_str());

		cout<<"URL: "<<strURL<<" TTL: "<<ttl<<" PermitedBitrate: "<<permittedBitrate<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"exportURL catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"exportURL catch an unknown exception "<<endl;
		return "";
	}

	return strURL;
}

std::string Parse::sourceURL()
{
	std::string strURL;
	try
	{
		strURL = _contentPrx->getSourceUrl();
		cout<<strURL<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"sourceURL catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"sourceURL catch an unknown exception "<<endl;
		return "";
	}
	return strURL;
}

void Parse::provision(std::string& sourceUrl,std::string& sourceContentType,bool overwrite,std::string startTimeUTC,std::string stopTimeUTC,int maxTransferBitrate)
{
	try
	{
		_contentPrx->provision(sourceUrl,sourceContentType,overwrite,startTimeUTC,stopTimeUTC,maxTransferBitrate);
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"provision catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"provision catch an unknown exception "<<endl;
	}

}

std::string Parse::provisionPassive(std::string& sourceContentType,bool overwrite,std::string startTimeUTC,std::string stopTimeUTC,int maxTransferBitrate)
{
	std::string strURL;
	try
	{
		_contentPrx->provisionPassive(sourceContentType,overwrite,startTimeUTC,stopTimeUTC,maxTransferBitrate);
		cout<<strURL<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"provisionpassive catch a exception "<<ex.ice_name()<<endl;
		return "";
	}
	catch(...)
	{
		cout<<"provisionpassive catch an unknown exception "<<endl;
		return "";
	}
	return strURL;
}

void Parse::cancelProvision()
{
	try
	{
		_contentPrx->cancelProvision();
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"cancelprovision catch a exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"cancelprovision catch an unknown exception "<<endl;
	}
}

std::string Parse::mountPath()
{
	std::string strPath;
	try
	{
		strPath = TianShanIce::Storage::VolumeExPrx::checkedCast(_volumePrx)->getMountPath();
		cout<<strPath<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"mount path catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"mount path catch an unknown exception"<<endl;
	}
	
	return strPath;
}

TianShanIce::Storage::VolumeInfo Parse::getInfo()
{
	TianShanIce::Storage::VolumeInfo info;
	try
	{
		info = TianShanIce::Storage::VolumeExPrx::checkedCast(_volumePrx)->getInfo();
		cout<<"name: "<<info.name<<"  bVirtual:";
		if(info.isVirtual)
			cout<<"true";
		else
			cout<<"false";
		cout<<endl;
		cout<<"properties:"<<endl;
		std::map<std::string,std::string>::iterator it;
		for(it = info.metaData.begin(); it != info.metaData.end(); it++)
			cout<<it->first<<": "<<it->second<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"get info catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"get info catch an unknown exception"<<endl;
	}

	return info;
}

void Parse::capacity(Ice::Long& freeMB, Ice::Long& totalMB)
{
	try
	{
		_volumePrx->getCapacity(freeMB, totalMB);
		cout<<"free capacity: "<<freeMB<<"    total capacity: "<<totalMB<<endl;
	}
	catch(const ::Ice::Exception &ex)
	{
		cout<<"capacity catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"capacity catch an unknown exception "<<endl;
	}

}

std::string Parse::getVolumeName()
{
	std::string strName;
	try
	{
		strName = _volumePrx->getVolumeName();
		cout<<strName<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"get volume name catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"get volume name catch an unknown exception"<<endl;
	}
	
	return strName;
}

std::vector<std::string> Parse::listAll(std::string condition)
{
	std::vector<std::string> contents;
	try
	{
//		contents = _contentStorePrx->listContent(condition);
		contents = _volumePrx->listContent(condition);
		std::vector<std::string>::iterator it;
		for(it = contents.begin(); it != contents.end(); it++)
			cout<<(*it)<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"list all content catch an exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"list all content catch unknown exception"<<endl;
	}
	
	return contents;
}

TianShanIce::Storage::VolumePrx Parse::openSubVolume(std::string& subname, bool createIfNotExist, long& quotaSpaceMB)
{
	try
	{
		_volumePrx = _volumePrx->openSubVolume(subname,createIfNotExist,quotaSpaceMB);
		if(_volumePrx == NULL)
			cout<<"open sub volume failed,return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"open subvolume catch exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"open subvolume catch unknow exception"<<endl;
		return NULL;
	}

	return _volumePrx;
}

TianShanIce::Storage::VolumePrx Parse::parent()
{
	try
	{
		_volumePrx = _volumePrx->parent();
		if(_volumePrx == NULL)
			cout<<"get parent volume failed,return a NULL point"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"open parent catch exception "<<ex.ice_name()<<endl;
		return NULL;
	}
	catch(...)
	{
		cout<<"open parent catch unknow exception"<<endl;
		return NULL;
	}
	
	return _volumePrx;
}

void Parse::destroyVolume()
{
	try
	{
		_volumePrx->destroy();
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"destroy volume catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"destroy volume catch unknow exception"<<endl;
	}
}

void Parse::fileModify()
{
	try
	{
		_contentPrx->OnFileModified();
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"file modify catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"file modify catch an unknown exception"<<endl;
	}
}

void Parse::fileRename(std::string& strNewName)
{
	try
	{
		_contentPrx->OnFileRenamed(strNewName);
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"file rename catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"file rename catch an unknown exception"<<endl;
	}

}

TianShanIce::Storage::ContentState Parse::enterState(TianShanIce::Storage::ContentState targetState)
{
	TianShanIce::Storage::ContentState cState;
	try
	{
		cState = _contentPrx->enterState(targetState);
		switch(cState)
		{
		case TianShanIce::Storage::csNotProvisioned:
			cout<<"csNotProvisioned"<<endl; break;
		case TianShanIce::Storage::csProvisioning:
			cout<<"csProvisioning"<<endl; break;
		case TianShanIce::Storage::csProvisioningStreamable:
			cout<<"csProvisioningStreamable"<<endl; break;
		case TianShanIce::Storage::csInService:
			cout<<"csInService"<<endl; break;
		case TianShanIce::Storage::csOutService:
			cout<<"csOutService"<<endl; break;
		case TianShanIce::Storage::csCleaning:
			cout<<"csCleaning"<<endl; break;
		default:
			cout<<"unknown contentstate"<<endl;

		}
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"enterstate catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"enterstate catch an unknown exception"<<endl;
	}

	return cState;
	
}

void Parse::provisionEvent(std::string& provisionEvent, std::string& storeNetId, std::string& volumeName, std::string& contentName, TianShanIce::Properties& params)
{
	try
	{
		TianShanIce::Storage::ProvisionEvent pEvent;
		if(stricmp(provisionEvent.c_str(),"start") == 0)
			pEvent = TianShanIce::Storage::peProvisionStarted;
		else if(stricmp(provisionEvent.c_str(),"stop") == 0)
			pEvent = TianShanIce::Storage::peProvisionStopped;
		else if(stricmp(provisionEvent.c_str(),"progress") == 0)
			pEvent = TianShanIce::Storage::peProvisionProgress;
		else if(stricmp(provisionEvent.c_str(),"streamable") == 0)
			pEvent = TianShanIce::Storage::peProvisionStreamable;
		else 
		{
			cout<<"unknown provision event"<<endl;
			return;
		}

		TianShanIce::Storage::ContentStoreExPrx::checkedCast(_contentStorePrx)->OnProvisionEvent(pEvent, storeNetId, volumeName, contentName, params);
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"provision event catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"provision event catch an unknown exception"<<endl;
	}
}

bool Parse::isInUse()
{
	bool bInuse = false;
	try
	{
//		bInuse = _contentPrx->isInUse();
		if(bInuse)
			cout<<"true"<<endl;
		else
			cout<<"false"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"inuse catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"inuse catch an unknown exception"<<endl;
	}
	return bInuse;
}

std::string Parse::provisionTime()
{
	std::string provT;
	try
	{
		provT = _contentPrx->getProvisionTime();
		cout<<provT<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"provisiontime catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"provisiontime catch an unknown exception"<<endl;
	}
	return provT;
}

void Parse::syncFileSystem()
{
	try
	{
		TianShanIce::Storage::VolumeExPrx::checkedCast(_volumePrx)->syncWithFileSystem();
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"syncfilesystem catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"syncfilesystem catch an unknown exception"<<endl;
	}
}

uint64 Parse::checkResidentialStatus(uint64& flagsToTest)
{
	uint64 flag = 0;
	try
	{
		flag = _contentPrx->checkResidentialStatus(flagsToTest);
		if(flag & RSDFLAG(frfResidential))
			cout<<"frfResidential"<<endl;
		if(flag & RSDFLAG(frfReading))
			cout<<"frfReading"<<endl;
		if(flag & RSDFLAG(frfWriting))
			cout<<"frfWriting"<<endl;
		if(flag & RSDFLAG(frfAbsence))
			cout<<"frfAbsence"<<endl;
		if(flag & RSDFLAG(frfCorrupt))
			cout<<"frfCorrupt"<<endl;
		if(flag & RSDFLAG(frfDirectory))
			cout<<"frfDirectory"<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"checkresident catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"checkresident catch an unknown exception"<<endl;
	}

	return flag;
}

uint8 Parse::cacheLevel()
{
	uint8 level = 0;
	try
	{
		level = _contentStorePrx->getCacheLevel();
		cout<<level<<endl;
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"cachelevel catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"cachelevel catch an unknown exception"<<endl;
	}
	return level;
}

TianShanIce::Replicas Parse::streamServices()
{
	TianShanIce::Replicas reps;
	try
	{
		reps = _contentStorePrx->getStreamServices();
		std::vector<TianShanIce::Replica>::iterator it;
		for(it = reps.begin(); it < reps.end(); it++)
		{
			cout<<"category " << (*it).category<< " groupid "<<(*it).groupId<<" replicaId "<<(*it).replicaId
				<<" priority "<<(*it).priority <<" disabled "<<(*it).disabled<< " maxPrioritySeenInGroup "<<(*it).maxPrioritySeenInGroup
				<<" stampKnew "<<(*it).stampKnew<<" stampUpdated "<<(*it).stampUpdated<<endl;
		}
	}
	catch(const ::Ice::Exception& ex)
	{
		cout<<"streamservice catch exception "<<ex.ice_name()<<endl;
	}
	catch(...)
	{
		cout<<"streamservice catch an unknown exception"<<endl;
	}

	return reps;
}

