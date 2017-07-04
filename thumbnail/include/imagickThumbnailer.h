#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_THUMBNAILER_INCLUDE_H_
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_THUMBNAILER_INCLUDE_H_

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#define USING_FACEDETECT
#ifdef USING_FACEDETECT
#include "faceDetector.h"
#endif

#include <Magick++.h>

#include "imageOperator.h"
#include "thumbnailer.h"

class ImagickThumbnailer : public Thumbnailer {
  public:
    ImagickThumbnailer();
    virtual ~ImagickThumbnailer();

  public:
    // @brief create a thumbnail image from original image
    //        according to the target rect and target width & height
    // @param src the original image data
    // @param lenOfSrc the original image data length
    // @param desc to be filled with the target image data
    // @param lenOfSrc the size of target space
    // @param rectOfInterest
    // @param width
    // @param height
    // @return return the real size of thumbnail image if convert successful
    //         otherwise return the code < 0
    int create(const char* src, size_t lenOfSrc,
               char* desc, size_t lenOfDesc,
               const Rect& rectOfInterest,
               int width, int height);

    int create(const char* src, size_t lenOfSrc,
               char* desc, size_t lenOfDesc,
               const Rect& rectOfInterest,
               int width, int height,
               ImageOperator* opers,
               ExtraImageInfo* extraImageInfo);

  private:
    // @brief get the cropped rect
    // @param imageWidth original image width
    // @param imageHeight original image height
    // @param targetWidth target width
    // @param targetHeight target height
    // @return a geometry whose w/h ratio is same as target ratio
    Magick::Geometry getCropGeometry(int imageWidth, int imageHeight,
                              int targetWidth, int targetHeight);

    // @brief crop the original image into target w/h ratio,
    //        but keep the original width or height.
    // @param image original image
    // @param width target width
    // @param heigth target height
    // @return EC_OK if succuess
    ErrorCode cropImage(Magick::Image& image,
                        int width, int height,
                        ImageOperator* opers,
                        const char* src, size_t lenOfSrc,
                        bool isOrientation,
                        ExtraImageInfo* extraImageInfo);

    // @brief crop the original image into target rect
    // @param image original image
    // @param rectOfInterest the target rect
    // @return EC_OK if succuess
    ErrorCode cropImageWithRect(Magick::Image& image,
                                const Rect& rectOfInterest);

    // @brief resize the image to the target size
    //        make sure the ratio of image should be same as
    //        the ratio calculated by the given w&h
    // @param image original image
    // @param width target width
    // @param heigth target height
    // @return EC_OK if succuess
    ErrorCode resizeImage(Magick::Image& image,
                          int width, int height);

    // @brief adjustGeoByFace
    ErrorCode adjustGeoByFace(Magick::Geometry& tarGeometry,
                              const Magick::Geometry& bestFace);
    bool autoOrientImage(Magick::Image& image);

    ErrorCode loadImage(const char* src, size_t len);

  private:
    static log4cxx::LoggerPtr _logger;
};

#endif // end of _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGICK_THUMBNAILER_INCLUDE_H_
