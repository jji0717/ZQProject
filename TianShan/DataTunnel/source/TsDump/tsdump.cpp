// tsdump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tsdump.h"
#include "tssrc.h"
#include "tsdecoder.h"
#include "TsStatWorker.h"
#include "TsWriterWorker.h"
#include "Sniffer.h"
#include "pcapsrc.h"
#include "zqresource.h"
enum TSSrcType {
	TSST_NONE,
	TSST_FILE, 
	TSST_TCP, 
	TSST_UDP, 
	TSST_MULTICAST, 
	TSST_SNIFFER, 
	TSST_WINPCAP,
};

TSSrcType stream_source_type = TSST_NONE;

enum TSWorkerType {
	TSWT_DUMP,
	TSWT_FILE, 
	TSWT_STAT,
	TSWT_DECODER,
	TSWT_LISTITEMS,
};

TSWorkerType stream_worker_type = TSWT_DUMP;

ULONG stat_times = 3000;

bool dod_special = false;
int	netif_num = 0;
bool all_option = false;
bool bin_hex_option = false;
bool verbose_option = false;


void usage()
{
	printf(
		"tsdump version: %d.%d.%d.%d\n\n"
		"1) tsdump <source> [method] [option]\n"		
		"source:\n"
		"\t-f<filename>\tsource is a file\n"
		"\t-t<ip:port>\tsource is a tcp endpoint\n"
		"\t-u<ip:port>\tsource is a udp endpoint\n"
		"\t-m<ip:port>\tsource is a multicast endpoint\n"
		"\t-c[a/u/t]<ip:port>\tsource is a sniffer filter\n"
		"\t-p[a/u/t]<ip:port>\tsource is the winpcap\n"
		"method:\n"
		"\t-s[times]\tstatistic\n"
		"\t-d\t\tdump to the screen (default)\n"
		"\t-e<cachedir>\tdecode\n"
		"\t-w<filename>\twrite to a file\n"
		"\t-l\t\tlist the item of source\n"
		"option:\n"
		"\t-h\t\tshow help information\n"
		"\t-i<num>\t\tset interface number\n"
		"\t-a\t\tprocess all incoming data\n"
		"\t-b\t\tdump to hex if method is -d\n"
		"\t-v\t\tverbose\n"
		"\t-o\t\tdump special information of DOD if method is -d\n\n"
		"2) tsdump -x <indexfile> <datafile>\n"
		"\t make up the data of DOD\n", 
		ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR,ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD
		);
}

void analysisTsPackage(int index, unsigned char package[TS_PACKAGE_SIZE]);

class TsDumpWorker: public TSWorker {
public:
	virtual int process(ULONG index, char package[TS_PACKAGE_SIZE])
	{
		analysisTsPackage(index, (unsigned char * )package);
		return 1;
	}
};

size_t findLead(char* buf, size_t len)
{
	if (len == -1 || len < TS_PACKAGE_SIZE)
		return -1;

	char* c = buf;
	size_t offset = 0;

	while(offset <= len) {
		if (*c == TS_PACKAGE_LEAD) {
			
			size_t remain = len - offset;
						
			if (remain < TS_PACKAGE_SIZE) {

				return -1;

			} else if (remain == TS_PACKAGE_SIZE) {

				if (remain < len)
					memmove(buf, c, remain);

				return remain;

			} else if (*(c + TS_PACKAGE_SIZE) == TS_PACKAGE_LEAD) {

				if (remain < len)
					memmove(buf, c, remain);
	
				return remain;
			}
		}

		offset ++;
		c ++;
	}

	return -1;
}

void printData(unsigned char* data, int size);

void processTS(TSSource* src, TSWorker* worker)
{
	assert(src);
	assert(worker);

	char buf[TS_PACKAGE_SIZE * 348];
	// char tsPackage[TS_PACKAGE_SIZE];

	int index = 0;
	int pos = 0;
	if (!worker->init())
		return;

	size_t len = src->recv(buf, sizeof(buf));

	if (!all_option) {
		if (len >= TS_PACKAGE_SIZE)
			len = findLead(buf, len);
	}

	while(len != -1) {

		while (len >= TS_PACKAGE_SIZE) {
			// memcpy(tsPackage, &buf[pos], TS_PACKAGE_SIZE);
			// int r = worker->process(index, tsPackage);
			int r = worker->process(index, &buf[pos]);
			
			switch (r) {

			case PROCESS_FINISH:
				printf("*** worker process finished\n");
				goto l_final;

			case PROCESS_OK:
				break;

			case PROCESS_ERROR:

				if (verbose_option) {
					printf("*** invalid TS packet\n");
					printData((u_char* )&buf[pos], TS_PACKAGE_SIZE);
					printf("\n---\n");
				}

				break;

			case PROCESS_FAULT:
				printf("*** fault occoured in worker process\n");
				goto l_final;
				break;
			}
			
			index ++;
			len -= TS_PACKAGE_SIZE;
			pos += TS_PACKAGE_SIZE;
		}

		if (len) {
			memmove(buf, &buf[pos], len);
			pos = len;
		} else
			pos = 0;
		
		size_t size = src->recv(&buf[pos], sizeof(buf) - pos);
		if (size == -1) {
			len = -1;
			goto l_final;
		}

		len += size;

		if (!all_option) {

			size_t trimSize;

			if (len >= TS_PACKAGE_SIZE) {
				trimSize = findLead(buf, len);
				if (trimSize != len) {
					len = -1;
					printf("*** findLead *** trimSize = %d, len = %d\n", 
						trimSize, len);
					printData((u_char* )buf, TS_PACKAGE_SIZE);
					printf("\n---\n");
				}
				/*
				if (trimSize != len)
					pos = 0;
				len = trimSize;

				if (len == -1) {
					len = 0;
					pos = 0;
				}
				*/
			}
		}
	}

l_final:

	if (len == -1) {
		printf("*** source prcoess exited\n");
	}

	worker->final();
}

int makeDODFile(const char* fileName, const void* buf, size_t len)
{
	FILE* fp;
	fp = fopen(fileName, "w");
	if (fp == NULL)
		return 0;
	fwrite(buf, 1, len, fp);
	fclose(fp);

	return 1;
}

int makeDODData(const char* indexFile, const char* dataFile)
{
	ObjectHeader objHdr;
	ObjectDescriptor objDesc;
	// FILE* fpIndex;
	FILE* fpData;
	/*
	fpIndex = fopen(indexFile, "r");
	if (!fpIndex)
		return 0;
	*/

	fpData = fopen(dataFile, "r");
	if (!fpData) {
		// fclose(fpIndex);
		return 0;
	}

	char outFileName[MAX_PATH];
	int count = 1;
	void* buf = NULL;
	size_t bufLen = 0;
	size_t readLen;
	while(1) {
		readLen = fread(&objHdr, 1, sizeof(objHdr), fpData);
		if (readLen < sizeof(objHdr))
			break;		
		DWORD objLen = ntohl(*(DWORD* )objHdr.dwObjectContentLength);
		if (bufLen < objLen) {
			if (buf)
				delete[] buf;
			buf = new char[objLen];
			bufLen = objLen;
		}

		readLen = fread(buf, 1, objLen, fpData);
		if (readLen < objLen)
			break;
		sprintf(outFileName, "%s.%d", dataFile, count);
		makeDODFile(outFileName, buf, objLen);

		// Ìø¹ý crc
		// fseek(fpData, 4, SEEK_CUR);
		if (feof(fpData))
			break;
		count ++;
	}

	if (buf)
		delete[] buf;

	fclose(fpData);
	return 0;
}

void listItems(TSSource* src)
{
	std::vector<std::string> items;
	if (!src->listItems(items)) {
		printf("Can't list items in this source\n");
		return;
	}

	std::vector<std::string>::iterator it;

	printf("Items:\n");

	int i = 0;
	for (it = items.begin(); it != items.end(); it ++) {
		printf("if%d: %s\n", i++, it->c_str());
	}
}

#include <signal.h>
TSSource* ts_source = NULL;
void sig_break(int)
{
	printf("\nUser Break...\n");

	if (ts_source)
		ts_source->close();

	exit(0);
}

int main(int argc, char* argv[])
{
	WSAData wsad;
	WSAStartup(MAKEWORD(2, 0), &wsad);
	TSSource* src = NULL;
	TSWorker* worker = NULL;

	char srcName[MAX_PATH];
	char op[MAX_PATH];

	if (argc < 2) {
		usage();
		return -1;
	}

	for (int i = 1; i < argc; i ++) {
		if (strstr(argv[i], "-h")) {
			usage();
			return 0;
		} else if (strstr(argv[i], "-x")) {
			if (argc != 4) {
				usage();
				return -1;
			}
			
			return makeDODData(argv[1], argv[2]);

		} else if (strstr(argv[i], "-m")) {

			stream_source_type = TSST_MULTICAST;
			strcpy(srcName, &argv[i][2]);
		} else if (strstr(argv[i], "-f")) {

			stream_source_type = TSST_FILE;
			strcpy(srcName, &argv[i][2]);
		} else if (strstr(argv[i], "-u")) {

			stream_source_type = TSST_UDP;
			strcpy(srcName, &argv[i][2]);
		} else if (strstr(argv[i], "-t")) {

			stream_source_type = TSST_TCP;
			strcpy(srcName, &argv[i][2]);
		} else if (strstr(argv[i], "-c")) {

			stream_source_type = TSST_SNIFFER;
			strcpy(srcName, &argv[i][2]);

		} else if (strstr(argv[i], "-p")) {

			stream_source_type = TSST_WINPCAP;
			strcpy(srcName, &argv[i][2]);

		} else if (strstr(argv[i], "-d")) {

			stream_worker_type = TSWT_DUMP;
		} else if (strstr(argv[i], "-s")) {

			stream_worker_type = TSWT_STAT;
			strcpy(op, &argv[i][2]);
			if (strlen(op)) {
				stat_times = atoi(op);
			}
		} else if (strstr(argv[i], "-e")) {

			stream_worker_type = TSWT_DECODER;
			strcpy(op, &argv[i][2]);
			if (strlen(op) <= 0) {
				printf("invalid arguments\n");
				return -1;
			}
		} else if (strstr(argv[i], "-w")) {

			stream_worker_type = TSWT_FILE;
			strcpy(op, &argv[i][2]);
			if (strlen(op) <= 0) {
				printf("invalid arguments\n");
				return -1;
			}

		} else if (strstr(argv[i], "-l")) {

			stream_worker_type = TSWT_LISTITEMS;

		} else if (strstr(argv[i], "-v")) {

			verbose_option = true;

		} else if (strstr(argv[i], "-o")) {

			dod_special = true;
		} else if (strstr(argv[i], "-i")) {
			
			strcpy(op, &argv[i][2]);
			if (strlen(op) <= 0) {
				printf("invalid arguments\n");
				return -1;
			}
			
			netif_num = atoi(op);
		} else if (strstr(argv[i], "-a")) {

			all_option = true;
		} else if (strstr(argv[i], "-b")) {

			bin_hex_option = true;
		}
	}

	switch (stream_source_type) {
	case TSST_MULTICAST:
		src = new MCastTsSrc;
		break;

	case TSST_FILE:
		src = new FileTsSrc;
		break;
		
	case TSST_TCP:
		src = new TcpTsSrc;
		break;

	case TSST_UDP:
		src = new UdpTsSrc;
		break;

	case TSST_SNIFFER:
		src = new Sniffer(netif_num);
		break;

	case TSST_WINPCAP:
		src = new PCapSrc(netif_num);
		break;

	default:
		printf("invalid arguments\n");
		return -1;
	}

	switch (stream_worker_type) {
	case TSWT_DUMP:
		worker = new TsDumpWorker;
		break;

	case TSWT_STAT:
		worker = new TsStatWorker(stat_times);
		break;

	case TSWT_DECODER:
		worker = new TsDecoder(op);
		break;

	case TSWT_FILE:
		worker = new TsWriterWorker(op);
		break;

	case TSWT_LISTITEMS:
		listItems(src);
		return 0;

	default:
		printf("invalid arguments\n");
		return -1;
	}

	if (!src->open(srcName)) {
		printf("open ts source failed.\n");
		return -1;
	}

	ts_source = src;
	signal(SIGINT, sig_break);
	signal(SIGBREAK, sig_break);	

	processTS(src, worker);

	src->close();

	delete src;
	delete worker;
	return 1;
}
