#include "errorCode.h"
#include "misc.h"
#include "imagickMontage.h"

#include <sstream>

using namespace std;
using namespace Magick;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ImagickMontage::_logger = \
    Logger::getLogger("avos.backend.imageService.imagickMontage");

ImagickMontage::ImagickMontage() {
}

ImagickMontage::~ImagickMontage() {
}

int ImagickMontage::montage(vector<char*>& srcs,
                            vector<int>& lenOfSrcs,
                            int numOfImages,
                            char* desc,
                            size_t lenOfDesc,
                            int width,
                            int height,
                            int margin,
                            int numOfImagesInRow,
                            const string& backgroundColor) {
    try {
        list<Image> images;
        list<Image> montageImage;

        // segment fault?
        for (int i = 0; i < numOfImages; ++i) {
            Blob blob(srcs[i], lenOfSrcs[i]);
            Image image;
            image.read(blob);
            LOG4CXX_INFO(_logger, "read image " << i << " with length: " << lenOfSrcs[i]);
            images.push_back(image);
        }
        Montage montageOpts;
        char tmp[64];
        sprintf(tmp, "%dx%d!+%d+%d", width, height, margin, margin);
        montageOpts.geometry(tmp);

        int row = numOfImages / numOfImagesInRow;
        if (numOfImages % numOfImagesInRow) {
            row += 1;
        }
        sprintf(tmp, "%dx%d", numOfImagesInRow, row);
        montageOpts.tile(tmp);
        if (backgroundColor.length() > 0) {
            double r,g,b;
            r = g = b = -1;
            stringstream ss(backgroundColor);
            string color;
            char delimiter = ',';
            int c = 0;
            while (getline(ss, color, delimiter)) {
                int val = atoi(color.c_str());
                if (val > 255) val = 255;
                if (val < 0) val = 0;
                if (c == 0) {
                    r = val / 255.0;
                } else if (c == 1) {
                    g = val / 255.0;
                } else if (c == 2) {
                    b = val / 255.0;
                } else {
                    break;
                }
                c++;
            }
            if (r >= 0 && g >= 0 && b >=0) {
                ColorRGB bgColor(r, g, b);
                montageOpts.backgroundColor(bgColor);
            }
        }
        montageImages(&montageImage, images.begin(), images.end(), montageOpts);
        Image& ret = montageImage.front();
        Blob descBlob;
        ret.magick("jpeg");
        ret.write(&descBlob);
        if (descBlob.length() > lenOfDesc) {
            LOG4CXX_ERROR(_logger, "Buffer is not enough:" <<
                          "Need: " << descBlob.length() <<
                          "But given: " << lenOfDesc);
            return EC_BUFFER_NOT_ENOUGH;
        }
        memcpy(desc, descBlob.data(), descBlob.length());
        int imageSize = (int) descBlob.length();
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
