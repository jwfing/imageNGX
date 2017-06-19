#ifndef CN_LEANCLOUD_IMAGE_SERVICE_DEFAULT_FAVICON_INCLUDE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_DEFAULT_FAVICON_INCLUDE_H_

#include <iostream>
#include <string>
#include <unistd.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace std;

class DefaultFavicon {
public:
    ~DefaultFavicon();
    bool initialize(const string& filename);
    static DefaultFavicon* getInstance();
    void* data(size_t& size);
protected:
    DefaultFavicon();
    void* _buffer;
    size_t _length;
    static log4cxx::LoggerPtr _logger;
};

#endif

