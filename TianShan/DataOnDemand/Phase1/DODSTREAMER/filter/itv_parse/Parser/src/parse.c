#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parse.h"

#define IF_TRACE ITV_FALSE

PARSER_PARA g_parser_para;

ITV_DWORD IHeap_alloc(WI_ParseInfo* data , ITV_DWORD len)
{
	ITV_DWORD ret;
	ITV_DWORD new_top;
	
	ret = data->real_outsize;
	new_top = ret + len;
	if(new_top > data->output_size )
	{
		ITV_DBG_WARN(("IHeap_alloc() can't alloc the size in output buff, Error, return -211!\n"));
		return (-211);
	}else
	{
		data->real_outsize = new_top;
		return ret;	
	}
}

/************************************************************************************
Parameters:
data: the input structer.
buff: output buff.
len: the output buff length.
Returns:
>= 0: the output string length.
< 0: failed.
Description:
get the string in the input structer.
************************************************************************************/
ITV_DWORD ILP_Parse_EvaluateToNode(ST_NODE_T *ppnode, ITV_WORD type, ITV_DWORD value, ITV_DWORD length)
{
	if ((value > 0x200000)||(value<0))
	{
		ITV_DBG_ERR(("ILP_Parse_EvaluateToNode() input value over range! return error code -203\n"));
		return -203;
	}
	if((length > 0x2000)||(length<0))
	{
		ITV_DBG_ERR(("ILP_Parse_EvaluateToNode() input length over range! return error code -204\n"));
		return -204;
	}
	if((type > 0x2000)||(type<0))
	{
		ITV_DBG_ERR(("ILP_Parse_EvaluateToNode() input length over range! return error code -205\n"));
		return -205;
	}
	if(value < 0x10000)
	{
#ifdef ITV_SWAP_ORDER
		ppnode->id = itv_swap16((ITV_UWORD)type);
		ppnode->value = itv_swap16( (ITV_UWORD)value);
		ppnode->length =itv_swap16((ITV_UWORD)length);
#else
		ppnode->id = (ITV_UWORD)type;
		ppnode->value = (ITV_UWORD)value;
		ppnode->length = (ITV_UWORD)length;
#endif
	}
	else
	{
#ifdef ITV_SWAP_ORDER
		ppnode->id = itv_evaluateid(value, type);
		ppnode->value = itv_evaluatevaule(value);
		ppnode->length = itv_evaluatelength(value, length);
		ppnode->id = itv_swap16(ppnode->id);
		ppnode->value = itv_swap16(ppnode->value);
		ppnode->length = itv_swap16(ppnode->length);
#else
		ppnode->id = itv_evaluateid(value, type);
		ppnode->value = itv_evaluatevaule(value);
		ppnode->length = itv_evaluatelength(value, length);
#endif
	}
	return ITV_SUCCEED;
}



/************************************************************************************
Parameters:
data: the input structer.
buff: output buff.
len: the output buff length.
Returns:
>= 0: the output string length.
< 0: failed.
Description:
get the string in the input structer.
************************************************************************************/
#if 0
ITV_DWORD ILP_Parse_Get_Section(WI_ParseInfo *data, ITV_UBYTE * buff, ITV_DWORD len)
#endif
ITV_DWORD ILP_Parse_Get_Section(WI_ParseInfo *data, ITV_UBYTE ** buff, ITV_DWORD len)
{
	ITV_UBYTE *input_p;
	ITV_UBYTE *lenput_p;
	ITV_DWORD itv_i;
	ITV_DWORD length_q;
	
	if(data->pos >= data->input_size)
	{
		ITV_DBG_WARN(("ILP_Parse_Get_Section() position error, return -206!\n"));
		return (-206);
	}
	
	itv_i = data->pos;
	input_p = data->input + data->pos;
	
	while(itv_i < data->input_size)
	{
		if((*input_p !=0x0A) && (*input_p !=0x0D) && ( *input_p !=0x09) && ( *input_p !=0x20))
		{
			break;
		}
		else
		{
			itv_i++;
			input_p++;
		}
	}
	
	if ( itv_i >= data->input_size )
	{
		ITV_DBG_WARN(("ILP_Parse_Get_Section() input buff data error, maybe lost some data, the input size is %d, the request is %d, return -207!\n", data->input_size, itv_i));
		return (-207);
	}
	
	lenput_p = (ITV_UBYTE *)itv_strchr(input_p, ITV_PARSE_SEPARATOR);
	
	if ( lenput_p == NULL )
	{
		ITV_DBG_WARN(("ILP_Parse_Get_Section() input buff data error, maybe lost end data, return -208!\n"));
		return (-208);
	}
	
	length_q = (ITV_DWORD)(lenput_p - input_p);
	
	if( length_q >= len)
	{
		ITV_DBG_WARN(("ILP_Parse_Get_Section() output buff length is not enough, error, the need is %d, the length is %d, return -209!\n", length_q, len));
		return (-209);
	}
	
	//	itv_strncpy(buff, input_p, length_q);
	//	buff[length_q]=0;
	input_p[length_q] = 0;
	*buff = input_p;
	data->pos = itv_i + length_q + 1;
	return length_q;
}

/************************************************************************************
Parameters:
buff: the input string.
start: which postion in this input buff.
len: the input buff length.
output: the first integer number
Returns:
>= 0: the next postion in this buff.
< 0: failed.
Description:
get the integer in the string buff.
************************************************************************************/
ITV_DWORD ILP_Parse_Get_Number(ITV_UBYTE *buff, ITV_DWORD start, ITV_DWORD len, ITV_UWORD *output)
{
	ITV_UBYTE *cur_str;
	ITV_UBYTE *cur_point;
	ITV_DWORD itv_i;
	
	cur_str = buff + start;
	cur_point = (ITV_UBYTE *)itv_strchr(cur_str, ITV_DATA_SEPARATOR);
	if((cur_point == NULL))
	{
		cur_point = buff + len;
	}
	if((*cur_str < 0x3A)&&(*cur_str > 0x2F))
	{
		*cur_point = 0x00;
		itv_i = (ITV_DWORD)(cur_point - cur_str);
		*output = (ITV_UWORD)itv_atoi(cur_str);
		ITV_DBG_TRACE(IF_TRACE, ITV_FALSE, ("*output = %d \n ", *output ));
		itv_i += start;
		return itv_i;
	}
	else
	{
		ITV_DBG_WARN(("ILP_Parse_Get_Number() input buff data can't change to integer, maybe have error data return -210 ...\n"));
		return (-210);
	}
}

/************************************************************************************
Parameters:
buff: the input string.
len: the input buff length.
num: output number
Returns:
>= 0: the next postion in this buff.
< 0: failed.
Description:
get the integer in the string buff.
************************************************************************************/
ITV_DWORD ILP_Extract_Fields(ITV_UBYTE *buff, ITV_DWORD len,ITV_UWORD num[])
{
	ITV_DWORD itv_i;
	ITV_UBYTE * cur_str;
	
	itv_i = 0;
	cur_str = buff;
	
	itv_i = ILP_Parse_Get_Number(buff, itv_i, len, &num[0]);
	ITV_DBG_TRACE(IF_TRACE, ITV_FALSE, ("ILP_Parse_Get_Number() return num[0] = %d \n", num[0]));
	if(itv_i<0)
	{
		ITV_DBG_WARN(("ILP_Extract_Fields() can not get first number, return %d.\n ", itv_i));
		return itv_i;
	}
	
	itv_i++;
	itv_i = ILP_Parse_Get_Number(buff,itv_i,len,&num[1]);
	ITV_DBG_TRACE(IF_TRACE, ITV_FALSE, ("ILP_Parse_Get_Number() return num[1] = %d \n", num[2]));
	if(itv_i<0)
	{
		ITV_DBG_WARN(("ILP_Extract_Fields() can not get second number, return %d.\n ", itv_i));
		return itv_i;
	}
	
	itv_i++;
	itv_i = ILP_Parse_Get_Number(buff,itv_i,len,&num[2]);
	ITV_DBG_TRACE(IF_TRACE, ITV_FALSE, ("ILP_Parse_Get_Number() return num[2] = %d \n", num[2]));
	if(itv_i<0)
	{
		ITV_DBG_WARN(("ILP_Extract_Fields() can not get third number, return %d.\n ", itv_i));
		return itv_i;
	}
	
	return ITV_SUCCEED;
}

/************************************************************************************
Parameters:
data: the input structer.
Returns:
= 0: parse atrribute okay.
else: failed.
Description:
init the input structer.
************************************************************************************/
ITV_DWORD WI_Parse_init(WI_ParseInfo *data)
{
	data->pos = 0;
	data->cur_level = 0;
	data->options = 0;
	data->real_outsize = 0;
	data->flag = ITV_ILP_PARSE_NEW;
	data->input[data->input_size] = 0;
	itv_memset(data->expected, 0x00, sizeof(WI_Expected_t)*ITV_PARSE_EXPECT_DEPTH);
	return 0;
}

ITV_DWORD WI_Parse_parse(WI_ParseInfo *data)
{
	
	ITV_DWORD itv_err;
	
	if((data->input_size <= 0)||(data->input == NULL))
  		{
		ITV_DBG_ERR(("WI_Parse_parse() input buff Error , return -1!\n"));
		return (-201);
  		}
	if(data->flag == ITV_ILP_PARSE_NEW)
	{
		itv_err = ILP_Parse_Version(data);
		if(itv_err <0)
		{
			ITV_DBG_INFO(("ILP_Parse_All() ILP_Parse_Version() parse error! return %d ... \n", itv_err));
			return itv_err;
		}
	}
	
	while(data->flag != ITV_ILP_PARSE_END){
		switch(data->expected[data->cur_level].type)
		{
		case ITV_ILP_PARSE_TYPE_NODE:
			itv_err = ILP_Parse_Node(data);
			ITV_ERR_RETURN(itv_err);
			break;
		case ITV_ILP_PARSE_TYPE_ATTRIBUTELIST:
			itv_err = ILP_Parse_AttributeList(data);
			ITV_ERR_RETURN(itv_err);
			break;
		case ITV_ILP_PARSE_TYPE_NODELIST:
			itv_err = ILP_Parse_NodeList(data);
			ITV_ERR_RETURN(itv_err);
			break;
		case ITV_ILP_PARSE_TYPE_METADATALIST:
			itv_err = ILP_Parse_MetadataList(data);
			ITV_ERR_RETURN(itv_err);
			break;
		default:
			return ITV_ERROR;
		}
	};
	
	return ITV_NO_ERROR;
}


ITV_DWORD Itv_Parse_SimpleParse(WI_ParseInfo *data)
{
	ITV_DWORD itv_i;
	ITV_DWORD itv_length;
	ITV_UBYTE *input_p;
	ITV_UBYTE *output_p;
	
	itv_i = 0;
	input_p = data->input;
	output_p = data->output;
	itv_length = 0;
	while(itv_i < data->input_size)
	{
		if((*input_p !=0x0A) && (*input_p !=0x0D) && ( *input_p !=0x09))
		{
			*output_p = *input_p;
			output_p++;
			itv_length ++;
		}
		input_p++;
		itv_i++;
	}
	data->real_outsize = itv_length;
	
	return ITV_NO_ERROR;
}


ITV_DWORD ILP_Parse_Version(WI_ParseInfo *data)
{
	ITV_DWORD itv_err;
	ITV_DWORD loc0;
	ITV_DWORD loc1;
	ITV_DWORD next_pos;
	ITV_DWORD version_length;
	ITV_UBYTE *pnodeoffset;
	ITV_UBYTE *cur_str;
	
	if(data->pos == 0 && data->input[0]==239 && data->input[1]==187&& data->input[2]==191)
	{
		data->pos = 3;
	}
	if(data->input[data->pos] == ITV_PARSE_SEPARATOR)
	{
		data->pos ++;
	}
	data->sectionlength = ILP_Parse_Get_Section(data, &data->parse_buff, ITV_PARSE_BUFF_SIZE);
	
	if(data->sectionlength  < 0)
	{
		ITV_DBG_INFO(("ILP_Parse_Version() parse first input data error! return %d ... \n", data->sectionlength ));
		return data->sectionlength ;
	}
	
	loc0 = IHeap_alloc(data, 2 * SIZE_OF_STNODE);
	if(loc0 < 0)
	{
		ITV_DBG_WARN(("ILP_Parse_Version()  IHeap_alloc()  return %d!\n", loc0));
		return loc0;
	}
	
	pnodeoffset = data->output + loc0 + SIZE_OF_STNODE;
	
	next_pos = ILP_Parse_Get_Number(data->parse_buff, 0, data->sectionlength, &data->num[0]);
	if(next_pos < 0)
	{
		ITV_DBG_WARN(("ILP_Parse_Version() can not get content type, return %d ... \n", next_pos));
		return next_pos;
	}
	next_pos++;
	cur_str = &data->parse_buff[next_pos];
	version_length = data->sectionlength -next_pos;
	
	loc1 = IHeap_alloc(data, version_length);
	if(loc1 < 0)
	{
		ITV_DBG_WARN(("ILP_Parse_Version()  IHeap_alloc()_2  return %d!\n", loc1));
		return loc1;
	}
	
	itv_memcpy((data->output+loc1), cur_str, version_length);
	
	itv_err = ILP_Parse_EvaluateToNode(&data->pnode, 0x0FFF, loc1, version_length);
	if(itv_err !=ITV_SUCCEED)
	{
		ITV_DBG_WARN(("ILP_Parse_Version() ILP_Parse_EvaluateToNode() can't evaluate the node, return error code is %d \n", itv_err));
		return itv_err;
	}
	itv_memcpy((data->output +loc0), &data->pnode, SIZE_OF_STNODE);
	
	data->cur_level = 0;
	if(data->num[0] == 0)		//node
	{
		cur_str = (ITV_UBYTE *)itv_strchr((data->pos + data->input), ITV_PARSE_SEPARATOR);
#if 1
		if(cur_str == NULL)
		{
			data->flag = ITV_ILP_PARSE_END;
			itv_memset((data->output+loc1+version_length), 0x00, SIZE_OF_STNODE *2);
			data->real_outsize += SIZE_OF_STNODE *2;
		}
#endif
		data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODELIST;
		data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
		data->expected[data->cur_level].cur_position = SIZE_OF_STNODE;
		data->expected[data->cur_level].own_position = SIZE_OF_STNODE;
	}
	else if(data->num[0] == 1)		//node list
	{
		if(itv_strncmp(data->input+data->pos, "0,1,0|", 6)==0)
		{
			ITV_DWORD itv_len;
			data->pos += 6;
			cur_str = (ITV_UBYTE *)itv_strchr((data->pos + data->input), ITV_PARSE_SEPARATOR);
			itv_len = (ITV_DWORD)(cur_str - data->input);
			itv_memset(g_parser_para.init_para.err_code, 0x00, ITV_LIBRARY_PARSE_ILP_RETRUN_ERROR_CODE_LENGTH);
			itv_strncpy(g_parser_para.init_para.err_code, (data->pos + data->input), (itv_len - data->pos));
			ITV_DBG_WARN(("ILP_Parse_Version() get the error define is %s\n", g_parser_para.init_para.err_code));
			return (-250);
		}
		data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODE;
		data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
		data->expected[data->cur_level].cur_position = SIZE_OF_STNODE;
		data->expected[data->cur_level].own_position = SIZE_OF_STNODE;
	}
	else
	{
		ITV_DBG_WARN(("ILP_Parse_Version() the first number is unknown, return -202\n"));
		return (-202);
	}
	return ITV_NO_ERROR;
}



ITV_DWORD ILP_Parse_Node(WI_ParseInfo *data)
{
	ITV_DWORD itv_err;
	ITV_DWORD length_p;
	ITV_DWORD value_p; 
	
	if(data->expected[data->cur_level].flag == ITV_ILP_PARSE_NEW)
	{
		data->sectionlength = ILP_Parse_Get_Section(data, &data->parse_buff, ITV_PARSE_BUFF_SIZE);
		if(data->sectionlength < 0)
		{
			ITV_DBG_INFO(("ILP_Parse_Node() ILP_Parse_Get_Section() parse first input data error! return %d ... \n", data->sectionlength));
			return data->sectionlength;
		}
		itv_err = ILP_Extract_Fields(data->parse_buff, data->sectionlength, data->num);
		if(itv_err != ITV_SUCCEED)
		{
			ITV_DBG_WARN(("ILP_Parse_Node() can't extract fields, return %d ......", itv_err));
			return itv_err;
		}
		
		length_p = (ITV_DWORD) data->num[1] + (ITV_DWORD)data->num[2];
		value_p = IHeap_alloc(data, length_p * SIZE_OF_STNODE);
		if(value_p  < 0)
		{
			ITV_DBG_WARN(("ILP_Parse_Node()  IHeap_alloc()  return %d!\n", value_p));
			return value_p ;
		}
		
		itv_err = ILP_Parse_EvaluateToNode(&data->pnode, data->num[0], value_p, length_p);
		if(itv_err  < 0)
		{
			ITV_DBG_WARN(("ILP_Parse_Node()  ILP_Parse_EvaluateToNode()  return %d!\n", itv_err));
			return itv_err ;
		}
		itv_memcpy((data->expected[data->cur_level].cur_position + data->output), &data->pnode, SIZE_OF_STNODE);
		
		data->expected[data->cur_level].node_count = data->num[1]+data->num[2];
		data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
		data->expected[data->cur_level].cur_position = value_p;
		if(data->expected[data->cur_level].node_count ==0)
		{
			if(data->cur_level != 0)
			{
				data->cur_level --;
				data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
				data->expected[data->cur_level].node_count --;
			}
			else
			{
				data->flag = ITV_ILP_PARSE_END;
			}
		}
		else
		{
			data->cur_level++;
			data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
			if(data->num[1] ==0)			//no attribute
			{
				data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODELIST;
				data->expected[data->cur_level].own_position = value_p;
				data->expected[data->cur_level].cur_position = value_p;
			}
			else						//attribute list
			{
				data->expected[data->cur_level].node_count = data->num[1];
				data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_ATTRIBUTELIST;
				data->expected[data->cur_level].own_position = value_p + (ITV_DWORD)data->num[2]*SIZE_OF_STNODE;
				data->expected[data->cur_level].cur_position = data->expected[data->cur_level].own_position;
			}
		}
	}
	else
	{
		if(data->expected[data->cur_level].node_count == 0 )
		{
			if(data->cur_level != 0)
			{
				data->cur_level --;
				data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
				data->expected[data->cur_level].node_count --;
			}
			else
			{
				data->flag = ITV_ILP_PARSE_END;
			}
		}
		else
		{
			if(data->expected[data->cur_level].flag == ITV_ILP_PARSE_CONTINUE)
			{
				data->expected[data->cur_level].cur_position += SIZE_OF_STNODE;
			}
			data->cur_level ++;
			data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
			data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODELIST;
			data->expected[data->cur_level].own_position = data->expected[data->cur_level-1].cur_position;
			data->expected[data->cur_level].cur_position = data->expected[data->cur_level].own_position;
		}
	}
	return ITV_NO_ERROR;
}



ITV_DWORD ILP_Parse_MetadataList(WI_ParseInfo *data)
{
	ITV_DWORD itv_err;
	ITV_DWORD itv_i;
	ITV_DWORD loc0;
	ITV_DWORD tmp_node_count;
	ITV_DWORD next_pos;
	ITV_DWORD metadata_len;
	
	tmp_node_count = data->expected[data->cur_level].node_count;
	for(itv_i=0;itv_i<tmp_node_count;itv_i++)
	{
		data->sectionlength = ILP_Parse_Get_Section(data, &data->parse_buff, ITV_PARSE_BUFF_SIZE);
		if(data->sectionlength < 0)
		{
			ITV_DBG_INFO(("ILP_Parse_All() ILP_Parse_Get_Section() in attribute list error! return %d ... \n", data->sectionlength));
			return data->sectionlength;
		}
		
		next_pos = ILP_Parse_Get_Number(data->parse_buff, 0, data->sectionlength, data->num);
		if(next_pos <= 0)
		{
			ITV_DBG_WARN(("parse_all() can not get metadata type, return %d ... \n", next_pos));
			return next_pos;
		}
		next_pos++;
		metadata_len = data->sectionlength - next_pos;
		if(metadata_len>0)
		{
			loc0 = IHeap_alloc(data, metadata_len);
			if(loc0  < 0)
			{
				ITV_DBG_WARN(("parse_meta()  IHeap_alloc()  return %d!\n", loc0));
				return loc0 ;
			}
			itv_strncpy((data->output + loc0),(data->parse_buff + next_pos), metadata_len);
			itv_err = ILP_Parse_EvaluateToNode(&data->pnode, (ITV_WORD)(data->num[0]+ITV_ATTRIBUTE_MAX), loc0, metadata_len);
		}
		else
		{
			data->pnode.id = (ITV_UWORD)(data->num[0]+ITV_ATTRIBUTE_MAX);
			data->pnode.value = 0;
			data->pnode.length = 0;
			loc0 = 0;
		}
		itv_memcpy((data->output + data->expected[data->cur_level].cur_position), &data->pnode, SIZE_OF_STNODE);
		data->expected[data->cur_level].cur_position += SIZE_OF_STNODE;
		data->expected[data->cur_level].node_count--;
	}
	data->cur_level--;
	data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
	data->expected[data->cur_level].node_count --;
	return ITV_NO_ERROR;
}

ITV_DWORD ILP_Parse_AttributeList(WI_ParseInfo *data)
{
	ITV_DWORD itv_err;
	ITV_WORD itv_i;
	ITV_DWORD loc0;
	
	for(itv_i=0;itv_i<data->expected[data->cur_level].node_count;itv_i++)
	{
		data->sectionlength = ILP_Parse_Get_Section(data, &data->parse_buff, ITV_PARSE_BUFF_SIZE);
		if(data->sectionlength < 0)
		{
			ITV_DBG_INFO(("ILP_Parse_All() ILP_Parse_Get_Section() in attribute list error! return %d ... \n", data->sectionlength));
			return data->sectionlength;
		}
		else if (data->sectionlength == 0)
		{
			data->pnode.id = itv_i + ITV_ENTRYTYPE_MAX;
			data->pnode.length = 0;
			data->pnode.value = 0;
		}
		else
		{
			loc0 = IHeap_alloc(data, data->sectionlength);
			if(loc0  < 0)
			{
				ITV_DBG_WARN(("parse_attr()  IHeap_alloc()  return %d!\n", loc0));
				return loc0 ;
			}
			itv_err = ILP_Parse_EvaluateToNode(&data->pnode, (ITV_WORD)(itv_i + ITV_ENTRYTYPE_MAX), loc0, data->sectionlength);
			if(itv_err < 0)
			{
				ITV_DBG_INFO(("parse_attr() ILP_Parse_EvaluateToNode() return %d ... \n", itv_err));
				return itv_err;
			}
			itv_strncpy((data->output+loc0), data->parse_buff, data->sectionlength);
		}
		itv_memcpy((data->output + data->expected[data->cur_level].own_position + itv_i*SIZE_OF_STNODE), &data->pnode, SIZE_OF_STNODE);
		data->expected[data->cur_level].cur_position += SIZE_OF_STNODE;
	}
	data->cur_level--;
	data->expected[data->cur_level].flag = ITV_ILP_PARSE_FROM_ATTRIBUTE;
	data->expected[data->cur_level].node_count = data->expected[data->cur_level].node_count - data->expected[data->cur_level+1].node_count;
	return ITV_NO_ERROR;
}

ITV_DWORD ILP_Parse_NodeList(WI_ParseInfo *data)
{
	ITV_DWORD itv_err;
	
	if(data->expected[data->cur_level].flag == ITV_ILP_PARSE_NEW)
	{
		ITV_DWORD length_p;
		ITV_DWORD offset_p;
		
		data->sectionlength = ILP_Parse_Get_Section(data, &data->parse_buff, ITV_PARSE_BUFF_SIZE);
		if(data->sectionlength < 0)
		{
			ITV_DBG_INFO(("ILP_Parse_All() ILP_Parse_Get_Section() parse first input data error! return %d ... \n", data->sectionlength));
			return data->sectionlength;
		}
		
		itv_err = ILP_Extract_Fields(data->parse_buff, data->sectionlength, data->num);
		if(itv_err != ITV_SUCCEED)
		{
			ITV_DBG_WARN(("ILP_Parse_NodeListEx() can't extract fields, return %d ......", itv_err));
			return itv_err;
		}
		length_p = (ITV_DWORD)data->num[2];
		offset_p = IHeap_alloc(data, (SIZE_OF_STNODE*length_p));
		itv_err = ILP_Parse_EvaluateToNode(&data->pnode, data->num[0], offset_p, length_p);
		if(itv_err !=ITV_SUCCEED)
		{
			ITV_DBG_WARN(("ILP_Parse_NodeList() ILP_Parse_EvaluateToNode() can't evaluate the node, return error code is %d \n", itv_err));
			return itv_err;
		}
		itv_memcpy((data->output +data->expected[data->cur_level].own_position), &data->pnode, SIZE_OF_STNODE);
		
		data->expected[data->cur_level].cur_position = offset_p;
		data->expected[data->cur_level].node_count = data->num[2];
		if(data->num[2] == 0)
		{
			if(data->cur_level != 0)
			{
				data->cur_level --;
				data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
				data->expected[data->cur_level].node_count --;
			}
			else
			{
				data->flag = ITV_ILP_PARSE_END;
			}
		}
		else if(data->num[1] == 0)	//metadata list
		{
			data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_METADATALIST;
		}
		else
		{
			data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
			data->cur_level ++;
			data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODE;
			data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
			data->expected[data->cur_level].cur_position = data->expected[data->cur_level-1].cur_position;
		}
	}
	else
	{
		if(data->expected[data->cur_level].node_count == 0)
		{
			if(data->cur_level != 0)
			{
				data->cur_level --;
				data->expected[data->cur_level].flag = ITV_ILP_PARSE_CONTINUE;
				data->expected[data->cur_level].node_count --;
			}
			else
			{
				data->flag = ITV_ILP_PARSE_END;
			}		}
		else
		{
			data->expected[data->cur_level].cur_position += SIZE_OF_STNODE;
			data->cur_level ++;
			data->expected[data->cur_level].type = ITV_ILP_PARSE_TYPE_NODE;
			data->expected[data->cur_level].flag = ITV_ILP_PARSE_NEW;
			data->expected[data->cur_level].own_position = data->expected[data->cur_level -1].cur_position;
			data->expected[data->cur_level].cur_position = data->expected[data->cur_level].own_position;
		}
	}

	return ITV_NO_ERROR;	
}
ITV_UDWORD ItvParse_Compress(ITV_UBYTE *output, ITV_DWORD *output_length, const ITV_UBYTE *input, ITV_DWORD input_length);
ITV_DWORD ItvParse_ParseAndCompress(WI_ParseInfo *data)
{
	ITV_DWORD err;
	ITV_UBYTE *output_buffer;
	ITV_DWORD output_size;
	WI_Parse_init(data);
	err = WI_Parse_parse(data);
	if(ITV_NO_ERROR == err)
	{
		output_buffer = (ITV_UBYTE *)malloc(data->real_outsize + 1);
		if(NULL != output_buffer)
		{
			output_size = data->real_outsize + 1;
			err = ItvParse_Compress(output_buffer, &output_size, data->output, data->real_outsize);
			if(ITV_NO_ERROR == err)
			{
				memcpy(data->output, output_buffer, output_size);
				data->real_outsize = output_size;
			}
			free(output_buffer);
		}
	}
	return err;
}
ITV_DWORD ItvParse_SimpleParseAndCompress(WI_ParseInfo *data)
{
	ITV_DWORD err;
	ITV_UBYTE *output_buffer;
	ITV_DWORD output_size;
	WI_Parse_init(data);
	err = Itv_Parse_SimpleParse(data);
	if(ITV_NO_ERROR == err)
	{
		output_buffer = (ITV_UBYTE *)malloc(data->real_outsize + 1);
		if(NULL != output_buffer)
		{
			output_size = data->real_outsize + 1;
			err = ItvParse_Compress(output_buffer, &output_size, data->output, data->real_outsize);
			if(ITV_NO_ERROR == err)
			{
				memcpy(data->output, output_buffer, output_size);
				data->real_outsize = output_size;
			}
			free(output_buffer);
		}
	}
	return err;
}