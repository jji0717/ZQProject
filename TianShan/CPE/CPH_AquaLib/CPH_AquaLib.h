#ifndef _CPH_AQUALIB_H_
#define _CPH_AQUALIB_H_

#include <string>
#include <list>
#include <vector>
#include <TianShanIce.h>

#define CPH_AquaLib			    "CPH_AquaLib"

class NetworkIFSelector;
extern NetworkIFSelector*  _nNetSelector; 
class CdmiFuseOps;
extern CdmiFuseOps* _pCdmiFuseOps;

extern std::string formatSpeed(std::list<float> trickspeeds);
extern std::string formatSpeed(std::vector<float> trickspeeds);
extern bool fixpath(std::string& path, bool bIsLocal);
extern std::string replaceString(const std::string& inStr);
extern int randstring(char* buffer, int bufsize) ;
#ifdef ZQ_OS_LINUX
extern bool mountURL(std::string strsharePath, std::string strsystype, std::string strOpt, std::string& szMountPoint, const std::string& strLogHint);
extern bool umountURL(std::string sharePath, const std::string& strLogHint);
extern int listdir(const char *path, TianShanIce::StrValues& subDirs) ;
#endif
extern void getTrickExt(int speedNo, char* ext1, char* ext2);
extern void getUnifiedTrickExt(int speedNo, char* ext);
	
#endif

