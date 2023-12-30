#include "stdio.h"
#include "io.h"
#include "string.h"

#include "parse.h"
#include "zlib.h"

int Test_Parser()
{
	WI_ParseInfo *parser_p = &g_parser_para.init_para.parse_info;
	ITV_DWORD itv_err;
	FILE *fp;
	ITV_BYTE in_p[15*1024];
	ITV_UBYTE out_p[50*1024];
	ITV_DWORD in_length,out_length = 50 *1024;
	
	fp = fopen("datafetch","rb");
	in_length = _filelength(_fileno(fp));
	itv_memset(in_p, 0, 15*1024);
	fread(in_p,in_length,1,fp);
	fclose(fp);

	parser_p->input = in_p;
	parser_p->input_size = in_length;
	parser_p->output = out_p;
	parser_p->output_size = 50*1024;
	itv_err = WI_Parse_init(parser_p);
	if(itv_err != ITV_NO_ERROR)
	{
		printf("Error 1\n");
		return -1;
	}
	itv_err = WI_Parse_parse(parser_p);
	if(itv_err != ITV_NO_ERROR)
	{
		printf("Error 2\n");
		return -2;
	}
	printf("out_len = %d\n",parser_p->real_outsize);

	ItvParse_Compress(out_p, &out_length, in_p, in_length);
	fp = fopen("louis.cbs","wb");
	fwrite(out_p,out_length, 1, fp);
	fclose(fp);

	return 0;
}