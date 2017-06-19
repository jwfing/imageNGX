#ifndef CN_LEANCLOUD_IMAGESERVICE_STORAGE_MONGODB_GRIDFS_INCLUDE_
#define CN_LEANCLOUD_IMAGESERVICE_STORAGE_MONGODB_GRIDFS_INCLUDE_

#include <assert.h>
#include <mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <pthread.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "storage.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

class GridFSImage : public Image {
public:
    GridFSImage(const char* id, mongoc_gridfs_file_t& file)
        : Image(id, NULL, 0) {
        reInit(file);
    };
    virtual ~GridFSImage() {
        if (NULL != _BUFFER) {
            free(_BUFFER);
        }
        _BUFFER = NULL;
        _BUFFER_LEN = 0;
    };
    virtual const void* data(int& len) {
        len = _len;
        return _BUFFER;
    };
    virtual int len() {
        return _len;
    };
private:
    int calcFileSize(mongoc_gridfs_file_t& file);
    void reInit(mongoc_gridfs_file_t& file);
private:
    static __thread char* _BUFFER;
    static __thread int _BUFFER_LEN;
    static LoggerPtr _logger;
};

// Image Storage Interface thru GridFS
class GridFSStorage : public Storage {
public:
    GridFSStorage();
    GridFSStorage(const GridFSStorage& other);
    virtual ~GridFSStorage();

    // initialize
    // @param(in) host - mongoDB host
    // @param(in) dbname - mongoDB db name
    // @param(in) collection - mongoDB collection name
    // @param(in) username
    // @param(in) pwd
    // @param(in) digestPwd
    // @param(in) soTimeout
    bool init(const string& host, const string& dbname, const string& collection,
              const string& username, const string& pwd, bool digestPwd, double soTimeout);
    // save a image
    // @param(in) id - image id(filename, as for image, urlMD5 is a good choice)
    // @param(in) data - image binary data
    // @param(in) dataLen - length of image binary data
    // @return 0 - successful
    //         <0 - failed
    virtual int saveImage(const char* id, const void* data, const int data_len, const char* bucket);
    // retrive image by id
    // @param(in) id - image id(filename)
    // @return pointer to image instance
    //         NULL - for error
    // @notice: we need invoke freeImage to release image instance
    virtual Image* getImage(const char* id, const char* bucket);
    virtual Image* getImage(const char* id, const char* bucket, int hostStyle);
    virtual bool headImage(const char* id, const char* bucket, int hostStyle) {return true;};
    virtual void freeImage(Image*& ptr);
    virtual Storage* clone();
protected:
    static LoggerPtr _logger;
    static string IMAGE_FORMAT;
    bool _init;       // flag to indicate initialize
    string _host;     // mongodb host
    string _dbname;   // mongodb db name
    string _collection;
    string _username;
    string _passwd;
    bool _digestPasswd;
    double _soTimeout;
    mongoc_client_t* _client;

//    DBClientConnection _connection;
    mongoc_gridfs_t* _gridFS;  // pointer to gridfs object
private:
    bool initMongo();
    void destroyMongo();
    mongoc_gridfs_file_t* readFileFromMongo(const char* id, const char* bucket, int hostStyle);
    int saveImage2Mongo(const char* id, const char* bucket, const void* data, const int data_len);
};

#endif
