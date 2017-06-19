#ifndef CN_LEANCLOUD_IMAGE_SERVICE_MONTAGE_SERVICE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_MONTAGE_SERVICE_H_


#include "imagickMontage.h"
#include "config.h"
#include "commonService.h"

class MontageService : public CommonService {
 public:
    MontageService(MemoryPool& memoryPool,
                   StoragePool& storagePool);
    virtual ~MontageService();

    virtual void operator() (const HTTPRequestPtr& request,
                             const TCPConnectionPtr& tcp_conn);

 private:
    void parseQuery(const HTTPRequestPtr& queryString,
                    string& bukect,
                    string& keys,
                    int& hostStyle,
                    int& width,
                    int& height,
                    int& margin,
                    int& cols,
                    string& background);

 private:
    static LoggerPtr _logger;
    ImagickMontage _montager;
};

#endif // CN_LEANCLOUD_IMAGE_SERVICE_MONTAGE_SERVICE_H_

