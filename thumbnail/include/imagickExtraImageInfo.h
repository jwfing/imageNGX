#ifndef AVOS_BACKEND_IMAGE_SERVICE_IMAGICK_EXTRA_IMAGE_INFO_H
#define AVOS_BACKEND_IMAGE_SERVICE_IMAGICK_EXTRA_IMAGE_INFO_H

#include "extraImageInfo.h"

#include <Magick++.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

class ImagickExtraImageInfo : public ExtraImageInfo {
public:
    ImagickExtraImageInfo();
    virtual ~ImagickExtraImageInfo();

private:
    void extractPrimaryColor(Magick::Image *image);

public:
    virtual void doGet(void *image);
    virtual std::string getResult();

private:
    static log4cxx::LoggerPtr _logger;
};

#endif //AVOS_BACKEND_IMAGE_SERVICE_IMAGICK_EXTRA_IMAGE_INFO_H
