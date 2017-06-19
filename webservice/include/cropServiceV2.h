#ifndef CN_LEANCLOUD_IMAGESERVICE_CROP_SERVICE_V2_INCLUDE_H_
#define CN_LEANCLOUD_IMAGESERVICE_CROP_SERVICE_V2_INCLUDE_H_

#include "thumbnailer.h"
#include "config.h"
#include "commonService.h"

// crop service v2
// usage:
//    host/base/{bucket}/{filename}?w=x&h=x&hs=xx&extra_info=xxx&q=xx&fm=xx&hue=xx&sat=xx&bri=xx&crop=xxx&exp=xx&blur=xx&clips=xx&rbh=xxx
//
class CropServiceV2 : public CommonService{

public:
    CropServiceV2(Thumbnailer* thumbnailer,
                  MemoryPool& memoryPool,
                  StoragePool& storagePool);
    virtual ~CropServiceV2();
    // handle a new request
    virtual void operator() (const request_ptr& request,
                             const tcp::connection_ptr& tcp_conn);

private:
    void parseQueryString(const string& queryString,
                          int& w,
                          int& h,
                          int& hostStyle,
                          int& extraInfo,
                          ImageOperator* opers);

    bool shouldOriginalImage(int w, int h, ImageOperator* opers);

    int generateFileName(char* buffer,
                         int bufferLen,
			 const string& bucket,
                         const string& key,
                         int width,
                         int height,
                         ImageOperator* opers,
                         bool shouldOriginalImage);

    void sendImage(response_writer_ptr writer,
                   Image* result,
                   const string& fileName);

    int doThumbnail(const char* src, size_t lenOfSrc,
                    char* desc, size_t lenOfDesc,
                    const Rect& rectOfInterest,
                    int width, int height,
                    ImageOperator* opers, ExtraImageInfo* extraImageInfo);

    void handleNotFound(response_writer_ptr writer,
                        const string& bucket,
                        int hostStyle,
                        int targetWidth,
                        int targetHeight);

    std::string getOriginalFileUrl(const string& host,
                                   const string& bucket,
                                   const string& filename,
                                   int uriStyle);
protected:
    static LoggerPtr _logger;
    Thumbnailer* _thumbnailer;   // pointer to thumbnail
};

#endif // CN_LEANCLOUD_IMAGESERVICE_CROP_SERVICE_V2_INCLUDE_H_
