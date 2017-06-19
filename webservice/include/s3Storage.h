#ifndef CN_LEANCLOUD_IMAGESERVICE_STORAGE_S3_INCLUDE_
#define CN_LEANCLOUD_IMAGESERVICE_STORAGE_S3_INCLUDE_

#include <pthread.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <iostream>

#include "libs3.h"
#include "storage.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

typedef struct PutObjectCallbackData {
    const char* data;
    int contentLength;
    int offset;
    S3Status status;
} PutObjectCallbackData;

class S3Image : public Image {
public:
    S3Image(const char* id)
            : Image(id, NULL, 0) {
        _offset = 0;
        _BUFFER = NULL;
        _BUFFER_LEN = 1024*1024;
    };

    virtual ~S3Image() {
        if (_BUFFER != NULL) {
            free(_BUFFER);
            _BUFFER = NULL;
        }
    };
    virtual const void* data(int& len) {
        len = _len;
        return _BUFFER;
    };
    virtual int len() {
        return _len;
    };

public:
    void reInit(unsigned long long length);
    void readData(int bufferSize, const char *buffer);
    void setStatus(S3Status status) {_status = status;};
    S3Status getStatus() {return _status;};
    bool isCompelete() {return _offset == _len;};
private:
    int _offset;
    S3Status _status;
    char* _BUFFER;
    int _BUFFER_LEN;
};

class S3Storage : public Storage {
public:
    S3Storage();
    S3Storage(const S3Storage& other);
    virtual ~S3Storage();

    bool init(const string& host, const string& accessKey, const string& secretKey);

    virtual int saveImage(const char* id, const void* data, int data_len, const char* bucket);

    virtual Image* getImage(const char* id, const char* bucket);

    virtual Image* getImage(const char* id, const char* bucket, int hostStyle);

    virtual bool headImage(const char* id, const char* bucket, int hostStyle);

    virtual void freeImage(Image*& ptr);

    virtual Storage* clone();

protected:
    static LoggerPtr _logger;
    static string IMAGE_FORMAT;
    static bool _init;
    string _host;
    string _accessKey;
    string _secretKey;
};

#endif // CN_LEANCLOUD_IMAGESERVICE_STORAGE_S3_INCLUDE_
