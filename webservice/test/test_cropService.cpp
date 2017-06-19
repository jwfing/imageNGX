#include "cropService.h"
#include "testMacros.h"

#include <iostream>
using namespace std;

class test_cropService : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_cropService);
    CPPUNIT_TEST(test_queryString);
    CPPUNIT_TEST(test_writeResponse);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_queryString() {
        // do something
        const string query = "urlMD5=abhieifehife&x=98&y=9876&w=870&h=150";
        string md5;
        string domain = "";
        int w, h, x, y, x2, y2;
        CropService::parseQueryString(query, md5, domain, w, h, x, y, x2, y2);
        cout << "queryString=" << query << endl;
        cout << "\tmd5=" << md5 << endl;
        cout << "\tw=" << w << endl;
        cout << "\th" << h << endl;
        cout << "\tx=" << x << endl;
        cout << "\ty=" << y << endl;
        cout << "\tx2=" << x2 << endl;
        cout << "\ty2=" << y2 << endl;
    }
    void test_writeResponse() {
    }
};

START_TEST(test_cropService);

