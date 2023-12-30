#ifndef SLAB_POOL_TEST_H
#define SLAB_POOL_TEST_H

#include <iostream>
#include "SlabPoolDefine.h"
#include "SlabPool.h"
#include <TimeUtil.h>
#include <NativeThreadPool.h>
#include <assert.h>

//extern void singleThreadTest(size_t size, size_t number);
//extern void multiThreadTest(size_t threadNumber, size_t size, size_t number, size_t increment);

class Test
{
public:
    Test(){}
    virtual ~Test(){}
    SLABPOOL_DEFINE(Test)
private:
    char str[1024*100];
};

class ConcurrentTest : public ZQ::common::ThreadRequest
{
public:
    ConcurrentTest(ZQ::common::NativeThreadPool& pool, int64 id, size_t size, size_t number, size_t increment)
        : ZQ::common::ThreadRequest(pool), _id(id), _size(size), _number(number), _increment(increment)
    {}
    virtual ~ConcurrentTest(){}

    virtual int run(void) {
        int64 startTime = ZQ::common::TimeUtil::now();
        int64 allocSize = _size;
        for (size_t i=0; i<_number; i++)
        {
            /*Test* t = new Test();
            delete t;*/
            Slot::Ptr slotPtr = gSlabPool.getSlot(allocSize);
            void*p = slotPtr->getSlab();
            assert(p);
            slotPtr->free(p);
            if (i % 100 == 0)
            {
                allocSize += _increment;
            }
        }
        int64 endTime = ZQ::common::TimeUtil::now();
        int64 usedTime = endTime - startTime;
        std::cout<<"["<<_id<<"] size["<<_size<<"] "<<_number<<" times, used time: "<<usedTime<<"ms"<<std::endl;
        return 0;
    }

private:
    int64 _id;
    int64 _size;
    size_t  _number;
    size_t _increment;
    //char*   _c;
};


#endif //SLAB_POOL_TEST_H
