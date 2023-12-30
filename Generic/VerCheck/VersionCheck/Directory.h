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
	
	// ���ܣ�����һ��Directory����, name������һ��ʵ�ʴ��ڵ��ļ���
	// �׳��쳣����name�����ļ��е�ʱ���׳��쳣
	Directory(std::string userString);

	// ���ܣ�����Directory����
	virtual ~Directory();

	// ���ܣ������ļ��еĽṹ�������е��ļ�(File)������ļ���(Directory)����
	// �����vector��
	void parse(bool bSub = true);

	// ���ܣ���ʾ�ļ��еĽṹ
	// ������bSub�Ƿ���ʾ��Ŀ¼��Ϣ��Ĭ��Ϊ��ʾ
	void print(bool bSub = true);

	// ���ܣ���������Ŀ¼
	void setDirectory(std::string name);

	// ���ܣ�����ļ��е����ƣ�������·��
	std::string getName();

	// ���ܣ�����ļ��е����ƣ�����·��
	std::string getFullName();

	// ���ܣ���õ�ǰ�ļ��µ�ĳһ���ļ��ж����ָ��
	Directory* getDirectory(std::string name);

	// ���ܣ���õ�ǰ�ļ����µ�ĳһ���ļ������ָ��
	File* getFile(std::string name);

	// ����: ��Directory�Ľṹ������ļ���
	// ������nameΪ�ļ�����bSubΪtrueʱҲ�����Ŀ¼�е�����
	void output(std::string name,bool bSub=true);

	// ���ܣ�����ָ����
	void check(std::string dname,std::string fname,bool bSub = true);

	// ���ܣ���մ��Directory��vector��File��vector��������Ŀ¼
	void free();

public:

	// �Զ������ͣ����Directory����ָ���vector����iterator
	typedef std::vector<Directory*>	vector_d;
	typedef vector_d::iterator vector_d_itor;

	// �Զ������ͣ����File����ָ���vector����iterator
	typedef std::vector<File*> vector_f;
	typedef vector_f::iterator vector_f_itor;

protected:
	// ��ŵ�ǰ�ļ���������Directory����ָ���vector
	vector_d m_vectorDirectory;

	// ��ŵ�ǰ�ļ���������File����ָ���vector
	vector_f m_vectorFile;

	//��ǰ�ļ��е����ƣ�����·����Ϣ
	std::string m_directory;
	std::string m_query;
	bool bSubDirectoryScaned;
	
public:	
	// ���ܣ���ô��Directory�����vector������
	vector_d& getDirectoryVector();

	// ���ܣ���ô��File�����vector������
	vector_f& getFileVector();

protected:
	// ���ܣ��ж��ļ����Ƿ����
	bool isDirectoryExist(std::string name);
	
	// ���ܣ��ж�str�Ƿ�Ϸ�
	bool isValid(::std::string str);

	// ���ܣ��ж�str���Ƿ���ͨ���
	bool hasWildCard(std::string str);

	// ���ܣ�����name
	void preParse(std::string name,std::string& directory,std::string& query);

	// ���ܣ����صĹ��캯��
	Directory(std::string directory,std::string query);	
	
	// ����: ��Directory�Ľṹ������ļ���
	// ������nameΪ�ļ�����bSubΪtrueʱҲ�����Ŀ¼�е�����
	void output(ostream of,bool bSub);

	void buildDirectory(Directory& directory,istream& is);

	Directory();
};

typedef Directory::vector_d vector_d;
typedef Directory::vector_d_itor vector_d_itor;

typedef Directory::vector_f vector_f;
typedef Directory::vector_f_itor vector_f_itor;

#endif // _DIRECTORYINFO_H_

