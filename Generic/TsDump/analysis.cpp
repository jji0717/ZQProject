#include "stdafx.h"
#include "tsdump.h"

extern bool dod_special;
extern bool bin_hex_option;

void printChar(unsigned char c)
{
	if (isgraph(c))
		putchar(c);
	else
		putchar('.');
}

void printLine(unsigned char* data, int pos, int size)
{
	int i;

	printf("(%03d-%03d) ", pos, pos + size - 1);

	for (i = 0; i < 16; i ++) {
		if (i < size)
			printf("%0.2x ", data[i]);
		else
			printf("   ");
	}

	for (i = 0; i < 16; i ++) {
		if (i < size)
			printChar(data[i]);
		else
			printf(" ");
	}

	printf("\n");
}

void printData(unsigned char* data, int size)
{
	if (size <= 0)
		return;

	int lineSize;

	for (int i = 0; i < size; i +=16) {
		if ((i + 16) > size) 
			lineSize = size - i;
		else
			lineSize = 16;
		printLine(&data[i], i, lineSize);
	}
}

size_t printPrivateSectionInfo(unsigned char* package, size_t size)
{
	PrivateSectionHeader* hdr = (PrivateSectionHeader* )package;
	printf("table_id: %d(0x%x)\n", hdr->table_id, hdr->table_id);
	WORD section_len = MAKEWORD(hdr->section_length_lowbits, 
		hdr->section_length_highbits);
	printf("section_length: %d(0x%x)\n", section_len, section_len);
	printf("section_syntax_indicator: %d\n", 
		hdr->section_syntax_indicator);
	WORD table_id_ext = MAKEWORD(hdr->table_id_extension_lo, 
		hdr->table_id_extension_hi);
	printf("table_id_extension: %d(0x%04x)\n", table_id_ext, 
		table_id_ext);
	printf("current_next_indicator: %d\n", hdr->current_next_indicator);
	printf("version_number: %d\n", hdr->version_number);
	printf("section_number: %d\n", hdr->section_number);
	printf("last_section_number: %d\n", hdr->last_section_number);

	return sizeof(PrivateSectionHeader);
}

size_t printPatSectionInfo(unsigned char* package, size_t size)
{
	PatSectionHeader* hdr = (PatSectionHeader* )package;
	printf("table_id: %d(0x%x) PAT\n", hdr->table_id, hdr->table_id);
	WORD section_len = MAKEWORD(hdr->section_length_lowbits, 
		hdr->section_length_highbits);
	printf("section_length: %d(0x%x)\n", section_len, section_len);
	printf("section_syntax_indicator: %d\n", 
		hdr->section_syntax_indicator);
	WORD transport_stream_id = MAKEWORD(hdr->transport_stream_id_lo, 
		hdr->transport_stream_id_hi);
	printf("transport_stream_id: %d(0x%x)\n", transport_stream_id, 
		transport_stream_id);
	printf("current_next_indicator: %d\n", hdr->current_next_indicator);
	printf("version_number: %d\n", hdr->version_number);
	printf("section_number: %d\n", hdr->section_number);
	printf("last_section_number: %d\n", hdr->last_section_number);

	// section 从第三个字节开始, 长度包括 crc, 
	// + 3 /* 开始字节 */ - 4 /* crc */;
	int map_len = section_len - sizeof(PatSectionHeader) + 3 - 4;
	if (map_len > size)
		map_len = size - sizeof(PatSectionHeader);

	if (map_len >= (int )sizeof(PatSectionProgramMap)) {
		int prog_count = map_len / sizeof(PatSectionProgramMap);
		if (prog_count <= 0)
			map_len = 0;
		else {
			PatSectionProgramMap* prog_map = (PatSectionProgramMap* )
				&package[sizeof(PatSectionHeader)];

			for (int i = 0; i < prog_count; i ++) {
				WORD prog_num = MAKEWORD(prog_map[i].program_number_lo, 
					prog_map[i].program_number_hi);
				WORD prog_pid = MAKEWORD(prog_map[i].program_map_PID_lo, 
					prog_map[i].program_map_PID_highbits);
				printf("\tprogram_number: %d(0x%x) - ", 
					prog_num, prog_num);
				printf("program_map_pid: %d(0x%x)\n", 
					prog_pid, prog_pid);
			}
		}
	}

	DWORD crc = 0;
	if (map_len <= size - sizeof(PatSectionHeader) - 4) {
		DWORD crc = ntohl(*(DWORD* )&package[sizeof(PatSectionHeader) + map_len]);
		printf("crc: %d(0x%x)\n", crc, crc);
	}

	return sizeof(PatSectionHeader) + map_len + sizeof(crc);
}

char* stream_type_name(unsigned char type)
{
	static char* strm_type_name[] = {
		"mpeg1 vedio", "mpeg2 vedio", "mpeg1 audio", "mpeg2 audio",
	};

	if (type >= 1 && type <= 4) {
		return strm_type_name[type - 1];
	}

	return "unknown";
}

size_t printPmtSectionInfo(unsigned char* package, size_t size)
{
	PmtSectionHeader* hdr = (PmtSectionHeader* )package;
	printf("table_id: %d(0x%x) PMT\n", hdr->table_id, hdr->table_id);
	WORD section_len = MAKEWORD(hdr->section_length_lowbits, 
		hdr->section_length_highbits);
	printf("section_length: %d(0x%x)\n", section_len, section_len);
	printf("section_syntax_indicator: %d\n", 
		hdr->section_syntax_indicator);
	WORD prog_num = MAKEWORD(hdr->program_number_lo, 
		hdr->program_number_hi);
	printf("program_number: %d(0x%x)\n", prog_num, prog_num);
	printf("current_next_indicator: %d\n", hdr->current_next_indicator);
	printf("version_number: %d\n", hdr->version_number);
	printf("section_number: %d\n", hdr->section_number);
	printf("last_section_number: %d\n", hdr->last_section_number);
	WORD pcr_pid = MAKEWORD(hdr->PCR_PID_lo, hdr->PCR_PID_highbits);
	printf("PCR_PID: %d(0x%x)\n", pcr_pid, pcr_pid);
	WORD prog_info_len = MAKEWORD(hdr->program_info_length_lo, 
		hdr->program_info_length_highbits);
	printf("program_info_length: %d(0x%x)\n", prog_info_len, 
		prog_info_len);
	
	// section 从第三个字节开始, 长度包括 crc, 
	// + 3 /* 开始字节 */ - 4 /* crc */;
	int map_len = section_len - sizeof(PmtSectionHeader) + 3 - 4;
	if (map_len > size)
		map_len = size - sizeof(PmtSectionHeader);

	int remain_len = map_len;
	if (remain_len >= 0) {
		PmtSectionStreamMap* strm_map = (PmtSectionStreamMap* )
			&package[sizeof(PmtSectionHeader) + prog_info_len];

		while (remain_len >= (int )sizeof(PmtSectionStreamMap)) {
			printf("\tstream_type: %d(%s) - ", strm_map->stream_type, 
				stream_type_name(strm_map->stream_type));
			WORD elem_pid = MAKEWORD(strm_map->elementary_PID_lo, 
				strm_map->elementary_PID_highbits);
			printf("elementary_pid: %d(0x%x) - ", elem_pid, 
				elem_pid);
			WORD es_info_len = MAKEWORD(strm_map->ES_info_length_lo, 
				strm_map->ES_info_length_hibits);
			printf("es_info_length: %d(0x%x) - ", es_info_len, 
				es_info_len);
			if (es_info_len > 0) {
				printf("es_info: ");
				printData((unsigned char* )(strm_map + 1), es_info_len);
			}
			else 
				putchar('\n');
			
			strm_map = (PmtSectionStreamMap* )((unsigned char* )strm_map + 
				sizeof(PmtSectionStreamMap) + es_info_len);
			remain_len -= (sizeof(PmtSectionStreamMap) + es_info_len);
		}
	}

	DWORD crc = 0;
	if (map_len <= size - sizeof(PmtSectionHeader) - 4) {
		crc = ntohl(*(DWORD* )&package[sizeof(PmtSectionHeader) + 
			prog_info_len + map_len]);
		printf("crc: %d(0x%x)\n", crc, crc);
	}
	
	return sizeof(PmtSectionHeader) + prog_info_len + 
		map_len + sizeof(crc);
}

size_t printSectionInfo(unsigned char* package, size_t size)
{
	PrivateSectionHeader* hdr = (PrivateSectionHeader* )package;
	printf("\t*** SECTION ***\n");
	switch (hdr->table_id) {
	case 0:
		return printPatSectionInfo(package, size);
	case 2:
		return printPmtSectionInfo(package, size);
	default:
		return printPrivateSectionInfo(package, size);
	}
}

size_t printDODSpecial(unsigned char* package, size_t size, 
					   WORD table_id, WORD section_len)
{
	size_t spec_len = 0;

	if (table_id == 0x10 || table_id == 0x80) {
		printf("\t*** OBJECT ***\n");
		ObjectHeader* objHdr = (ObjectHeader* )package;
		printf("sObjectTag: 0x%x(%s)\n", 
			*(DWORD* )objHdr->sObjectTag, objHdr->sObjectTag);
		printf("nObjectKeyLength: %d(0x%x)\n", 
			objHdr->nObjectKeyLength, objHdr->nObjectKeyLength);

		printf("sObjectKey: 0x%x(0x%02hx,0x%02hx,0x%02hx,0x%02hx)\n",
			ntohl(*(DWORD* )objHdr->sObjectKey), objHdr->sObjectKey[0], 
			objHdr->sObjectKey[1], objHdr->sObjectKey[2], 
			objHdr->sObjectKey[3]);

		long content_len = 
			ntohl(*(DWORD* )objHdr->dwObjectContentLength);
		printf("dwObjectContentLength: %d(0x%x)\n", content_len, 
			content_len);
		spec_len = sizeof(ObjectHeader);
	}

	if (table_id == 0x80) {
		ObjectDescriptor* objDec = (ObjectDescriptor* )&package[spec_len];

		if (section_len > size)
			section_len = size;

		int desc_count = (section_len - sizeof(PrivateSectionHeader)) / 
			sizeof(ObjectDescriptor);

		if (desc_count > 0) {
			for (int i = 0; i < desc_count; i ++) {
				printf("\ttag: %x, length: %x, table_count: %x, key: %x, desc: %s\n", 
					objDec[i].byTag, objDec[i].byLength, objDec[i].byTableCount, 
					ntohl(*(DWORD* )objDec[i].pchID), objDec[i].pchDesc);
			}
		}

		DWORD crc = ntohl(*(DWORD* )&objDec[desc_count]);
		printf("crc: %d(0x%x)\n", crc, crc);
		spec_len += desc_count * sizeof(ObjectDescriptor) + sizeof(crc);
	}

	return spec_len;
}

void analysisTsPackage(int index, unsigned char package[TS_PACKAGE_SIZE])
{
	TsHeader* hdr = (TsHeader* )package;
	size_t hdrlen = sizeof(TsHeader);
	WORD pid;

	SYSTEMTIME t;
	GetLocalTime(&t);
	char outbuf[256];

	wsprintf(outbuf,"+++ %02d-%02d-%02d %02d:%02d:%02d:%03d +++\n",
		t.wYear,
		t.wMonth,
		t.wDay,
		t.wHour,
		t.wMinute,
		t.wSecond,
		t.wMilliseconds);

	printf(outbuf);

	if (bin_hex_option) {
		printf("Index: %d\n", index);
		printData(package, TS_PACKAGE_SIZE);
		printf("\n---\n");
		return;
	}

	printf("Index: %d\n", index);
	if (package[0] != TS_PACKAGE_LEAD) {
		printf("invalid ts package.\n");
		printf("\n---\n");
		return;
	}

	printf("\t*** TRANSPORT ***\n");	
	pid = MAKEWORD(hdr->pid_low, hdr->pid_high);
	printf("pid: %d(0x%x)\n", pid, pid);
	printf("transport_priority: %d\n", hdr->transport_priority);
	printf("payload_unit_start_indicator: %d\n", 
		hdr->payload_unit_start_indicator);
	printf("transport_error_indicator: %d\n", 
		hdr->transport_error_indicator);
	printf("continuity_counter: %d\n", hdr->continuity_counter);
	printf("_payload_present_flag: %d\n", hdr->_payload_present_flag);
	printf("_adaptation_field_present_flag: %d\n", 
		hdr->_adaptation_field_present_flag);
	printf("transport_scrambling_control: %d\n", 
		hdr->transport_scrambling_control);

	if (hdr->payload_unit_start_indicator) {
		printf("point_field: %d\n", *(char* )(hdr + 1));
		hdrlen ++;
		size_t section_hdr_len = printSectionInfo(
			&package[hdrlen], TS_PACKAGE_SIZE - hdrlen);

		size_t spec_len = 0;
		if (dod_special) {
			PrivateSectionHeader* generalHdr = 
				(PrivateSectionHeader* )&package[hdrlen];

			if (generalHdr->section_number == 0) {
				WORD section_len = MAKEWORD(generalHdr->section_length_lowbits, 
						generalHdr->section_length_highbits);
				

				spec_len = printDODSpecial(&package[hdrlen + section_hdr_len], 
					TS_PACKAGE_SIZE - hdrlen - section_hdr_len, 
					generalHdr->table_id, section_len);
			}
		}

		hdrlen += section_hdr_len + spec_len;
	}

	if (pid == 0x1fff) {
		printf("NULL\n");
	} else {
		printf("\t*** DATA ***\n");

		printData(&package[hdrlen], 
			TS_PACKAGE_SIZE - hdrlen);
	}

	printf("\n---\n");
}

int anaysisTsFile(LPCTSTR fileName)
{
	FILE* fp;
	unsigned char package[TS_PACKAGE_SIZE];
	int index = 0;

	if (!fileName) {
		fprintf(stderr, "invalid arguments.\n");
		return -1;
	}

	fp = fopen(fileName, "rb");
	if (fp == NULL) {
		fprintf(stderr, "invalid file path.\n");
		return -1;
	}

	while(fread(package, 1, TS_PACKAGE_SIZE, fp) == TS_PACKAGE_SIZE) {
		analysisTsPackage(index, package);
		index ++;
	}

	fclose(fp);
	return 0;
}
