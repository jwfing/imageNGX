#include "imagickMontage.h"
#include "testMacros.h"
#include <iostream>
#include <list>

using namespace Magick;
using namespace std;

class test_imagickMontage : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_imagickMontage);
    CPPUNIT_TEST(testMontage);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testMontage() {
        Image image1;
        Blob blob1, blob2, blob3, blob4;
        ImagickMontage montager;
        const char* images[4];
        size_t lenOfSrcs[4];
        char* desc;
        desc = new char[1024*1024];
        image1.read("./data/image-part-1.jpg");
        image1.write(&blob1);
        images[0] = (const char*)blob1.data();
        lenOfSrcs[0] = blob1.length();
        //list<Image> images;
        //list<Image> montage;
        //images.push_back(image1);

        image1.read("./data/image-part-2.jpg");
        image1.write(&blob2);
        images[1] = (const char*)blob2.data();
        lenOfSrcs[1] = blob2.length();
        //images.push_back(image1);

        image1.read("./data/image-part-3.jpg");
        image1.write(&blob3);
        images[2] = (const char*)blob3.data();
        lenOfSrcs[2] = blob3.length();
        //images.push_back(image1);

        image1.read("./data/image-part-4.jpg");
        image1.write(&blob4);
        images[3] = (const char*)blob4.data();
        lenOfSrcs[3] = blob4.length();
        string color = "red";
        int ret = montager.montage(images, lenOfSrcs, 4, desc, 1024*1024, 100, 100, 2, 2, color);
        CPPUNIT_ASSERT_EQUAL(23413, ret);
        cout << ret << endl;
        /*
        FILE *fp = fopen("./data/montage_output.jpg", "w");
        fwrite (desc, 1, ret, fp);
        fclose(fp);
        */
    }
};

START_TEST(test_imagickMontage);
