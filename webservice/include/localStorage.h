#ifndef CN_LEANCLOUD_IMAGESERVICE_STORAGE_LOCAL_DISK_INCLUDE_
#define CN_LEANCLOUD_IMAGESERVICE_STORAGE_LOCAL_DISK_INCLUDE_

#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>
#include <string>
#include <map>

#include <pion/config.hpp>
#include <pion/logger.hpp>
#include <pion/hash_map.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "storage.h"

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

using pion::logger;

///
/// DiskFile: class used to represent files stored on disk
/// 
class DiskFile : public Image {
public:
	DiskFile(const char* id, const void* data, const int data_len) : Image(id, data, data_len), _remalloc(false)
	{
		;
	}

    /// used to construct new disk file objects
    DiskFile(const char* path, const void* content, unsigned long size, int64_t modified, const std::string& mime)
        : Image(path, content, size), _remalloc(false)
    {
        setLastUpdateTime(modified);
        setMimeType(mime);
    }

    /// copy constructor
    DiskFile(const DiskFile& f):Image(f), _remalloc(false)
    {}

    virtual ~DiskFile() {
        if (_remalloc) {
            free((void*)_data);
        }
    }

    /// updates the file_size and last_modified timestamp to disk
    void update(void);

    /// reads content from disk into file_content buffer (may throw)
    void read(void);

    void flush();
    /**
     * checks if the file has been updated and updates vars if it has (may throw)
     *
     * @return true if the file was updated
     */
//    bool checkUpdated(void);

    /// return path to the cached file
    inline const boost::filesystem::path getFilePath(void) {
       return boost::filesystem::path(getIdentifiedString());
    }

    /// returns content of the cached file
    inline char *getFileContent(void) {
        return (char*)getContentBuf();
    }

    /// returns true if there is cached file content
    inline bool hasFileContent(void) {
       return len() > 0;
    }

    /// returns size of the file's content
    inline unsigned long getFileSize(void) {
       return len();
    }

protected:
    bool _remalloc;
};

class LocalStorage : public Storage {
public:
    LocalStorage();

    virtual ~LocalStorage();

    // save a image
    // @param(in) id - image id(filename, as for image, urlMD5 is a good choice)
    // @param(in) data - image binary data
    // @param(in) dataLen - length of image binary data
    // @return 0 - successful
    //         <0 - failed
    virtual int saveImage(const char* id, const void* data, const int dataLen, const char* bucket);

    // retrive image by id
    // @param(in) id - image id(filename)
    // @return pointer to image instance
    //         NULL - for error
    // @notice: we need invoke freeImage to release image instance
    virtual Image* getImage(const char* id, const char* bucket);

    virtual bool deleteImage(const char* id, const char* bucket);

    virtual Image* getImage(const char* id, const char* bucket, int hostStyle);

    // try to check if image is exist
    // @param(in) id - image keyfile
    // @param(in) bucket - image bucket
    // @param(in) hostStyle - path or virtual proxy
    // @return true if image is exist, otherwise false
    virtual bool headImage(const char* id, const char* bucket, int hostStyle);

    // release image instance
    virtual void freeImage(Image*& ptr);

    // clone for multiple threading
    virtual Storage* clone();

    void set_option(const std::string& name, const std::string& value);
    bool initialize();

protected:

    /// data type for map of file names to cache entries
    typedef PION_HASH_MAP<std::string, DiskFile, PION_HASH_STRING >     CacheMap;

    /// data type for map of file extensions to MIME types
    typedef PION_HASH_MAP<std::string, std::string, PION_HASH_STRING >  MIMETypeMap;

    /**
     * adds all files within a directory to the cache
     *
     * @param dir_path the directory to scan (sub-directories are included)
     */
    void scanDirectory(const boost::filesystem::path& dir_path);

    /**
     * adds a single file to the cache
     *
     * @param relative_path path for the file relative to the root directory
     * @param file_path actual path to the file on disk
     * @param placeholder if true, the file's contents are not cached
     *
     * @return std::pair<CacheMap::iterator, bool> if an entry is added to the
     *         cache, second will be true and first will point to the new entry
     */
    std::pair<CacheMap::iterator, bool> addCacheEntry(const std::string& relative_path,
                      const boost::filesystem::path& file_path,
                      const bool placeholder);

    /**
     * searches for a MIME type that matches a file
     *
     * @param file_name name of the file to search for
     * @return MIME type corresponding with the file, or DEFAULT_MIME_TYPE if none found
     */
    static std::string findMIMEType(const std::string& file_name);

private:
    boost::filesystem::path getLocalPath(const char* id, const char* bucket);

    /// function called once to initialize the map of MIME types
    static void createMIMETypes(void);


    /// mime type used if no others are found for the file's extension
    static const std::string    DEFAULT_MIME_TYPE;

    /// default setting for cache configuration option
    static const unsigned int   DEFAULT_CACHE_SETTING;

    /// default setting for scan configuration option
    static const unsigned int   DEFAULT_SCAN_SETTING;

    /// default setting for the maximum cache size option
    static const unsigned long  DEFAULT_MAX_CACHE_SIZE;

    /// default setting for the maximum chunk size option
    static const unsigned long  DEFAULT_MAX_CHUNK_SIZE;

    /// flag used to make sure that createMIMETypes() is called only once
    static boost::once_flag     _mime_types_init_flag;

    /// map of file extensions to MIME types
    static MIMETypeMap *        _mime_types_ptr;

    /// directory containing files that will be made available
    boost::filesystem::path     _directory;

    /// used to cache file contents and metadata in memory
    CacheMap                    _cache_map;

    /// mutex used to make the file cache thread-safe
    boost::mutex                _cache_mutex;

    /**
     * cache configuration setting:
     * 0 = do not cache files in memory
     * 1 = cache files in memory when requested, check for any updates
     * 2 = cache files in memory when requested, ignore any updates
     */
    unsigned int                _cache_setting;

    /**
     * scan configuration setting (only applies to directories):
     * 0 = do not scan the directory; allow files to be added at any time
     * 1 = scan directory when started, and do not allow files to be added
     * 2 = scan directory and pre-populate cache; allow new files
     * 3 = scan directory and pre-populate cache; ignore new files
     */
    unsigned int                _scan_setting;

    /**
     * maximum cache size (in bytes): files larger than this size will never be
     * cached in memory.  A value of zero means that the size is unlimited.
     */
    unsigned long               _max_cache_size;

    /**
     * maximum chunk size (in bytes): files larger than this size will be
     * delivered to clients using HTTP chunked responses.  A value of
     * zero means that the size is unlimited (chunking is disabled).
     */
    unsigned long               _max_chunk_size;

    /**
     * Whether the file and/or directory served are writable.
     */
    bool                        _writable;
    logger                  m_logger;
};

#endif
