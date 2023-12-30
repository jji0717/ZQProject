//////////////////////////////////////////////////////////////////////////
// Module: Directory.h
// Author: Han Guan
//////////////////////////////////////////////////////////////////////////

#ifndef _DIRECTORYINFO_H_
#define _DIRECTORYINFO_H_

#include <Windows.h>
#include <string>
#include <vector>
#include <exception>
#include <fstream>
using namespace std;

class Exception : public exception
{
public:
	Exception(std::string _message) {
		m_message = _message;
	}
	std::string getMessage(){
		return m_message;
	}
protected:
	std::string m_message;
};

class File;
class Directory
{
public:
	
	// 功能：构造一个Directory对象, name必须是一个实际存在的文件夹
	// 抛出异常：当name不是文件夹的时候，抛出异常
	Directory(std::string userString);

	// 功能：析构Directory对象
	virtual ~Directory();

	// 功能：分析文件夹的结构，将其中的文件(File)对象和文件夹(Directory)对象
	// 存放在vector中
	void parse(bool bSub = true);

	// 功能：显示文件夹的结构
	// 参数：bSub是否显示子目录信息，默认为显示
	void print(bool bSub = true);

	// 功能：重新设置目录
	void setDirectory(std::string name);

	// 功能：获得文件夹的名称，不包含路径
	std::string getName();

	// 功能：获得文件夹的名称，包含路径
	std::string getFullName();

	// 功能：获得当前文件下的某一个文件夹对象的指针
	Directory* getDirectory(std::string name);

	// 功能：获得当前文件夹下的某一个文件对象的指针
	File* getFile(std::string name);

	// 功能: 将Directory的结构输出至文件流
	// 参数：name为文件名，bSub为true时也输出子目录中的内容
	void output(std::string name,bool bSub=true);

	// 功能：根据指定的
	void check(std::string dname,std::string fname,bool bSub = true);

	// 功能：清空存放Directory的vector和File的vector，包括子目录
	void free();

public:

	// 自定义类型，存放Directory对象指针的vector和其iterator
	typedef std::vector<Directory*>	vector_d;
	typedef vector_d::iterator vector_d_itor;

	// 自定义类型，存放File对象指针的vector和其iterator
	typedef std::vector<File*> vector_f;
	typedef vector_f::iterator vector_f_itor;

protected:
	// 存放当前文件夹下所有Directory对象指针的vector
	vector_d m_vectorDirectory;

	// 存放当前文件夹下所有File对象指针的vector
	vector_f m_vectorFile;

	//当前文件夹的名称，包含路径信息
	std::string m_directory;
	std::string m_query;
	bool bSubDirectoryScaned;
	
public:	
	// 功能：获得存放Directory对象的vector的引用
	vector_d& getDirectoryVector();

	// 功能：获得存放File对象的vector的引用
	vector_f& getFileVector();

protected:
	// 功能：判断文件夹是否存在
	bool isDirectoryExist(std::string name);
	
	// 功能：判断str是否合法
	bool isValid(::std::string str);

	// 功能：判断str中是否含有通配符
	bool hasWildCard(std::string str);

	// 功能：分析name
	void preParse(std::string name,std::string& directory,std::string& query);

	// 功能：隐藏的构造函数
	Directory(std::string directory,std::string query);	
	
	// 功能: 将Directory的结构输出至文件流
	// 参数：name为文件名，bSub为true时也输出子目录中的内容
	void output(ostream of,bool bSub);

	void buildDirectory(Directory& directory,istream& is);

	Directory();
};

typedef Directory::vector_d vector_d;
typedef Directory::vector_d_itor vector_d_itor;

typedef Directory::vector_f vector_f;
typedef Directory::vector_f_itor vector_f_itor;

#endif // _DIRECTORYINFO_H_

