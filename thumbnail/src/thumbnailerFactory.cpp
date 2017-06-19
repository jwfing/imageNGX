#include "thumbnailerFactory.h"
#include "imagickThumbnailer.h"

Thumbnailer* ThumbnailerFactory::Create(std::string policy) {
    if (policy == "openCV") {
        //TODO to realize the opencv interface
        return NULL;
    } else if (policy == "imagick"){
        return new ImagickThumbnailer();
    } else if (policy == "graphImagick") {
        //TODO to realize the opencv interface
        return NULL;
    } else {
        return NULL;
    }
}
