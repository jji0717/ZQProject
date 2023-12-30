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

		static void trimRight(std::string& Str);

		static void trimLeft(std::string& Str);

		static void trimAll(std::string& Str);

		static std::string getTrimRight(const std::string& Str);

		static std::string getTrimLeft(const std::string& Str);

		static std::string getTrimAll(const std::string& Str);
	};
}

#endif // _STROPRT_H_