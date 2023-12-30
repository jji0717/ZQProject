#include "../DsmccCRG/DsmccMsg.h"
#include "wtconfig.h"

extern PROPERTIESMAP g_propertyMap;
extern std::string g_setuptime;
CWTCONFIG::CWTCONFIG()
{
#ifdef _KWG_DEBUG	
	cout<<"CWTCONFIG constuction"<<endl;
#endif
}
CWTCONFIG::~CWTCONFIG()
{
#ifdef _KWG_DEBUG	
	cout<<"CWTCONFIG deconstuction"<<endl;
#endif
}

std::string& CWTCONFIG::trim(std::string &s)
{
	if (s.empty()) 
		return s;
	s.erase(0,s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;

}
std::wstring CWTCONFIG::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	std::wstring r(len, L'\0');     
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);     
	return r; 
}
std::string CWTCONFIG::ws2s(const std::wstring& s)
{
	int len;     
	int slength = (int)s.length() + 1;     
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);      
	std::string r(len, '\0');     
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);      
	return r; 
}

std::string& CWTCONFIG::replace_all_distinct(std::string& str,const std::string& old_value,const std::string& new_value)  
{   
	for(std::string::size_type   pos(0);   pos!=std::string::npos;   pos+=new_value.length())   
	{   
		if(   (pos=str.find(old_value,pos))!=std::string::npos   )   
			str.replace(pos,old_value.length(),new_value);   
		else   
			break;   
	}   
	return   str;   
}   

bool CWTCONFIG::readData(const char* ppath,CFILESTATUS& refComs)
{
	FILE* pfd=NULL;
	int i=0,  nRetData;
	struct _stat  bufstate;
	if(NULL == (pfd=fopen(ppath,"rb")))
	{
		perror("open");
		return false;	
	}
	//
	if(_stat(ppath, &bufstate)<0)
	{
		perror("stat");
		return false;
	}
	refComs.filesize=(unsigned long)bufstate.st_size;
	//
	refComs.pmloc = (char*)malloc((size_t)(refComs.filesize + 10));
	nRetData=fread(refComs.pmloc,(size_t)(refComs.filesize),1,pfd);
	if(-1 == fclose(pfd))
	{
		perror("close");
		return false;
	}
	return true ;	
}
bool CWTCONFIG::loadConfigureFile(const char* cpath)
{
	if(""==cpath)
		return false ;
	std::string commandName;
	std::string commandPath;

	size_t  nlen=0;
	size_t  nrtSize;
	std::string  tempBuf,tempBuf1;
	FILE* pfd = fopen(cpath,"r");
	if(NULL==pfd)
	{
		perror("fopen") ;
		return false;
	}
	char *plinebuf=(char*)malloc(MAXLINEBUFFER),*p;
	memset(plinebuf,0x00,MAXLINEBUFFER);
	//bool bsetupF=false,bplayF=false,bstatusF=false,bpauseF=false,bcloseF=false;
	while (p = fgets(plinebuf,MAXLINEBUFFER,pfd)) 
	{
		int k=0;
		if('#'==plinebuf[0])
			continue;
		tempBuf1=plinebuf;
		replace_all_distinct(tempBuf1,"\n","");
		CommnadLines.push_back(tempBuf1);
#ifdef _KWG_DEBUG
		// printf("nlen=%d ;Retrieved line of length %zu :",nlen ,nrtSize);
		// printf("%s\n", linebuf);
#endif	
	}
	if(plinebuf)
		free(plinebuf);
#ifdef _KWG_DEBUG
	std::list<std::string>::iterator iter;
	for(iter = CommnadLines.begin(); iter != CommnadLines.end();iter++)
	{
		cout<<"11..:"<<iter->c_str() <<endl;	
	}
#endif
	return true ;	
}

void CWTCONFIG::parse()
{
	std::list<std::string>::iterator itr ;
	SCOMMANDS commandLine;
	//setup;100;./data/setup.data;udp;192.168.81.132:9527	
	for(itr = CommnadLines.begin(); itr != CommnadLines.end(); ++itr)
	{
		//int pos=(*itr).find(';',0);
		int k=0 ;
		char *delims= "; ";
		char *result = NULL;
		char *pcommandname=NULL;
		result = strtok((char*)itr->c_str(),delims);
		pcommandname=result;
		while( result != NULL ) 
		{		
			if(0 == k)
			{
				commandLine.scommand=result;
				trim(commandLine.scommand);
			}
			if(1 == k) 
			{
				commandLine.stime=result;  
				trim(commandLine.stime);
				if (0 == commandLine.scommand.compare("setup"))
					g_setuptime=result;
			}
			if(2 == k)	
			{
				commandLine.spathfile=result;
				trim(commandLine.spathfile);
				CFILESTATUS cfileStatus;
				readData(result,cfileStatus);
				commandLine.filesize = cfileStatus.filesize;
				commandLine.pmloc = cfileStatus.pmloc;
			}
			if(3 == k)	
			{
				commandLine.sprotocol=result;
				trim(commandLine.sprotocol);
			}
			if(4 == k) 
			{
				commandLine.sipport=result;  
				trim(commandLine.sipport);
			}
			//	printf( "result is \"%s\"\n", result );

			result = strtok( NULL, delims );
			k++;
		} 
		_sendlist.push_back(commandLine);
	}

#ifdef _KWG_DEBUG	
	char *commandArray[]={"setup","play","status","pause","close"};
	int indx=0;
	for(indx=0; indx<5; indx++)
	{
		itor=sendDatalist.find(commandArray[indx]);
		cout<<itor->first<<endl;
		COMMANDLIST::const_iterator g = (*itor).second.begin();
		for(;g!=(*itor).second.end();g++)
		{
			printf("commnad-Name:%s ;time(ms)[%s] ;path[%s] ; protocol[%s] ; ipport[%s] ; sendPData[%8x]; filesize[%u] \n",g->scommand.c_str(),g->stime.c_str(),g->spathfile.c_str(),\
				g->sprotocol.c_str(),g->sipport.c_str(),&(g->pmloc),g->filesize);		
		}
	}

	printf("\n\n");
#endif

}
/*
std::string& CWTCONFIG::get_setuptime()
{
	return _setuptime;
}*/
bool CWTCONFIG::loadProperyFile(const char* ppath)
{
	ifstream infile(".\\data\\propertiesfile");
	std::string strtmp;
	std::map<std::string,std::string> properMap;
	while(getline(infile, strtmp, '\n'))
	{
		if (0 == strtmp.compare(0,1,"#")|| strtmp == "" || strtmp.length()<=1)
			continue;
		std::string strkey,strvalue;
		int npos = strtmp.find("=");
		strkey = strtmp.substr(0, npos);
		strvalue = strtmp.substr(npos+1,strtmp.length());
		g_propertyMap.insert(std::pair<std::string,std::string>(strkey,strvalue));
	}
	return true;
}

