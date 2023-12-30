
#ifndef _ZQ_IARMSCRIPT_H_
#define _ZQ_IARMSCRIPT_H_

#pragma warning(disable:4786)
#include <string>
#include <vector>

#define DEC_TO_HEX_DIGIT 8


//#define FUNC_LOC2UTC "LOC2UTC("
#define FUNC_DEC2HEX "DEC2HEX("
#define FUNC_MSATIME "MSA_TIME("

std::string ApplyFunctions(const std::string& str);


#endif//_ZQ_IARMSCRIPT_H_
