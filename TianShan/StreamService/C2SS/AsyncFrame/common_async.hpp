#include <string>
#include <assert.h>
#include <iostream>

//fail
#define ERROR_CODE_OPERATION_FAIL		-10
#define ERROR_CODE_OPERATION_TIMEOUT	-11
#define ERROR_CODE_OPERATION_PENDING	-20
#define ERROR_CODE_OPERATION_CLOSED		-1
//success
#define ERROR_CODE_OPERATION_OK			1

#ifdef ZQ_OS_MSWIN
  #define CLASSINDLL_CLASS_DECL
  #if defined(CLASS_DLL_IMPORT) && !defined(CLASSINDLL_CLASS_DECL) 
    #define CLASSINDLL_CLASS_DECL __declspec(dllexport)
  #elif !defined(CLASSINDLL_CLASS_DECL)
    #define CLASSINDLL_CLASS_DECL __declspec(dllimport)
  #endif//CLASS_DLL_EXPORT
#else
  #define CLASSINDLL_CLASS_DECL
#endif//ZQ_OS_MSWIN
