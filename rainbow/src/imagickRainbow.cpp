#include "imagickRainbow.h"
#include "errorCode.h"
#include "misc.h"
#include <Magick++.h>
#include <sstream>

using namespace Magick;
using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ImagickRainbow::_logger = \
    Logger::getLogger("leancloud.backend.imageService.imagickRainbow");

ImagickRainbow::ImagickRainbow() {
    resetColors();
}

ImagickRainbow::~ImagickRainbow() {
}

void ImagickRainbow::resetColors() {
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

void ImagickRainbow::initColors(std::string colors) {
    /*
    char delimiter = ',';
    stringstream ss(colors);
    string tmp;
    _colors.clear();
    while (getline(ss, tmp, delimiter)) {
        if (tmp.length() > 0) {
            _colors.push_back(tmp);
        }
    }
    if (_colors.empty()) {
        resetColors();
    }
    LOG4CXX_INFO(_logger, "init pannel with " << _colors.size() << "
    colors");
    */
}

int ImagickRainbow::createRainbow(int height,
                                  std::vector<int>& clips,
                                  char* desc,
                                  size_t lenOfDesc) {
    try {
        LOG4CXX_INFO(_logger, "ready to crete rainbow");
        int width = 0;
        int numOfClips = (int)clips.size();
        for (int i = 0; i < numOfClips; ++i) {
            width += clips[i];
            LOG4CXX_INFO(_logger, "length: " << clips[i]);
        }
        char tmp[64];
        sprintf(tmp, "%dx%d", width, height);
        Image image(tmp, "white");
        int offset = 0;
        LOG4CXX_INFO(_logger, "colors: " << _colors.size());
        for (int i=0; i < numOfClips; i++) {
            image.fillColor(_colors[i % _colors.size()]);
            image.draw(DrawableRectangle(offset, 0, offset + clips[i], height));
            offset += clips[i];
        }
        image.magick("gif");
        Blob descBlob;
        image.write(&descBlob);
        if (descBlob.length() > lenOfDesc) {
            LOG4CXX_ERROR(_logger, "Buffer is not enough:" <<
                          "Need: " << descBlob.length() <<
                          "But given: " << lenOfDesc);
            return EC_BUFFER_NOT_ENOUGH;
        }
        memcpy(desc, descBlob.data(), descBlob.length());
        int imageSize = (int) descBlob.length();
        LOG4CXX_INFO(_logger, "finish to crete rainbow");
        return imageSize;
    }
    catch (WarningCoder &warning) {
        LOG4CXX_ERROR(_logger, "Coder Warning: " << warning.what());
        return EC_CONVERT_FAILED;
    }
    catch (Warning &warning) {
        // Handle any other Magick++ warning.
        LOG4CXX_ERROR(_logger, "Warning: " << warning.what());
        return EC_CONVERT_FAILED;
    }
    catch (ErrorBlob &error) {
        // Process Magick++ file open error
        LOG4CXX_ERROR(_logger, "Error: " << error.what());
        return EC_CONVERT_FAILED;
    }
    catch (ErrorCorruptImage &error) {
        LOG4CXX_ERROR(_logger, "Error: " << error.what());
        return EC_CONVERT_FAILED;
    }
    catch (Error &error) {
        LOG4CXX_ERROR(_logger, "Error: " << error.what());
        return EC_CONVERT_FAILED;
    }
    catch (...) {
        LOG4CXX_ERROR(_logger, "Error");
        return EC_CONVERT_FAILED;
    }
}

