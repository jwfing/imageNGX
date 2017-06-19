#include "imagickThumbnailer.h"
#include "misc.h"
#include <cstdlib>
#include <cstring>

using namespace Magick;
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

LoggerPtr ImagickThumbnailer::_logger = \
        Logger::getLogger("avos.backend.imageService.thumbnailer");

ImagickThumbnailer::ImagickThumbnailer() {
    LOG4CXX_INFO(_logger, "create ImagickThumbnailer instance");
}

ImagickThumbnailer::~ImagickThumbnailer() {
    LOG4CXX_INFO(_logger, "Destory ImagickThumbnailer");
}

int ImagickThumbnailer::create(const char* src, size_t lenOfSrc,
                               char* desc, size_t lenOfDesc,
                               const Rect& rectOfInterest,
                               int width, int height) {
    return create(src, lenOfSrc,
                  desc, lenOfDesc,
                  rectOfInterest,
                  width, height,
                  NULL, NULL);
}

int ImagickThumbnailer::create(const char* src, size_t lenOfSrc,
                               char* desc, size_t lenOfDesc,
                               const Rect& rectOfInterest,
                               int width, int height,
                               ImageOperator* opers,
                               ExtraImageInfo* extraImageInfo) {
    try {
        LOG4CXX_DEBUG(_logger, "data length: " << lenOfSrc);
        LOG4CXX_DEBUG(_logger, "begin to read");
        Blob oriBlob(src, lenOfSrc);
        Image image;
        // Try reading image file
        image.read(oriBlob);
        LOG4CXX_DEBUG(_logger, "begin to autoOrient");
        bool isOrientation = autoOrientImage(image);

        Geometry imageGeo = image.size();
        int imageWidth = (int)imageGeo.width();
        int imageHeight = (int)imageGeo.height();
        ErrorCode ecRet;

        //Crop according to the rectOfInterest if given
        ecRet = cropImageWithRect(image, rectOfInterest);
        if (ecRet != EC_OK) {
            LOG4CXX_DEBUG(_logger, "rectOfInterest is not right");
        }

        if (width <= 0 && height <= 0) {
            width = imageWidth;
            height = imageHeight;
        }

        if (width <= 0 ) {
            width = (int)(1.0 * imageWidth / imageHeight * height);
        }

        if (height <= 0) {
            height = (int)(1.0 * imageHeight / imageWidth * width);
        }

        /*
        ** support scaling
        **
        if (height > imageHeight) {
            width = (int)(1.0 * imageHeight * width / height);
            height = imageHeight;
        }

        if (width > imageWidth) {
            height = (int)(1.0 * imageWidth * height / width);
            width = imageWidth;
        }
        */
        //do crop and resize
        ecRet = cropImage(image, width, height, opers, src, lenOfSrc, isOrientation, extraImageInfo);
        if (ecRet != EC_OK) {
            LOG4CXX_ERROR(_logger, "Crop image with size" <<
                          width << " x " << height <<
                          "Error Code is" << ecRet);
            return ecRet;
        }
        resizeImage(image, width, height);

        if (NULL != opers) {
            opers->doRun(&image);
        }

        //write to desc memory
        Blob descBlob;
        image.write(&descBlob);
        if (descBlob.length() > lenOfDesc) {
            LOG4CXX_ERROR(_logger, "Buffer is not enough:" <<
                          "Need: " << descBlob.length() <<
                          "But given: " << lenOfDesc);
            return EC_BUFFER_NOT_ENOUGH;
        }
        LOG4CXX_DEBUG(_logger, "width: " << image.size().width() << " height: " << image.size().height());
        memcpy(desc, descBlob.data(), descBlob.length());
        int imageSize = (int)descBlob.length();

        // must run the code after `memcpy(desc, descBlob.data(),
        // descBlob.length());` since below function will change the image;
        if (NULL != extraImageInfo) {
            extraImageInfo->doGet(&image);
        }

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

ErrorCode ImagickThumbnailer::cropImageWithRect(Image& image,
                            const Rect& rectOfInterest) {
    Geometry imageGeo = image.size();
    int imageWidth = (int)imageGeo.width();
    int imageHeight = (int)imageGeo.height();

    if (!rectOfInterest.isValidRect()) {
        return EC_BAD_SIZE;
    }

    if (rectOfInterest._x2 > imageWidth ||
        rectOfInterest._y2 > imageHeight) {
        return EC_BAD_SIZE;
    }

    Geometry tarGeometry(rectOfInterest._x2 - rectOfInterest._x1,
                         rectOfInterest._y2 - rectOfInterest._y1,
                         rectOfInterest._x1, rectOfInterest._y1);

    image.crop(tarGeometry);
    return EC_OK;
}

ErrorCode ImagickThumbnailer::cropImage(Image& image,
                                        int width, int height,
                                        ImageOperator* opers,
                                        const char* src, size_t lenOfSrc,
                                        bool isOrientation,
                                        ExtraImageInfo* extraImageInfo) {
    Geometry imageGeo = image.size();
    int imageWidth = (int)imageGeo.width();
    int imageHeight = (int)imageGeo.height();

    if (0 >= width && 0 >= height) {
        return EC_BAD_SIZE;
    }
    // FIXME: disable face detection because mac do not support     __thread key word
    bool shouldFaceDetect = (opers->getCrop().compare("faces") == 0) && !isOrientation;
    int faceCount = 0;
    int faceLeft, faceTop, faceWidth, faceHeight;

    if (shouldFaceDetect) {
        //facedetect here
        //TODO this not a good idea to init the facedetector every time
        static __thread FaceDetector* gfd = NULL;
        if (gfd == NULL) {
            gfd = new FaceDetector();
            bool ret = gfd->init();
            if (false == ret) {
                delete gfd;
                gfd = NULL;
            }
        }
        if (gfd != NULL) {
            faceCount = gfd->getBestFace(src, lenOfSrc,
                                         faceLeft, faceTop,
                                         faceWidth, faceHeight);
        }
    }


    Geometry tarGeometry = getCropGeometry(imageWidth, imageHeight,
                                           width, height);

    if (shouldFaceDetect && faceCount != 0) {
        LOG4CXX_DEBUG(_logger, faceCount << " faces detected");
        if (extraImageInfo  != NULL) {
            extraImageInfo->setHasFaces(true);
        }
        adjustGeoByFace(tarGeometry,
                        Geometry(faceWidth, faceHeight,
                                 faceLeft, faceTop));
    }

    LOG4CXX_DEBUG(_logger, "width: " << tarGeometry.width());
    image.crop(tarGeometry);
    return EC_OK;
}

Geometry ImagickThumbnailer::getCropGeometry(int imageWidth, int imageHeight,
                                              int targetWidth, int targetHeight) {
    double currenctRatio = 1.0 * imageWidth / imageHeight;
    double targetRatio = 1.0 * targetWidth / targetHeight;
    int x, y, w, h;

    if (currenctRatio < targetRatio) {
        x = 0;
        w = imageWidth;
        h = (int) (1.0 * w / targetRatio);
        int midPoint = imageHeight / 2;
        y = midPoint - (h / 2);
    } else {
        y = 0;
        h = imageHeight;
        w = (int)(1.0 * h * targetRatio);
        int midPoint = imageWidth / 2;
        x = midPoint - (w / 2);
    }
    return Geometry(w, h, x, y);
}

ErrorCode ImagickThumbnailer::resizeImage(Image& image,
                                          int width, int height) {
  /*
  ** support scale image when target size is bigger than
  **
    Geometry imageGeo = image.size();
    int imageWidth = (int)imageGeo.width();
    int imageHeight = (int)imageGeo.height();

    int targetWidth;
    int targetHeight;
    double widthCorrectionRatio = (double) width / (double) imageWidth;
    double heightCorrectionRatio = (double) height / (double) imageHeight;
    double least = min(widthCorrectionRatio, heightCorrectionRatio);
    if (least >= 1) {
        // no scaling is required, target width and height are under current
        // width and height.
        targetWidth = imageWidth;
        targetHeight = imageHeight;
    } else {
        targetWidth = width;
        targetHeight = height;
    }
  */
    Geometry geoZoom(width, height);
    image.zoom(geoZoom);
    image.page(geoZoom);
    return EC_OK;
}

ErrorCode ImagickThumbnailer::adjustGeoByFace(Geometry& tarGeometry,
                                              const Geometry& bestFace) {
#ifdef USING_FACEDETECT
    if (0 == tarGeometry.xOff()) {
        //need to adjust height
        if (tarGeometry.height() > bestFace.height()) {
            if (tarGeometry.yOff() > bestFace.yOff()) {
                tarGeometry.yOff((size_t)std::max<long>((long)0, bestFace.yOff()));
            } else if (tarGeometry.height() + tarGeometry.yOff() <
                       bestFace.height() + bestFace.yOff()) {
                tarGeometry.yOff(bestFace.yOff() +
                                 bestFace.height() -
                                 tarGeometry.height());
            }
        }
    } else if (0 == tarGeometry.yOff()) {
        //need to adjust width
        if (tarGeometry.width() > bestFace.width()) {
            if (tarGeometry.xOff() > bestFace.xOff()) {
                tarGeometry.xOff(bestFace.xOff());
            } else if (tarGeometry.width() + tarGeometry.xOff() <
                       bestFace.width() + bestFace.xOff()) {
                tarGeometry.xOff(bestFace.xOff() +
                                 bestFace.width() -
                                 tarGeometry.width());
            }
        }
    }
#endif
    return EC_OK;
}

bool ImagickThumbnailer::autoOrientImage(Image& image) {
    bool ret = true;
    switch (image.orientation()) {
      case TopRightOrientation:
        {
          image.flop();
          break;
        }
      case BottomRightOrientation:
        {
          image.rotate(180.0);
          break;
        }
      case BottomLeftOrientation:
        {
          image.flip();
          break;
        }
      case LeftTopOrientation:
        {
          image.rotate(90.0);
          image.flip();
          break;
        }
      case RightTopOrientation:
        {
          image.rotate(90.0);
          break;
        }
      case RightBottomOrientation:
        {
          image.rotate(270);
          image.flip();
          break;
        }
      case LeftBottomOrientation:
        {
          image.rotate(270);
          break;
        }
      default:
        {
          ret = false;
          break;
        }
    }
    image.orientation(TopLeftOrientation);
    return ret;
}

ErrorCode ImagickThumbnailer::loadImage(const char* src, size_t len) {
    return EC_OK;
}
