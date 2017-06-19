#include "thumbnailerFactory.h"
#include "imagickExtraImageInfo.h"
#include "testMacros.h"
#include <Magick++.h>

using namespace Magick;
using namespace std;

class test_thumbnailer : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_thumbnailer);
    // CPPUNIT_TEST(testThumbnailWithFaceDetect);
    CPPUNIT_TEST(testLoadImage);
    CPPUNIT_TEST(testGetPrimaryColor);
    CPPUNIT_TEST_SUITE_END();
  protected:
    void testLoadImage() {
        /*
        Thumbnailer* thumbnailer = ThumbnailerFactory::Create("imagick");
        thumbnailer->testLoadImage("/home/tangxm/avos_image/thumbnail/test/data/test_png3.png");
        */
        Image image;
        image.read("./data/test_png3.png");
    }

    void testGetPrimaryColor() {
        Image image;
        image.read("./data/abc.png");
        ImagickExtraImageInfo extraImageInfo;
        extraImageInfo.doGet(&image);
        cout << extraImageInfo.getPrimaryColor().r << "," \
             << extraImageInfo.getPrimaryColor().g << "," \
             << extraImageInfo.getPrimaryColor().b << "," << endl;
    }

    void testThumbnailWithFaceDetect() {

        Thumbnailer* thumbnailer =  ThumbnailerFactory::Create("imagick");
        Rect rect(0, 0, 0, 0);
        Image orgImage("./data/test_thumbnail_with_facedetect.jpg");
        Blob orgBlob;
        orgImage.write(&orgBlob);
        Image descImage("./data/test_thumbnail_with_facedetect.jpg");
        Blob descBlob;
        descImage.write(&descBlob);
        thumbnailer->create((const char*)orgBlob.data(), orgBlob.length(), (char*)descBlob.data(), descBlob.length(), rect, 225, 150);

        //write to file
        descImage.read(descBlob);
        descImage.write("./data/groundtruth_thumbnail_1.jpg");

        descImage.read("./data/groundtruth_thumbnail_1.jpg");
        descImage.write(&descBlob);

        Image groundTruthImage;
        groundTruthImage.read("./data/groundtruth_thumbnail_with_facedetect.jpg");
        Blob groundTruthBlob;
        groundTruthImage.write(&groundTruthBlob);

        CPPUNIT_ASSERT_EQUAL(groundTruthBlob.length(),
                            descBlob.length());

        delete thumbnailer;
    }
};

START_TEST(test_thumbnailer);
