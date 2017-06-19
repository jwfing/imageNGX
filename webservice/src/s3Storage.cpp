#include "s3Storage.h"
#include <iostream>

LoggerPtr S3Storage::_logger = Logger::getLogger("cn.leancloud.image.webservice.s3Storage");

string S3Storage::IMAGE_FORMAT = "image/jpeg";

bool S3Storage::_init = false;

const char* DEFAULT_BUCKET = "del-dev-content";

//__thread char* S3Image::_BUFFER = NULL;
//__thread int S3Image::_BUFFER_LEN = 1024*1024;

//======================= S3 Put Object Callback ==============================
static void putObjectCompleteCallback(S3Status status,
                                      const S3ErrorDetails *error,
                                      void *callbackData) {
    if (callbackData != NULL) {
        PutObjectCallbackData* data = (PutObjectCallbackData*) callbackData;
        data->status = status;
        if (data->offset != data->contentLength) {
            data->status = S3StatusErrorIncompleteBody;
        }
    }
}

static S3Status putObjectPropertiesCallback(const S3ResponseProperties *properties,
                                            void *callbackData) {
    return S3StatusOK;
}

static int putObjectDataCallback(int bufferSize, char * buffer, void *callbackData) {
    if (callbackData != NULL) {
        PutObjectCallbackData* data = (PutObjectCallbackData*) callbackData;
        if (bufferSize > data->contentLength - data->offset) {
            bufferSize = data->contentLength - data->offset;
        }
        memcpy(buffer, data->data + data->offset, bufferSize);
        data->offset += bufferSize;
    }
    return bufferSize;
}
//=======================End S3 Put Object Callback ==============================

//========================S3 HEAD Object Callback=================================
static void headObjectCompleteCallback(S3Status status,
                                      const S3ErrorDetails *error,
                                      void *callbackData) {
    S3Status* data = (S3Status*) callbackData;
    *data = status;
}

static S3Status headObjectPropertiesCallback(const S3ResponseProperties *properties,
                                            void *callbackData) {
    return S3StatusOK;
}

//========================END S3 HEAD Object Callback=============================

//========================S3 Get Object Callback==================================

static void getObjectCompleteCallback(S3Status status,
                                      const S3ErrorDetails *error,
                                      void *callbackData) {
    if (callbackData != NULL) {
        S3Image* image = (S3Image*) callbackData;
        image->setStatus(status);
        if (image->isCompelete() == false) {
            image->setStatus(S3StatusErrorIncompleteBody);
        }
    }
}

static S3Status getObjectPropertiesCallback(const S3ResponseProperties *properties,
                                            void *callbackData) {
    if (properties != NULL && callbackData != NULL) {
        unsigned long long length = (unsigned long long)properties->contentLength;
        S3Image* image = (S3Image*) callbackData;
        image->reInit(length);
    }
    return S3StatusOK;
}

static S3Status getObjectDataCallback(int bufferSize, const char *buffer,
                                      void *callbackData) {
    if (callbackData != NULL) {
        S3Image* image = (S3Image*) callbackData;
        image->readData(bufferSize, buffer);
    }
    return S3StatusOK;
}

//=================End S3 Get Object Callback ==============================

void S3Image::reInit(unsigned long long length) {
    _len = (int)length;
    if (_len > _BUFFER_LEN) {
        _BUFFER_LEN += _len;
        if (NULL != _BUFFER) {
            free(_BUFFER);
            _BUFFER = NULL;
        }
    }
    if (NULL  == _BUFFER) {
        _BUFFER = (char*) malloc(_BUFFER_LEN);
    }
}

void S3Image::readData(int bufferSize, const char* buffer) {
    memcpy(_BUFFER + _offset, buffer, bufferSize);
    _offset += bufferSize;
}

S3Storage::S3Storage() {
    _init = false;
    _host = "";
    _accessKey = "";
    _secretKey = "";
}

S3Storage::S3Storage(const S3Storage& other) {
    if (this !=&other) {
        //_init = false;
        init(other._host, other._accessKey, other._secretKey);
    }
}

S3Storage::~S3Storage() {
    S3_deinitialize();
}

bool S3Storage::init(const string& host, const string& accessKey, const string& secretKey) {
    _host = host;
    _accessKey = accessKey;
    _secretKey = secretKey;
    if (_init) {
        LOG4CXX_DEBUG(_logger, "re-init");
        return true;
    }
    S3Status status;
    if ((status = S3_initialize("s3", S3_INIT_ALL, host.c_str())) != S3StatusOK) {
        LOG4CXX_ERROR(_logger, "Init S3 Error, status: " << S3_get_status_name(status));
        return false;
    }
    _init = true;
    LOG4CXX_INFO(_logger, "Init Successful");
    return true;
}

int S3Storage::saveImage(const char* id, const void* data, int data_len, const char * bucket) {
    S3Status status = S3StatusOK;
    PutObjectCallbackData callbackData = {
        (const char*)data,
        data_len,
        0,
        status
    };
    S3BucketContext bucketContext =
            {
                0,
                bucket,
                S3ProtocolHTTPS,
                S3UriStylePath,
                _accessKey.c_str(),
                _secretKey.c_str(),
            };

    S3PutProperties putProperties =
            {
                IMAGE_FORMAT.c_str(),
                NULL,
                NULL,
                NULL,
                NULL,
                0,
                S3CannedAclPublicRead,
                0,
                NULL
            };

    S3PutObjectHandler putObjectHandler =
            {
                { &putObjectPropertiesCallback, &putObjectCompleteCallback },
                &putObjectDataCallback
            };

    S3_put_object(&bucketContext, id, data_len, &putProperties, 0,
                  &putObjectHandler, &callbackData);

    if (callbackData.status != S3StatusOK) {
        LOG4CXX_ERROR(_logger, "Failed to upload image to s3, status:" << S3_get_status_name(callbackData.status) << " data remaining:" << callbackData.contentLength - callbackData.offset);
    }
    return 0;
}

bool S3Storage::headImage(const char* id, const char* bucket, int hostStyle) {
    S3UriStyle style = S3UriStylePath;
    if (hostStyle == 1) {
        // 1 for virtual host style
        style = S3UriStyleVirtualHost;
    }
    S3BucketContext bucketContext =
            {
                0,
                bucket,
                S3ProtocolHTTPS,
                style,
                _accessKey.c_str(),
                _secretKey.c_str(),
            };
    S3ResponseHandler responseHandler =
        {
            &headObjectPropertiesCallback,
            &headObjectCompleteCallback
        };
    S3Status status = S3StatusOK;
    S3_head_object(&bucketContext, id, 0, &responseHandler, &status);
    if (status == S3StatusOK) {
        return true;
    }
    return false;
}

Image* S3Storage::getImage(const char* id, const char* bucket, int hostStyle) {
    S3UriStyle style = S3UriStylePath;
    if (hostStyle == 1) {
        // 1 for virtual host style
        style = S3UriStyleVirtualHost;
    }
    S3BucketContext bucketContext =
            {
                0,
                bucket,
                S3ProtocolHTTPS,
                style,
                _accessKey.c_str(),
                _secretKey.c_str(),
            };
    S3GetObjectHandler getObjectHandler =
            {
                { &getObjectPropertiesCallback, &getObjectCompleteCallback },
                &getObjectDataCallback
            };
    S3Image* image = new S3Image(id);
    S3_get_object(&bucketContext, id, NULL, 0, 0, 0,  &getObjectHandler, image);
    if (image->getStatus() != S3StatusOK) {
        LOG4CXX_ERROR(_logger, "Read from s3 error: " <<  S3_get_status_name(image->getStatus())
                      << " for id: " << id);
        return NULL;
    }
    return image;
}

Image* S3Storage::getImage(const char* id, const char* bucket) {
    return getImage(id, bucket, 0);
}

void S3Storage::freeImage(Image*& ptr) {
    if (ptr != NULL) {
        delete ptr;
        ptr = NULL;
    }
}

Storage* S3Storage::clone() {
    return new S3Storage(*this);
}


