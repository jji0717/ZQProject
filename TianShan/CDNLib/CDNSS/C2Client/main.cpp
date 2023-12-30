#include "C2ClientConf.h"
#include "OutputHandle.h"
#include "LoopRequest.h"
#include <getopt.h>
#include <SystemUtils.h>
#ifdef ZQ_OS_LINUX
#include <unistd.h>
#include <sys/resource.h>
#else
#include <direct.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

void usage()
{
    printf("usage: C2Client -f <filename>\n");
    printf("       -f <config file>\n");
    printf("       -h display this help\n");
}

int main(int argc, char **argv)
{
    const std::string defaultConfigFile = "/opt/TianShan/etc/C2Client.xml";
    const std::string logFileName = "C2Client.log";
    int ch;
    std::string strConfFile;
    while((ch = getopt(argc, argv, "hHf:")) != EOF)
    {
        switch (ch)
        {
        case '?':
        case 'h':
        case 'H':
            usage();
            return 0;

        case 'f':
            strConfFile = optarg;
            break;
        default:
            printf("Error: unknown option %c specified\n\n", ch);
            usage();
            return 0;
        }
    }

    if (strConfFile.empty())
    {
        strConfFile = defaultConfigFile;
        //printf("Error: no config file was specified\n\n", ch);
        //usage();
        //return 0;
    }

    // set rlimit
#ifdef ZQ_OS_LINUX
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = 10240;
    int rc = setrlimit(RLIMIT_NOFILE, &rt);
#endif

    std::string confFolder;
    size_t spos = strConfFile.find_last_of(FNSEPC);
    if (std::string::npos !=  spos)
    {
        confFolder = strConfFile.substr(0, spos);
        strConfFile = strConfFile.substr(spos + 1);
    }else{
        // get current path
        char buf[256];
        getcwd(buf, 256);
        confFolder = buf;
    }

    ZQ::common::Config::Loader<ZQ::StreamService::C2ClientConf> c2conf(strConfFile);
    ZQ::common::Config::ILoader	*configLoader = &c2conf;
    bool loadConfigOK = configLoader->loadInFolder(confFolder.c_str(), true);

    std::string strLogPath;
    if (c2conf._logFileHolder.dir.at(c2conf._logFileHolder.dir.size() - 1) == FNSEPC)
    {
        strLogPath = c2conf._logFileHolder.dir + logFileName;
    }else{
        strLogPath = c2conf._logFileHolder.dir + FNSEPC + logFileName;
    }

    ZQ::common::FileLog g_log(strLogPath.c_str(), ZQ::common::Log::L_DEBUG);
	g_log.setLevel(c2conf._logFileHolder.level);
    g_log.setFileCount(c2conf._logFileHolder.maxCount);
    g_log.setBufferSize(c2conf._logFileHolder.bufferSize);
    g_log.setFileSize(c2conf._logFileHolder.size);

	std::vector<int> cpuS;
	cpuS.push_back(0);
	cpuS.push_back(1);
	cpuS.push_back(2);
	cpuS.push_back(3);
	cpuS.push_back(4);
	cpuS.push_back(5);
	cpuS.push_back(6);
	cpuS.push_back(7);
	cpuS.push_back(8);
	cpuS.push_back(9);
    ZQ::StreamService::RequestHandle::setup(g_log, /*c2conf.eventloop*/cpuS);
    //printf("%d\n%d\n%d\n", c2conf.client, c2conf.loop, c2conf.interval);
    //system("pause");

    int loop = c2conf.loop;
    int client = c2conf.client;
    size_t fileNum = c2conf._filesHolder.files.size();
    size_t index = 0;
    size_t fileIndex = 0;

    ZQ::StreamService::LoopRequest::Ptr loopRequestPtr = new ZQ::StreamService::LoopRequest(c2conf, g_log);
    loopRequestPtr->startRequest();
    ZQ::StreamService::OutputHandle::Ptr outputPtr = new ZQ::StreamService::OutputHandle(g_log, c2conf._statisticHolder.printInterval, loopRequestPtr);
    outputPtr->start();

    while(true)
    {
        if (outputPtr->isQuit())
        {
            outputPtr = NULL;
            loopRequestPtr = NULL;
            SYS::sleep(1);
            exit(0);
        }
        SYS::sleep(1);
    }
    return 0;
}
