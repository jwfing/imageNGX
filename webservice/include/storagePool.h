#ifndef CN_LEANCLOUD_IMAGESERVICE_STORAGEPOOL_INCLUDE_H_
#define CN_LEANCLOUD_IMAGESERVICE_STORAGEPOOL_INCLUDE_H_

#include <vector>
#include <map>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "storage.h"
#include "thread.h"
#include "memoryPool.h"
#include "atomic.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

// Storage Pool
// it can queue write task and support threading.
class StoragePool{
private:
    // write item
    struct StoreItem {
        string _fileID;    // file id
        MP_ID _mpID;       // memory pool ID
        const void* _address;  // real data address
        int64_t _dataLen;      // binary data length
        const char* _bucket;

        StoreItem() {
            _fileID = "";
            _mpID = MP_NULL;
            _address = NULL;
            _dataLen = 0;
            _bucket = "";
        }
        StoreItem(string fileID, MP_ID mpID, const void* address, int64_t dataLen, const char* bucket) {
            _fileID = fileID;
            _mpID = mpID;
            _address = address;
            _dataLen = dataLen;
            _bucket = bucket;
        }

    };
    // inner writer thread class.
    // each thread takes a seperate storage instance, so storage doesn't care multi-thread.
    class WriterThread : public Thread {
    public:
        WriterThread(Storage* storage) {
            _storage = storage->clone();
        }
        WriterThread(const WriterThread& other) {
            if (this != &other) {
                _storage = other._storage->clone();
            }
        }
        ~WriterThread() {
            if (NULL != _storage) {
                delete _storage;
                _storage = NULL;
            }
        }
    protected:
        virtual void* _main(void* param);
        Storage* _storage;
        static LoggerPtr _logger;

    };
    Storage* _storage;
    vector<WriterThread> _threads; // make threading flexiable? not a big deal
    vector<StoreItem> _tasks;      // task queue
    pthread_mutex_t _mutex;        // mutex for updating task queue
    int64_t _maxThreadNum;         // max storage thread num
    int64_t _maxTaskNum;           // max task num
    MemoryPool& _mp;               // reference of memory pool
    volatile uint64_t _savedCounter;       // normally saved image counter
    volatile uint64_t _saveFailedCounter;  // failed saved image counter
    volatile uint64_t _discardCounter;     // discard image counter
    static LoggerPtr _logger;
public:
    StoragePool(MemoryPool& mp, int64_t maxThreadNum, int64_t maxTaskNum)
        : _storage(NULL), _maxThreadNum(maxThreadNum), _maxTaskNum(maxTaskNum), _mp(mp),
        _savedCounter(0), _saveFailedCounter(0), _discardCounter(0) {
        pthread_mutex_init(&_mutex, NULL);
    };
    ~StoragePool() {
        destroy();
    };
    // initialize storage instance, and start storage threading.
    bool initStorageObject(Storage* storage);
    // destroy inner data and stop threading
    void destroy();
    // save a image under async mode
    // @param(in) id -
    // @param(in) data -
    // @param(in) dataLen -
    // @param(in) id -
    // @return 0 - successful
    //         <0 - queue is full
    int saveImage(const char* id, const void* data, const int dataLen, MP_ID mpID, const char* bucket);
    // retrive a image with sync mode
    // @return pointer to image instance
    //          NULL if error
    Image* getImage(const char* id, const char* bucket);
    Image* getImage(const char* id, const char* bucket, int hostStyle);

    bool deleteImage(const char* id, const char* bucket);
    
    // try to check if image is exist
    // @param(in) id - image keyfile
    // @param(in) bucket - image bucket
    // @param(in) hostStyle - path or virtual proxy
    // @return true if image is exist, otherwise false
    bool headImage(const char* id, const char* bucket, int hostStyle);

    // relase image instance
    void freeImage(Image*& ptr);
    // status report
    string stat();
    void incSaveCounter() {
        atomic_inc(&_savedCounter);
    };
    void incSaveFailedCounter() {
        atomic_inc(&_saveFailedCounter);
    };
    void incDiscardCounter() {
        atomic_inc(&_discardCounter);
    };

private:
    Storage* initLocalStorage();

protected:
    // push back a new task
    // @return 0 - successful
    //         <0 - failed.
    int pushTask(const StoreItem& task);
    // pop back a new task
    // @return 0 - successful
    //         <0 - failed
    int popTask(StoreItem& task);
};

#endif
