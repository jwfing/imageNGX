#include "memoryPool.h"
#include "testMacros.h"
#include "mempool_consumer.h"
#include "misc.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

class test_MemoryPool : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_MemoryPool);
    CPPUNIT_TEST(test_chunk_usage_statistic);
    CPPUNIT_TEST(test_alloc_single_level);
    CPPUNIT_TEST(test_recycle_single_thread);
    CPPUNIT_TEST(test_alloc_multi_level);
    CPPUNIT_TEST(test_alloc_multi_thread);
    CPPUNIT_TEST(test_string_reverse);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_string_reverse() {
        string original = "this is from meiwei.fm";
        std::reverse(original.begin(), original.end());
        cout << original << endl;
    }
    void test_chunk_usage_statistic() {
        int64_t time1 = get_cur_microseconds_time();
        ChunkUsageStatistic statistic;
        statistic.incCounter();
        statistic.incCounter();
        statistic.decCounter();
        statistic.incCounter();
        statistic.setTimestamp(time1/1000000);
        CPPUNIT_ASSERT_MESSAGE("counter failed", statistic.getCounter() == 2);
        CPPUNIT_ASSERT_MESSAGE("time failed", (uint64_t)time1/1000000 == statistic.getTimestamp());
    }
    void test_recycle_single_thread() {
        MemoryPool pool(32*1024*1024, false, 60);
        MP_ID id = pool.alloc(1024*4);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 4k size", id != MP_NULL);
        int64_t len = 0;
        void* addr = pool.getElementAddress(id, len);
        CPPUNIT_ASSERT_MESSAGE("failed to getAddress", ((addr != NULL) && (len > 4*1024)));
        memset(addr, 0, len);
        pool.freeNode(id);
        pool.garbageCollect();
        sleep(150);
        pool.garbageCollect();
        id = pool.alloc(3*1024);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 3k size", id != MP_NULL);
        addr = pool.getElementAddress(id, len);
        memset(addr, 0, len);
        CPPUNIT_ASSERT_MESSAGE("failed to getAddress", ((addr != NULL) && (len > 4*1024)));
        pool.freeNode(id);
    };
    void test_alloc_single_level() {
        MemoryPool pool(32*1024*1024);
        MP_ID id = pool.alloc(1024*4);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 4k size", id != MP_NULL);
        int64_t len = 0;
        void* addr = pool.getElementAddress(id, len);
        CPPUNIT_ASSERT_MESSAGE("failed to getAddress", ((addr != NULL) && (len > 4*1024)));
        pool.freeNode(id);
        id = pool.alloc(3*1024);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 3k size", id != MP_NULL);
        addr = pool.getElementAddress(id, len);
        CPPUNIT_ASSERT_MESSAGE("failed to getAddress", ((addr != NULL) && (len > 4*1024)));
        pool.freeNode(id);
    }
    void test_alloc_multi_level() {
        MemoryPool pool(32*1024*1024);
        MP_ID id = pool.alloc(1024*4);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 4k size", id != MP_NULL);
        pool.freeNode(id);
        id = pool.alloc(13*1024);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 13k size", id != MP_NULL);
        pool.freeNode(id);
    }
    void test_alloc_multi_thread() {
        MemoryPool pool(320*1024*1024);
        MempoolConsumer* first = new MempoolConsumer(&pool, 32*1024);
        MempoolConsumer* second = new MempoolConsumer(&pool, 128*1024);
        MempoolConsumer* three = new MempoolConsumer(&pool, 1024*1024);
        first->start();
        second->start();
        three->start();
        sleep(120);
        first->join();
        second->join();
        three->join();
        delete first;
        delete second;
        delete three;
    }
};

START_TEST(test_MemoryPool);
