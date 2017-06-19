#include "config.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr Config::_logger = Logger::getLogger("cn.leancloud.image.webservice.config");

Config::Config(const string& filename, const string& delimiter, const string& comment)
    : _delimiter(delimiter), _comment(comment) {
    ifstream in(filename.c_str());
    if (!in) {
        throw FileNotFound(filename);
    }
    in >> (*this);
    LOG4CXX_INFO(_logger, "read config file");
}

Config::Config() : _delimiter("="), _comment("#") {
}

bool Config::keyExists(const string& key) const {
    mapcit p = _contents.find(key);
    return _contents.end() != p;
}

void Config::trim(string& str) {
    static const char whitespace[] = "\n\t\v\r\f";
    str.erase(0, str.find_first_not_of(whitespace));
    str.erase(str.find_last_not_of(whitespace) + 1);
}

ostream& operator<<(ostream& os, const Config& cf) {
    for (Config::mapcit p = cf._contents.begin(); p != cf._contents.end(); ++p) {
        os << p->first << "" << cf._delimiter << "";
        os << p->second << std::endl;
    }
    return os;
}

istream& operator>>(istream& is, Config& cf) {
    typedef string::size_type pos;
    const string& delimiter = cf._delimiter;
    const string& comment = cf._comment;
    const pos skip = delimiter.length();
    string nextLine = "";
    while(is || nextLine.length() > 0) {
        string line;
        if (nextLine.length() > 0) {
            line = nextLine;
            nextLine = "";
        } else {
            std::getline(is, line);
        }
        line = line.substr(0, line.find(comment));
        pos delimPos = line.find(delimiter);
        if (delimPos < string::npos) {
            // extract key
            string key = line.substr(0, delimPos);
            line.replace(0, delimPos + skip, "");
            // see if value continues on the next line
            // stop at blank line, next line with a key, end of stream,
            // or end of file sentry
            bool terminate = false;
            while(!terminate && is) {
                std::getline(is, nextLine);
                terminate = true;
                string nlcopy = nextLine;
                Config::trim(nlcopy);
                if (nlcopy == "") {
                    continue;
                }
                nextLine = nextLine.substr(0, nextLine.find(comment));
                if (nextLine.find(delimiter) != string::npos) {
                    continue;
                }
                nlcopy = nextLine;
                Config::trim(nlcopy);
                if (nlcopy != "") {
                    line += "\n";
                }
                line += nextLine;
                terminate = false;
            }
            Config::trim(key);
            Config::trim(line);
            cf._contents[key] = line;
        }
    }
    return is;
}

void Config::loadFile(const string& filename, const string& delimiter, const string& comment) {
    _delimiter = delimiter;
    _comment = comment;
    ifstream in(filename.c_str());
    if (!in) {
        throw FileNotFound(filename);
    }
    in >> (*this);
}
