#ifndef CN_LEANCLOUD_IMAGESERVICE_STORAGE_INCLUDE_
#define CN_LEANCLOUD_IMAGESERVICE_STORAGE_INCLUDE_

#include "misc.h"
#include <string>
#include <pion/http/types.hpp>

class Image {
public:
    Image(const char* id, const void* data, const int data_len) : _id(id), _data(data), _len(data_len) {
        _last_modified = get_cur_microseconds_time() / 1000000; // convert to seconds.
    };
    Image(const Image& cp)
      : _id(cp._id), _data(cp._data), _len(cp._len), _last_modified(cp._last_modified),
       _last_modified_string(cp._last_modified_string), _mime_type(cp._mime_type) {
    }
    virtual ~Image() {
        _id = NULL;
        _data = NULL;
        _len = 0;
        _last_modified = 0;
    };

    virtual const void* data(int& len) {
        len = _len;
        return _data;
    };
    virtual int len() {
        return _len;
    };
    virtual const void* getContentBuf() {
        return _data;
    }
    inline bool hasContent() {
        return _len > 0;
    }
    inline void setTime(int64_t time) {
        setLastUpdateTime(time);
    }
    inline void setLastUpdateTime(int64_t time) {
        _last_modified = time;
        _last_modified_string = pion::http::types::get_date_string(time);
    };
    inline const char* getIdentifiedString() {
        return _id;
    }
    inline int64_t getLastUpdateTime() {
        return _last_modified;
    };
    inline int64_t getTime() {
        return getLastUpdateTime();
    }
    inline std::string getLastModifiedString() {
        return _last_modified_string;
    }
    inline std::string getMimeType() {
        return _mime_type;
    }
    inline void setMimeType(const std::string& type) {
        _mime_type = type;
    }

protected:
    const char* _id;
    const void* _data;
    int _len;
    
    int64_t _last_modified;
    /// timestamp that the cached file was last modified (string format)
    std::string                 _last_modified_string;
    /// mime type for the cached file
    std::string                 _mime_type;
};

// Image Storage Interface
class Storage{
public:
    Storage() {};

    virtual ~Storage() {};

    // save a image
    // @param(in) id - image id(filename, as for image, urlMD5 is a good choice)
    // @param(in) data - image binary data
    // @param(in) dataLen - length of image binary data
    // @return 0 - successful
    //         <0 - failed
    virtual int saveImage(const char* id, const void* data, const int dataLen, const char* bucket) = 0;

    // retrive image by id
    // @param(in) id - image id(filename)
    // @return pointer to image instance
    //         NULL - for error
    // @notice: we need invoke freeImage to release image instance
    virtual Image* getImage(const char* id, const char* bucket) = 0;

    virtual bool deleteImage(const char* id, const char* bucket) {
        return true;
    };

    virtual Image* getImage(const char* id, const char* bucket, int hostStyle) = 0;

    // try to check if image is exist
    // @param(in) id - image keyfile
    // @param(in) bucket - image bucket
    // @param(in) hostStyle - path or virtual proxy
    // @return true if image is exist, otherwise false
    virtual bool headImage(const char* id, const char* bucket, int hostStyle) = 0;

    // release image instance
    virtual void freeImage(Image*& ptr) = 0;

    // clone for multiple threading
    virtual Storage* clone() = 0;
};

#endif
