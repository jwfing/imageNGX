#include "memoryPool.h"
#include "testMacros.h"
#include "misc.h"

#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

class test_MemoryPool : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_MemoryPool);
    CPPUNIT_TEST(test_alloc_single_level);
    CPPUNIT_TEST(test_alloc_multi_level);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_alloc_single_level() {
        MemoryPool pool(32*1024*1024);
        MP_ID id = pool.alloc(1024*4);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 4k size", id != MP_NULL);
        int64_t len = 0;
        void* addr = pool.getElementAddress(id, len);
        CPPUNIT_ASSERT_MESSAGE("failed to getAddress", ((addr != NULL) && (len > 4*1024)));
        pool.freeNode(id);
        id = pool.alloc(7*1024*1024);
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
        id = pool.alloc(13*1024*1024);
        CPPUNIT_ASSERT_MESSAGE("failed to alloc element with 13k size", id == MP_NULL);
    }
};

START_TEST(test_MemoryPool);
