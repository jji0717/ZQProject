// LogTrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "logtrigger.h"
ZQ::common::Log * pGlog;
int main(int argc, char* argv[])
{
	ZQ::SNMP::Triggers trg(REG_ITEM.ConfigFileName());

	if (trg.Ok())
		trg.run();

	return 0;
}

