#ifndef _ITV_PARSE_H_
#define _ITV_PARSE_H_

#include "itv_pal.h"
#include "object_debug.h"
#include "itv_types.h"
#include "itv_errors.h"

#define ITV_PARSE_BUFF_SIZE 4096
#define ITV_ATTRIBUTE_MAX 3500 
#define ITV_ENTRYTYPE_MAX 3000
#define ITV_LIBRARY_PARSE_ILP_RETRUN_ERROR_CODE_LENGTH 32 

#define SIZE_OF_STNODE	sizeof(ST_NODE_T)
#define ITV_PARSE_EXPECT_DEPTH	20
#define ITV_PARSE_SEPARATOR '|' 
#define ITV_DATA_SEPARATOR ',' 

typedef struct ST_NODE_S{
	ITV_UWORD id;
	ITV_UWORD value;
	ITV_UWORD length;
}ST_NODE_T;

typedef enum{
	ITV_ILP_PARSE_CONTINUE = 0x4001,
	ITV_ILP_PARSE_FROM_ATTRIBUTE,
	ITV_ILP_PARSE_NEW,
	ITV_ILP_PARSE_ALL_DATA,
	ITV_ILP_PARSE_END,
	ITV_ILP_PARSE_OBJECT_START
}ITV_ILP_Parse_flag_t;

typedef struct WI_Expected_s{
	ITV_UWORD  type;
	ITV_ILP_Parse_flag_t  flag;
	ITV_WORD  node_count;
	ITV_DWORD  cur_position;
	ITV_DWORD  own_position;
}WI_Expected_t;

typedef enum{
	ITV_ILP_PARSE_TYPE_NODE = 0x8001,
	ITV_ILP_PARSE_TYPE_NODELIST,
	ITV_ILP_PARSE_TYPE_METADATALIST,
	ITV_ILP_PARSE_TYPE_ATTRIBUTELIST,
	ITV_ILP_PARSE_TYPE_END
}ITV_ILP_Parse_Status_Type_t;

typedef struct T_ParseInfo{
	ITV_UBYTE *input;
	ITV_UBYTE *output;
	ITV_UBYTE *data_buff;
	ITV_UBYTE *pnodeoffset;
	ITV_DWORD input_size;
	ITV_DWORD output_size;
	ITV_DWORD real_outsize;
	ITV_DWORD  pos;
	ITV_DWORD options;
	ITV_DWORD data_size;
	ITV_DWORD cur_level;
	ITV_DWORD sectionlength;
	ST_NODE_T pnode;
	ITV_ILP_Parse_flag_t flag;
	ITV_UWORD num[3];
	ITV_UBYTE *parse_buff;
	//	ITV_UBYTE parse_buff[ITV_PARSE_BUFF_SIZE];
	WI_Expected_t expected[ITV_PARSE_EXPECT_DEPTH];
}WI_ParseInfo;

ITV_DWORD IHeap_alloc(WI_ParseInfo* data, ITV_DWORD len);
ITV_DWORD WI_Parse_init(WI_ParseInfo *data);
ITV_DWORD WI_Parse_parse(WI_ParseInfo *data);
ITV_DWORD Itv_Parse_SimpleParse(WI_ParseInfo *data);
ITV_DWORD ILP_Parse_EvaluateToNode(ST_NODE_T *ppnode, ITV_WORD type, ITV_DWORD value, ITV_DWORD length);

ITV_DWORD ILP_Parse_Get_Section(WI_ParseInfo *data, ITV_UBYTE ** buff, ITV_DWORD len);
ITV_DWORD ILP_Parse_Get_Number(ITV_UBYTE *buff, ITV_DWORD start, ITV_DWORD len,ITV_UWORD *output);
ITV_DWORD ILP_Extract_Fields(ITV_UBYTE *buff, ITV_DWORD len,ITV_UWORD num[]);
ITV_DWORD ILP_Parse_Version(WI_ParseInfo *data);
ITV_DWORD ILP_Parse_Node(WI_ParseInfo *data);
ITV_DWORD ILP_Parse_MetadataList(WI_ParseInfo *data);
ITV_DWORD ILP_Parse_AttributeList(WI_ParseInfo *data);
ITV_DWORD ILP_Parse_NodeList(WI_ParseInfo *data);
#endif

typedef struct itv_object_init_parameter_s
{
	ITV_BYTE err_code[ITV_LIBRARY_PARSE_ILP_RETRUN_ERROR_CODE_LENGTH];
	WI_ParseInfo parse_info;	
}PARSER_INIT_PARA;//itv_object_init_parameter_t;

typedef struct Itv_Common_Parameter_s
{
#ifdef ITV_LEAK_DEBUG
	ITV_DWORD nodecount;
#endif
	ITV_DWORD instance_id;
	ITV_DWORD nodemutex;
	ITV_DWORD objsleep;
	ITV_DWORD stream_count;
	ITV_DWORD channel_count;
	ITV_DWORD pchannel_index;
	ITV_DWORD print_level;
	ITV_DWORD print_sleeptime;
	PARSER_INIT_PARA init_para;	
}PARSER_PARA;//Itv_Common_Parameter_t;
extern PARSER_PARA g_parser_para;

ITV_UDWORD ItvParse_Compress(ITV_UBYTE *output, ITV_DWORD *output_length, const ITV_UBYTE *input, ITV_DWORD input_length);
ITV_DWORD ItvParse_SimpleParseAndCompress(WI_ParseInfo *data);
ITV_DWORD ItvParse_ParseAndCompress(WI_ParseInfo *data);