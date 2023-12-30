
#include <testI.h>
#include <iostream>

using namespace std;

void
Test::TestIfI::TestFn(const ::std::string& s, const Ice::Current& current)
{
	cout << "TestIfI::TestFn()" << s << endl;
}
