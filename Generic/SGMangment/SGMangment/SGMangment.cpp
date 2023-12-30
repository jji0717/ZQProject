// SGMangment.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServiceGroupManage.h"
#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

#define num 50

int _tmain(int argc, _TCHAR* argv[])
{
	namespace aa = ZQTianShan::EdgeRM;
	ZQTianShan::EdgeRM::ServiceGroupManage SGtest;
	
	char ch[num];
	char *buf;
	string str;
	ifstream ifile("range_number.txt");

	if( !ifile.is_open() )
		cout<<"error:ifile"<<endl;

	while(ifile.getline( ch ,num ))
	{
		str += ch;
		str += '\n';
	}

//	cout<<"str:"<<str.data()<<endl;
	buf = (char *)str.c_str();
//	cout<<"buf:"<<buf<<endl;
	if( SGtest.addSGs( buf ,(int)strlen(buf),'-') )
	{
		cout<<"addSGs( char*, int, char ):true"<<endl;
	}

	if( SGtest.addSGs(3,3) ) 
	{
		cout <<"addSGs( int, int ):true"<<endl;
	}

	if( SGtest.addSGs( 6000 ) )
	{
		cout <<"addSGs( int ):true"<<endl;
	}

	if( SGtest.remove(201,299) )
	{
		cout <<"remove:true"<<endl;
	}

	SGtest.reset();

	if( SGtest.find(655) )
	{
		cout <<"find:true"<<endl;
	}


	if( SGtest.find(8000) )
	{
		cout <<"find:true"<<endl;
	}

	if( SGtest.find(49) )
	{
		cout <<"find:true"<<endl;
	}

	system("Pause");

	return 0;
}

