// SOPRestriction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SopInfo.h"

int main(int argc, char* argv[])
{
	SopInfo sop;
	sop.parse(argc,argv);

	return 0;
}

