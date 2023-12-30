#include "RpcGeekCommon.h"
#include "RpcGeekUtils.h"

#pragma comment(lib, "xmlrpc" XMPRPCLIBEXT)

namespace ZQ {
namespace RpcGeek {

//void throw_if_fault_occurred (const xmlrpc_env *penv, const char* prompt_str)
//{
//	char buf[256];
//	const char* prompt = (prompt_str != NULL) ? prompt_str : "Something failed";
//   if (penv && penv->fault_occurred)
//	{
//        snprintf(buf, sizeof(buf)-1, "%s. (%d)%s", prompt, penv->fault_code, penv->fault_string);
//        throw ZQ::common::IOException(buf);
//	}
//}

xmlrpc_value* _convert2xmlrpc(xmlrpc_env* pEnv, ZQ::common::Variant& value)
{
	xmlrpc_value* ret =NULL;
	
	try
	{
		switch(value.type())
		{
		case ZQ::common::Variant::T_BOOL: 
			ret = xmlrpc_bool_new(pEnv, (bool) value);
			break;
		case ZQ::common::Variant::T_INT:
			ret = xmlrpc_int_new(pEnv, (int) value);
			break;
		case ZQ::common::Variant::T_DOUBLE:   
			ret = xmlrpc_double_new(pEnv, (const double) value); 
			break;
		case ZQ::common::Variant::T_TIME:
			{
				struct tm tv = (struct tm)value;
				ret = xmlrpc_bool_new(pEnv, mktime(&tv));
			}
			break;
		case ZQ::common::Variant::T_STRING: 
			{
				tstring v = (tstring&)value;
				ret = xmlrpc_string_new(pEnv, v.c_str());
			}
			break;
			
		case ZQ::common::Variant::T_BASE64:
			{
				ZQ::common::Variant::BinaryData v = value;
				int count = v.size();
				BYTE* vbuf = new BYTE[count];
				for (int i =0; vbuf && i<count; i++);
				vbuf[i] = v[i];
				
				ret = xmlrpc_base64_new(pEnv, count, vbuf);
				delete[] vbuf;
			}
			break;
			
		case ZQ::common::Variant::T_ARRAY:
			{
				ret = xmlrpc_array_new(pEnv);
				int count = value.size();
				for (int i = 0; ret && i<count; i++)
				{
					xmlrpc_value* tmp = _convert2xmlrpc(pEnv, value[i]);
					if (NULL != tmp)
					{
						xmlrpc_array_append_item(pEnv, ret, tmp);
						xmlrpc_DECREF(tmp);
					}
				}
			}
			break;
			
		case ZQ::common::Variant::T_STRUCT:
			{
				ret = xmlrpc_struct_new(pEnv);
				int count = value.size();
				for (int i = 0; ret && i<count; i++)
				{
					tstring key = value.key(i);
					xmlrpc_value* tmp = _convert2xmlrpc(pEnv, value[key]);
					if (NULL != tmp)
					{
						xmlrpc_struct_set_value(pEnv, ret, key.c_str(), tmp);
						xmlrpc_DECREF(tmp);
					}
				}
			}
			break;
			
		default:
			break;
		}
	}
	catch(...)
	{
		xmlrpc_env_set_fault_formatted(pEnv, XMLRPC_PARSE_ERROR, "_convert2xmlrpc() failed");
		return NULL;
	}
	return ret;
}

xmlrpc_value* _convert2xmlrpc_parameters(xmlrpc_env* pEnv, ZQ::common::Variant& paramArray)
{
	xmlrpc_value* ret =_convert2xmlrpc(pEnv, paramArray);
	
	if (ret == NULL)
		return NULL;
	
	if (ZQ::common::Variant::T_ARRAY != paramArray.type())
	{
		xmlrpc_value* tmp = xmlrpc_array_new(pEnv);
		if (NULL == tmp)
		{
			xmlrpc_DECREF(ret);
			return NULL;
		}
		
		xmlrpc_array_append_item(pEnv, tmp, ret);
		xmlrpc_DECREF(ret);
		ret = tmp;
		
	}
	return ret;
}

bool _convert2variant(xmlrpc_env* pEnv, xmlrpc_value* value, ZQ::common::Variant& var)
{
	if(NULL == value)
		return false;
	
	var = ZQ::common::Variant(); // preset to NIL
	
	try
	{
		switch(xmlrpc_value_type(value))
		{
		case XMLRPC_TYPE_INT:
			{
				int v =0;
				xmlrpc_read_int(pEnv, value, &v);
				var = v;
			}
			break;
		case XMLRPC_TYPE_BOOL:
			{
				xmlrpc_bool v =0;
				xmlrpc_read_bool(pEnv, value, &v);
				var = ZQ::common::Variant((bool)(v!=0));
			}
			break;
		case XMLRPC_TYPE_DOUBLE:
			{
				xmlrpc_double v =0;
				xmlrpc_read_double(pEnv, value, &v);
				var = v;
			}
			break;
		case XMLRPC_TYPE_DATETIME:
			{
				time_t v =0;
				xmlrpc_read_datetime_sec(pEnv, value, &v);
				struct tm* time = gmtime(&v);
				var = ZQ::common::Variant(gmtime(&v));
			}
			break;
		case XMLRPC_TYPE_STRING:
			{
				const char* str=NULL;
				xmlrpc_read_string(pEnv, value, &str);
				
				if (NULL != str)
					var = str;
				free((char*)str);
			}
			break;
		case XMLRPC_TYPE_BASE64:
			{
				ZQ::common::Variant::BinaryData v;
				
				const unsigned char * buf;
				size_t size =0;
				xmlrpc_read_base64(pEnv, value, &size, &buf);
				if (NULL !=buf)
				{
					var = ZQ::common::Variant((void*)buf, size);
					free((void*)buf);
				}
				else var = ZQ::common::Variant();
			}
			break;
		case XMLRPC_TYPE_ARRAY:
			{
				int size = xmlrpc_array_size(pEnv, value);
				for (int i = 0; i< size; i++)
				{
					ZQ::common::Variant v;
					xmlrpc_value* item = NULL;
					xmlrpc_array_read_item(pEnv, value, i, &item);
					if (NULL != item)
					{
						XValueGuard vw(item);
						_convert2variant(pEnv, item, v);
					}
					
					var.set(i, v);
				}
			}
			break;
		case XMLRPC_TYPE_STRUCT:
			{
				int size = xmlrpc_struct_size(pEnv, value);
				for (int i = 0; i< size; i++)
				{
					xmlrpc_value* key=NULL, *val=NULL;
					xmlrpc_struct_read_member(pEnv, value, i, &key, &val);
					XValueGuard kg(key), vg(val);
					if (NULL != key && XMLRPC_TYPE_STRING == xmlrpc_value_type(key) && NULL != val)
					{
						ZQ::common::Variant k, v;
						if (_convert2variant(pEnv, key, k) && _convert2variant(pEnv, val, v))
						{
							tstring keystr = k;
							var.set(keystr.c_str(), v);
						}
					}
				}
			}
			break;
		case XMLRPC_TYPE_C_PTR:
			break;
		case XMLRPC_TYPE_NIL:
			break;
		case XMLRPC_TYPE_DEAD:
			break;
		default:
			break;
		}
	}
	catch(...)
	{
		xmlrpc_env_set_fault_formatted(pEnv, XMLRPC_PARSE_ERROR, "_convert2variant() failed");
		return false;
	}
	
	return true;
};

} // namespace RpcGeek
} // namespace ZQ
