
#ifndef _ZQ_IMPLINCLUDE_H_
#define _ZQ_IMPLINCLUDE_H_
	
//for dynamic
typedef bool (*ACTION_PROC)(char* const* key, char* const* data, size_t count);
typedef bool (*INIT_PROC)();
typedef void (*UNINIT_PROC)();

#ifndef PORT_TYPE
#define PORT_TYPE extern "C"
#endif//PORT_TYPE

#ifndef GEN_DLL
#ifdef _WINDLL
#define GEN_DLL
#elif _USRDLL
#define GEN_DLL
#elif _AFXDLL
#define GEN_DLL
#endif
#endif//GEN_DLL

#ifndef DLL_PORT
#ifdef GEN_DLL
#define DLL_PORT PORT_TYPE __declspec(dllexport)
#else//_DLL
#define DLL_PORT PORT_TYPE
#endif//_DLL
#endif//DLL_PORT
	
#define PROC_INIT	"Init_Proc"
#define PROC_UNINIT	"UnInit_Proc"
#define PROC_ACTION	"Action_Proc"

//for static, implementation dll must realize those function
DLL_PORT bool Init_Proc();
DLL_PORT void UnInit_Proc();
DLL_PORT bool Action_Proc(char* const* key, char* const* data, size_t count);

#endif//_ZQ_IMPLINCLUDE_H_
