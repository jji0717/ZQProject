#include <iostream>
#include "FileLog.h"
#include "IndexFileParser.h"
using namespace std;
using namespace ZQ::IdxParser;

int main(int argc, char* argv[])
{
    ZQ::common::FileLog servlog("./log/server.log",7);
    servlog(ZQ::common::Log::L_INFO,CLOGFMT(main,"start"));
    IndexRecord record;
    IdxParserEnv env;
    env.AttchLogger(&servlog); 
    IndexFileParser IdxParser(env);
    if (argc == 2 && (  (strcmp(argv[1],"-h")==0) || (strcmp(argv[1],"-help")==0) ) )
    {
        printf("param 1: index file path, param 2: index file name\n");
        exit(1);
    }
    if (argc ==3){
    IdxParser.ParseIndexRecordFromCommonFS(argv[1],record,argv[2]); 
    }else{
        printf("para error!\n");
        exit(1);
    }
    
    return 0;
}
