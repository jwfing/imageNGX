#include "defaultFavicon.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr DefaultFavicon::_logger = Logger::getLogger("cn.leancloud.image.webservice.favicon");

DefaultFavicon::DefaultFavicon() {
    _buffer = NULL;
    _length = 0;
}

DefaultFavicon::~DefaultFavicon() {
    if (NULL != _buffer) {
        free(_buffer);
        _buffer = NULL;
    }
    _length = 0;
}

bool DefaultFavicon::initialize(const string& filePath) {
    bool result = false;
    if (NULL != _buffer) {
        LOG4CXX_WARN(_logger, "have been initialized");
        return result;
    }
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        LOG4CXX_WARN(_logger, "failed to open file:" << filePath);
        return result;
    }
    struct stat buff;
    fstat(fd, &buff);
    _length = buff.st_size;
    _buffer = malloc(buff.st_size);
    ssize_t readSize = read(fd, _buffer, _length);
    if (readSize == (ssize_t)_length) {
        result = true;
    } else {
        LOG4CXX_WARN(_logger, "failed to read file:" << filePath << ", fileSize="
                     << _length << ", readSize=" << readSize);
    }
    close(fd);
    return result;
}

DefaultFavicon* DefaultFavicon::getInstance() {
    static DefaultFavicon instance;
    return &instance;
}

void* DefaultFavicon::data(size_t& size) {
    size = _length;
    return _buffer;
}
