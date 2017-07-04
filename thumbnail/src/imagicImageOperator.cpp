#include "imagicImageOperator.h"

#include <cstring>
#include <sstream>

using namespace Magick;
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

LoggerPtr ImagicImageOperator::_logger = \
    Logger::getLogger("leancloud.backend.imageService.ImagicImageOperator");

ImagicImageOperator::ImagicImageOperator() {
    _colors.clear();
    _colors.push_back(ColorRGB(249/255.0, 251/255.0, 0/255.0));//yellow
    _colors.push_back(ColorRGB(0, 255/255.0, 253/255.0));//blue
    _colors.push_back(ColorRGB(59/255.0, 251/255.0, 4/255.0));//green
    _colors.push_back(ColorRGB(205/255.0, 151/255.0, 247/255.0));//purple
    _colors.push_back(ColorRGB(255/255.0, 0/255.0, 24/255.0));//red
    _colors.push_back(ColorRGB(0/255.0, 104/255.0, 255/255.0));//DarkBlue
    _colors.push_back(ColorRGB(224/255.0, 224/255.0, 224/255.0));//white
    LOG4CXX_INFO(_logger, "init pannel with " << _colors.size() << " colors");
}

ImagicImageOperator::~ImagicImageOperator() {
}

void ImagicImageOperator::doRun(void* image) {
    Image* imagicImage = (Image*) image;
    // change format;
    if (strlen(_fmt.c_str()) > 0) {
        imagicImage->magick(_fmt.c_str());
    }

    if (_quality >= 0 && _quality <= 100) {
        imagicImage->quality(_quality);
    }

    // change hue
    double hue, saturation, brightness;
    hue = saturation = brightness = 100;
    if (_hue >= 0 && _hue <= 200) {
        hue = _hue;
    }
    if (_saturation >= 0 && _saturation <= 200) {
        saturation = _saturation;
    }
    if (_brightness >= 0 && _brightness <= 200) {
        brightness = _brightness;
    }

    // change exposure
    imagicImage->modulate(brightness, saturation, hue);
    if (_exposure >= -100 && _exposure <= 100) {
        exposure(imagicImage);
    }

    // blur adult images
    if (_blur >= 0 && _blur <= 100) {
        imagicImage->blur(0, _blur);
    }

    // draw rainbow
    if (!_rainbows.empty() && _rainbowHeight > 0) {
        drawRainbow(imagicImage);
    }
}

void ImagicImageOperator::drawRainbow(Image* image) {
    // parse clips
    vector<int> clips;
    char delimiter = ',';
    stringstream ss(_rainbows);
    string tmp;
    while (getline(ss, tmp, delimiter)) {
        if (tmp.length() > 0) {
            int clipLength = atoi(tmp.c_str());
            if (clipLength > 0)
                clips.push_back(clipLength);
        }
    }

    Geometry imageGeo = image->size();
    int imageHeight = (int)imageGeo.height();
    int offset = 0;
    int numOfClips = (int)clips.size();
    for (int i=0; i < numOfClips; i++) {
        image->fillColor(_colors[i % _colors.size()]);
        image->draw(DrawableRectangle(offset, imageHeight-_rainbowHeight,
                                      offset + clips[i], imageHeight));
        offset += clips[i];
    }
}

void ImagicImageOperator::exposure(Image* image) {
    int exposure = (int) _exposure;
    if (exposure > 0) {
        exposure = 100 - exposure;
    } else {
        exposure = 100 + exposure;
    }

    char temp[64];
    sprintf(temp, "0%%x%d%%", exposure);

    MagickCore::MagickRealType
        black_point,
        gamma,
        white_point;

    MagickCore::MagickStatusType
        flags;

    MagickCore::GeometryInfo
        geometry_info;

    /*
      Parse levels.
    */
    flags = MagickCore::ParseGeometry(temp, &geometry_info);
    black_point = geometry_info.rho;
    white_point = (MagickCore::MagickRealType) QuantumRange;
    if ((flags & MagickCore::SigmaValue) != 0)
        white_point = geometry_info.sigma;
    gamma = 1.0;
    if ((flags & MagickCore::XiValue) != 0)
        gamma = geometry_info.xi;
    if ((flags & MagickCore::PercentValue) != 0)
        {
            black_point *= (MagickCore::MagickRealType) (QuantumRange/100.0);
            white_point *= (MagickCore::MagickRealType) (QuantumRange/100.0);
        }
    if ((flags & MagickCore::SigmaValue) == 0)
        white_point = (MagickCore::MagickRealType) QuantumRange - black_point;
    if (_exposure < 0) {
        LevelizeImage(image->image(), black_point, white_point, gamma);
    } else {
        image->level(black_point, white_point, gamma);
    }
}
