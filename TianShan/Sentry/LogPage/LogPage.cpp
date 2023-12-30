// LogPage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include <string>

#include "LogPageOpe.h"

#define PRINFHEAD printf("LogPage:\\cmd>");


int main(int argc, char* argv[])
{	
//	printf("Wlecom logpageopt,please input your command!\n");
	
	LogPageOpe opt;
	
	opt.Analyse(argc,argv);	
	printf("\n");

	return 0;
}
