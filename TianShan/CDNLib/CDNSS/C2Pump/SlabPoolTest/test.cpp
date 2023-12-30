#include "test.h"
#include <gtest/gtest.h>

ZQ::common::NativeThreadPool gThreadPool(100);

void singleThreadTest(size_t size, size_t number)
{
    int64 startTime = ZQ::common::TimeUtil::now();
    int64 index = number;
    while(index--)
    {
        /*Test* t = new Test();
        delete t;*/
        Slot::Ptr slotPtr = gSlabPool.getSlot(size);
        void*p = slotPtr->getSlab();
        slotPtr->free(p);
    }
    int64 endTime = ZQ::common::TimeUtil::now();
    printf("%ld times, alloc size[%ld] used time: %ld\n", number, size, endTime - startTime);
}

void multiThreadTest(size_t threadNumber, size_t size, size_t number, size_t increment)
{
    size_t index = 0;
    while(index++ < threadNumber)
    {
        ConcurrentTest* t = new ConcurrentTest(gThreadPool, index, size, number, increment);
        t->start();
    }
}

TEST(SingleThreadTest, sameClass)
{
    size_t size = 5;
    size_t number = 10000000;
    int64 count = 1;
    while(count--)
    {
        EXPECT_NO_THROW(singleThreadTest(size, number));
    }
}

TEST(SingleThreadTest, diffClass)
{
    size_t size = 5;
    size_t number = 1000000;
    size_t increment = 9;
    int64 count = 100;
    while(count--)
    {
        EXPECT_NO_THROW(singleThreadTest(size, number));
        size += increment;
    }
}

TEST(MultiThreadTest, sameClass)
{
    size_t threadNumber = 100;
    size_t size = 5;
    size_t number = 100000;
    size_t increment = 0;
    EXPECT_NO_THROW(multiThreadTest(threadNumber, size, number, increment));
}

TEST(MultiThreadTest, diffClass)
{
    size_t threadNumber = 24;
    size_t size = 5;
    size_t number = 100000;
    size_t increment = 1;
    EXPECT_NO_THROW(multiThreadTest(threadNumber, size, number, increment));
}
