#ifndef AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_INCLUDE_H_
#define AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_INCLUDE_H_

#include <iostream>
#include <string>
#include "errorCode.h"
#include "imageOperator.h"
#include "extraImageInfo.h"

class Rect {

  public:
    Rect(int x1, int y1, int x2, int y2) {
        _x1 = x1;
        _y1 = y1;
        _x2 = x2;
        _y2 = y2;
    };

    bool isValidRect() const {
        if (_x1 < 0 || _x2 < 0 ||
            _y1 < 0 || _y2 < 0 ||
            _x1 >= _x2 || _y1 >= _y2) {
            return false;
        }
        return true;
    }

  public:
    int _x1;
    int _y1;
    int _x2;
    int _y2;
};

class Thumbnailer {

  public:
    Thumbnailer();
    virtual ~Thumbnailer();

  public:
    /**
     * @brief create the thumbnailer
     * @param src To fill
     * @param lenOfsrc TO fill
     * @param desc To fill
     * @param lenOfDesc To fill
     * @param rectOfInterest To fill
     * @param width To fill
     * @param height To fill
     * @return if create success return the size of thumbnail image,
     *         otherwise return -1
     */
    virtual int create(const char* src, size_t lenOfSrc,
                       char* desc, size_t lenOfDesc,
                       const Rect& rectOfInterest,
                       int width, int height) = 0;

    virtual int create(const char* src, size_t lenOfSrc,
                       char* desc, size_t lenOfDesc,
                       const Rect& rectOfInterest,
                       int width, int height,
                       ImageOperator* opers,
                       ExtraImageInfo* extraImageInfo) = 0;

}; // end of Thumbnailer

#endif //end of AVOS_BACKEND_IMAGE_SERVICE_THUMBNAILER_INCLUDE_H_
