#include "config.h"
#include "testMacros.h"

#include <iostream>
using namespace std;

class test_config : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_config);
    CPPUNIT_TEST(test_read);
    CPPUNIT_TEST(test_write);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_read() {
        // do something
        Config config("./config.dat");
        int startX = config.read<int>("startX");
        string cmd = config.read<string>("command");
        cout << "cmd: " << cmd << ", and startX:" << startX << endl;
    }
    void test_write() {
    }
};

START_TEST(test_config);

