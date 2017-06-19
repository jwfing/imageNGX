#ifndef AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_FACTORY_INCLUDE_H_
#define AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_FACTORY_INCLUDE_H_

#include "thumbnailer.h"
#include <string>

class ThumbnailerFactory {
  public:
    /**
     * @brief Create a thumbnail according to the policy
     * @param policy: openCV, imagick, graphImagick
     */
    static Thumbnailer* Create(std::string policy);
};

#endif // endof AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_FACTORY_INCLUDE_H_

