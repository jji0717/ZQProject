#ifndef __MACROHANDLER__
#define __MACROHANDLER__

#include "XML_Handler.h"
#define INCREEAMENT "INC("	//increase the int variable with 1

class MacroHandler
{
public:
	MacroHandler();
	~MacroHandler();

	bool updateMacro(::std::string &value);
	bool fixupMacro(::std::string &str);

	::std::vector<::std::string> _macroVec;
};

#endif __MACROHANDLER__