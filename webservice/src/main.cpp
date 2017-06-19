#include <unistd.h>
#include <iostream>
#include <string>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include <pion/scheduler.hpp>
#include <pion/http/plugin_server.hpp>

#include "common.h"
#include "cropService.h"
#include "cropServiceV2.h"
#include "montageService.h"
#include "rainbowService.h"
#include "statusService.h"
#include "thumbnailerFactory.h"
#include "defaultFavicon.h"
#include "fileService.h"
#include "localStorage.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

LoggerPtr logger_asm(Logger::getLogger("cn.leancloud.imageservice"));

void processPipe(int signo) {
    LOG4CXX_ERROR(logger_asm, "recieve broken pip signo");
    signal(signo, SIG_IGN);
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, processPipe);
    BasicConfigurator::configure();
    Config config(CONF_FILE_PATH);
    int tcpPort = config.read<int>(
                CONF_LISTEN_PORT,
                CONF_DEFAULT_LISTEN_PORT);
    if (argc == 2) {
        // config port through command line
        tcpPort = atoi(argv[1]);
    }
    int memoryLimit = config.read<int>(CONF_MAX_MEMORY,
                CONF_DEFAULT_MAX_MEMORY);
    int maxThreadNum = config.read<int>(CONF_MAX_STORAGE_THREAD,
                CONF_DEFAULT_MAX_STORAGE_THREAD);
    int maxQueue = config.read<int>(CONF_MAX_TASK_QUEUE,
                CONF_DEFAULT_MAX_TASK_QUEUE);
    int maxResponseThreadNum = config.read<int>(CONF_MAX_RESPONSE_THREAD,
                CONF_DEFAULT_MAX_RESPONSE_THREAD);
/*
    string gridfsHost = config.read<string>(CONF_GRIDFS_HOST,
                CONF_DEFAULT_MONGO_HOST);
    string gridfsDBName = config.read<string>(CONF_GRIDFS_DB,
                CONF_DEFAULT_MONGO_DB);
    string gridfsCollection = config.read<string>(CONF_GRIDFS_COLLECTION,
                CONF_DEFAULT_MONGO_COLLECTION);
    string gridfsUsername = config.read<string>(CONF_GRIDFS_USERNAME,
                CONF_DEFAULT_MONGO_USERNAME);
    string gridfsPwd = config.read<string>(CONF_GRIDFS_PASSWD,
                CONF_DEFAULT_MONGO_PASSWD);
    bool   gridfsDigestPwd = config.read<bool>(CONF_GRIDFS_DIGESTPWD,
                CONF_DEFAULT_MONGO_DIGESTPWD);
    int gridfsTimeout = config.read<int>(CONF_GRIDFS_SOCKETTIMEOUT,
                CONF_DEFAULT_MONGO_TIMEOUT);
*/
    string thumbnailerPolicy = config.read<string>(CONF_THUMBNAIL_POLICY,
                CONF_DEFAULT_THUMBNAIL_POLICY);
    string defaultFavicon = config.read<string>(CONF_FAVICON, CONF_DEFAULT_FAVICON_PATH);

    PropertyConfigurator::configure(CONF_LOG4CXX_PATH);

    LOG4CXX_INFO(logger_asm, "listenPort:" << tcpPort << ", memoryLimit:" << memoryLimit
                << ", storageThreads:" << maxThreadNum << ", serveThreads:" << maxResponseThreadNum
                << ", maxQueue:" << maxQueue);
//    LOG4CXX_INFO(logger_asm, "gridfsHost:" << gridfsHost << ", DB:" << gridfsDBName << ", Collection:" << gridfsCollection
//                << ", soTimeout:" << gridfsTimeout);
    LOG4CXX_INFO(logger_asm, "thumbnailPolicy:" << thumbnailerPolicy << ", defaultFavicon:" << defaultFavicon);

//    GridFSStorage storage;
//    bool storageRes = storage.init(gridfsHost, gridfsDBName, gridfsCollection,
//                 gridfsUsername, gridfsPwd, gridfsDigestPwd, (double)gridfsTimeout);

    LocalStorage storage;
    storage.set_option("directory", "/tmp/images");
    bool storageRes = storage.initialize();
/*
    S3Storage storage;
    bool storageRes = storage.init("s3.amazonaws.com", "yourkey", "youraccesstoken");
*/
    if (!storageRes) {
        LOG4CXX_WARN(logger_asm, "failed to initialized GridFS storage.");
        return -1;
    }

    Thumbnailer* thumbnailer = ThumbnailerFactory::Create(thumbnailerPolicy);
    if (NULL == thumbnailer) {
        LOG4CXX_WARN(logger_asm, "failed to create thumbnailer instance.");
        return -2;
    }
    // just a little question, how to release thumbnailer instance?
    // b/c it is singleton, not a big deal even though memory-leak

    DefaultFavicon* df = DefaultFavicon::getInstance();
    if (!df->initialize(defaultFavicon)) {
        LOG4CXX_WARN(logger_asm, "failed to initialize default favicon.");
    }
    MemoryPool memoryPool(memoryLimit);

    StoragePool storagePool(memoryPool, maxThreadNum, maxQueue);
    storagePool.initStorageObject(&storage);

    pion::single_service_scheduler scheduler;
//    scheduler.setNumThreads(maxResponseThreadNum);

    pion::http::plugin_server server(scheduler, tcpPort);
    CropService cropService(thumbnailer, memoryPool, storagePool);
    CropServiceV2 cropServiceV2(thumbnailer, memoryPool, storagePool);
    StatusService statusService(memoryPool, storagePool);
    MontageService montageService(memoryPool, storagePool);
    RainbowService rainbowService(memoryPool, storagePool);
    FileService fileService(memoryPool, storagePool);
//    fileService.set_option("writable", "true");
//    fileService.set_option("directory", "/tmp/images");

    server.add_service("/crop", &cropService);
    server.add_service("/status", &statusService);
    server.add_service("/v2", &cropServiceV2);
    server.add_service("/montage", &montageService);
    server.add_service("/rainbow", &rainbowService);
    server.add_service("/file", &fileService);
    LOG4CXX_INFO(logger_asm, "start image service, listenPort:" << tcpPort);

    server.start();
    server.join();
    LOG4CXX_INFO(logger_asm, "terminate image service...");
    return 0;
}
