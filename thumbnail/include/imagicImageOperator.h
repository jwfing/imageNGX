#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGIC_IMAGE_OPERATOR_H_
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGIC_IMAGE_OPERATOR_H_

#include <vector>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "imageOperator.h"
#include <Magick++.h>

class ImagicImageOperator : public ImageOperator {
public:
    ImagicImageOperator();
    virtual ~ImagicImageOperator();

private:
    void exposure(Magick::Image* image);
    void drawRainbow(Magick::Image* image);

public:
    virtual void doRun(void* image);

private:
    std::vector<Magick::ColorRGB> _colors;
    static log4cxx::LoggerPtr _logger;
};

#endif // end of _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGIC_IMAGE_OPERATOR_H_
