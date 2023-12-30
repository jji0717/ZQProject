// tslicapp.cpp : Defines the entry point for the console application.
//
#include "SignatureGenerate.h"

void SignatureGenerCmd::getFingerPrint()
{
	_sigData.clear();
	_sigData = _pFingerPrint->getFingerPrint();
}

#ifdef _SHOWLICENSE
std::string SignatureGenerCmd::loadLicense(std::string& data)
{
	return  _pFingerPrint->loadLicense(data);
}
#endif

//write data into file
bool SignatureGenerCmd::writeFile(const std::string& fileName, const std::string& data)
{
	FILE* file = fopen(fileName.c_str(),"wb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}
	int res = fwrite(data.c_str(), sizeof(unsigned char), data.length() * sizeof(unsigned char), file);
	if ( res < data.length())
	{
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}
// read data from file
bool SignatureGenerCmd::readFile(const std::string& fileName, std::string& data)
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
}
//print usage
void usage()
{
	printf("display the help infromation of the command line:\n");
	printf(	"-h                   display this screen\n"
			"-g                   generate the signature\n"
			"-o <filename>        specify the name of output file\n\
		     display the data ou the screen if the param is not given\n"
#ifdef _SHOWLICENSE
			"-v <licfile>         display the license info on screen\n"
#endif

		);
}
int main(int argc, char* argv[])
{	
	if( argc <  ARGC_H || argc > ARGC_G_O )
	{
		usage();
		return -1;
	}
	char ch;
	bool boolGenerate = false;
	std::string outPutFile, vFilePath;
#ifdef _SHOWLICENSE
	while((ch = getopt(argc, argv, "hgo:v:")) != EOF)
#else
	while((ch = getopt(argc, argv, "hgo:")) != EOF)
#endif
	{
		switch (ch)
		{
		case 'h':
			{
				usage();
				return 0;
			}
		case 'g':
			{
				boolGenerate = true; 
				break;
			}
		case 'o':
			{	
				outPutFile.assign(optarg);
				break;
			}
#ifdef _SHOWLICENSE
		case 'v':
			{	
				vFilePath.assign(optarg);
				break;
			}
#endif

		default:
			{
				fprintf(stderr, "unknown option: -%c\n", (char)optopt);
				break;
			}
		}
	}

	ZQ::common::FileLog fingerPrintLog("tslicapp.log",7);
	SignatureGenerCmd sigGener(fingerPrintLog);
	if (boolGenerate)
		sigGener.getFingerPrint();
	else
		printf("no param [-g] ,no signature data will be generated\n");
	if (!outPutFile.empty())
	{
		std::string sigatureData = sigGener.getSigData();
		if(sigatureData.empty())
		{
			printf("the signature data is empty\n");
		}
		else
		{
			if(!SignatureGenerCmd::writeFile(outPutFile, sigatureData))
				printf("failed to write the data to file[%s] \n", outPutFile.c_str());
		}
	}
	
#ifdef _SHOWLICENSE
	if (!vFilePath.empty())
	{
		std::string loadtmp;
		if(SignatureGenerCmd::readFile(vFilePath, loadtmp))
		{
			std::string outStr = sigGener.loadLicense(loadtmp);
			if (outStr.empty())
			{
				printf("failed to load the license \n");
			}
			printf("%s \n", outStr.c_str());
		}
		else
		{
			printf("failed to read file [%s]\n",vFilePath.c_str());
		}
	}
#endif
	return 0;
}

