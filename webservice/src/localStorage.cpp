#include "localStorage.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <pion/plugin.hpp>
#include <pion/algorithm.hpp>

using namespace pion;

// DiskFile member functions

void DiskFile::update(void)
{
    boost::filesystem::path filePath = getFilePath();
    // set file_size and last_modified
    _len = boost::numeric_cast<std::streamsize>(boost::filesystem::file_size(filePath));
    _last_modified = boost::filesystem::last_write_time(filePath);
    _last_modified_string = http::types::get_date_string( _last_modified );
}

void DiskFile::read(void)
{
    if (NULL == _data) {
        _data = malloc(_len);
        _remalloc = true;
    }
    // open the file for reading
    boost::filesystem::ifstream file_stream;
    file_stream.open(getFilePath(), std::ios::in | std::ios::binary);

    // read the file into memory
    if (!file_stream.is_open() || !file_stream.read((char*)_data, _len)) {
        BOOST_THROW_EXCEPTION( error::read_file() << error::errinfo_file_name(getIdentifiedString()));
    }
}

void DiskFile::flush() {
    boost::filesystem::ofstream file_stream(getFilePath(), std::ios::out | std::ios::binary);
    file_stream.write((char*)_data, _len);
    file_stream.close();
}

// bool DiskFile::checkUpdated(void)
// {
//     boost::filesystem::path filePath = getFilePath();
//     // get current values
//     std::streamsize cur_size = boost::numeric_cast<std::streamsize>(boost::filesystem::file_size( filePath ));
//     time_t cur_modified = boost::filesystem::last_write_time( filePath );

//     // check if file has not been updated
//     if (cur_modified == _last_modified && cur_size == _len)
//         return false;

//     // file has been updated

//     // update file_size and last_modified timestamp
//     _len = cur_size;
//     _last_modified = cur_modified;
//     _last_modified_string = http::types::get_date_string( _last_modified );

//     // read new contents
//     read();

//     return true;
// }

const std::string           LocalStorage::DEFAULT_MIME_TYPE("application/octet-stream");
const unsigned int          LocalStorage::DEFAULT_CACHE_SETTING = 1;
const unsigned int          LocalStorage::DEFAULT_SCAN_SETTING = 0;
const unsigned long         LocalStorage::DEFAULT_MAX_CACHE_SIZE = 0;    /* 0=disabled */
const unsigned long         LocalStorage::DEFAULT_MAX_CHUNK_SIZE = 0;    /* 0=disabled */
boost::once_flag            LocalStorage::_mime_types_init_flag = BOOST_ONCE_INIT;
LocalStorage::MIMETypeMap    *LocalStorage::_mime_types_ptr = NULL;

LocalStorage::LocalStorage()
   :_cache_setting(DEFAULT_CACHE_SETTING),
    _scan_setting(DEFAULT_SCAN_SETTING),
    _max_cache_size(DEFAULT_MAX_CACHE_SIZE),
    _max_chunk_size(DEFAULT_MAX_CHUNK_SIZE),
    _writable(true), m_logger(PION_GET_LOGGER("localStorage")) {
    ;
}

LocalStorage::~LocalStorage() {
    _cache_map.clear();
}

void LocalStorage::set_option(const std::string& name, const std::string& value)
{
    PION_LOG_INFO(m_logger, "set option: key=" << name << ", value=" << value);
    if (name == "directory") {
        _directory = value;
        _directory.normalize();
        plugin::check_cygwin_path(_directory, value);
        // make sure that the directory exists
        if (! boost::filesystem::exists(_directory) || ! boost::filesystem::is_directory(_directory)) {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
            const std::string dir_name = _directory.string();
#else
            const std::string dir_name = _directory.directory_string();
#endif
            BOOST_THROW_EXCEPTION( error::directory_not_found() << error::errinfo_dir_name(dir_name) );
        }
    } else if (name == "cache") {
        if (value == "0") {
            _cache_setting = 0;
        } else if (value == "1") {
            _cache_setting = 1;
        } else if (value == "2") {
            _cache_setting = 2;
        } else {
            BOOST_THROW_EXCEPTION( error::bad_arg() << error::errinfo_arg_name(name) );
        }
    } else if (name == "scan") {
        if (value == "0") {
            _scan_setting = 0;
        } else if (value == "1") {
            _scan_setting = 1;
        } else if (value == "2") {
            _scan_setting = 2;
        } else if (value == "3") {
            _scan_setting = 3;
        } else {
            BOOST_THROW_EXCEPTION( error::bad_arg() << error::errinfo_arg_name(name) );
        }
    } else if (name == "max_chunk_size") {
        _max_chunk_size = boost::lexical_cast<unsigned long>(value);
    } else if (name == "writable") {
        if (value == "true") {
            _writable = true;
        } else if (value == "false") {
            _writable = false;
        } else {
            BOOST_THROW_EXCEPTION( error::bad_arg() << error::errinfo_arg_name(name) );
        }
    } else {
        BOOST_THROW_EXCEPTION( error::bad_arg() << error::errinfo_arg_name(name) );
    }
}

void LocalStorage::scanDirectory(const boost::filesystem::path& dir_path)
{
    // iterate through items in the directory
    boost::filesystem::directory_iterator end_itr;
    for ( boost::filesystem::directory_iterator itr( dir_path );
          itr != end_itr; ++itr )
    {
        if ( boost::filesystem::is_directory(*itr) ) {
            // item is a sub-directory

            // recursively call scanDirectory()
            scanDirectory(*itr);

        } else {
            // item is a regular file

            // figure out relative path to the file
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
            std::string file_path_string( itr->path().string() );
            std::string relative_path( file_path_string.substr(_directory.string().size() + 1) );
#else
            std::string file_path_string( itr->path().file_string() );
            std::string relative_path( file_path_string.substr(_directory.directory_string().size() + 1) );
#endif

            // add item to cache (use placeholder if scan == 1)
            addCacheEntry(relative_path, *itr, _scan_setting == 1);
        }
    }
}

std::pair<LocalStorage::CacheMap::iterator, bool>
LocalStorage::addCacheEntry(const std::string& relative_path,
                           const boost::filesystem::path& file_path,
                           const bool placeholder)
{
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
    DiskFile cache_entry(file_path.string().c_str(), NULL, 0, 0, findMIMEType(file_path.filename().string()));
#else
    DiskFile cache_entry(file_path.string().c_str(), NULL, 0, 0, findMIMEType(file_path.leaf()));
#endif
    if (! placeholder) {
        cache_entry.update();
        // only read the file if its size is <= max_cache_size
        if (_max_cache_size==0 || cache_entry.getFileSize() <= _max_cache_size) {
            try { cache_entry.read(); }
            catch (std::exception&) {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
                PION_LOG_ERROR(m_logger, "Unable to add file to cache: "
                               << file_path.string());
#else
                PION_LOG_ERROR(m_logger, "Unable to add file to cache: "
                               << file_path.file_string());
#endif
                return std::make_pair(_cache_map.end(), false);
            }
        }
    }

    std::pair<CacheMap::iterator, bool> add_entry_result
        = _cache_map.insert( std::make_pair(relative_path, cache_entry) );

    if (add_entry_result.second) {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
        PION_LOG_DEBUG(m_logger, "Added file to cache: "
                       << file_path.string());
#else
        PION_LOG_DEBUG(m_logger, "Added file to cache: "
                       << file_path.file_string());
#endif
    } else {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
        PION_LOG_ERROR(m_logger, "Unable to insert cache entry for file: "
                       << file_path.string());
#else
        PION_LOG_ERROR(m_logger, "Unable to insert cache entry for file: "
                       << file_path.file_string());
#endif
    }

    return add_entry_result;
}

std::string LocalStorage::findMIMEType(const std::string& file_name) {
    // initialize _mime_types if it hasn't been done already
    boost::call_once(LocalStorage::createMIMETypes, _mime_types_init_flag);

    // determine the file's extension
    std::string extension(file_name.substr(file_name.find_last_of('.') + 1));
    boost::algorithm::to_lower(extension);

    // search for the matching mime type and return the result
    MIMETypeMap::iterator i = _mime_types_ptr->find(extension);
    return (i == _mime_types_ptr->end() ? DEFAULT_MIME_TYPE : i->second);
}

void LocalStorage::createMIMETypes(void) {
    // create the map
    static MIMETypeMap mime_types;

    // populate mime types
    mime_types["js"] = "text/javascript";
    mime_types["txt"] = "text/plain";
    mime_types["xml"] = "text/xml";
    mime_types["css"] = "text/css";
    mime_types["htm"] = "text/html";
    mime_types["html"] = "text/html";
    mime_types["xhtml"] = "text/html";
    mime_types["gif"] = "image/gif";
    mime_types["png"] = "image/png";
    mime_types["jpg"] = "image/jpeg";
    mime_types["jpeg"] = "image/jpeg";
    mime_types["svg"] = "image/svg+xml";
    mime_types["eof"] = "application/vnd.ms-fontobject";
    mime_types["otf"] = "application/x-font-opentype";
    mime_types["ttf"] = "application/x-font-ttf";
    mime_types["woff"] = "application/font-woff";
    // ...

    // set the static pointer
    _mime_types_ptr = &mime_types;
}

bool LocalStorage::initialize()
{
    // scan directory/file if scan setting != 0
    if (_scan_setting != 0) {
        // force caching if scan == (2 | 3)
        if (_cache_setting == 0 && _scan_setting > 1)
            _cache_setting = 1;

        boost::mutex::scoped_lock cache_lock(_cache_mutex);

        // scan directory if one is defined
        if (! _directory.empty())
            scanDirectory(_directory);
    }
    return true;
}

int LocalStorage::saveImage(const char* id, const void* data, const int dataLen, const char* bucket) {
    PION_LOG_INFO(m_logger, "try to save image to path: " << bucket << ", identifiedName:" << id);
    boost::filesystem::path localPath = getLocalPath(id, bucket);
    PION_LOG_INFO(m_logger, "localpath: " << localPath.string().c_str());
    DiskFile file(localPath.string().c_str(), data, dataLen);
    file.flush();
    return 0;
}

Image* LocalStorage::getImage(const char* id, const char* bucket) {
    return getImage(id, bucket, 0);
}

Image* LocalStorage::getImage(const char* id, const char* bucket, int hostStyle) {
    if (0 != hostStyle) {
        return NULL;
    }
    boost::filesystem::path localPath = getLocalPath(id, bucket);
    DiskFile* file = new DiskFile(localPath.string().c_str(), NULL, 0);
    file->update();
    file->read();
    return file;
}

bool LocalStorage::headImage(const char* id, const char* bucket, int hostStyle) {
    if (0 != hostStyle) {
        return false;
    }
    if (boost::filesystem::exists(getLocalPath(id, bucket))) {
        return true;
    }
    return false;
}

bool LocalStorage::deleteImage(const char* id, const char* bucket) {
    boost::filesystem::path localPath = getLocalPath(id, bucket);
    if (boost::filesystem::exists(localPath)) {
        boost::filesystem::remove(localPath);
    }
    return true;
}

void LocalStorage::freeImage(Image*& ptr) {
    if (ptr) {
        delete ptr;
    }
    ptr = NULL;
}

boost::filesystem::path LocalStorage::getLocalPath(const char* id, const char* bucket) {
    boost::filesystem::path ret(_directory / bucket / id);
    return ret.normalize();
}

Storage* LocalStorage::clone() {
    LocalStorage* stg = new LocalStorage();
    stg->set_option("directory", _directory.string());
    stg->initialize();
    return stg;
}
