#ifndef AVOS_BACKEND_IMAGE_SERVICE_TEST_MEMPOOL_CONSUMER_INCLUDE_H_
#define AVOS_BACKEND_IMAGE_SERVICE_TEST_MEMPOOL_CONSUMER_INCLUDE_H_

#include <unistd.h>
#include "thread.h"
#include "memoryPool.h"
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

LoggerPtr mmcLogger = Logger::getLogger("mempoolConsumer");

class MempoolConsumer: public Thread {
private:
    MemoryPool* _pool;
    int64_t _sizeLevel;
    vector<MP_ID> _idList;

public:
    MempoolConsumer(MemoryPool* pool, int64_t sizeLevel) {
        _pool = pool;
        _sizeLevel = sizeLevel;
        srand((unsigned int)time(NULL));
    }
    ~MempoolConsumer() {
        while(_idList.size() > 0) {
            MP_ID id = _idList[_idList.size() -1];
            _idList.pop_back();
            _pool->freeNode(id);
        }
    }
    virtual void * _main(void * param) {
        LOG4CXX_WARN(mmcLogger, "enter thread main...");
        while(isRunning()) {
            int loop = rand() % 17;
            int isRead = rand() % 2;
            for(int i = 0; i < loop && _idList.size() > 0; i++) {
                if (isRead) {
                    int diff = rand() % 997 + 32;
                    MP_ID id = _pool->alloc(_sizeLevel - diff);
                    if (MP_NULL == id) {
                        //
                        LOG4CXX_WARN(mmcLogger, "failed to alloc element from pool, sizeLevel:" << _sizeLevel << ", elementSize:" << _sizeLevel - diff);
                    } else {
                        int64_t len = 0;
                        char* addr = (char*)_pool->getElementAddress(id, len);
                        memset(addr, ('a' + (diff % 20)), _sizeLevel - diff);
                        _idList.push_back(id);
                    }
                } else {
                    MP_ID id = _idList[_idList.size() -1];
                    _idList.pop_back();
                    if (MP_NULL == id) {
                        //
                        LOG4CXX_WARN(mmcLogger, "illegal MP_ID pop from cache");
                    } else {
                        _pool->freeNode(id);
                    }
                }
            }
            LOG4CXX_INFO(mmcLogger, "finished one trip for testing in multi-thread mode");
            int tm = rand() % 3;
            sleep(tm);
        }
        return NULL;
    }
};

#endif
