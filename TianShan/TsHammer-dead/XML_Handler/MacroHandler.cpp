#include "MacroHandler.h"
#include <sstream>
#include <cmath>

using namespace std;

MacroHandler::MacroHandler()
{
	_tailorType = TAILOR_DECIMAL;
	_tailorRange = 4;
	_tailorMaxValue = DWORD (std::pow(10.0, _tailorRange) - 1);
	_tailorValue = new (std::nothrow) char[_tailorRange + 1];

	_macroVec.push_back(INCREEAMENT);
	_macroVec.push_back(TAILORINCREMENT);
}

MacroHandler::~MacroHandler()
{
	_macroVec.clear();
}

void MacroHandler::setTailorProp(std::string strTailorType, int tailorRange)
{
	if (_tailorRange != tailorRange)
	{
		if (_tailorRange != NULL)
		{
			delete [] _tailorValue;
		}
		_tailorValue = new (std::nothrow) char[tailorRange + 1];
	}

	_tailorRange = tailorRange;
	if (strTailorType == "HEX")
	{
		_tailorType = TAILOR_HEX;
		_tailorMaxValue = DWORD (std::pow(16.0, _tailorRange) - 1);
	}
	else
	{
		_tailorType = TAILOR_DECIMAL;
		_tailorMaxValue = DWORD (std::pow(10.0, _tailorRange) - 1);
	}
}

std::string MacroHandler::getTailorValue(const std::string strTailor)
{
	for (int i = 0; i < _tailorRange; i++)
	{
		*(_tailorValue + i) = '0';
	}
	*(_tailorValue + _tailorRange) = '\0';

	if (_tailorType == TAILOR_DECIMAL)
	{
		int j = _tailorRange - 1;
		for (int i = strTailor.size() - 1; i >= 0 && j >= 0; i--)
		{
			_tailorValue[j] = strTailor[i];
			j--;
		}
	}
	else
	{
		char buffer[256];
		sprintf(buffer, "%lx", atol(strTailor.c_str()));
		int j = _tailorRange - 1;
		for (int i = strlen(buffer) - 1; i >= 0 && j >=0 ; i--)
		{
			_tailorValue[j] = buffer[i];
			j--;
		}
	}
	return _tailorValue;
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
			{
				break;
			}

			pos_begin = pos_end;
			pos_end = str.find_first_of(')', pos_begin);

			if(::std::string::npos == pos_end) // not a valid macro reference
			{
				break;
			}

			if ((*iter).compare(INCREEAMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				DWORD i = atol(line.c_str());
				stringstream ss;
				ss << i + 1;
				str.replace(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length(), ss.str());
			}

			if ((*iter).compare(TAILORINCREMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				DWORD i = atol(line.c_str());
				if (i >= _tailorMaxValue)
				{
					i = 1;
				}
				else
				{
					i++;
				}
				stringstream ss;
				ss << i;
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
			{
				break;
			}

			pos_begin = pos_end;
			pos_end = str.find_first_of(')', pos_begin);

			if(::std::string::npos == pos_end) // not a valid macro reference
			{
				break;
			}

			if ((*iter).compare(INCREEAMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				DWORD i = atol(line.c_str());
				stringstream ss;
				ss << i;
				str.replace(pos_begin, pos_end + 1 - pos_begin, ss.str());
			}

			if ((*iter).compare(TAILORINCREMENT) == 0)
			{
				::std::string line = str.substr(pos_begin + (*iter).length(), pos_end - pos_begin - (*iter).length());
				std::string strTailor = getTailorValue(line);
				str.replace(pos_begin, pos_end + 1 - pos_begin, strTailor);
			}

			//move pos_begin to next line
			pos_begin = pos_end + 1;
		}
	}

	return true;
}