#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include <afx.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include  <io.h>
#define   MAX_KEY_NUM   200  

typedef std::vector<std::string> VECSTR;
typedef std::pair   <std::string, std::string>   spair__;   
typedef std::vector<spair__>   vector_type;
typedef std::map<std::string , vector_type >ELEMENTMDMP;
typedef std::map<std::string, std::string>STRMAP;


extern int AssetCount;
extern int AEcount;
extern std::string strProduct;

extern std::string     Version_Major;
extern std::string     Version_Minor;

extern vector_type ADIAMS;
extern vector_type ADIAMSAPPMD;
extern vector_type ASSETMD;
extern vector_type ASSETCLASS;
extern vector_type ASSETClASSTYPE;
extern ELEMENTMDMP ElementMDmap;
extern ELEMENTMDMP KeyValuemap;
extern STRMAP      SysDefineKey;
bool getKeyConfig();

bool GetSessionValue(std::string SessionName, vector_type& SessionResult, 
					 std::string strFilePath);

void ConvertITVtoADI(std::string FilePath, std::string exportPath);
bool GetKeyValue(vector_type& MetaData, std::string strADIkey,std::string strITVkey, 
				 VECSTR& result, vector_type& ADITVMAP);

void compartStr( const std::string& src, char delimiter, VECSTR& result);

std::string GetCurrentDatatime();

int CreateOutDir(std::string& outdir);

int  ListFile(const char *argv, std::list<std::string> &File);
bool ListDir(const char *lpSrcFile, std::list<std::string> &Directory); 

bool getKeyString(vector_type& MetaData, std::string Key, 
								vector_type& KeyValueMap,VECSTR& strvalue);
void initDefaultKey();
