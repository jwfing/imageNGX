#ifndef CN_LEANCLOUD_IMAGE_SERVICE_RAINBOW_SERVICE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_RAINBOW_SERVICE_H_

#include "imagickRainbow.h"
#include "config.h"
#include "commonService.h"


class RainbowService : public CommonService {
 public:
    RainbowService(MemoryPool& memoryPool,
                   StoragePool& storagePool);
    virtual ~RainbowService();

    virtual void operator() (const HTTPRequestPtr& request,
                             const TCPConnectionPtr& tcp_conn);

 private:
    void parseQuery(const HTTPRequestPtr& request,
                    vector<int>& clips,
                    int& height);

 private:
    static LoggerPtr _logger;
    ImagickRainbow _rainbow;
};

#endif // CN_LEANCLOUD_IMAGE_SERVICE_RAINBOW_SERVICE_H_

