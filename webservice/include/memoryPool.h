/**======================================================================
 *
 *  memoryPool.h
 *
 *  -------------------------------------------------------------------
 *  Description
 *
 *     definition of memory pool for fixed-length elemen
 *     provide allocation feature
 *     recycle is supported
 *
 *  -------------------------------------------------------------------
 *
 *  Definition:
 *
 *      MP ID: addess in a memory pool. Each MP_ID can be illustrated as
 *               |element size level | chunk idx |     offset     |
 *      64-bit   |-------5 bits----- |--12bits---|----24bit-------|
 *
 *  Macros and Constants:
 *
 *      MP_NULL - NULL of Memory Pool. The top limit of a memory pool
 *                 is 64G on 64-bit platform.
 *                 So we defines NULL as a big number that MP ID can
 *                 never reached.
 *      MAXCHUNKNUM - max chunk number. Each chunk is a block of real memory.
 *                    memory_pool_t calls malloc() to get a chunk each time. Each
 *                    chunk can hold a number of elements. It's value is 2^12 on 64-bit platform.
 *      MAXCHUNKSIZE - top limit of chunk size. It's value is 2^24.
 *      OFFSETMASK - a const for get offset from a MP ID.
 *      IDXMASK - a const for get chunk idx form a MP ID.
 *
 =====================================================================**/

#ifndef CN_LEANCLOUD_IMAGE_SERVICE_MEMORY_POOL_INCLUDE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_MEMORY_POOL_INCLUDE_H_

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <map>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "thread.h"
#include "misc.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

typedef size_t MP_ID;

const int MAXCHKIDXBIT=24;
const int MAXCHUNKNUMBIT=12;
const MP_ID MP_NULL = 0xFFFFFFFFFFFFFFFF;
const int64_t MAXCHUNKNUM = 0x0001000; // 2^12
const int64_t MAXCHUNKSIZE = 0x01000000; // 2^24
const int64_t OFFSETMASK = 0x00FFFFFF;
const int64_t CHKIDXMASK = (int64_t)0x00000FFF << MAXCHKIDXBIT;
const int64_t BLOCKSIZEMASK = (int64_t)0x0000001F << (MAXCHKIDXBIT + MAXCHUNKNUMBIT);

const int64_t INVALID_CHUNK_INDEX = -1;

const int64_t ELEMENT_SIZE_8K = 8 * 1024;
const int64_t ELEMENT_SIZE_16K = 16 * 1024;
const int64_t ELEMENT_SIZE_32K = 32 * 1024;
const int64_t ELEMENT_SIZE_64K = 64 * 1024;
const int64_t ELEMENT_SIZE_128K = 128 * 1024;
const int64_t ELEMENT_SIZE_256K = 256 * 1024;
const int64_t ELEMENT_SIZE_512K = 512 * 1024;
const int64_t ELEMENT_SIZE_1M = 1 * 1024 * 1024;
const int64_t ELEMENT_SIZE_4M = 4 * 1024 * 1024;
const int64_t ELEMENT_SIZE_8M = 8 * 1024 * 1024;
const int64_t ELEMENT_SIZE_16M = 16 * 1024 * 1024;

const int64_t ELEMENT_SIZE_ARRAY[] = {
    ELEMENT_SIZE_8K,
    ELEMENT_SIZE_16K,
    ELEMENT_SIZE_32K,
    ELEMENT_SIZE_64K,
    ELEMENT_SIZE_128K,
    ELEMENT_SIZE_256K,
    ELEMENT_SIZE_512K,
    ELEMENT_SIZE_1M,
    ELEMENT_SIZE_4M,
    ELEMENT_SIZE_8M,
    ELEMENT_SIZE_16M,
};

const int64_t ELEMENT_SIZE_COUNTER = sizeof(ELEMENT_SIZE_ARRAY) / sizeof(ELEMENT_SIZE_ARRAY[0]);

struct ChunkHeader {
    void* _memoryAddress;
};

/*
 * ChunkBank, alloc and recycle chunk as a manager.
 */
class ChunkBank {
private:
    static LoggerPtr _logger;
    pthread_mutex_t _mutex;
    int64_t _limit;
    int64_t _usingCounter;      // chunk using counter
    ChunkHeader _chunks[MAXCHUNKNUM];
public:
    // constructor
    // @param(in) limitation - memory limitation in chunks.
    ChunkBank(int64_t limitation);
    // destructor
    ~ChunkBank() {
        for (int64_t i = 0; i < MAXCHUNKNUM; ++i) {
            if (NULL != _chunks[i]._memoryAddress) {
                free(_chunks[i]._memoryAddress);
                _chunks[i]._memoryAddress = NULL;
            }
        }
        pthread_mutex_destroy(&_mutex);
    }
    // create a new chunk
    // @return chunk index
    //         -1 - error
    int64_t createChunk();
    // destroy a chunk.
    // Notice: invoked by GC thread
    int64_t destroyChunk(const int64_t& index);
    // get valid memory address by index
    // @param(in) index - chunk index;
    void* getMemoryAddress(const int64_t& index);
    // report status
    string stat();
};

struct ElementNode {
    MP_ID _next;      // next pointer
    char  _buffer[0]; // buffer pointer;
};

struct ElementQueue {
    MP_ID _head; // head
    MP_ID _tail; // tail
};

struct ChunkUsageStatistic {
    uint64_t _counter;// high 32 bit: usingcounter
                      // low 32 bit: timestamp in seconds
    ChunkUsageStatistic() {
        _counter = 0;
    };
    uint64_t getCounter() {
        uint64_t ret = (_counter & 0xFFFFFFFF00000000);
        return ret >> 32;
    };
    uint64_t getTimestamp() {
        return (_counter & 0x00000000FFFFFFFF);
    };
    void decCounter() {
        _counter -= ((uint64_t)0x0000000000000001 << 32);
    };
    void incCounter() {
        _counter += ((uint64_t)0x0000000000000001 << 32);
    };
    void setCounter(uint64_t counter) {
        uint64_t timestamp = getTimestamp();
        _counter = timestamp + (counter << 32);
    };
    void setTimestamp(uint64_t timestamp) {
        uint64_t counter = getCounter();
        _counter = (counter << 32) + timestamp;
    };
};

/*
 * BlockStore with special size.
 */
class BlockStore {
private:
    static LoggerPtr _logger;
    pthread_mutex_t _mutex;
    int64_t _elementSize;          // element max size, equals _nodeSize - sizeof(ElementNode)
    int64_t _nodeSize;             // node size, i.e 8k, 16k, 32k,..4M
    int64_t _blockCode;            // block code indicate the index in ELEMENT_SIZE_ARRAY
    int64_t _chunkCapacity;        // chunk Capacity = chunkSize / nodeSize
    volatile int64_t _currentElementCounter;// current element counter
    ElementQueue _free;            // free element queue
    ChunkUsageStatistic _usageStatistic[MAXCHUNKNUM];
    ChunkBank* _bank;              // chunk bank
    int64_t _idleTime;             // recycle interval
public:
    // contructor
    BlockStore(int64_t nodeSize, ChunkBank* chunkBank, int64_t idleTime);
    // destructor
    ~BlockStore() {
        _bank = NULL;
    }

    // alloc a new element
    // MP_NULL for error
    MP_ID alloc();

    // free an element
    // @return int64_t
    //        0 - success
    //        -1 - error
    int64_t freeNode(MP_ID);

    // calculate recyclable chunk
    // @return chunk index
    //         -1 for nothing
    int64_t calcRecyclableChunk();

    string stat();

    // return element max size.
    int64_t getElementSize();
    ElementNode* getNodeFromID(MP_ID id);
    static int64_t calcBlockCode(int64_t blockSize);
    static int64_t getBlockCodeFromID(MP_ID id);
    static int64_t getBlockSizeFromID(MP_ID id);
    static int64_t getChunkIndexFromID(MP_ID id);
    static int64_t getOffsetFromID(MP_ID id);

private:
    int64_t addElement(MP_ID id);
    // add a new chunk to free queue.
    void addNewChunk(const int64_t chunkIndex);
    MP_ID allocFromFreeQueue();
    bool idleForLongtime(int64_t now, int64_t recentIdleTime);
};

inline ElementNode* BlockStore::getNodeFromID(MP_ID id) {
    ElementNode* result = NULL;
    int64_t index = getChunkIndexFromID(id);
    int64_t offset = getOffsetFromID(id);
    void* address = _bank->getMemoryAddress(index);
    if (NULL != address) {
        result = (ElementNode*)((char*)address + offset);
    }
    return result;
};

inline int64_t BlockStore::getElementSize() {
    return _elementSize;
};

inline int64_t BlockStore::calcBlockCode(int64_t blockSize) {
    int64_t retval = -1;
    for (int64_t i = 0; i < ELEMENT_SIZE_COUNTER; ++i) {
        if (blockSize == ELEMENT_SIZE_ARRAY[i]) {
            retval = i;
            break;
        }
    }
    return retval;
};

inline int64_t BlockStore::getBlockCodeFromID(MP_ID id) {
    int64_t retval = -1;
    if (MP_NULL == id) {
        return retval;
    }
    retval = (id & BLOCKSIZEMASK) >> (MAXCHUNKNUMBIT + MAXCHKIDXBIT);
    return retval;
};

inline int64_t BlockStore::getBlockSizeFromID(MP_ID id) {
    int64_t retval = getBlockCodeFromID(id);
    if (-1 != retval) {
        retval = ELEMENT_SIZE_ARRAY[retval];
    }
    return retval;
};

inline int64_t BlockStore::getChunkIndexFromID(MP_ID id) {
    int64_t retval = -1;
    if (MP_NULL == id) {
        return retval;
    }
    retval = (id & CHKIDXMASK) >> MAXCHKIDXBIT;
    return retval;
};

inline int64_t BlockStore::getOffsetFromID(MP_ID id) {
    if (MP_NULL == id) {
        return -1;
    }
    int64_t retval = id & OFFSETMASK;
    return retval;
};

inline bool BlockStore::idleForLongtime(int64_t now, int64_t recentIdleTime) {
    if (now - recentIdleTime > _idleTime) {
        return true;
    }
    return false;
};

class GCThread;
class MemoryPool {
private:
    static LoggerPtr _logger;
    GCThread* _gcThread;// internal GC thread
    ChunkBank _bank;
    vector<BlockStore> _repos;
public:
    // constructor
    // @param(in) limitation - limitation in bytes
    // @param(in) autoGC - whether startup inner thread for automation GC
    // @param(in) idleTime - idle time for GC(unit: second), default is 900(15 miutes)
    MemoryPool(int64_t limitation, bool autoGC = false, int64_t idleTime = 900);
    ~MemoryPool();
    // create a new Element with the specified size
    // @param(in) - expectedSize
    MP_ID alloc(size_t expectedSize);
    // get element valid data address
    // @param(out) - len
    // @return: memory address
    //          NULL for error
    void* getElementAddress(MP_ID id, int64_t& len);
    // free a element
    void freeNode(MP_ID id);
    void garbageCollect();
    string stat();
private:
    int64_t getTargetRepoIndex(size_t expectedSize);
};

class GCThread : public Thread {
    static LoggerPtr _logger;
protected:
    virtual void* _main(void* param);
};


#endif

