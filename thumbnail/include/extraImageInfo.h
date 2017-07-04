#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_EXTRA_IMAGE_INFO_H
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_EXTRA_IMAGE_INFO_H

#include <string>

typedef struct color_rgb
{
    unsigned int
      r, g, b;
} PrimaryColor;

class ExtraImageInfo {
public:
    ExtraImageInfo() {};
    virtual ~ExtraImageInfo() {};

public:
    PrimaryColor getPrimaryColor() {return _primaryColor;};
    bool getHasFaces() {return _hasFaces;};
    void setHasFaces(bool hasFaces) {_hasFaces = hasFaces;};

public:
    virtual void doGet(void* image) = 0;
    virtual std::string getResult() = 0;

protected:
    PrimaryColor _primaryColor;
    bool _hasFaces;
};


#endif //_CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_EXTRA_IMAGE_INFO_H
