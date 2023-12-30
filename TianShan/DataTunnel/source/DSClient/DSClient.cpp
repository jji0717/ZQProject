// DSClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Command.h"
#include "handler.h"

static Commander commander;
char cmdlinePrompt[64] = "DSC>";
Ice::CommunicatorPtr iceComm;

void usage();
int processCmd();
int processCmdLine(const char* cmdLine);
int initIce();

int main(int argc, char* argv[])
{
	// WSADATA wsad;
	// WSAStartup(MAKEWORD(2, 0), &wsad);

	if (argc != 1 && argc != 3) {
		usage();
		return -1;
	}
	
	if (initIce() != 0) {
		fprintf(stderr, "Ice::initialize failed\n");
		return -1;
	}
	
	if (initHandlers(commander) != 0) {
		fprintf(stderr, "initialization failed\n");
		return -1;
	}

	processCmdLine("exec init.dsc init");

	if (argc == 3) {

		if (strstr(argv[1], "-f")) {
			char buf[512];
			sprintf(buf, "exec %s", argv[2]);
			return processCmdLine(buf);
		} else if (strstr(argv[1], "-c")) {
			return processCmdLine(argv[2]);
		} else if (strstr(argv[1], "-k")) {
			processCmdLine(argv[2]);
		} else {
			usage();
			return -1;
		}
	}

	processCmd();


	// WSACleanup();

	return 0;
}

int initIce()
{
	Ice::InitializationData initData;
	Ice::PropertiesPtr props = Ice::createProperties();
	props->setProperty("Ice.Override.Timeout", "10000");
	props->setProperty("Ice.Override.ConnectTimeout", "10000");
	initData.properties = props;
	int i = 0;
	iceComm = Ice::initialize(i, NULL, initData);
	if (iceComm == NULL) {
		
		return -1;
	}

	return 0;
}

void usage()
{
	fprintf(stderr, 
		"dsclient [-f <script file>] [-c <command>] [-k <command>]\n");
}

#include <signal.h>
#include <setjmp.h>

jmp_buf mark;

void sign_handler(int sig_no)
{
	
}

int processCmdLine(const char* cmdLine)
{
	int r;
	
	r = CMD_R_FAILED;

	try {
		
			r = commander.handle(cmdLine);
	} catch (...) {
		commander.print("command handler has a exception\n");
		r = CMD_R_FAILED;
	}

	if (r == CMD_R_BAD) {
		commander.print("invalid command line\n");
	}

	if (r == CMD_R_INVLIADARGS) {
		commander.print("invalid arguments.\n");
	}

	if (r == CMD_R_UNKNOWN) {
		commander.print("unknown command\n");
	}

	return r;
}

int processCmd()
{	
	// signal(SIGINT, sign_handler);
	// signal(SIGBREAK, sign_handler);

	char cmdline[1024];
	// setjmp(mark);

	int r;

	while (true) {
		printf(cmdlinePrompt);
		gets(cmdline);
		// cmdline = readline(prompt);
		// add_history(cmdline);

		if (!strlen(cmdline))
			continue;

		r = processCmdLine(cmdline);
		
		if (r == CMD_R_QUIT)
			break;
	}

	return r;
}
