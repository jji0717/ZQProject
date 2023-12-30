
#include <stdio.h>
#include "globa.h"
#include "getopt.h" 
  
///function ShowHelpInfo
void ShowHelpInfo()
{
	printf("run the application as follow:\n\n");
	printf("ListITVFile  -f [ITV File path] -o [ADI file export path]\n");
	printf("e.g.\tITVtoadi -f \"c:\\ITVfolder\" -o \"d:\\\" \n");
	printf("  or\tITVtoadi -f \"c:\\ITVfolder\\Asset.itv\" -o \"d:\\\" \n");
	printf("options:\n");
	printf("\t-f <n>	ITV File path\n");
	printf("\t-o <o>	ADI file export path. default value=.\\ADIExportDir + [CurrentTime]\n");
	printf("\t-h	display this help\n");
}
int IsDIR(std::string strpath)
{
	DWORD dwAttrs = GetFileAttributes(strpath.c_str()); 
	if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) 
		return 1; 
	else 
		return 0;
}

int main(int argc, char* argv[])
{
	int ch = 1;
	std::string strITVPath="";
	std::string strExportDir ="";
    bool bFile = false;
	printf("-----------------------Convert Asset ITV file to ADI file-------------------\n\n");

	if(argc < 3)
	{  
		ShowHelpInfo();
		return 0;
	}

	while((ch = getopt(argc, argv, "ho:f:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			ShowHelpInfo();
			exit(0);
		case 'f':
			strITVPath = optarg;
			if (_access (optarg, 0 ) != 0  )
			{
				printf("error:ITV File path is not exist!\n\n");
				ShowHelpInfo();
				return 0;
			}
			if(IsDIR(strITVPath.c_str()))
			{
               bFile = true;
			}
			break;
		case 'o':
			strExportDir = optarg;
			if (_access (optarg, 0 ) != 0  )
			{
				printf("error:ADI file export path is not exist!\n\n");
				return 0;
			}
			break;
//		case '?':
		default:
			printf("Error: unknown option specified\n");
			exit(1);
		}
	}
   	if(strExportDir.size() < 1)
	{
		if(!CreateOutDir(strExportDir))
		{
			printf("Create export directory Fail!");
			return 0;
		}
	}
	else
	{		
		if(strExportDir[strExportDir.size() -1] != '\\')
		{
			strExportDir += "\\";
		}
	}
    if(strITVPath.empty())
	{
		printf("please specify ITV file path!\n\n");
		ShowHelpInfo();
		return 0;
	}
	std::list<std::string> File;
	std::list<std::string>::iterator iterFile;
	std::list<std::string> Directory;
	std::list<std::string>::iterator iterDirectory;
	if(bFile)
	{
		Directory.push_front(strITVPath);
		ListDir(strITVPath.c_str(), Directory);
		
		for (iterDirectory = Directory.begin(); iterDirectory != Directory.end(); 
		++iterDirectory)
		{
			if(ListFile( (*iterDirectory).c_str(), File ))
			{
				printf("List ITV File Error!\n");
				return 0;
			}
		}	
	}
	else
	{
		File.push_back(strITVPath);
	}
    if( !getKeyConfig())
	{
		return 0;
	}
	int i = 1;
	for (iterFile = File.begin(); iterFile != File.end(); ++iterFile,++i)
	{
		std::string strTemp = (*iterFile).substr((*iterFile).size() -4, 4);

		CharLowerBuff((char*)strTemp.c_str(), 4);
		if(strTemp != ".itv")
			continue;

		printf("%-3d ITV file path = %s \n", i,(*iterFile).c_str());

        ConvertITVtoADI(*iterFile,strExportDir);
	}
	return 1;
}
