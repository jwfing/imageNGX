#include "gridfsStorage.h"

LoggerPtr GridFSImage::_logger = Logger::getLogger("cn.leancloud.image.webservice.GridFSImage");
LoggerPtr GridFSStorage::_logger = Logger::getLogger("cn.leancloud.image.webservice.gridfsStorage");
string GridFSStorage::IMAGE_FORMAT = "image/jpeg";

__thread char* GridFSImage::_BUFFER = NULL;
__thread int GridFSImage::_BUFFER_LEN = 1024*1024;

int GridFSImage::calcFileSize(mongoc_gridfs_file_t& file) {
    int result = mongoc_gridfs_file_get_length(&file);
    return result;
}

void GridFSImage::reInit(mongoc_gridfs_file_t& file) {
    _len = calcFileSize(file);
    if (_len > _BUFFER_LEN) {
        _BUFFER_LEN += _len;
        if (NULL != _BUFFER) {
            free(_BUFFER);
            _BUFFER = NULL;
        }
    }
    if (NULL == _BUFFER) {
        _BUFFER = (char*)malloc(_BUFFER_LEN);
    }
    if (NULL == _BUFFER) {
        _len = 0;
        return;
    }

    mongoc_stream_t *stream = mongoc_stream_gridfs_new(&file);
    ssize_t r = 0;
    ssize_t readLen = 0;
    for(;readLen < _len;) {
        mongoc_iovec_t iov;
        iov.iov_base = (void*)((char*)_BUFFER + readLen);
        iov.iov_len = _len - readLen;
        r = mongoc_stream_readv(stream, &iov, 1, -1, 0);
        if (r == 0) {
            LOG4CXX_DEBUG(_logger, "end of gridfs。 ret:" << r);
            break;
        }
        if (r < 0) {
            // error
            LOG4CXX_WARN(_logger, "failed to read gridfs。 ret:" << r);
            break;
        }
        LOG4CXX_DEBUG(_logger, "read gridfs. cnt:" << r);
        readLen += r;
    }
    mongoc_stream_destroy(stream);
    int64_t uploadDate = mongoc_gridfs_file_get_upload_date(&file);
    setLastUpdateTime(uploadDate/1000);// uploadDate is milliseconds
}

GridFSStorage::GridFSStorage() {
    _init = false;
    _host = "";
    _dbname = "";
    _collection = "";
    _username = "";
    _passwd = "";
    _digestPasswd = false;
    _soTimeout = 0.0;
    _gridFS = NULL;
}

GridFSStorage::GridFSStorage(const GridFSStorage& other) {
    if (this != &other) {
        _init = false;
        _gridFS = NULL;
        init(other._host, other._dbname, other._collection, other._username, other._passwd,
             other._digestPasswd, other._soTimeout);
    }
}

GridFSStorage::~GridFSStorage() {
    destroyMongo();
}

bool GridFSStorage::initMongo() {
    mongoc_init();
    char connecturi[1024] = {0};
    if (_username.length() > 0) {
        sprintf(connecturi, "mongodb://%s:%s@%s/?appname=%s", _username.c_str(),
            _passwd.c_str(), _host.c_str(), _dbname.c_str());
    } else {
        sprintf(connecturi, "mongodb://%s/?appname=%s",
            _host.c_str(), _dbname.c_str());
    }
    LOG4CXX_INFO(_logger, "initialize mongodb connection uri: " << connecturi);
    _client = mongoc_client_new(connecturi);
    assert(_client);
    mongoc_client_set_error_api(_client, 2);
    bson_error_t error;
    _gridFS = mongoc_client_get_gridfs(_client, _collection.c_str(), "fs", &error);
    assert(_gridFS);
    return true;
}

void GridFSStorage::destroyMongo() {
    LOG4CXX_INFO(_logger, "destroy mongodb connection");
    if (_gridFS) {
        mongoc_gridfs_destroy(_gridFS);
        _gridFS = NULL;
    }
    if (_client) {
        mongoc_client_destroy(_client);
        _client = NULL;
    }
    mongoc_cleanup();
}

bool GridFSStorage::init(const string& host, const string& dbname, const string& collection,
    const string& username, const string& pwd, bool digestPwd, double soTimeout) {
    if (_init) {
        LOG4CXX_DEBUG(_logger, "re-init");
        return true;
    }
    _host = host;
    _dbname = dbname;
    _collection = collection;
    _username = username;
    _passwd = pwd;
    _digestPasswd = digestPwd;
    _soTimeout = soTimeout;

    _init = initMongo();
    LOG4CXX_INFO(_logger, "successfully init gridfs storage");
    return true;
}

int GridFSStorage::saveImage2Mongo(const char* id, const char* bucket,
    const void* data, const int data_len) {
    char filename_buf[1024] = {0};
    snprintf(filename_buf, 1023, "%s-%s", bucket, id);
    mongoc_gridfs_file_opt_t opt;
    opt.filename = filename_buf;
    mongoc_gridfs_file_t *file = mongoc_gridfs_create_file(_gridFS, &opt);
    if (NULL == file) {
	LOG4CXX_WARN(_logger, "failed to create gridfs file.");
        return -1;
    }
    
    bson_value_t bid;
    bid.value_type = BSON_TYPE_INT32;
    bid.value.v_int32 = 1;
    
    mongoc_iovec_t iovec;
    iovec.iov_base = (void*)data;
    iovec.iov_len = data_len;
    int writedCnt = mongoc_gridfs_file_writev(file, &iovec, 1, 1000);
    if (writedCnt <=0) {
        LOG4CXX_WARN(_logger, "failed to write gridfs. cnt=" << writedCnt);
    }
    mongoc_gridfs_file_save(file);
    mongoc_gridfs_file_destroy(file);
    return writedCnt <= 0 ? -1 : 0;
}

mongoc_gridfs_file_t* GridFSStorage::readFileFromMongo(const char* id,
    const char* bucket, int hostStyle) {
    bson_error_t error;
    char filename_buf[1024] = {0};
    snprintf(filename_buf, 1023, "%s-%s", bucket, id);
    mongoc_gridfs_file_t* file = mongoc_gridfs_find_one_by_filename(_gridFS,
        filename_buf, &error);
    return file;
}

int GridFSStorage::saveImage(const char* id, const void* data,
    const int data_len, const char* bucket)
{
    if (NULL == _gridFS) {
        LOG4CXX_WARN(_logger, "internal error: gridFS isn't initialized");
        return -1;
    }
    int ret = saveImage2Mongo(id, bucket, data, data_len);
    return ret;
}

Image* GridFSStorage::getImage(const char* id, const char* bucket)
{
    return getImage(id, bucket, 0);
}

Image* GridFSStorage::getImage(const char* id, const char* bucket,
    int hostStyle) {
    if (NULL == _gridFS) {
        LOG4CXX_WARN(_logger, "internal error: gridFS isn't initialized");
        return NULL;
    }
    mongoc_gridfs_file_t* gridFile = readFileFromMongo(id, bucket, hostStyle);

    if (NULL == gridFile) {
        // error
        return NULL;
    }
    GridFSImage* image = new GridFSImage(id, *gridFile);
    mongoc_gridfs_file_destroy(gridFile);
    return image;
}

void GridFSStorage::freeImage(Image*& ptr)
{
    if (NULL != ptr) {
        delete ptr;
        ptr = NULL;
    }
}

Storage* GridFSStorage::clone() {
    return new GridFSStorage(*this);
}
