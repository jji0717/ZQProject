#ifndef __TsCtrlClient_h__
#define __TsCtrlClient_h__

namespace ZQTianShan 
{
namespace TsCtrlClient 
{

typedef struct _att
{
	char name[64];
	char value[256];
} Attrbute;

typedef int (*GetData_Proc)(const Attrbute* attributes, const int  * attrCount, int* columnCount, char** columnNames, int* rowCount, char** cells);
typedef int (CALLBACK *OnEvent_Proc)(const char* category, const int * level, const char* timestamp, const char* message);

} 
} // endof namespaces


#endif // __TsCtrlClient_h__

