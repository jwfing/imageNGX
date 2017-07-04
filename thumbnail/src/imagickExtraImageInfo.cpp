#include "imagickExtraImageInfo.h"
#include "misc.h"
#include <iostream>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

LoggerPtr ImagickExtraImageInfo::_logger = \
        Logger::getLogger("leancloud.backend.imageService.imagickExtraImageInfo");

ImagickExtraImageInfo::ImagickExtraImageInfo() {
    _hasFaces = false;
}


ImagickExtraImageInfo::~ImagickExtraImageInfo() {
}

void ImagickExtraImageInfo::extractPrimaryColor(Magick::Image* image) {
    // first quantize image
    image->quantizeColorSpace(Magick::YIQColorspace);
    image->quantizeColors(24);
    image->quantize();

    map<Magick::Color,unsigned long> histogram;
    Magick::colorHistogram( &histogram, *image );
    std::map<Magick::Color,unsigned long>::const_iterator p=histogram.begin();
    double maxScore = 0;
    const Magick::Color* popularColor = NULL;
    while (p != histogram.end()) {
        int count = (int)p->second;
        Magick::ColorHSL hsl(p->first);
        //double score = count * (hsl.hue() * 0.5 + hsl.saturation() * 0.5 + hsl.luminosity() * 0);
        double score = count;
        if (maxScore < score) {
            maxScore = score;
            popularColor = &(p->first);
        }
        p++;
    }

    if (popularColor != NULL) {
        Magick::ColorRGB colorRGB(*popularColor);
        //cout << "\n" << maxScore << " " << colorRGB.red() << "," << colorRGB.green() << "," << colorRGB.blue() << endl;
        _primaryColor.r = (int)(colorRGB.red() * 255);
        _primaryColor.g = (int)(colorRGB.green() * 255);
        _primaryColor.b = (int)(colorRGB.blue() * 255);
    }
}

void ImagickExtraImageInfo::doGet(void *image) {
    Magick::Image* imagickImage = (Magick::Image*) image;
    // get primary info according to histogram
    extractPrimaryColor(imagickImage);
}

string ImagickExtraImageInfo::getResult() {
    char buffer[256] = {0};
    snprintf(buffer, 256, "\"color\":\"(%u,%u,%u)\", \"hasFaces\":\"%s\"", _primaryColor.r, _primaryColor.g, _primaryColor.b, _hasFaces ? "true":"false");
    return string(buffer);
}
