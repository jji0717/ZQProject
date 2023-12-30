
#include "../alarminclude.h"

using namespace ZQ::ALARM;

void testReadLine()
{
	HANDLE	hLogFile = ::CreateFileA("c:\\icm.log",
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	while (true)
	{
		char strLine[512] = {0};
		int line_count = SCLogTrigger::readLine(hLogFile, strLine, 512);

		if (line_count>0)
			printf("[%d]%s\n", line_count, strLine);
	}

	::CloseHandle(hLogFile);
}

void testReadLogLine()
{
	HANDLE	hLogFile = ::CreateFileA("c:\\icm.log",
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	while (true)
	{
		char strLine[512] = {0};
		int line_count = SCLogTrigger::readLogFileLine(hLogFile, strLine, 512);

		if (line_count>0)
			printf("[%d]%s\n", line_count, strLine);
	}

	::CloseHandle(hLogFile);
}

int main(void)
{
	testReadLogLine();

	system("pause");
	return 0;
}

