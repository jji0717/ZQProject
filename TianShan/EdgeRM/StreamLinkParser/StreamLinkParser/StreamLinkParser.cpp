// StreamLinkParser.cpp : Defines the entry point for the console application.

#include <algorithm>
#include "TianShanDefines.h"
#include "EdgeRM.h"
#include "CfgParser.h"

#ifdef ZQ_OS_LINUX
#include <unistd.h>
#else
#include <io.h>
#endif

std::string strERMEndpoints;
std::string strOutputPath;
std::string strDevicePrefix;
int linkType;
std::string strSopName;
int addDeviceInfoToDB = 0;

std::string s6ServerIp = "127.0.0.1";
int			s6ServerPort = 10554;
std::string deviceZone="xor";

std::string strLinkType = "";

typedef struct
{
	std::string servicegroup;
	std::string Qam_modulationFormat;
	std::string Qam_IP;
	std::string Qam_Mac;
	std::string Qam_basePort;
	std::string Qam_portMask;
	std::string Qam_portStep;
	std::string Qam_symbolRate;
	std::string Qam_frequency;
	std::string PN;
	std::string TotalBandwidth;
	std::string streamerId;
}ChannelInfo;
typedef std::vector< ChannelInfo >ChanneliInfos;
typedef std::map<std::string, ChanneliInfos>Ports;
typedef std::map<std::string, Ports>Devices;
typedef std::map<std::string, std::string>ServiceGroups;
/// create Device struct 
typedef struct 
{
	Ice::Short portId;
	Ice::Int serviceGroup;
    ::TianShanIce::LValues  channels;
}PortInfo;
typedef std::vector<PortInfo> PortInfos;
typedef struct 
{
	std::string filename;
	std::string QAMname;
	std::string QAMZone;
	PortInfos   portinfos;
}DeviceInfo;
typedef std::vector<DeviceInfo> DeviceInfos;
typedef std::map<std::string , ::TianShanIce::StrValues> ServiceToQAMId;
typedef std::map<std::string, ServiceToQAMId> StreamLinkToServiceGroups;
bool mod_less(ChannelInfo chinfo1, ChannelInfo chinfo2)
{
	int nres = stricmp(chinfo1.Qam_IP.c_str() , chinfo2.Qam_IP.c_str());
	if( nres >= 0)
	{
		if(nres ==0)
		{
          if(stricmp(chinfo1.Qam_frequency.c_str(), chinfo2.Qam_frequency.c_str()) >= 0)
			  return false;
		  else
			  return true;
		}
		return false;
	}
	else
		return true;
}
bool mod_less_serviceGroup(ChannelInfo chinfo1, ChannelInfo chinfo2)
{
	int nres = stricmp(chinfo1.Qam_IP.c_str() , chinfo2.Qam_IP.c_str());
	if( nres >= 0)
	{
		if(nres ==0)
		{
			if(stricmp(chinfo1.servicegroup.c_str(), chinfo2.servicegroup.c_str()) >= 0)
				return false;
			else
				return true;
		}
		return false;
	}
	else
		return true;
}
void ParserDatas(std::string& key, std::string& _type, int range, ITEMS& items, ChannelInfo& channelinfo)
{
	std::string strTemp="";
	if(items.size() > 0)
		strTemp = items[0];

	if(key=="Qam.modulationFormat")
	{
	   channelinfo.Qam_modulationFormat = strTemp;
	}
	else if(key =="Qam.IP")
	{
      channelinfo.Qam_IP = strTemp;
	}
	else if(key =="Qam.Mac")
	{
	  channelinfo.Qam_Mac = strTemp;
	}
	else if(key =="Qam.basePort")
	{
	channelinfo.Qam_basePort = strTemp;
	}
	else if(key =="Qam.portMask")
	{
		channelinfo.Qam_portMask = strTemp;
	}
	else if(key =="Qam.portStep")
	{
		channelinfo.Qam_portStep = strTemp;
	}
	else if(key =="Qam.symbolRate")
	{
		channelinfo.Qam_symbolRate = strTemp;
	}
	else if(key =="Qam.frequency")
	{
		channelinfo.Qam_frequency = strTemp;
	}
	else if(key =="PN")
	{
		if(items.size() == 2)
			channelinfo.PN = items[0] + "~" + items[1];
		else
			channelinfo.PN = strTemp;
	}
	else if(key =="TotalBandwidth")
	{
		channelinfo.TotalBandwidth = strTemp;
	}			
}
void addDeviceToEdgeRM(std::string endpoints, DeviceInfos deviceinfos);
void GenerateQAMInfo(Devices& devices)
{
	StreamLinkToServiceGroups streamLinkToSGs;
	DeviceInfos deviceinfos;
	Devices::iterator itorDevice;
	int QamCount = 1;
	for(itorDevice = devices.begin(); itorDevice != devices.end(); itorDevice++)
	{
		DeviceInfo deviceinfo;
		
		std::string deviceIp = itorDevice->first;

		Ports& ports = itorDevice->second;
		std::string deviceFilename = deviceIp;
		replace(deviceFilename.begin(), deviceFilename.end(), '.','_');
		deviceFilename = strOutputPath + deviceFilename + ".xml";
		FILE* fp = fopen(deviceFilename.c_str(), "w");
		if(fp == NULL)
		{
			printf("failed open %s file \n", deviceFilename.c_str());
			continue;
		}

        char QamName[128]= "";
		if(!strDevicePrefix.empty())
		{
			sprintf(QamName, "%s%d", strDevicePrefix.c_str(), QamCount);
			deviceinfo.QAMname = QamName;
		}
		else
		{
			deviceinfo.QAMname = deviceIp;
			replace(deviceinfo.QAMname.begin(), deviceinfo.QAMname.end(), '.','_');
		}
		deviceinfo.filename = deviceFilename;
		deviceinfo.QAMZone = deviceZone;

		///write Device info, begin EdgeDevice
		char strTemp[2048];
		sprintf(strTemp, "<EdgeDevices>\n  <EdgeDevice name=\"%s\" zone=\"%s\"\n            vendor=\"CASA\" model=\"2700\"\n            tftp=\"tfp://192.168.81.108:6060\"\n            adminUrl=\"192.168.81.108:9090\"\n    desc=\"qam on rack 11-3\" > \n\0",deviceinfo.QAMname.c_str(), deviceinfo.QAMZone.c_str());
		fwrite(strTemp, strlen(strTemp), 1, fp);
		memset(strTemp, 0, 2048);

		bool bWriteDeviceMac =false;
		//write portinfo
		Ports::iterator itorPorts = ports.begin();
		for(int i = 0; itorPorts != ports.end(); i++, itorPorts++)
		{
			ChanneliInfos chinfos = itorPorts->second;
			if(chinfos.size() < 1)
				continue;
            //deviceMac在Channel中存储，只需要取到其中一个即可。
			if(!bWriteDeviceMac)
			{
				std::string& deviceMac =  chinfos[0].Qam_Mac;
				///write DeviceIp info
				sprintf(strTemp, "    <DeviceIP>\n       <Address ip=\"%s\" mac=\"%s\"/>\n    </DeviceIP>\n\0", deviceIp.c_str(),deviceMac.c_str());
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);
				bWriteDeviceMac = true;
			}

			PortInfo portinfo;
			portinfo.portId = (Ice::Short)(i+1);
			portinfo.serviceGroup = atoi((itorPorts->first).c_str());
			///begin write port info
			sprintf(strTemp, "    <EdgePort id=\"%d\" powerLevel=\"50\">\n        <AtscModulationMode modulationFormat=\"%s\" interleaverMode=\"1\" interleaverLevel=\"1\" />\n\0", i+1, chinfos[0].Qam_modulationFormat.c_str());
			fwrite(strTemp, strlen(strTemp), 1, fp);
			memset(strTemp, 0, 2048);
			for(int j = 0; j < chinfos.size(); j++)
			{
				ChannelInfo chinfo = chinfos[j];

				StreamLinkToServiceGroups::iterator itorSG;
				itorSG = streamLinkToSGs.find(chinfo.streamerId);
				if(itorSG != streamLinkToSGs.end())
				{
                   ServiceToQAMId &SgToQAMID = itorSG->second;
				   ServiceToQAMId::iterator itorSGQamID = SgToQAMID.find(chinfo.servicegroup);
				   if(itorSGQamID != SgToQAMID.end())
				   {
					   TianShanIce::StrValues&QamIDs = itorSGQamID->second;
					   TianShanIce::StrValues::iterator itorQAMIDs = std::find(QamIDs.begin(), QamIDs.end(),deviceinfo.QAMname);
					   if(itorQAMIDs == QamIDs.end())
					   {
						   QamIDs.push_back(deviceinfo.QAMname);
						//   QamIDs.push_back(deviceIp);
					   }
				   }
				   else
				   {
					   TianShanIce::StrValues QamIDs;
					   QamIDs.push_back(deviceinfo.QAMname); 
					 //  QamIDs.push_back(deviceIp); 
					   SgToQAMID[chinfo.servicegroup] =  QamIDs;
				   }
				}
				else
				{
					TianShanIce::StrValues QamIDs;
					QamIDs.push_back(deviceinfo.QAMname);
					//QamIDs.push_back(deviceIp); 
                   ServiceToQAMId SGtoQAMID;
				   SGtoQAMID[chinfo.servicegroup] =  QamIDs;
				   streamLinkToSGs[chinfo.streamerId] =  SGtoQAMID;
				}
				///begin write channel info

				int freq = atoi(chinfo.Qam_frequency.c_str()) / 1000;
				int symbolRate= atoi(chinfo.Qam_symbolRate.c_str());
				sprintf(strTemp, "        <EdgeChannel id=\"%d\" freq=\"%d\" symbolRate=\"%d\" tsId=\"200\" nitpid=\"6\">\n", j, freq, symbolRate);
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);

				sprintf(strTemp, "          <Provision enabled=\"1\" inbandMarker=\"type=4;pidType=A;pidValue=01EE;dataType=T;insertDuration=10000;data=4002003030\" reportTrafficMismatch=\"1\" jitterBuffer=\"200\"/>\n");
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);

				int startPN = 21, maxSession = 200;
				int npos = chinfo.PN.find("~");
				if(npos > 0)
				{
					startPN = atoi((chinfo.PN.substr(0, npos)).c_str());
					maxSession = atoi((chinfo.PN.substr(npos+1).c_str())) - startPN + 1;
				}


				int baseprot_temp = atoi(chinfo.Qam_basePort.c_str());
				int basePort = atoi(chinfo.Qam_basePort.c_str()) & atoi(chinfo.Qam_portMask.c_str());
				sprintf(strTemp, "          <UDP startPort=\"%d\" step=\"%s\" startPN=\"%d\" maxSession=\"%d\" />\n", 
					basePort, chinfo.Qam_portStep.c_str(), startPN, maxSession);
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);

				if(baseprot_temp != basePort)
					printf("%s, channelId: %d, portmask[%s] basePort[%d--->%d]\n", deviceinfo.QAMname.c_str(), j, chinfo.Qam_portMask.c_str(), baseprot_temp, basePort);

				sprintf(strTemp, "          <UtilizationThreshold low=\"100\" high=\"%s\" />\n", chinfo.TotalBandwidth.c_str());
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);

				sprintf(strTemp, "          <MPTS intervalPAT=\"40\" intervalPMT=\"400\" />\n");
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);
				// end write channel info
				sprintf(strTemp, "        </EdgeChannel>\n");
				fwrite(strTemp, strlen(strTemp), 1, fp);
				memset(strTemp, 0, 2048);
				portinfo.channels.push_back(atol(chinfo.Qam_frequency.c_str()) / 1000);
			}
			///end write port info
			sprintf(strTemp, "    </EdgePort>\n");
			fwrite(strTemp, strlen(strTemp), 1, fp);
			memset(strTemp, 0, 2048);

			deviceinfo.portinfos.push_back(portinfo);
		}
		//end of EdgeDevice
		sprintf(strTemp, "  </EdgeDevice>\n</EdgeDevices>");
		fwrite(strTemp, strlen(strTemp), 1, fp);
		memset(strTemp, 0, 2048);
		fclose(fp);
		deviceinfos.push_back(deviceinfo);
		QamCount++;
	}

	std::string strSLtoSG = strOutputPath + "StreamLinkToSG.xml";
	FILE* fpSLtoSG = fopen(strSLtoSG.c_str(), "w");
	if(fpSLtoSG == NULL)
	{
		printf("failed open file %s\n", strSLtoSG.c_str());
		return;
	}

	StreamLinkToServiceGroups::iterator itorSLtoSG = streamLinkToSGs.begin(); 
	while(!strLinkType.empty() && itorSLtoSG != streamLinkToSGs.end())
	{
		ServiceToQAMId& SgToQAMid = itorSLtoSG->second;
		for(ServiceToQAMId::iterator itorSG = SgToQAMid.begin(); itorSG != SgToQAMid.end(); itorSG++)
		{
/*			fprintf(fpSLtoSG, "<Link linkId=\"\" streamerId=\"%s\" servicegroupId=\"%s\" type=\"%s\" ><PrivateData><Data key=\"EdgeRMEndpoint\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data><Data key=\"QAM-IDs\" type=\"String\" range=\"0\" >", 
				itorSLtoSG->first.c_str(), itorSG->first.c_str(), strLink.c_str(), strERMEndpoints.c_str());

			TianShanIce::StrValues& QamIDs = itorSG->second;
			for(TianShanIce::StrValues::iterator itorQamIds = QamIDs.begin(); itorQamIds != QamIDs.end(); itorQamIds++)
			{
				fprintf(fpSLtoSG, "<Item value=\"%s\" />", (*itorQamIds).c_str());
			}
			if(linkType == 1)
			{
				fprintf(fpSLtoSG, "</Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"RoutingMode\" type=\"Int\" range=\"0\" ><Item value=\"1\" /></Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data></PrivateData>\n");
			}
			else if(linkType == 2)
			{
				fprintf(fpSLtoSG, "</Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"RoutingMode\" type=\"Int\" range=\"0\" ><Item value=\"1\" /></Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"SopName\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data></PrivateData>\n", 
					strSopName.c_str());
			}
			else if(linkType ==3)
			{
				fprintf(fpSLtoSG, "</Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"RoutingMode\" type=\"Int\" range=\"0\" ><Item value=\"1\" /></Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"SopName\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data></PrivateData>\n", 
				strSopName.c_str());

			}*/

			fprintf(fpSLtoSG, "<Link linkId=\"\" streamerId=\"%s\" servicegroupId=\"%s\" type=\"%s\" ><PrivateData>", 
				itorSLtoSG->first.c_str(), itorSG->first.c_str(), strLinkType.c_str(), strERMEndpoints.c_str());

			if(linkType == 1 || linkType == 2)
			{
				fprintf(fpSLtoSG, "<Data key=\"CtrlStatus\" type=\"Int\" range=\"0\" ><Item value=\"0\" /></Data><Data key=\"EdgeRMEndpoint\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data><Data key=\"QAM-IDs\" type=\"String\" range=\"0\" >", 
					itorSLtoSG->first.c_str(), itorSG->first.c_str(), strLinkType.c_str(), strERMEndpoints.c_str());

				TianShanIce::StrValues& QamIDs = itorSG->second;
				for(TianShanIce::StrValues::iterator itorQamIds = QamIDs.begin(); itorQamIds != QamIDs.end(); itorQamIds++)
				{
					fprintf(fpSLtoSG, "<Item value=\"%s\" />", (*itorQamIds).c_str());
				}

				fprintf(fpSLtoSG, "</Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"RoutingMode\" type=\"Int\" range=\"0\" ><Item value=\"1\" /></Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data>", 
					strSopName.c_str());

				if(linkType == 2)
					fprintf(fpSLtoSG, "</Data><Data key=\"SopName\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data>\n", 
					strSopName.c_str());
			
			}
			else if(linkType == 3)
			{
               fprintf(fpSLtoSG,"<Data key=\"CtrlStatus\" type=\"Int\" range=\"0\" ><Item value=\"0\" /></Data><Data key=\"DefaultSymbolRate\" type=\"Int\" range=\"0\" ><Item value=\"6875000\" /></Data><Data key=\"IP\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data><Data key=\"Port\" type=\"Int\" range=\"0\" ><Item value=\"%d\" /></Data><Data key=\"Qam.ModulationFormat\" type=\"Int\" range=\"0\" ><Item value=\"8\" /></Data><Data key=\"Route-IDs\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data><Data key=\"SopName\" type=\"String\" range=\"0\" ><Item value=\"%s\" /></Data><Data key=\"TotalBandWidth\" type=\"Long\" range=\"0\" ><Item value=\"0\" /></Data>",
				   s6ServerIp.c_str(), s6ServerPort,itorSG->first.c_str(),  strSopName.c_str());  
			}
			fprintf(fpSLtoSG, "</PrivateData>\n</Link>\n");

		}
		itorSLtoSG++;
	}
	fclose(fpSLtoSG);
	if(addDeviceInfoToDB)
		addDeviceToEdgeRM(strERMEndpoints, deviceinfos);
}
void addDeviceToEdgeRM(std::string endpoints, DeviceInfos deviceinfos)
{
  Ice::CommunicatorPtr _ic  = Ice::initialize();
   TianShanIce::EdgeResource::EdgeRMPrx edgeRmPrx;
   try
   { 
	   edgeRmPrx =  TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_ic->stringToProxy(endpoints.c_str()));; 
   }
   catch (Ice::Exception& ex)
   {
   	  std::cerr << ex.ice_name() << std::endl;
	  return;
   }

   for(DeviceInfos::iterator itorDevice = deviceinfos.begin(); itorDevice != deviceinfos.end(); itorDevice++)
   {
      DeviceInfo& deviceinfo = *itorDevice;
	  TianShanIce::Properties props;
	  try
	  {	 
		  TianShanIce::EdgeResource::EdgeDevices edgeDevices;
		  printf("add device[%s], deviceZone[%s]\n", deviceinfo.QAMname.c_str(), deviceinfo.filename.c_str());
		  edgeDevices = edgeRmPrx->importDevice(deviceinfo.QAMname, deviceinfo.QAMZone, deviceinfo.filename, 0, props);

		  if(edgeDevices.size() > 0)
		  {
			  TianShanIce::EdgeResource::EdgeDevicePrx edgeDevicePrx = edgeDevices[0];
			  for(PortInfos::iterator itorPort = deviceinfo.portinfos.begin(); itorPort != deviceinfo.portinfos.end(); itorPort++)
			  {
				  PortInfo& portinfo = *itorPort;
				  TianShanIce::Variant varfreqs;
				  varfreqs.bRange = false;
				  varfreqs.type = TianShanIce::vtLongs;
				  varfreqs.lints = portinfo.channels;
				  char buf[64] = "";
				  sprintf(buf, "%d", portinfo.serviceGroup);
				  printf("device[%s], link portId[%d] routes[%s]\n", deviceinfo.QAMname.c_str(), portinfo.portId, buf);
				  edgeDevicePrx->linkRoutes(portinfo.portId, buf, varfreqs);
			  }
		  }
	  }
	  catch (Ice::Exception&ex)
	  {
         printf("%s, %s, %s", deviceinfo.QAMname.c_str(), deviceinfo.filename.c_str(), ex.ice_name().c_str());
	  }  
   }
   try{
	   if(_ic)
	     _ic->destroy();
   } 
   catch(const Ice::Exception& e) {
	   std::cerr << e.ice_name() << std::endl;
   }
   catch(const std::exception& e) {
	   std::cerr << e.what() << std::endl;
   }
   catch(...) {
	   std::cerr << "unknown error" << std::endl;
   }
}
void ShowHelp()
{
	printf("run the application as follow:\n");
	printf("  TransportsToQAMDevice  FilePath  OutPutPath ERMendpoint LinkType S6ServerIp S6ServerPort importDevice deviceZone <deviceNamePrefix>\n");
	printf("\tFilePath:    transport file path\n");
	printf("\tOutPutPath:  output file path\n");
	printf("\tERMendpoint: EdgeRM serivce endpoint\n");
	printf("\tLinkType:     1: XOR-media.IpEdge.EdgeRM 2. XOR-media.NSS.EdgeRM 3. XOR-media.NSS.S6ERM \n");
	printf("\tS6ServerIP:   S6 server Ip(ERM service Ip)\n");
	printf("\tS6ServerPort: S6 server port(default 10554) \n");
	printf("\timportDevice:  add device to ERM DB: 1 add , 0 not(default 0)\n");
	printf("\tdeviceZone:   device zone, default(xor.com)\n");
	printf("\tPrefix:		deviceName prefix, if not config, deviceName = deviceIp(default=\"\")\n");
	printf("e.g.\t\n");
	printf("  TransportsToQAMDevice \"d:\\transport.xml\"  \"d:\\QAMTest\\\" \"EdgeRM: tcp -h 192.168.81.134 -p 11400\" 3 \"192.168.81.134\" 10554 0 xor.com \n");
};
int main(int argc, char*argv[])
{
	s6ServerIp = "127.0.0.1";
	s6ServerPort = 10554;
	addDeviceInfoToDB = 0;

	if(argc < 6)
	{
		ShowHelp();
		return 0;
	}
	std::string strTransport = argv[1];
	strOutputPath = argv[2];
	strERMEndpoints = argv[3];
	linkType = atoi(argv[4]);
	s6ServerIp = argv[5];
	if(argc > 6)
		s6ServerPort = atoi(argv[6]);
	if(argc > 7)
		addDeviceInfoToDB = atoi(argv[7]);

	if(argc > 8)
		deviceZone = argv[8];

	if(argc > 9)
		strDevicePrefix = argv[9];
#ifdef ZQ_OS_MSWIN
	if(_access(strTransport.c_str(), 0) != 0)
	{
		printf("Invaild transport file path %s\n", strTransport.c_str());
		return 0;
	}
	if(_access(strOutputPath.c_str(), 0) != 0)
	{
		printf("Invaild output file path %s\n", strOutputPath.c_str());
		return 0;
	}

#else
	if(access(strTransport.c_str(), F_OK) != 0)
	{
		printf("Invaild transport file path %s\n", strTransport.c_str());
		return 0;
	}
	if(access(strOutputPath.c_str(), F_OK) != 0)
	{
		printf("Invaild output file path %s\n", strOutputPath.c_str());
		return 0;
	}
#endif	    
	int nlen = strOutputPath.size();
	if(strOutputPath[nlen -1] != FNSEPC )
	{
		strOutputPath+= FNSEPS;
	}

	if(linkType == 1)
	{
		strLinkType = "XOR-media.IpEdge.EdgeRM";
	}
	else if (linkType == 2 || linkType == 3)
	{
		char temp[128];
		printf("please input soapname:\n");
		scanf("%s", temp);
		strSopName = temp;
		printf("SoapName:    %s\n", strSopName.c_str());
		if(linkType ==2)
			strLinkType = "XOR-media.NSS.EdgeRM";
		else
			strLinkType = "XOR-media.NSS.S6ERM";
	}

	printf("TransPort FilePath:	   %s\n", strTransport.c_str());
	printf("OutPutPath:			   %s\n", strOutputPath.c_str());
	printf("ERMEndpoint:	       %s\n", strERMEndpoints.c_str());
	printf("StreamLink Type:	   %s\n", strLinkType.c_str());
	printf("S6 Service Ip:		   %s\n", s6ServerIp.c_str());
	printf("S6 Service Port:       %d\n", s6ServerPort);
	printf("add Device Info to ERM or not: %s\n", addDeviceInfoToDB ? "true":"false");
	printf("Device Zone:		   %s\n", deviceZone.c_str());
	if(!strDevicePrefix.empty())
		printf("Device Prefix:		   %s\n", strDevicePrefix.c_str());

	//write streamLink type = SeaChange_VSS_NGOD_DVBC
	FILE* fp = NULL;
//	fp = fopen("d:\\StreamLink_SeaChange_VSS_NGOD_DVBC.txt","w");
//	if(fp == NULL)
// 	{
//		printf("failed to create file StreamLink_SeaChange.VSS.NGOD.DVBC.txt");
//		return 0;
//	}
	//write QAM information
	std::string strQAM = strOutputPath + "Qam.txt";
	FILE* fpQam = NULL;
	fpQam = fopen(strQAM.c_str(),"w");
	if(fpQam == NULL)
	{
		printf("failed to create file %s", strQAM.c_str());
		return 0;
	}

	//write QamChanneInfo information
	std::string strQAMChannel = strOutputPath + "QamChanneInfo.txt";
	FILE* fpQamCh= NULL;
	fpQamCh = fopen(strQAMChannel.c_str(),"w");
	if(fpQamCh == NULL)
	{
		printf("failed to create file %s", strQAMChannel.c_str());
		return 0;
	}

	//write QAM information by serviceGroup
	std::string strQAMSG = strOutputPath + "QamSG.txt";
	FILE* fpQamChSG= NULL;
	fpQamChSG = fopen(strQAMSG.c_str(),"w");
	if(fpQamChSG == NULL)
	{
		printf("failed to create file %s", strQAMSG.c_str());
		return 0;
	}

	try
	{	
		ServiceGroups servicegroups;
		ChanneliInfos channelinfos;
		ZQ::common::Config::Loader<StreamLink> pConfig(strTransport.c_str());
		pConfig.load(strTransport.c_str());
        int iStreamLinkCount = 0;
		for(int k = 0; k < pConfig.streamlinks.size(); k++)
		{	
			if(pConfig.streamlinks[k].type != "SeaChange.VSS.NGOD.DVBC")
				continue;
			char strTemp[2048] ="";
			//sprintf(strTemp, "[%d]  linkId=%s, streamerId=%s, servicegroupId=%s ,type=%s\n\0",
			//	k+1, pConfig.streamlinks[k].linkId.c_str(), pConfig.streamlinks[k].streamerId.c_str(),
			//	pConfig.streamlinks[k].servicegroupId.c_str(), pConfig.streamlinks[k].type.c_str());
			//fwrite(strTemp, strlen(strTemp), 1, fp);
			
			ChannelInfo chInfo;
			chInfo.servicegroup = pConfig.streamlinks[k].servicegroupId;
			chInfo.streamerId = pConfig.streamlinks[k].streamerId;

			MAPSET(ServiceGroups, servicegroups, chInfo.servicegroup, chInfo.servicegroup);

			for(int i = 0; i < pConfig.streamlinks[k].privates.linkdatas.size(); i++)
			{
				StreamLinkData& streamdata = pConfig.streamlinks[k].privates.linkdatas[i];
				//printf("key=%s, type=%s, range=%d ,Item=",streamdata.key.c_str(), streamdata.type.c_str(), streamdata.rang);
				ParserDatas(streamdata.key, streamdata.type, streamdata.rang, streamdata.items, chInfo);
				/*for(int j = 0; j < streamdata.items.size(); j++)
				{
					printf(" %s ", streamdata.items[j].c_str());
					;
				}
				printf("\n");*/
			}
			channelinfos.push_back(chInfo);
			iStreamLinkCount++;
		}
		printf("Total SeaChange_VSS_NGOD_DVBC StreamLink: %d\n", iStreamLinkCount);

		//write servicegroup list
		{
			FILE* fpSg= NULL;
			std::string strSglist = strOutputPath + "ServiceGroupList.txt";
			fpSg = fopen(strSglist.c_str(),"w");
			if(fpSg == NULL)
			{
				printf("failed to create file %s", strSglist.c_str());
				return 0;
			}
			else
			{
				ServiceGroups::iterator itorSG = servicegroups.begin();
				while(itorSG != servicegroups.end())
				{
					const std::string&sg = itorSG->first;
					fprintf(fpSg, "%s;", sg.c_str());
					itorSG++;
				}
				fclose(fpSg);
			}
		}
	    std::sort(channelinfos.begin(), channelinfos.end(), mod_less);
		ITEMS qams;
		for(int i = 0; i < channelinfos.size(); i++)
		{
           ChannelInfo& chinfo = channelinfos[i]; 
		   ITEMS::iterator itor = std::find(qams.begin(), qams.end(), chinfo.Qam_IP);
		   if(itor == qams.end())
		   {
			   char strtemp[256];
               sprintf(strtemp, "%s, %s\n\0", chinfo.Qam_IP.c_str(), chinfo.Qam_Mac.c_str());
			   fwrite(strtemp, strlen(strtemp), 1, fpQam);
			   qams.push_back(chinfo.Qam_IP);
		   }
		   char strCh[2048];
		   sprintf(strCh,"StreamId(%s)Ip(%-15s), mac(%s),serviceGroup(%-10s) frequency(%-15s), Bandwidth(%-15s), baseport(%-10s), PN(%-15s), symbolRate(%s), modulationFormat(%s), portMask(%s), portStep(%s)\n\0",
			   chinfo.streamerId.c_str(),
			   chinfo.Qam_IP.c_str(), chinfo.Qam_Mac.c_str(),chinfo.servicegroup.c_str(), chinfo.Qam_frequency.c_str(), 
			   chinfo.TotalBandwidth.c_str(), chinfo.Qam_basePort.c_str(), chinfo.PN.c_str(),
			   chinfo.Qam_symbolRate.c_str(),chinfo.Qam_modulationFormat.c_str(),  
			   chinfo.Qam_portMask.c_str(),chinfo.Qam_portStep.c_str());
		   fwrite(strCh, strlen(strCh), 1, fpQamCh);
		}
		printf("Total QAM %d \n", qams.size());

		std::sort(channelinfos.begin(), channelinfos.end(), mod_less_serviceGroup);
		Devices devices;
		for(int i = 0; i < channelinfos.size(); i++)
		{
            ChannelInfo& chinfo = channelinfos[i];  
			Devices::iterator itorDevice = devices.find(chinfo.Qam_IP);
			if(itorDevice == devices.end())
			{
			   Ports ports;
			   ChanneliInfos chinfos;
			   chinfos.push_back(chinfo);
			   ports[chinfo.servicegroup] = chinfos;
			   devices[chinfo.Qam_IP] = ports;
			}
			else
			{
				Ports::iterator itor = itorDevice->second.find(chinfo.servicegroup);
				if(itor != itorDevice->second.end())
					itor->second.push_back(chinfo);
				else
				{
					ChanneliInfos chinfos;
					chinfos.push_back(chinfo);
					(itorDevice->second).insert(Ports::value_type(chinfo.servicegroup, chinfos));
				}
			}
			
			char strCh[2048];
			sprintf(strCh,"StreamId(%s)Ip(%-15s), mac(%s),serviceGroup(%-10s) frequency(%-15s), Bandwidth(%-15s), baseport(%-10s), PN(%-15s), symbolRate(%s), modulationFormat(%s), portMask(%s), portStep(%s)\n\0",
				chinfo.streamerId.c_str(),
				chinfo.Qam_IP.c_str(), chinfo.Qam_Mac.c_str(),chinfo.servicegroup.c_str(), chinfo.Qam_frequency.c_str(), 
				chinfo.TotalBandwidth.c_str(), chinfo.Qam_basePort.c_str(), chinfo.PN.c_str(),
				chinfo.Qam_symbolRate.c_str(),chinfo.Qam_modulationFormat.c_str(),  
				chinfo.Qam_portMask.c_str(),chinfo.Qam_portStep.c_str());
			fwrite(strCh, strlen(strCh), 1, fpQamChSG);
		}

		GenerateQAMInfo(devices);
		
	}
	catch (...)
	{	
	}
//    fclose(fp);
	fclose(fpQam);
	fclose(fpQamCh);
	fclose(fpQamChSG);
	return 0;
}

