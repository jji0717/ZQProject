//////////////////////////////////////////////////////////////////////////
// file: stroprt.h
// author: Han Guan
// copyright: 2007-03-30 All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#ifndef _STROPRT_H_
#define _STROPRT_H_

#include <string>
#include <vector>
using namespace std;

namespace ZQ {

	class StringOperation {
	public:
		
		static std::string leftStr(const std::string& cstStr, int pos);
		
		// Get the left string which behinds the char lies in the string "splitStr"
		// the param first indicates the first occurance or the last occurance
		static std::string getLeftStr(const std::string& cstStr, const std::string& splitStr, bool first = true);
		
		static std::string rightStr(const std::string& cstStr, int pos);
		
		// Get the right string which behinds the char lies in the string "splitStr"
		// the param first indicates the first occurance or the last occurance
		static std::string getRightStr(const std::string& cstStr, const std::string& splitStr, bool first = true);
		
		// Functionality: Get the path of a filename
		// Example: if the filename is "E:\TianShan\xxx.doc", then it will return "E:\TianShan"
		static std::string getPath(const std::string& cstStr);
		
		// Get the string lies between f_pos and l_pos in the string "cstStr"
		// Notice the chars lie at f_pos and l_pos are not included
		static std::string midStr(const std::string& cstStr, int f_pos, int l_pos);
		
		// Split the string "cstStr" into a lot of string, and storing them into a string vector strVect
		static void splitStr(const std::string& cstStr, const std::string split, vector<std::string>& strVect);
		
		// Replace the char 'from' to the char 'to'
		static std::string replaceChar(std::string& Str, const char& from, const char& to);
		
		// Replace all the chars lies in string from to the char 'to'
		static std::string replaceChars(std::string& Str, const std::string& from, const char& to);
		
		// Replace the string "from" to string "to"
		static std::string replaceStr(std::string& Str, const std::string& from, const std::string& to);
		
		// Stores the first position of the any char lies in string Str to pos
		static bool hasChars(const std::string& Str, const std::string& idetifier, int& pos);
		
		// Stores the first position of the char 'ch' in the string Str to pos
		static bool hasChar(const std::string& Str, const char& ch, int& pos);
		
		// Stores the position of the char 'ch' in the string Str to vector poss
		static bool hasChar(const std::string& Str, const char& ch, std::vector<int>& poss);
		
		// Remove the chars lies in idetifier from the string Str
		static std::string removeChars(std::string& Str, const std::string& idetifier);
		
		// Remove the char 'ch' from the string Str
		static std::string removeChar(std::string& Str, const char& ch);
		
		// Get the left num chars of the string cstStr
		static std::string nLeftStr(const std::string& cstStr, int num);

		// Get the right num chars of the string cstStr
		static std::string nRightStr(const std::string& cstStr, int num);

		// Check if a string is a number
		static bool isNumber(const std::string& cstStr);

		static bool isInt(const std::string& cstStr);

		//
	};

	std::string StringOperation::leftStr(const std::string& cstStr, int pos)
	{
		int size = cstStr.size();
		if (size -1 < pos)
		{
			return cstStr;
		}
		int cur = 0;
		std::string strRet;
		for (; cur < pos; cur++)
		{
			strRet += cstStr[cur];
		}
		return strRet;
	}

	std::string StringOperation::getLeftStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
	{
		std::string::size_type null_pos = -1, find_pos = -1;
		if (first)
		{
			find_pos = cstStr.find_first_of(splitStr);
			if (find_pos != null_pos) 
			{
				return (leftStr(cstStr,find_pos));
			}
			else 
			{
				return std::string("");
			}
		}
		else 
		{
			find_pos = cstStr.find_last_of(splitStr);
			if (find_pos != null_pos) 
			{
				return (leftStr(cstStr,find_pos));
			}
			else 
			{
				return std::string("");
			}
		}
	}
	
	std::string StringOperation::rightStr(const std::string& cstStr, int pos)
	{
		if (pos <= -1)
		{
			return cstStr;
		}
		std::string strRet;
		int len = cstStr.size();
		for (int cur = pos + 1; cur < len; cur++)
		{
			strRet += cstStr[cur];
		}
		return strRet;
	}
	
	std::string StringOperation::getRightStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
	{
		std::string::size_type null_pos = -1, find_pos = -1;
		if (first)
		{
			find_pos = cstStr.find_first_of(splitStr);
			if (find_pos != null_pos)
			{
				return rightStr(cstStr,find_pos);
			}
			else
			{
				return std::string("");
			}
		}
		else
		{
			find_pos = cstStr.find_last_of(splitStr);
			if (find_pos != null_pos)
			{
				return rightStr(cstStr,find_pos);
			}
			else
			{
				return std::string("");
			}
		}
	}
	
	std::string StringOperation::getPath(const std::string& cstStr)
	{
		return getLeftStr(cstStr, "\\/",false);
	}
	
	std::string StringOperation::midStr(const std::string& cstStr, int f_pos, int l_pos)
	{
		if (f_pos >= l_pos)
		{
			return "";
		}
		if (f_pos < -1)
		{
			f_pos = -1;
		}
		int size = cstStr.size();
		if (l_pos > size)
		{
			l_pos = size;
		}
		int cur = f_pos + 1;
		std::string strRet;
		for (; cur < l_pos; cur++) {
			strRet += cstStr[cur];
		}
		return strRet;
	}
	
	void StringOperation::splitStr(const std::string& cstStr, const std::string split, vector<std::string>& strVect)
	{
		strVect.clear();
		std::string::size_type null_pos = -1, find_pos = -1, last_find_pos = -1;
		find_pos = cstStr.find_first_of(split);
		while (find_pos != null_pos)
		{
			strVect.push_back(midStr(cstStr, last_find_pos, find_pos));
			last_find_pos = find_pos;
			find_pos = cstStr.find_first_of(split, last_find_pos + 1);
		}
		strVect.push_back(midStr(cstStr, last_find_pos, cstStr.size()));
	}
	
	std::string StringOperation::replaceChar(std::string& Str, const char& from, const char& to)
	{
		int size = Str.size();
		for (int i = 0; i < size; i++)
		{
			if (Str[i] == from)
			{
				Str[i] = to;
			}
		}
		return Str;
	}
	
	std::string StringOperation::replaceChars(std::string& Str, const std::string& from, const char& to)
	{
		int size = from.size();
		for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
		{
			replaceChar(Str, from[tmp_cur], to);
		}
		return Str;
	}
	
	std::string StringOperation::replaceStr(std::string& Str, const std::string& from, const std::string& to)
	{
		int size = Str.size();
		int f_size = from.size();
		char* Str_copy = new char[size+1];
		strcpy(Str_copy, Str.c_str());
		Str = "";
		char* tmp_copy = Str_copy;
		char* ret_str = strstr(tmp_copy, from.c_str());
		while (ret_str != NULL)
		{
			char* temp = NULL;
			temp = tmp_copy;
			if (ret_str != temp)
			{
				char* tmp_buff = new char[ret_str - temp + 1];
				strncpy(tmp_buff, temp, ret_str - temp);
				tmp_buff[ret_str - temp] = '\0';
				Str += tmp_buff;
				delete[] tmp_buff;
			}
			Str += to;
			tmp_copy = ret_str + f_size;
			ret_str = strstr(tmp_copy, from.c_str());
		}
		if (strlen(tmp_copy))
			Str += tmp_copy;
		delete[] Str_copy;
		return Str;
	}
	
	// Check if the std::string "Str" has any of the char in std::string "identifier", if true pos will store the
	// position of the first char which is equal to some one of the std::string "idetifier"
	bool StringOperation::hasChars(const std::string& Str, const std::string& idetifier, int& pos)
	{
		std::string::size_type null_pos = -1;
		std::string::size_type find_pos = -1;
		find_pos = Str.find_first_of(idetifier);
		if (find_pos != null_pos)
		{
			pos = find_pos;
			return true;
		}
		else 
		{
			pos = -1;
			return false;
		}
	}
	
	// if ch can be found in Str, the function will return true, otherwise false.
	// and pos will be set with value -1 if not found, or other value more than -1
	// if ch can be found.
	bool StringOperation::hasChar(const std::string& Str, const char& ch, int& pos)
	{
		int size = Str.size();
		for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
		{
			if (Str[tmp_cur] == ch)
			{
				pos = tmp_cur;
				return true;
			}
		}
		pos = -1;
		return false;
	}
	
	// if ch can be found in Str, all the found position will be stored into vector which stores int values.
	// and function will return true, otherwise there will be no items in poss and function will return false.
	bool StringOperation::hasChar(const std::string& Str, const char& ch, std::vector<int>& poss)
	{
		poss.clear();
		int size = Str.size();
		for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
		{
			if (Str[tmp_cur] == ch)
			{
				poss.push_back(tmp_cur);
			}
		}
		if (poss.size())
			return true;
		return false;
	}
	
	std::string StringOperation::removeChars(std::string& Str, const std::string& idetifier)
	{
		std::string copy_str(Str);
		int size = copy_str.size();
		Str = "";
		for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
		{
			std::string tmp_str;
			tmp_str += copy_str[tmp_cur];
			std::string::size_type find_pos = -1, null_pos = -1;
			find_pos = idetifier.find_first_of(tmp_str);
			if (find_pos == null_pos)
			{
				Str += copy_str[tmp_cur];
			}
		}
		return Str;
	}
	
	std::string StringOperation::removeChar(std::string& Str, const char& ch)
	{
		std::string copy_str(Str);
		int size = copy_str.size();
		Str = "";
		for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
		{
			if (copy_str[tmp_cur] != ch)
			{
				Str += copy_str[tmp_cur];
			}
		}
		return Str;
	}

	std::string StringOperation::nLeftStr(const std::string& cstStr, int num)
	{
		return leftStr(cstStr, num);
	}

	std::string StringOperation::nRightStr(const std::string& cstStr, int num)
	{
		return rightStr(cstStr, cstStr.size() - num - 1);
	}

	bool StringOperation::isInt(const std::string& cstStr)
	{
		std::string::size_type null_pos = -1;
		std::string::size_type find_pos = -1;
		int size = cstStr.size();
		if (size == 0)
			return false;
		find_pos = cstStr.find_first_not_of("0123456789");
		if (find_pos != null_pos)
			return false;
		return true;
	}

	bool StringOperation::isNumber(const std::string& cstStr)
	{
		std::string::size_type null_pos = -1;
		std::string::size_type find_pos = -1;
		int size = cstStr.size();
		if (size == 0)
			return false;
		find_pos = cstStr.find_first_of(".");
		if (find_pos != null_pos)
		{
			find_pos = cstStr.find_first_of(".", find_pos + 1);
			if (find_pos != null_pos)
			{
				return false; // have two '.'
			}
		}
		else 
		{
			find_pos = cstStr.find_first_not_of("0123456789");
			if (find_pos == null_pos)
				return true;
			return false;
		}
		return true;
	}

}

#endif // _STROPRT_H_