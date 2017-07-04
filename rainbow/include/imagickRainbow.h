#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_RAINBOW_H_
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_RAINBOW_H_

#include <iostream>
#include <string>
#include <list>
#include <vector>

#include <Magick++.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

class ImagickRainbow {
 public:
    ImagickRainbow();
    ~ImagickRainbow();
 public:
    void initColors(std::string colors);
    int createRainbow(int height,
                      std::vector<int>& clips,
                      char* desc,
                      size_t lenOfDesc);
 private:
    void resetColors();
 private:
    std::vector<Magick::ColorRGB> _colors;
    static log4cxx::LoggerPtr _logger;
};

#endif //_CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_RAINBOW_H_
