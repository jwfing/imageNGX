#include "imagicImageOperator.h"
#include "testMacros.h"
#include <Magick++.h>

using namespace Magick;
using namespace std;

class test_imagicImageOperator : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_imagicImageOperator);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

protected:
    void test() {
        ImageOperator* opers = new ImagicImageOperator();
        CPPUNIT_ASSERT_EQUAL(-1, opers->getQuality());
        CPPUNIT_ASSERT_EQUAL(-1.0, opers->getHue());
        CPPUNIT_ASSERT_EQUAL(-1.0, opers->getSaturation());
        CPPUNIT_ASSERT_EQUAL(-1.0, opers->getBrightness());

        Image image1, image2;
        image1.read("./data/groundtruth_thumbnail_1.jpg");
        image2.read("./data/groundtruth_thumbnail_1.jpg");

        opers->setQuality(50);
        opers->doRun(&image2);
        image2.write("./data/test_opers1.jpg");
        image2.read("./data/test_opers1.jpg");
        CPPUNIT_ASSERT(image1.fileSize() > image2.fileSize());
        /*
        image2.read("./data/groundtruth_thumbnail_1.jpg");
        opers->setQuality(100);
        opers->setSaturation(120);
        opers->setBrightness(120);
        opers->doRun(&image2);
        image2.write("./data/test_opers1.jpg");
        */
    }
};

START_TEST(test_imagicImageOperator);
