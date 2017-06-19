#include "storagePool.h"
#include <unistd.h>

LoggerPtr StoragePool::WriterThread::_logger = Logger::getLogger("cn.leancloud.imageservice.writerThread");
LoggerPtr StoragePool::_logger = Logger::getLogger("cn.leancloud.imageservice.storagePool");

void* StoragePool::WriterThread::_main(void* param) {
    sleep(1);
    if (NULL == _storage) {
        LOG4CXX_WARN(_logger, "storage instance isn't initialized.");
        return NULL;
    }
    StoragePool* pool = (StoragePool*)param;
    if (NULL == pool) {
        LOG4CXX_WARN(_logger, "storage pool is NULL.");
        return NULL;
    }
    LOG4CXX_INFO(_logger, "start storage thread - " << pthread_self());
    int writeResult = 0;
    int pickTask = 0;
    while(isRunning()) {
        StoreItem task;
        pickTask = pool->popTask(task);
        if (pickTask != 0) {
            sleep(1);
        } else {
            LOG4CXX_DEBUG(_logger, "save a image with id:" << task._fileID << ", mpID:" << task._mpID << ", address:" << task._address << ", dataLen:" << task._dataLen);
            writeResult = _storage->saveImage(task._fileID.c_str(), task._address, (const int)task._dataLen, task._bucket);
            if (writeResult != 0) {
                pool->incSaveFailedCounter();
                // log output
                LOG4CXX_WARN(_logger, "failed to saveImage: " << task._fileID);
            } else {
                pool->incSaveCounter();
            }
            pool->_mp.freeNode(task._mpID);
        }
    }
    LOG4CXX_INFO(_logger, "terminate storage thread - " << pthread_self());
    return NULL;
}

bool StoragePool::initStorageObject(Storage* storage) {
    if (NULL == storage) {
        LOG4CXX_WARN(_logger, "invalid param. storage is NULL");
        return false;
    }
    if (NULL != _storage) {
        delete _storage;
        _storage = NULL;
    }
    _storage = storage->clone();
    LOG4CXX_INFO(_logger, "begin to start writer thread, num=" << _maxThreadNum);
    for (int64_t i = 0; i < _maxThreadNum; ++i) {
        _threads.push_back(WriterThread(_storage));
    }
    for (int64_t i = 0; i < _maxThreadNum; ++i) {
        _threads[i].start(false, this);
    }
    return true;
}

void StoragePool::destroy() {
    LOG4CXX_INFO(_logger, "ternimate writer threads...");
    for(int64_t i = 0; i < _maxThreadNum && i < (int64_t)_threads.size(); ++i) {
        _threads[i].join();
    }
    _threads.clear();
    for(size_t j = 0; j < _tasks.size(); ++j) {
        if (MP_NULL != _tasks[j]._mpID) {
            _mp.freeNode(_tasks[j]._mpID);
        }
    }
    _tasks.clear();
    pthread_mutex_destroy(&_mutex);
}

Storage* StoragePool::initLocalStorage() {
    if (NULL == _storage) {
        LOG4CXX_WARN(_logger, "internal error: storage instance is NULL");
        return NULL;
    }
    // making the storage thread local
    static __thread Storage* storage = NULL;

    if(NULL == storage)
    {
        LOG4CXX_INFO(_logger, "***********StoragePool::getImage: creating the storage for read for the first time***************");
        storage = _storage->clone();
    }

    if (NULL == storage) {
        LOG4CXX_WARN(_logger, "internal error: getImage cannot clone proper storage to read");
        return NULL;
    }
    return storage;
}

int StoragePool::saveImage(const char* id, const void* data, const int data_len, MP_ID mpID, const char* bucket) {
    int retVal = pushTask(StoreItem(string(id), mpID, data, data_len, bucket));
    if (retVal != 0) {
        incDiscardCounter();
    }
    return retVal;
}

Image* StoragePool::getImage(const char* id, const char* bucket) {
    return getImage(id, bucket, 0);
}

Image* StoragePool::getImage(const char* id, const char* bucket, int hostStyle) {
    Storage* storage = initLocalStorage();
    if (storage == NULL) {
        return NULL;
    }
    return storage->getImage(id, bucket, hostStyle);
}

bool StoragePool::headImage(const char* id, const char* bucket, int hostStyle) {
    Storage* storage = initLocalStorage();
    if (storage == NULL) {
        return false;
    }
    return storage->headImage(id, bucket, hostStyle);
}

bool StoragePool::deleteImage(const char* id, const char* bucket) {
    Storage* storage = initLocalStorage();
    if (storage == NULL) {
        return false;
    }
    return storage->deleteImage(id, bucket);
}

string StoragePool::stat() {
    char buffer[128] = {0};
    snprintf(buffer, 128, "queuedSize:%ld, discardSize:%lld, savedCounter:%lld, saveFailedCounter:%lld\n",
        _tasks.size(), _discardCounter, _savedCounter, _saveFailedCounter);
    return string(buffer);
}

void StoragePool::freeImage(Image*& ptr) {
    if (NULL == _storage) {
        LOG4CXX_WARN(_logger, "internal error: storage instance is NULL");
        return;
    }
    if (NULL != ptr) {
        _storage->freeImage(ptr);
    }
}

int StoragePool::pushTask(const StoreItem& task) {
    int result = 0;
    pthread_mutex_lock(&_mutex);
    if ((int64_t)_tasks.size() >= _maxTaskNum) {
        result = -1;
    } else {
        _tasks.push_back(task);
    }
    pthread_mutex_unlock(&_mutex);
    if (result < 0) {
        LOG4CXX_INFO(_logger,
            "discard item b/c queue is full. id:" << task._fileID << ", memID:" << task._mpID);
    }
    return result;
}

int StoragePool::popTask(StoreItem& task) {
    int retval = -1;
    pthread_mutex_lock(&_mutex);
    if (_tasks.size() > 0) {
        task = _tasks[_tasks.size() - 1];
        _tasks.pop_back();
        retval = 0;
    }
    pthread_mutex_unlock(&_mutex);
    return retval;
}
