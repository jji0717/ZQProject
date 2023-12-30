// NLEemulator.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FileParser.h"
#include "NLESimulation.h"


typedef boost::shared_ptr<NLESimulation> NLESimulationPtr;
typedef std::vector< NLESimulationPtr > NLESimulations;
HANDLE hComplete = CreateEvent(NULL, false, false, NULL);
int main(int argc, char** argv)
{
    if(argc != 5)
	{
        printf("usage: NLESimulation <OperationsTrace><TestDirectory><Iteration><Interval(ms)>\n");
		printf("\t nleemulator c:\1.csv  c:\test\  5  5000\n");
		return 0;
	}
	std::string filepath = argv[1];
	std::string testdir = argv[2];
	int nIteration = atoi(argv[3]);
	int nInterval = atoi(argv[4]);

	int len = testdir.size();
	if(testdir[len -1] != '\\' && testdir[len -1] != '/')
		testdir+= "\\";

	printf("Got trace file: %s, test directory: %s, Iteration: %d, start simulation Interval: %d\n",
		filepath.c_str(), testdir.c_str(), nIteration, nInterval);

	std::string logpath;
	char path[MAX_PATH];
	if (::GetModuleFileName(NULL, path, MAX_PATH-2) >0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			logpath = path;
		}
		else
		{
			logpath = path;
		}
	}
	else 
		logpath = ".";

	logpath +=  "\\NLEemulator.log";

    ZQ::common::FileLog filelog(logpath.c_str(), ZQ::common::Log::L_DEBUG);
    printf("Created log file: %s\n", logpath.c_str());

	FileParser fileparser(filepath, &filelog); 
    ZQ::common::NativeThreadPool thrdPool(30);
	NLE::Watchdog watchdog_(filelog, thrdPool); 
	watchdog_.start();

    NLESimulations::iterator itor;
	NLESimulations nlesimulations;
	if(fileparser.parserFile())
	{
        FileInfos fileinfos =  fileparser.getFileItems();  
		for(int i = 0; i < nIteration; i++)
		{
			char filePath[512] = "";
			sprintf(filePath, "%sSimulation%03d\\", testdir.c_str(), i+1);
			::CreateDirectory(filePath, NULL);
			printf("\nCreate Directory: %s\n", filePath);
			NLESimulationPtr  pNLESimulation( new NLESimulation(filelog, fileinfos, filePath));
			char temp[64] = "";
			sprintf(temp, "Simulation%03d\0", i+1);
			if(!pNLESimulation->initNLE(temp, watchdog_))
			{
				return false;
			}
			nlesimulations.push_back(pNLESimulation);
		}
		NLESimulations::iterator itor = nlesimulations.begin();
		for( ;itor != nlesimulations.end(); itor++)
		{
			(*itor)->start();
			Sleep(nInterval);
		}
	} 
	else 
	{
		printf("Failed to parse file %s \n", filepath.c_str());
	}

	 
	while(nlesimulations.size() != 0)
	{
		for( itor = nlesimulations.begin();itor != nlesimulations.end(); itor++)
		{
			if((*itor)->isComplete())
			{
				nlesimulations.erase(itor);
				if(nlesimulations.size() < 1)
					break;
				itor = nlesimulations.begin();
			}
		}
		Sleep(2000);
	}
    filelog.flush();
	return 0;
}

