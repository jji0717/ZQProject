
#include "../alarminclude.h"

#define MAX_TRIGGER_COUNT 256

int main(void)
{
	using namespace ZQ::ALARM;

	INIDataSource source("c:\\sample.ini");

	//create implementation instance
	Implementation impl("D:\\vss\\ZQProjs\\Generic\\Alarm\\test\\testImpl.dll", source);

	//create syntax parser
	RegularParser sp(source);
	//add implementation into parser
	sp.addImpl(impl);

	//--create triggerlist--
	TriggerList list;

	SCLogTrigger* pTrigger[MAX_TRIGGER_COUNT] = {0};

	//--add triggers into list--
	for (size_t i = 0; i < source.countSource(); ++i)
	{
		pTrigger[i] = new SCLogTrigger(source.listSource(i).c_str(), sp, i);
		list.push_back(pTrigger[i]);
	}

	//start trigger
	list.startTrigger();

	system("pause");

	//stop trigger
	list.stopTrigger();

	for (i = 0; i < list.size(); ++i)
	{
		delete list[i];
	}

	return 0;
}
