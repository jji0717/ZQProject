#include "TianShanIceHelper.h"
#include "definition.h"
#include <sstream>
#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif

::Ice::Byte modulationStr2Int(::std::string &modulation)
{
	if (stricmp(modulation.c_str() ,"qam256") == 0)
		return 0x10;
	else if (stricmp(modulation.c_str() ,"qam128") == 0)
		return 0x0c;
	else if (stricmp(modulation.c_str() ,"qam64") == 0)
		return 0x08;
	else if (stricmp(modulation.c_str(),"qam32") == 0)
		return 0x07;
	else if (stricmp(modulation.c_str() ,"qam16") == 0)
		return 0x06;
	else
		return 0x00;
}

::std::string modulationInt2Str(::Ice::Byte &modulation)
{
	if (modulation == 0x10)
		return "qam256";
	else if (modulation == 0x0c)
		return "qam128";
	else if (modulation == 0x08)
		return "qam64";
	else if (modulation == 0x07)
		return "qam32";
	else if (modulation == 0x06)
		return "qam16";
	else
		return "unknown";
}
