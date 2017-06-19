#ifndef CN_LEANCLOUD_IMAGESERVICE_STATUS_SERVICE_INCLUDE_H_
#define CN_LEANCLOUD_IMAGESERVICE_STATUS_SERVICE_INCLUDE_H_

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "gridfsStorage.h"
#include "storagePool.h"
#include "memoryPool.h"
#include "config.h"
#include "pion_common.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

class StatusService : public plugin_service {
public:
    StatusService(MemoryPool& memoryPool, StoragePool& storagePool);
    virtual ~StatusService();
    virtual void operator() (const HTTPRequestPtr& request, const TCPConnectionPtr& tcp_conn);
    string stat();
protected:
    static LoggerPtr _logger;
    MemoryPool& _memoryPool;
    StoragePool& _storagePool;
    int64_t _startTime;
};

#endif
