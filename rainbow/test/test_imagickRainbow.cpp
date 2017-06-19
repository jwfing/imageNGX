#include "testMacros.h"
#include <Magick++.h>
#include <iostream>
#include <vector>
#include <string>

using namespace Magick;
using namespace std;

class test_imagickRainbow : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_imagickRainbow);
    CPPUNIT_TEST(testRainbow);
    CPPUNIT_TEST(testRainbowWithImage);
    CPPUNIT_TEST(testBlur);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testBlur() {
        Image image("./data/blur.jpeg");
        image.blur(0, 13);
        image.write("./data/blur-result.jpeg");
    }

  void testRainbowWithImage() {
        vector<string> colors;
        colors.push_back("blue");
        colors.push_back("green");
        colors.push_back("purple");
        colors.push_back("red");
        colors.push_back("DarkBlue");
        colors.push_back("white");
        colors.push_back("yellow");

        vector<int> clips;
        for (int i = 0 ; i < 18; i++) {
            clips.push_back(5);
        }
        int height = 2;
        Image image("./data/youtube-logo.png");
        int offset = 0;
        int imageHeight = 72;
        for (unsigned int i=0; i < clips.size(); i++) {
            image.fillColor(colors.at(i%colors.size()).c_str());
            image.draw(DrawableRectangle(offset, imageHeight - height, offset + clips[i], imageHeight));
            offset += clips[i];
        }
        image.write( "./data/image-with-rainbow.png");
    }

    void testRainbow() {
        vector<string> colors;
        colors.push_back("blue");
        colors.push_back("green");
        colors.push_back("purple");
        colors.push_back("red");
        colors.push_back("DarkBlue");
        colors.push_back("white");
        colors.push_back("yellow");

        vector<int> clips;
        for (int i = 0 ; i < 40; i++) {
            clips.push_back(20);
        }
        int width = 0;
        int height = 5;
        for (unsigned int i=0; i < clips.size(); i++) {
            width += clips[i];
        }
        char tmp[64];
        sprintf(tmp, "%dx%d", width, height);
        Image image(tmp, "white");
        int offset = 0;
        for (unsigned int i=0; i < clips.size(); i++) {
            image.fillColor(colors.at(i%colors.size()).c_str());
            image.draw(DrawableRectangle(offset, 0, offset + clips[i], height));
            offset += clips[i];
        }
        image.magick("gif");
        image.write( "./data/red_pixel.gif");
    }
};

START_TEST(test_imagickRainbow);
