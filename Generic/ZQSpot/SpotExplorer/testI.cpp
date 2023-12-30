
#include "testI.h"
#include <iostream>
#include <atlbase.h>

using namespace std;

void
Test::TestIfI::TestFn(const ::std::string& s, const Ice::Current& current)
{
	AtlTrace("TestIfI::TestFn():%s\n", s.c_str()) ;
}
