#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_MONTAGE_H_
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_MONTAGE_H_

#include <iostream>
#include <string>
#include <list>
#include <vector>

#include <Magick++.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

class ImagickMontage {
 public:
    ImagickMontage();
    ~ImagickMontage();

 public:
    int montage(std::vector<char*>& srcs,
                std::vector<int>& lenOfSrcs,
                int numOfImages,
                char* desc,
                size_t lenOfDesc,
                int width,
                int height,
                int margin,
                int numOfImagesInRow,
                const std::string& backgroundColor);

 private:
    static log4cxx::LoggerPtr _logger;
};

#endif // _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_MONTAGE_H_
