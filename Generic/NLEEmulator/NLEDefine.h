#ifndef _NLEEMULATOR_NLEDEFINE_FILE__
#define _NLEEMULATOR_NLEDEFINE_FILE__
#include <string>
#include <vector>
#include <list>


/// file opreate
enum FileOperate{
	opCreate = 0, opRead, opWrite, opClose };

struct OPITEM
{
	FileOperate		opID;		
	__int64	        stampMs;
	__int64	        offset;
	__int64         opLen;
	std::string     filepath;
};

typedef std::vector<OPITEM>OPITEMS;
typedef struct  
{
	std::string fid;
	OPITEMS     opitems;
}FileInfo;
typedef std::vector<FileInfo> FileInfos;

#endif //  _NLEEMULATOR_NLEDEFINE_FILE__