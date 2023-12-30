#ifndef  __WTCONFIG__DEFINE__H_
#define  __WTCONFIG__DEFINE__H_
#include <stdio.h>
#include <utility>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <errno.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <list>
#include <map>
#include <iomanip>
#include <fstream>

using namespace std;
#define  MAXLINEBUFFER 1024
typedef struct _tag_Command
{
	std::string	scommand;
	std::string stime;
	std::string spathfile;
	std::string sprotocol;
	std::string sipport;
	char* pmloc;
	unsigned long  filesize;
}SCOMMANDS,*PSCOMMAND;
typedef struct _tag_CFILESTATUS
{
	std::string	scommand;
	char* pmloc;
	unsigned long  filesize;
}CFILESTATUS,*PCFILESTATUS;
typedef std::list<SCOMMANDS> COMMANDLIST;
typedef std::map<std::string,COMMANDLIST> SENDDATALIST;
typedef std::map<std::string,std::string> PROPERTIESMAP;


class CWTCONFIG
{
public:
	SENDDATALIST sendDatalist;
	COMMANDLIST  _sendlist;
public:
	CWTCONFIG();
	~CWTCONFIG();
	static std::wstring s2ws(const std::string& s);
	static std::string ws2s(const std::wstring& s);
	static std::string& trim(std::string &s);
	static std::string& replace_all_distinct(std::string& str,const std::string& old_value,const std::string& new_value);   
	bool loadConfigureFile(const char* cpath);
	bool loadProperyFile(const char* ppath);
	bool  readData(const char* ppath ,CFILESTATUS& refComs);
	void  parse();
	
protected:
	
private:
	std::list<std::string> CommnadLines;	
	std::map<string,COMMANDLIST>::iterator itor;
	//CFILESTATUS _cfileStatus;//_csetups,  _cplays,_cstatuss,_cpauses,_ccloses;
	PROPERTIESMAP properMap;
	
};

#endif //__WTCONFIG__DEFINE__H_