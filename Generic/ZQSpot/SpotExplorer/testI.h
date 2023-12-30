#ifndef __testI_h__
#define __testI_h__

#include <test.h>

namespace Test
{

class TestIfI : virtual public TestIf
{
public:

    virtual void TestFn(const ::std::string& s, const Ice::Current&);
};

}

#endif
