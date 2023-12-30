

#include "SubjectWrite.h"
#include "FakeObserverWrite.h"


#include "CppUTest/TestHarness.h"
#include "SubjectWrite.h"

using namespace ZQTianShan::ContentProvision;


TEST_GROUP(SubjectWrite)
{
	SubjectWrite*	_subject;

	void setup()
	{
		_subject = new SubjectWrite();
	}
	void teardown()
	{
		delete _subject;
	}
};

TEST(SubjectWrite, Register)
{
	ObserverWriteI* pObserver[10];
	for(int i=0;i<10;i++)
	{
		pObserver[i] = new FakeObserverWrite();
		_subject->registerObserver(pObserver[i]);
	}

	std::string strFile = "hhhx";
	_subject->notifyObserverWrite(strFile, 0, 0);

	_subject->notifyObserverDestroy();
	for(int i=0;i<10;i++)
	{
		_subject->removeObserver(pObserver[i]);
		delete pObserver[i];
	}

	CHECK_EQUAL(_subject->getObserverCount(), 0);
}

