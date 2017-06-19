#ifndef CN_LEANCLOUD_IMAGE_SERVICE_CONFIG_INCLUDE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_CONFIG_INCLUDE_H_

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace std;

class Config {
protected:
    string _delimiter;
    string _comment;
    map<string, string> _contents;
    typedef map<string, string>::iterator mapit;
    typedef map<string, string>::const_iterator mapcit;
    static log4cxx::LoggerPtr _logger;

public:
    Config(const string& filename, const string& delimiter = "=", const string& comment = "#");
    Config();

    template <typename T>
    T read(const string& key) const;
    template <typename T>
    T read(const string& key, const T& defaultValue) const;
    template <typename T>
    bool readInfo(const string& key, T& value) const;
    template <typename T>
    bool readInfo(const string& key, const T& defaultValue, T& value) const;

    void loadFile(const string& filename, const string& delimiter = "=", const string& comment = "#");
    bool keyExists(const string& key) const;
    template <typename T>
    void add(const string& key, const T& value);
    void remove(const string& key);

    string getDelimiter() const {
        return _delimiter;
    }
    string getComment() const {
        return _comment;
    }
    void setDelimiter(const string& delimiter) {
        _delimiter = delimiter;
    }
    void setComment(const string& comment) {
        _comment = comment;
    }
    friend ostream& operator<<(ostream& os, const Config& cf);
    friend istream& operator>>(istream& is, Config& cf);

protected:
    template <typename T>
    static string T_as_string(const T& t);
    template <typename T>
    static T string_as_T(const string& s);
    static void trim(string& str);

public:
    struct FileNotFound {
        string _filename;
        FileNotFound(const string& filename) : _filename(filename) {}
    };
    struct KeyNotFound {
        string _keyname;
        KeyNotFound(const string& keyname) : _keyname(keyname) {}
    };
};

/* static func */
template <typename T>
string Config::T_as_string(const T& t) {
    // convert from T to string
    // type T must support << operator
    ostringstream ost;
    ost << t;
    return ost.str();
}

template <typename T>
T Config::string_as_T(const string& s) {
    // convert from string to T
    // type T must support >> operator
    T t;
    istringstream ist(s);
    ist >> t;
    return t;
}

/* partial template */
template <>
inline string Config::string_as_T<string>(const string& s) {
    return s;
}

template <>
inline bool Config::string_as_T<bool>(const string& s) {
    bool b = true;
    string sup = s;
    for (string::iterator p = sup.begin(); p!= sup.end(); ++p) {
        *p = (char)toupper(*p);
    }
    if (sup == string("FALSE") || sup == string("F")
        || sup == string("NO") || sup == string("N")
        || sup == string("0") || sup == string("NONE")) {
        b = false;
    }
    return b;
}

template <typename T>
inline T Config::read(const string& key) const {
    mapcit p = _contents.find(key);
    if (_contents.end() == p) {
        throw KeyNotFound(key);
    }
    return string_as_T<T>(p->second);
}

template <typename T>
inline T Config::read(const string& key, const T& defaultValue) const {
    mapcit p = _contents.find(key);
    if (_contents.end() == p) {
        return defaultValue;
    }
    return string_as_T<T>(p->second);
}

template <typename T>
inline bool Config::readInfo(const string& key, T& var) const {
    mapcit p = _contents.find(key);
    bool found = (p != _contents.end());
    if (found) {
        var = string_as_T<T>(p->second);
    }
    return found;
}

template <typename T>
inline bool Config::readInfo(const string& key, const T& defaultValue, T& var) const {
    mapcit p = _contents.find(key);
    bool found = (p != _contents.end());
    if (found) {
        var = string_as_T<T>(p->second);
    } else {
        var = defaultValue;
    }
    return found;
}

template <typename T>
inline void Config::add(const string& key, const T& value) {
    string v = T_as_string(value);
    string keyStr = key;
    trim(keyStr);
    trim(v);
    _contents[keyStr] = v;
}

#endif
