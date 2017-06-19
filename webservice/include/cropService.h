#ifndef CN_LEANCLOUD_IMAGESERVICE_CROP_SERVICE_INCLUDE_H_
#define CN_LEANCLOUD_IMAGESERVICE_CROP_SERVICE_INCLUDE_H_

#include "thumbnailer.h"
#include "gridfsStorage.h"
#include "config.h"
#include "commonService.h"

// crop service
class CropService : public CommonService {
public:
    CropService(Thumbnailer* thumbnailer, MemoryPool& memoryPool, StoragePool& storagePool);
    virtual ~CropService();
    // handle a new request
    virtual void operator() (const pion::http::request_ptr& request,
                             const pion::tcp::connection_ptr& tcp_conn);
    // parse query string
    static void parseQueryString(const string& original, string& urlMD5, string& domain,
        int& width, int& height, int& x, int& y, int& x2, int& y2);

protected:
    // generate file name with md5/width/height/x/y/x2/y2
    int generateFileName(char* buffer, int bufferLen, const string& md5, int width,
                int height, int x, int y, int x2, int y2);
    // if need decode
    static bool isNeedDecode(const char* url);

    // decode url
    static char* decodeUrl(char* url);

protected:
    static LoggerPtr _logger;
    Thumbnailer* _thumbnailer;   // pointer to thumbnail
};

#endif
