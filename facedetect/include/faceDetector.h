#ifndef AVOS_BACKEND_IMAGE_SERVICE_FACE_DETECTOR_H_
#define AVOS_BACKEND_IMAGE_SERVICE_FACE_DETECTOR_H_

#include <iostream>
#include <string>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

class FaceDetector {
  public:
    FaceDetector();
    ~FaceDetector();

    /**
     * @brief read cascade classifier
     */
    bool init();

    /**
     * @brief get the best face from faces, currently choose the largest one
     * @param(in) src
     * @param(in) len
     * @param(out) left
     * @param(out) top
     * @param(out) width
     * @param(out) height
     * @return return the total num of faces
     */
    int getBestFace(const char* src, size_t len,
                    int& left, int& top,
                    int& width, int& height);

  private:
    /**
     * @brief detect all the faces in image and return in Rect
     * @param(in) src
     * @param(in) len
     * @return the postions of all the faces in image
     */
    std::vector<cv::Rect> doDetect(const char* src, size_t len);

  private:
    static log4cxx::LoggerPtr _logger;
    std::string _cascadeName;
    //TODO: added later
    std::string _nestedCascadeName;
    double _scale;
    cv::CascadeClassifier _cascade;
    //TODO: added later
    cv::CascadeClassifier _nestedCascade;
};

#endif //end of AVOS_BACKEND_IMAGE_SERVICE_FACE_DETECTOR_H_
