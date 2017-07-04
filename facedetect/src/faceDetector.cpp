#include "faceDetector.h"
#include <cstdio>

using namespace std;
using namespace cv;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr FaceDetector::_logger = \
        Logger::getLogger("leancloud.backend.imageService.faceDetector");

FaceDetector::FaceDetector() {
    _cascadeName = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";
    _scale = 1.0;
}

FaceDetector::~FaceDetector() {
    LOG4CXX_INFO(_logger, "Destory FaceDetector");
}

bool FaceDetector::init() {
    if (_cascadeName.empty() || !_cascade.load(_cascadeName)) {
        LOG4CXX_ERROR(_logger, "Error model file name: " << _cascadeName);
        return false;
    }
    LOG4CXX_INFO(_logger, "Create faceDetector instance");
    return true;
}

vector<Rect> FaceDetector::doDetect(const char* src, size_t len) {
    vector<Rect> allFaces;
    if (NULL == src || 0 >= len) {
        LOG4CXX_ERROR(_logger, "Src is empty");
        return allFaces;
    }

    if (_cascade.empty()) {
        LOG4CXX_ERROR(_logger, "Classifier Loaded Errory");
        return allFaces;
    }

    //load image from buffer
    vector<char> vec;
    vec.reserve(len);
    vec.insert(vec.end(), src, src + len);
    Mat img = imdecode(Mat(vec), 1);
    Mat gray;
    if (16 == img.type() && 3 == img.channels()) {
        cvtColor(img, gray, CV_BGR2GRAY);
        Size originalSize = img.size();
        int minSize = max(originalSize.width/32, originalSize.height/32);
        if (minSize > 300) {
            minSize = 300;
        }
        if (minSize < 30) {
            minSize = 30;
        }
        _cascade.detectMultiScale(gray, allFaces,
                                  1.1, 2,
                                  0 | CV_HAAR_SCALE_IMAGE,
                                  Size(minSize, minSize));
    }
    return allFaces;
}

int FaceDetector::getBestFace(const char* src, size_t len,
                              int& left, int& top,
                              int& width, int& height) {
    vector<Rect> faces = doDetect(src, len);
    left = top = width = height = 0;
    for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++) {
        if (width * height <
            r->width * r->height) {
            left = r->x;
            top = r->y;
            width = r->width;
            height = r->height;
        }
    }
    return (int)faces.size();
}
