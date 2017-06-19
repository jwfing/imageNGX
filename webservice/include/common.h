#ifndef CN_LEANCLOUD_IMAGE_SERVICE_COMMON_DEFINE_INCLUDE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_COMMON_DEFINE_INCLUDE_H_

#include <stdint.h>

#define CONF_FILE_PATH          "./conf/image.conf"
#define CONF_LISTEN_PORT        "listenPort"
#define CONF_MAX_MEMORY         "maxUsageMemory"
#define CONF_MAX_STORAGE_THREAD "maxStorageThread"
#define CONF_MAX_RESPONSE_THREAD "maxResponseThread"
#define CONF_MAX_TASK_QUEUE     "maxTaskQueue"
#define CONF_GRIDFS_HOST        "mongoHost"
#define CONF_GRIDFS_DB          "mongoDB"
#define CONF_GRIDFS_COLLECTION  "mongoCollection"
#define CONF_GRIDFS_USERNAME    "mongoUser"
#define CONF_GRIDFS_PASSWD      "mongoPasswd"
#define CONF_GRIDFS_DIGESTPWD   "mongoDigestPwd"
#define CONF_GRIDFS_SOCKETTIMEOUT "mongoSOTimeout"
#define CONF_THUMBNAIL_POLICY   "thumbnailPolicy"
#define CONF_LOG4CXX_PATH       "./conf/log4cxx.properties"
#define CONF_FAVICON            "defaultFavicon"
#define CONF_NOT_FOUND_IMAGE    "not-found.png"

#define CONF_DEFAULT_LISTEN_PORT         18989
#define CONF_DEFAULT_MAX_MEMORY          1024*1024*256
#define CONF_DEFAULT_MAX_STORAGE_THREAD  10
#define CONF_DEFAULT_MAX_RESPONSE_THREAD 10
#define CONF_DEFAULT_MAX_TASK_QUEUE      100
#define CONF_DEFAULT_MONGO_HOST          "127.0.0.1"
#define CONF_DEFAULT_MONGO_DB            "meiwei"
#define CONF_DEFAULT_MONGO_COLLECTION    "images"
#define CONF_DEFAULT_MONGO_USERNAME      ""
#define CONF_DEFAULT_MONGO_PASSWD        ""
#define CONF_DEFAULT_MONGO_DIGESTPWD     false
#define CONF_DEFAULT_MONGO_TIMEOUT       30
#define CONF_DEFAULT_THUMBNAIL_POLICY    "imagick"
#define CONF_DEFAULT_FAVICON_PATH        "./conf/defaultFav.ico"

#define QUERY_URLMD5      "urlmd5"
#define QUERY_FORMAT      "fm"
#define QUERY_QUALITY     "q"
#define QUERY_WIDTH       "w"
#define QUERY_HEIGHT      "h"
#define QUERY_MARGIN      "margin"
#define QUERY_KEYS        "keys"
#define QUERY_BUCKET      "bucket"
#define QUERY_BACKGROUND  "bg"
#define QUERY_COLS        "cols"
#define QUERY_X           "x"
#define QUERY_X2          "x2"
#define QUERY_Y           "y"
#define QUERY_Y2          "y2"
#define QUERY_UKEY        "u"
#define QUERY_DOMAIN      "d"
#define QUERY_HOST_STYLE  "hs"
#define QUERY_HUE         "hue"
#define QUERY_SAT         "sat"
#define QUERY_BRI         "bri"
#define QUERY_EXP         "exp"
#define QUERY_BLUR        "blur"
#define QUERY_CROP        "crop"
#define QUERY_EXTRA_INFO  "extra_info"
#define QUERY_CLIPS "clips"
#define QUERY_RAINBOW_HEIGHT "rbh"

#define EPS 1e-8
#endif
