#include <iostream>
#include <CloneIndexFile.h>
#include <FileLog.h>

void test1()
{
	ZQ::common::FileLog fileLog("d:\\log\\CloneIndexFile.log", 7);
	ZQ::IdxParser::IndexFileClone indexClone(fileLog, "D:\\project\\index\\SEAC0000000000000001schange.com.index", "D:\\project\\index\\my.index");
	indexClone.replaceSubFile("0x0008049C295315C1", "0x0008049C295315D1");
	indexClone.replaceSubFile("0x0008049C295315C3", "0x0008049C295315D3");
	indexClone.replaceSubFile("0x0008049C295315C8", "0x0008049C295315D8");
	indexClone.clone();
}

void test2()
{
	ZQ::common::FileLog fileLog("d:\\log\\CloneIndexFile.log", 7);
	ZQ::IdxParser::IndexFileClone indexClone(fileLog, "D:\\project\\index\\SEAC0000000000000001schange.com.index", "D:\\project\\index\\my.index");
	indexClone.clone();
}

int main()
{
	test1();
	system("pause");
	return 0;
}