#include "memoryPool.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "misc.h"

LoggerPtr ChunkBank::_logger = Logger::getLogger("cn.leancloud.image.webservice.chunkBank");
LoggerPtr BlockStore::_logger = Logger::getLogger("cn.leancloud.image.webservice.elemRepo");
LoggerPtr MemoryPool::_logger = Logger::getLogger("cn.leancloud.image.webservice.memPool");
LoggerPtr GCThread::_logger = Logger::getLogger("cn.leancloud.image.webservice.mempool.GC");

ChunkBank::ChunkBank(int64_t limitation) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    _limit = limitation;
    if (_limit >= MAXCHUNKNUM) {
        _limit = MAXCHUNKNUM;
    }
    _usingCounter = 0;
    for (int64_t i = 0; i < MAXCHUNKNUM; ++i) {
        _chunks[i]._memoryAddress = NULL;
    }
}

int64_t ChunkBank::createChunk() {
    int64_t retval = INVALID_CHUNK_INDEX;
    pthread_mutex_lock(&_mutex);
    if (_usingCounter < _limit) {
        // find out free chunk
        for (int64_t i = 0; i < MAXCHUNKNUM; ++i) {
            if (NULL == _chunks[i]._memoryAddress) {
                retval = i;
                break;
            }
        }
        if (INVALID_CHUNK_INDEX != retval) {
            _chunks[retval]._memoryAddress = malloc(MAXCHUNKSIZE);
            ++_usingCounter;
        }
    }
    pthread_mutex_unlock(&_mutex);
    if (retval != INVALID_CHUNK_INDEX) {
        LOG4CXX_INFO(_logger, "malloc a new chunk, usingCounter:" << _usingCounter
                     << ", limit:" << _limit);
    } else {
        LOG4CXX_WARN(_logger, "internal error: can't find out free chunk. limit:"
                     << _limit << ", usingCounter:" << _usingCounter);
    }
    return retval;
}

int64_t ChunkBank::destroyChunk(const int64_t& index) {
    int64_t retval = -1;
    pthread_mutex_lock(&_mutex);
    if (index >=0 && index < MAXCHUNKNUM) {
        if (NULL != _chunks[index]._memoryAddress) {
            free(_chunks[index]._memoryAddress);
            _chunks[index]._memoryAddress = NULL;
        }
        _usingCounter--;
        retval = 0;
    }
    pthread_mutex_unlock(&_mutex);
    if (0 != retval) {
        LOG4CXX_WARN(_logger, "invalid index:" << index << ", usingCounter:" << _usingCounter);
    }
    return retval;
}

void* ChunkBank::getMemoryAddress(const int64_t& index) {
    void* retval = NULL;
    pthread_mutex_lock(&_mutex);
    if (index >= 0 && index < MAXCHUNKNUM) {
        retval = _chunks[index]._memoryAddress;
    }
    pthread_mutex_unlock(&_mutex);
    if (NULL == retval) {
        LOG4CXX_WARN(_logger, "invalid index:" << index << ", usingCounter:" << _usingCounter);
    }
    return retval;
}

string ChunkBank::stat() {
    static char buffer[128] = {0};
    snprintf(buffer, 128, "chunkLimit:%lld, usingChunkNum:%lld\n", _limit, _usingCounter);
    return string(buffer);
}

BlockStore::BlockStore(int64_t nodeSize, ChunkBank* chunkBank,
    int64_t idleTime) : _bank(chunkBank) {
    pthread_mutex_init(&_mutex, NULL);
    _elementSize = nodeSize - sizeof(ElementNode);
    _nodeSize = nodeSize;
    _blockCode = calcBlockCode(nodeSize);
    _chunkCapacity = MAXCHUNKSIZE / _nodeSize;
    _currentElementCounter = 0;
    _free._head = _free._tail = MP_NULL;
    _idleTime = idleTime;
    LOG4CXX_INFO(_logger, "create a new BlockStore with nodeSize:" << _nodeSize
        << ", eleSize:" << _elementSize << ", blockCode:" << _blockCode << ", cap:" << _chunkCapacity);
}

MP_ID BlockStore::allocFromFreeQueue() {
    MP_ID retval = MP_NULL;
    pthread_mutex_lock(&_mutex);
    if (_free._head != MP_NULL) {
        retval = _free._head;
        // set _free._head with the next
        int64_t index = getChunkIndexFromID(retval);
        ElementNode* node = getNodeFromID(retval);
        if (NULL == node || index < 0 || index >= MAXCHUNKNUM) {
            LOG4CXX_WARN(_logger, "failed to get element node from id:" << hex << retval);
        } else {
            _free._head = node->_next;
            --_currentElementCounter;
            _usageStatistic[index].decCounter();
        }
    }
    pthread_mutex_unlock(&_mutex);
    return retval;
}

MP_ID BlockStore::alloc() {
    MP_ID retval = allocFromFreeQueue();
    if (MP_NULL == retval) {
        int64_t index = _bank->createChunk();
        if (INVALID_CHUNK_INDEX == index) {
            LOG4CXX_WARN(_logger, "failed to alloc a new chunk");
        } else {
            addNewChunk(index);
            retval = allocFromFreeQueue();
        }
    }
    return retval;
}

int64_t BlockStore::addElement(MP_ID id) {
    int64_t retval = -1;
    int64_t index = getChunkIndexFromID(id);
    int64_t now = get_cur_microseconds_time() / 1000000;
    ElementNode* node = getNodeFromID(id);
    if (NULL == node) {
        LOG4CXX_WARN(_logger, "MP_ID error: can't get address from chunkBank, id:"
            << std::hex << id << ", index:" << std::hex << index);
    } else {
        node->_next = MP_NULL;

        if (MP_NULL == _free._head) {
            _free._head = id;
            _free._tail = id;
            ++_currentElementCounter;
            _usageStatistic[index].incCounter();
            _usageStatistic[index].setTimestamp(now);
            retval = 0;
        } else {
            MP_ID tailID = _free._tail;
            ElementNode* tailNode = getNodeFromID(tailID);
            if (NULL == tailNode) {
                LOG4CXX_WARN(_logger,
                    "internal error, can't get memory address for tail item of free queue, id=" << tailID);
            } else {
                tailNode->_next = id;
                _free._tail = id;
                ++_currentElementCounter;
                _usageStatistic[index].incCounter();
                _usageStatistic[index].setTimestamp(now);
                retval = 0;
            }
        }
    }
    return retval;
}

int64_t BlockStore::freeNode(MP_ID id) {
    int64_t retval = -1;
    if (MP_NULL == id) {
        return retval;
    }
    int64_t blockSize = getBlockSizeFromID(id);
    if (blockSize != _nodeSize) {
        LOG4CXX_WARN(_logger, "blocksize is wrong! size:" << blockSize << ", nodeSize:" << _nodeSize);
        return retval;
    }
    pthread_mutex_lock(&_mutex);
    retval = addElement(id);
    pthread_mutex_unlock(&_mutex);

    return retval;
}

int64_t BlockStore::calcRecyclableChunk() {
    LOG4CXX_DEBUG(_logger, "begin to calculate Recyclable chunk nodeSize=" 
                  << _nodeSize << ", eleSize=" << _elementSize << ", curEleCounter=" 
                  << _currentElementCounter << ", chkCapacity=" << _chunkCapacity);
    int64_t result = INVALID_CHUNK_INDEX;
    int64_t now = get_cur_microseconds_time() / 1000000;
    pthread_mutex_lock(&_mutex);
    for (int i = 0; i < MAXCHUNKNUM; ++i) {
        if (_usageStatistic[i].getCounter() >= (uint64_t)_chunkCapacity
            && idleForLongtime(now, _usageStatistic[i].getTimestamp())) {
            LOG4CXX_INFO(_logger, "recycle chunk index=" << i);
            // found out a chunk to recycle. scan free queue to pick up element which chunk index is i.
            result = i;
            _usageStatistic[i].setCounter(0);
            _usageStatistic[i].setTimestamp(0);
            MP_ID curID = _free._head;
            int64_t curIndex = INVALID_CHUNK_INDEX;
            while ((curID != MP_NULL) && (curIndex = getChunkIndexFromID(curID)) == i) {
                ElementNode* curNode = getNodeFromID(curID);
                if (NULL == curNode) {
                    break;
                }
                curID = curNode->_next;
            };
            _free._head = curID;
            if (MP_NULL != curID) {
                MP_ID prevID = curID;
                ElementNode* prevNode = getNodeFromID(prevID);
                while (MP_NULL != prevID && NULL != prevNode) {
                    curID = prevNode->_next;
                    curIndex = getChunkIndexFromID(curID);
                    ElementNode* curNode = getNodeFromID(curID);
                    while ((curID != MP_NULL)
                            && (curIndex == i) && (NULL != curNode)) {
                        curID = curNode->_next;
                        curIndex = getChunkIndexFromID(curID);
                        curNode = getNodeFromID(curID);
                    };
                    prevNode->_next = curID;
                    prevID = curID;
                    prevNode = getNodeFromID(prevID);
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&_mutex);
    return result;
}

string BlockStore::stat() {
    static char buffer[256] = {0};
    snprintf(buffer, 256, "blockCode:%lld, nodeSize:%lld, capacity:%lld, usingNum:%lld\n",
        _blockCode, _nodeSize, _chunkCapacity, _currentElementCounter);
    return string(buffer);
}

void BlockStore::addNewChunk(const int64_t chunkIndex) {
    // add new element to free queue;
    MP_ID start = (_blockCode << (MAXCHUNKNUMBIT + MAXCHKIDXBIT)) + (chunkIndex << MAXCHKIDXBIT) + 0;
    LOG4CXX_INFO(_logger, "add new chunk with blockCode:" << std::hex << _blockCode
        << ", index:" << chunkIndex << ", startID:" << std::hex << start);
    pthread_mutex_lock(&_mutex);
    MP_ID newId = start;
    addElement(newId);
    for (int i = 1; i < _chunkCapacity; i++) {
        newId += _nodeSize;
        addElement(newId);
    }
    pthread_mutex_unlock(&_mutex);
}

MemoryPool::MemoryPool(int64_t limitation, bool autoGC, int64_t idleTime)
    : _bank(limitation/MAXCHUNKSIZE) {
    _repos.reserve(ELEMENT_SIZE_COUNTER);
    _repos.push_back(BlockStore(ELEMENT_SIZE_8K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_16K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_32K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_64K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_128K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_256K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_512K, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_1M, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_4M, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_8M, &_bank, idleTime));
    _repos.push_back(BlockStore(ELEMENT_SIZE_16M, &_bank, idleTime));
    _gcThread = NULL;
    if (autoGC) {
        _gcThread = new GCThread();
        _gcThread->start(true, this);
    }
}

MemoryPool::~MemoryPool() {
    if (NULL == _gcThread) {
        return;
    }
    _gcThread->join();
    delete _gcThread;
    _gcThread = NULL;
}

MP_ID MemoryPool::alloc(size_t expectedSize) {
    int64_t repoIndex = getTargetRepoIndex(expectedSize);
    if (repoIndex < 0) {
        LOG4CXX_WARN(_logger, "expected size is too big:" << expectedSize);
        return MP_NULL;
    }
    return _repos[repoIndex].alloc();
}

void* MemoryPool::getElementAddress(MP_ID id, int64_t& len) {
    len = 0;
    char* address = NULL;
    int64_t index = BlockStore::getChunkIndexFromID(id);
    int64_t blockCode = BlockStore::getBlockCodeFromID(id);
    int64_t offset = BlockStore::getOffsetFromID(id);
    address = (char*)_bank.getMemoryAddress(index);
    if (NULL != address && blockCode >= 0 && blockCode < ELEMENT_SIZE_COUNTER) {
        address += offset;
        len = _repos[blockCode].getElementSize();
        return address;
    } else {
        return NULL;
    }
}

string MemoryPool::stat() {
    string result = _bank.stat();
    for (size_t i = 0; i < _repos.size(); ++i) {
        result += _repos[i].stat();
    }
    return result;
}

void MemoryPool::freeNode(MP_ID id) {
    int64_t blockCode = BlockStore::getBlockCodeFromID(id);
    if (blockCode < 0 || blockCode >= ELEMENT_SIZE_COUNTER) {
        LOG4CXX_WARN(_logger, "invalid MP_ID:" << hex << id);
    } else {
        _repos[blockCode].freeNode(id);
    }
    return;
}

void MemoryPool::garbageCollect() {
    LOG4CXX_INFO(_logger, "begin to garbage collect...");
    int64_t targetChunkIndex = INVALID_CHUNK_INDEX;
    int64_t destroyResult = -1;
    for (size_t i = 0; i < _repos.size(); ++i) {
        targetChunkIndex = _repos[i].calcRecyclableChunk();
        if (INVALID_CHUNK_INDEX != targetChunkIndex) {
            destroyResult = _bank.destroyChunk(targetChunkIndex);
            if (0 == destroyResult) {
                LOG4CXX_INFO(_logger, "recycle chunk index=" << targetChunkIndex
                    << " from repository index=" << i);
            } else {
                LOG4CXX_WARN(_logger, "failed to recycle chunk index="
                    << targetChunkIndex << " from repository index=" << i);
            }
        }
    }
}

int64_t MemoryPool::getTargetRepoIndex(size_t expectedSize) {
    int64_t retval = -1;
    for (unsigned int i = 0; i < _repos.size(); ++i) {
        if ((int64_t)expectedSize <= _repos[i].getElementSize()) {
            retval = i;
            break;
        }
    }
    return retval;
}

void* GCThread::_main(void* param) {
    MemoryPool* pool = (MemoryPool*)param;
    if (NULL == pool) {
        LOG4CXX_WARN(_logger, "GCThread exit because MemoryPool instance is NULL");
        return NULL;
    } else {
        while(isRunning()) {
            LOG4CXX_INFO(_logger, "start to GarbageCollection...");
            pool->garbageCollect();
            sleep(120);
        }
    }
    LOG4CXX_INFO(_logger, "terminate GCThread");
    return NULL;
}
