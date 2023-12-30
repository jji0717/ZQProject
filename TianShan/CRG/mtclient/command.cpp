#include "../DsmccCRG/DsmccMsg.h"
#include "command.h"
#include "wtlog.h"
extern char* g_pmv_resourse;
extern PROPERTIESMAP g_propertyMap;
extern ZQ::DSMCC::ProtocolType protocol_type;
C_COMMANDS::C_COMMANDS()
{
	bflag=false;
	b_initsocket2_flag=false;
	b_initsocket1_flag=false;
}

C_COMMANDS::~C_COMMANDS()
{
#ifdef _KWG_DEBUG	
	cout<<"C_COMMANDS deconstuction"<<endl;
#endif
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"C_COMMANDS beginning to de-construction OK"));
}
int C_COMMANDS::gindexCount=0;
char* C_COMMANDS::getg_pmv_resourse()
{
	memset(arrg_pmv_resourse,0x00,sizeof(arrg_pmv_resourse));
	Lock locki(_gcrits);
	sprintf_s(arrg_pmv_resourse,sizeof(arrg_pmv_resourse),"%s%04d",g_pmv_resourse,gindexCount);
	//(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"arrg_pmv_resourse string =[%s]"),arrg_pmv_resourse);
	gindexCount++;
	return arrg_pmv_resourse;
}

bool C_COMMANDS::cSutUp(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	std::string  sipportl = commandEle.sipport;//g->sipport;
	if (false == b_initsocket1_flag)
	{
		tuc.inisocket(sipportl,1,0);
		b_initsocket1_flag=true;
	}
	//TODO replace  transactionId of the Header with new TransactionID.
	char  binarybuf[1024]={0};
// 	std::string data;
// 	readFile(commandEle.spathfile ,data);
	memcpy(binarybuf,commandEle.pmloc, commandEle.filesize); 

	if(commandEle.filesize < 8)
		return false;
	//memcpy(binarybuf+4,(BYTE*)(&loc_TransactionID),4);
	memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);

	//1 static Ptr parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed);
	size_t loc_bytesProcessed=0;
	ZQ::DSMCC::DsmccMsg::Ptr loc_ptr;ZQ::DSMCC::StringMap loc_mapmetadata;
	std::map<std::string, std::string>::iterator itor;
	loc_ptr = ZQ::DSMCC::DsmccMsg::parseMessage((uint8*)binarybuf,commandEle.filesize,loc_bytesProcessed, protocol_type);
	//2 virtual uint32 toMetaData(StringMap& metadata);
	loc_ptr->toMetaData(loc_mapmetadata);

	proitor = g_propertyMap.find("analogcopypurchase");
	if (g_propertyMap.end() != proitor || "" != proitor->second )
	{
		if (1 == proitor->second.compare("1"))
		{
			itor= loc_mapmetadata.find(CRMetaData_analogCopyPurchase);
			if (loc_mapmetadata.end() != itor)
			{
				itor->second = "255"; 
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_analogCopyPurchase[%s]"),itor->second.c_str());
			}
			else//TODO
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_analogCopyPurchase didn't find!!"));
		}

	}
	// 替换 client 相当于机顶盒的ID
	proitor = g_propertyMap.find("cssrclientid");
	if (g_propertyMap.end() != proitor || "" != proitor->second)
	{
		itor = loc_mapmetadata.find(CRMetaData_CSSRclientId);
		if (loc_mapmetadata.end() != itor)
		{
			char cssrclientID[41]={0};
			memcpy(cssrclientID,itor->second.c_str(),40);
			memcpy(cssrclientID,proitor->second.c_str(),19);
			itor->second = cssrclientID;
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_CSSRclientId[%s] replace with [%s]"),itor->second.c_str(),proitor->second.c_str() );
		}
		else
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_CSSRclientId didn't find!!"));
	}

	proitor = g_propertyMap.find("nodegroup");
	if (g_propertyMap.end() != proitor)
	{
		itor = loc_mapmetadata.find(CRMetaData_nodeGroupId);
		if (loc_mapmetadata.end() != itor)
		{
			itor->second = proitor->second.empty()== 1 ? "1308" :proitor->second;
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_nodeGroupId[%s]"),itor->second.c_str());
		}
		else
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_nodeGroupId didn't find!!"));
	}
	else
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"nodegroup field didn't find in propertiesfile"));


	itor= loc_mapmetadata.find(CRMetaData_assetId);
	if (loc_mapmetadata.end() != itor)
	{
		if (cgi.get_suffix())
			itor->second=getg_pmv_resourse();
		else
			itor->second = g_pmv_resourse; 

		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"g_pmv_resourse[%s],sleeping[%s]"),itor->second.c_str(),commandEle.stime.c_str());
	}
	else//TODO
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_assetId didn't find!!"));

	//3 virtual bool readMetaData(const StringMap& metadata);
	loc_ptr->readMetaData(loc_mapmetadata);

	//4 int toMessage(uint8* buf, size_t maxLen);
	uint8 binary8buf[2048]={0};size_t binary8bufLen=2048,retbinary8Len;
	retbinary8Len =loc_ptr->toMessage(binary8buf,binary8bufLen);
	//waiting 
	Sleep(atol(commandEle.stime.c_str()));
	
	// test for message add by ketao.zhang
	size_t byteSize = 0;
	ZQ::DSMCC::DsmccMsg::parseMessage((uint8*)binary8buf,binary8bufLen,byteSize, protocol_type);

	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"setup  count  sessionID:[%s] ;count mtclient.setup "),cgi._strdecimalserialn.c_str());
	if(tuc.send_dataSyn(cgi._strdecimalserialn,sipportl,(char*)binary8buf,retbinary8Len,"setup",1,0))
	{
		bflag=true;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"bflag=true success!"));
	}
	else
	{
		bflag=false;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"setup failed by sessionID:[%s] ,and thread will exit!"),cgi._strdecimalserialn.c_str());
	}
	return  bflag;
}
bool C_COMMANDS::cSutUp(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	std::string  sipportl = commandEle.sipport;//g->sipport;

	tuc.inisocket(sipportl,1,0);
	b_initsocket1_flag=true;
	//TODO replace  transactionId of the Header with new TransactionID.
	char  binarybuf[1024]={0};
// 	std::string data;
// 	readFile(commandEle.spathfile ,data);
	memcpy(binarybuf,commandEle.pmloc,commandEle.filesize); 

	if(commandEle.filesize < 8)
		return false;
	//memcpy(binarybuf+4,(BYTE*)(&loc_TransactionID),4);
	memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);

	//1 static Ptr parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed);
	size_t loc_bytesProcessed=0;
	ZQ::DSMCC::DsmccMsg::Ptr loc_ptr;ZQ::DSMCC::StringMap loc_mapmetadata;
	std::map<std::string, std::string>::iterator itor;
	loc_ptr = ZQ::DSMCC::DsmccMsg::parseMessage((uint8*)binarybuf,commandEle.filesize,loc_bytesProcessed, protocol_type);
	//2 virtual uint32 toMetaData(StringMap& metadata);
	loc_ptr->toMetaData(loc_mapmetadata);
	proitor = g_propertyMap.find("analogcopypurchase");
	if (g_propertyMap.end() != proitor || "" != proitor->second )
	{
		if (1 == proitor->second.compare("1"))
		{
			itor= loc_mapmetadata.find(CRMetaData_analogCopyPurchase);
			if (loc_mapmetadata.end() != itor)
			{
				itor->second = "255"; 
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_analogCopyPurchase[%s]"),itor->second.c_str());
			}
			else//TODO
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_analogCopyPurchase didn't find!!"));
		}

	}
	// 替换 client 相当于机顶盒的ID
	proitor = g_propertyMap.find("cssrclientid");
	if (g_propertyMap.end() != proitor || "" != proitor->second)
	{
		itor = loc_mapmetadata.find(CRMetaData_CSSRclientId);
		if (loc_mapmetadata.end() != itor)
		{
			char cssrclientID[41]={0};
			memcpy(cssrclientID,itor->second.c_str(),40);
			memcpy(cssrclientID,proitor->second.c_str(),19);
			itor->second = cssrclientID;
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_CSSRclientId[%s] replace with [%s]"),itor->second.c_str(),proitor->second.c_str() );
		}
		else
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_CSSRclientId didn't find!!"));
	}
	proitor = g_propertyMap.find("nodegroup");
	if (g_propertyMap.end() != proitor)
	{
		itor = loc_mapmetadata.find(CRMetaData_nodeGroupId);
		if (loc_mapmetadata.end() != itor)
		{
			itor->second = proitor->second.empty()== 1 ? "1308" :proitor->second;
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_nodeGroupId[%s]"),itor->second.c_str());
		}
		else
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_nodeGroupId didn't find!!"));
	}
	else
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"nodegroup field didn't find in propertiesfile"));

	itor= loc_mapmetadata.find(CRMetaData_assetId);
	if (loc_mapmetadata.end() != itor)
	{
		if (cgi.get_suffix())
			itor->second=getg_pmv_resourse();
		else
			itor->second = g_pmv_resourse; 

		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"g_pmv_resourse[%s],sleeping[%s]"),itor->second.c_str(),commandEle.stime.c_str());
	}
	else//TODO
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"CRMetaData_assetId didn't find!!"));
	//3 virtual bool readMetaData(const StringMap& metadata);
	loc_ptr->readMetaData(loc_mapmetadata);

	//4 int toMessage(uint8* buf, size_t maxLen);
	uint8 binary8buf[2048]={0};size_t binary8bufLen=2048,retbinary8Len;
	retbinary8Len =loc_ptr->toMessage(binary8buf,binary8bufLen);
	//waiting 
	Sleep(atol(commandEle.stime.c_str()));

	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"setup  count  sessionID:[%s] ;count mtclient.setup "),cgi._strdecimalserialn.c_str());
	if(tuc.send_dataSyn(cgi._strdecimalserialn,sipportl,(char*)binary8buf,retbinary8Len,"setup",1,0))
	{
		bflag=true;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"bflag=true success!"));
	}
	else
	{
		bflag=false;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"setup failed by sessionID:[%s] ,and thread will exit!"),cgi._strdecimalserialn.c_str());
	}
	return  bflag;
}
//
/*bool C_COMMANDS::readFile(const std::string& fileName, std::string& data)
{
	data.clear();
	FILE* file = fopen(fileName.c_str(),"rb");
	if (file == NULL)
	{
		return false;
	}
	unsigned char* buffer = new unsigned char[512];
	memset(buffer,0,512);
	int res = 0;
	while((res = fread(buffer, sizeof(unsigned char), 512, file)) != 0)
	{
		data.append((char *)buffer, res);
	}
	fclose(file);
	delete[] buffer;
	return true;
}*/
bool C_COMMANDS::cPlay(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle) //lscp->9528
{
	if(bflag)//TODO PLAY
	{
		//printf("commnad-Name:%s ;time(ms)[%s] ;path[%s] ; protocol[%s] ; ipport[%s] ; sendPData[%8x]; filesize[%u] \n",g->scommand.c_str(),g->stime.c_str(),g->spathfile.c_str(),\
		//	g->sprotocol.c_str(),g->sipport.c_str(),&(g->pmloc),g->filesize);	
		std::string  sipportl = commandEle.sipport;//g->sipport;
		if (false == b_initsocket2_flag)
		{
			tuc.inisocket(sipportl,0,1);
			b_initsocket2_flag = true;
		}
		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
// 		std::string data;
// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize); //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//waiting 
		Sleep(atol(commandEle.stime.c_str())); //Sleep(atol(g->stime.c_str()));
		std::string _strstreamHandle;
		CWTGUUID::HexToString(_strstreamHandle,(BYTE*)(&tuc.original_StreamHandle),sizeof(tuc.original_StreamHandle));

		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"play  count  sessionID:[%s] streamhandle:[%s]"),cgi._strdecimalserialn.c_str(),_strstreamHandle.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"play");

	}
	else 
		return false;

	return true;	
}
//
bool C_COMMANDS::cPlay(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle) //lscp->9528
{
	if(bflag)//TODO PLAY
	{
		std::string  sipportl = commandEle.sipport;//g->sipport;

		tuc.inisocket(sipportl,0,1);
		b_initsocket2_flag = true;

		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
// 		std::string data;
// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//waiting 
		Sleep(atol(commandEle.stime.c_str())); //Sleep(atol(g->stime.c_str()));
		std::string _strstreamHandle;
		CWTGUUID::HexToString(_strstreamHandle,(BYTE*)(&tuc.original_StreamHandle),sizeof(tuc.original_StreamHandle));

		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"play  count  sessionID:[%s] streamhandle:[%s]"),cgi._strdecimalserialn.c_str(),_strstreamHandle.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"play");

	}
	else 
		return false;

	return true;	
}
//
bool C_COMMANDS::cstatus(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	if(bflag)
	{
		//printf("commnad-Name:%s ;time(ms)[%s] ;path[%s] ; protocol[%s] ; ipport[%s] ; sendPData[%8x]; filesize[%u] \n",g->scommand.c_str(),g->stime.c_str(),g->spathfile.c_str(),\
		//	g->sprotocol.c_str(),g->sipport.c_str(),&(g->pmloc),g->filesize);	
		std::string  sipportl =commandEle.sipport; //g->sipport;
		if (false == b_initsocket2_flag)
		{
			tuc.inisocket(sipportl,0,1);
			b_initsocket2_flag = true;
		}

		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
// 		std::string data;
// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);

		//waiting
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"status  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"status");

	}
	else 
		return false;

	return true;	
}

//
bool C_COMMANDS::cstatus(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	if(bflag)
	{
		std::string  sipportl =commandEle.sipport; //g->sipport;

		tuc.inisocket(sipportl,0,1);
		b_initsocket2_flag = true;


		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
		// 		std::string data;
		// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);

		//waiting
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"status  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"status");

	}
	else 
		return false;

	return true;	
}

//
bool C_COMMANDS::cpause(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	if(bflag)
	{
		//printf("commnad-Name:%s ;time(ms)[%s] ;path[%s] ; protocol[%s] ; ipport[%s] ; sendPData[%8x]; filesize[%u] \n",g->scommand.c_str(),g->stime.c_str(),g->spathfile.c_str(),\
		//	g->sprotocol.c_str(),g->sipport.c_str(),&(g->pmloc),g->filesize);	
		std::string  sipportl = commandEle.sipport;//g->sipport;
		if (false == b_initsocket2_flag)
		{
			tuc.inisocket(sipportl,0,1);
			b_initsocket2_flag = true;
		}

		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
		// 		std::string data;
		// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);
		//waiting 
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"pause  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"pause");

	}
	else 
		return false;

	return true;	
}	
//
bool C_COMMANDS::cpause(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)
{
	if(bflag)
	{
		std::string  sipportl = commandEle.sipport;//g->sipport;

		tuc.inisocket(sipportl,0,1);
		b_initsocket2_flag = true;


		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
		// 		std::string data;
		// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		//memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);
		//waiting 
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"pause  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataAsyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"pause");

	}
	else 
		return false;

	return true;	
}	

//
bool C_COMMANDS::cClose(SocketClient& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)//dsmcc->9527
{
	if(bflag)
	{
		//printf("commnad-Name:%s ;time(ms)[%s] ;path[%s] ; protocol[%s] ; ipport[%s] ; sendPData[%8x]; filesize[%u] \n",g->scommand.c_str(),g->stime.c_str(),g->spathfile.c_str(),\
		//	g->sprotocol.c_str(),g->sipport.c_str(),&(g->pmloc),g->filesize);	
		std::string  sipportl = commandEle.sipport; //g->sipport;
		if (false == b_initsocket1_flag)
		{
			tuc.inisocket(sipportl,1,0);
			b_initsocket1_flag=true;
		}
		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
		// 		std::string data;
		// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		//memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);
		//waiting 
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"close  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataSyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"close",0,1);
	}
	else 
		return false;

	return true;	
}

//
bool C_COMMANDS::cClose(SocketClientU& tuc,CWTGUUID& cgi,SCOMMANDS& commandEle)//dsmcc->9527
{
	if(bflag)
	{	
		std::string  sipportl = commandEle.sipport; //g->sipport;
		tuc.inisocket(sipportl,1,0);
		b_initsocket1_flag=true;

		//replace pmloc point to data with  tuc._StreamHandle
		char binarybuf[1024]={0};
		// 		std::string data;
		// 		readFile(commandEle.spathfile ,data);
		memcpy(binarybuf,commandEle.pmloc,commandEle.filesize);  //memcpy(binarybuf,(*g).pmloc,(*g).filesize);
		//memcpy(binarybuf+4,(BYTE*)(tuc.original_StreamHandle),4);
		memcpy(binarybuf+12,(BYTE*)(cgi._decimalserialn),10);
		//waiting 
		Sleep(atol(commandEle.stime.c_str()));
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(C_COMMANDS,"close  count  sessionID:[%s]"),cgi._strdecimalserialn.c_str());
		tuc.send_dataSyn(cgi._strdecimalserialn,sipportl,binarybuf,commandEle.filesize,"close",0,1);
	}
	else 
		return false;

	return true;	
}
