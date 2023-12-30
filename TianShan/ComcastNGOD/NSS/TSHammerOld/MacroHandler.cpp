#include "MacroHandler.h"
#include <sstream>

MacroHandler::MacroHandler()
{
	_macroVec.push_back(INCREEAMENT);
}

MacroHandler::~MacroHandler()
{
	_macroVec.clear();
}

bool MacroHandler::updateMacro(::std::string &str)
{
	::std::string::size_type pos_begin = 0;
	::std::string::size_type pos_end = 0;

	for (::std::vector<::std::string>::iterator iter = _macroVec.begin(); iter != _macroVec.end(); iter++)
	{
		while(1)
		{
			pos_end = str.find((*iter).c_str(), pos_begin);


			if(::std::string::npos == pos_end) // not a valid macro reference
				break;

			pos_begin = pos_end;
			pos_end = str.find_first_of(')', pos_begin);

			if(::std::string::npos == pos_end) // not a valid macro reference
				break;

			if ((*iter).compare(INCREEAMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				uint16 i = atoi(line.c_str());
				stringstream ss;
				ss << i + 1;
				str.replace(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length(), ss.str());
			}

			//move pos_begin to next line
			pos_begin = pos_end + 1;
		}
	}

	return true;
}

bool MacroHandler::fixupMacro(::std::string &str)
{
	::std::string::size_type pos_begin = 0;
	::std::string::size_type pos_end = 0;

	for (::std::vector<::std::string>::iterator iter = _macroVec.begin(); iter != _macroVec.end(); iter++)
	{
		while(1)
		{
			pos_end = str.find((*iter).c_str());
			

			if(::std::string::npos == pos_end) // not a valid macro reference
				break;

			pos_begin = pos_end;
			pos_end = str.find_first_of(')', pos_begin);

			if(::std::string::npos == pos_end) // not a valid macro reference
				break;

			if ((*iter).compare(INCREEAMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				uint16 i = atoi(line.c_str());
				stringstream ss;
				ss << i;
				str.replace(pos_begin, pos_end + 1 - pos_begin, ss.str());
			}

			//move pos_begin to next line
			pos_begin = pos_end + 1;
		}
	}

	return true;
}