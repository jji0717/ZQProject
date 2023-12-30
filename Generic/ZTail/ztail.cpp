//////////////////////////////////////////////////////////////////////////
// the main file for ctail.exe
//////////////////////////////////////////////////////////////////////////

#define _WINSOCK2API_
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

std::string leftStr(const std::string& cstStr, int pos)
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

std::string getLeftStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
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

std::string rightStr(const std::string& cstStr, int pos)
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

std::string getRightStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
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

void showHelp()
{
	cout<<"help: "<<"ztail.exe <readfile> [<outputfile>]"<<endl;
	cout<<"where <readfile> indicates the file on which to tail"<<endl;
	cout<<"where <outputfile> indicates the file storing the tail data. if no <outputfile> specified, the data will be written to console window."<<endl;
}

int main(int argc, char* argv[])
{
	if (2 != argc && 3 != argc)
	{
		showHelp();
		return 1;
	}
	
	ofstream writeFile;
	if (3 == argc)
	{
		writeFile.open(argv[2], ios::binary);
		if (false == writeFile.is_open())
		{
			cout<<"open "<<argv[2]<<" for writing failed"<<endl;
			return 1;
		}
	}
	
	ifstream readFile;
	readFile.open(argv[1], ios::ate | ios::binary);
	if (false == readFile.is_open())
	{
		cout<<"open "<<argv[1]<<" for read failed"<<endl;
		return 1;
	}

	// gain the position where to read data
	__int64 curReadPos = 0;
	curReadPos = readFile.tellg();
	readFile.close();
	readFile.clear();

	while (true)
	{
		// keeping open file if failed
		readFile.open(argv[1], ios::ate | ios::binary); // 这里以二进制形式打开文件，以防止gcount()返回的值小于实际读取的数值
		if (false == readFile.is_open())
		{
			::Sleep(5);
			continue;
		}

		// gain current write position
		__int64 nextReadPos = 0;
		nextReadPos = readFile.tellg();
		
		// means that no rename operation has been carried since last read.
		if (nextReadPos > curReadPos)
		{
			__int64 nReadNum = nextReadPos - curReadPos;
			char* readBuff = new char[nReadNum + 1];
			readFile.seekg(-nReadNum, ios::end);
			readFile.read(readBuff, nReadNum);
			__int64 nTrueRead = readFile.gcount(); // 获得实际读书的字节数
			readBuff[nTrueRead] = '\0';
			if (true == writeFile.is_open())
			{
				writeFile.write(readBuff, nTrueRead);
				writeFile.flush();
			}
			else 
			{
				cout<<readBuff;
				cout.flush();
			}
			curReadPos += nTrueRead;
			delete []readBuff;
		}
		else if (nextReadPos < curReadPos)
		{
			// 读取被改名的文件中没有读取的数据
			std::string preFileName;
			std::string name = getLeftStr(argv[1], ".", false);
			if (true == name.empty()) // if no extension name, name will be empty.
				name = argv[1];
			std::string ext = getRightStr(argv[1], ".", false);
			if (true == ext.empty()) // if no extension name, ext will be empty.
				preFileName = name + ".0";
			else 
				preFileName = name + ".0." + ext;
			ifstream preReadFile;
			preReadFile.open(preFileName.c_str(), ios::ate | ios::binary);
			if (preReadFile.is_open())
			{
				nextReadPos = preReadFile.tellg();
				if (nextReadPos > curReadPos)
				{
					__int64 nReadNum = nextReadPos - curReadPos;
					char* readBuff = new char[nReadNum + 1];
					preReadFile.seekg(-nReadNum, ios::end);
					preReadFile.read(readBuff, nReadNum);
					__int64 nTrueRead = preReadFile.gcount(); // 获得实际读书的字节数
					readBuff[nTrueRead] = '\0';
					if (true == writeFile.is_open())
					{
						writeFile.write(readBuff, nTrueRead);
						writeFile.flush();
					}
					else 
					{
						cout<<readBuff;
						cout.flush();
					}
					delete []readBuff;
				}
			}
			if (true == preReadFile.is_open())
				preReadFile.close();
			preReadFile.clear();

			// 读取当前文件中的数据
			curReadPos = 0;
			nextReadPos = readFile.tellg();
			__int64 nReadNum = nextReadPos - curReadPos; // 当前文件中的数据全部应该写入
			char* readBuff = new char[nReadNum + 1];
			readFile.seekg(0, ios::beg);
			readFile.read(readBuff, nReadNum);
			__int64 nTrueRead = readFile.gcount(); // 获得实际读书的字节数
			readBuff[nTrueRead] = '\0';
			if (true == writeFile.is_open())
			{
				writeFile.write(readBuff, nTrueRead);
				writeFile.flush();
			}
			else 
			{
				cout<<readBuff;
				cout.flush();
			}
			curReadPos += nTrueRead;
			delete []readBuff;
		}

		// close file
		readFile.close();
		readFile.clear();
		::Sleep(500);
	}
	
	// close write file
	if (true == writeFile.is_open())
		writeFile.close();

	return 0;
} // end main()

