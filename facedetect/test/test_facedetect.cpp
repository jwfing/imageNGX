#include "faceDetector.h"
#include "testMacros.h"
#include <iostream>
using namespace std;

class test_facedetect : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_facedetect);
    CPPUNIT_TEST(testFaceCount);
    CPPUNIT_TEST_SUITE_END();
protected:
    void testFaceCount() {
        FaceDetector fd;
        CPPUNIT_ASSERT(fd.init());

        vector<char> from_file;
        FILE *f = fopen("./data/test_faceCount.jpg", "rb");
        CPPUNIT_ASSERT(f);
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        from_file.resize((size_t)len);
        fseek(f, 0, SEEK_SET);
        from_file.resize(fread(&from_file[0], 1, from_file.size(), f));
        fclose(f);

        int x,y,w,h;
        int faceCount = fd.getBestFace((const char*)&from_file[0], (size_t)len,
                                    x, y, w, h);
        CPPUNIT_ASSERT_EQUAL(9, faceCount);
    }
};
START_TEST(test_facedetect);

