#include "gridfsStorage.h"
#include "testMacros.h"

#include <iostream>
#include <time.h>

class test_gridFS : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_gridFS);
    CPPUNIT_TEST(test_read);
    CPPUNIT_TEST(test_write);
    CPPUNIT_TEST(test_write_rawfile);
    CPPUNIT_TEST_SUITE_END();
private:
    string fileId;
public:
    test_gridFS() {
        fileId = "a15ac655c095d5564b9532e8189ca90e";
    }
protected:
    void test_read() {
        cout << "begin testcase: " << __func__ << endl;
        // do something
        GridFSStorage storage;
        storage.init("127.0.0.1", "meiwei", "images", "", "", false, 500);
        string readFile = fileId + "-orig";
        Image* image = storage.getImage(readFile.c_str(), "bucket");
        if (NULL == image) {
            cout << __func__ << " failed to read image from gridfs filename=" << readFile << endl;
        } else {
            cout << __func__ << " successfully read image data from gridfs. filename=" << readFile << endl;
            storage.freeImage(image);
        }
    }
    void test_write_rawfile() {
        cout << "begin testcase: " << __func__ << endl;
	    time_t cur = time(NULL);
        GridFSStorage storage;
        storage.init("127.0.0.1", "meiwei", "images", "", "", false, 500);
        string saveFile = fileId + "-orig-" + std::to_string((int64_t)cur);
        string rawFile = "./data/user2_thumb_small.jpg";
        FILE* fd = fopen(rawFile.c_str(), "r");
        if (NULL == fd) {
            return;
        }
        cout << __func__ << " begin to write image into gridfs. filename=" << saveFile << endl;
        char* buffer = (char*)malloc(1024*1024);
        size_t fileLen = fread(buffer, 1, 1024*1024, fd);
        int result = storage.saveImage(saveFile.c_str(), buffer, (int)fileLen, "bucket");
        if (0 == result) {
            cout << __func__ << " successfully write image data to gridfs. filename=" << saveFile << endl;
        } else {
            cout << __func__ << " failed to write image data to gridfs. filename=" << saveFile << endl;
        }
        free(buffer);
        fclose(fd);
    }
    void test_write() {
        cout << "begin testcase: " << __func__ << endl;
        GridFSStorage storage;
        storage.init("127.0.0.1", "meiwei", "images", "", "", false, 500);
        string readFile = fileId + "-orig";
        Image* image = storage.getImage(readFile.c_str(), "bucket");
        if (NULL == image) {
            cout << __func__ << " failed to read image from gridfs filename=" << readFile << endl;
        } else {
            cout << __func__ << " successed to read image from gridfs" << endl;
            time_t cur = time(NULL);
            string writeFile = fileId + "-origx-" + std::to_string((int64_t)cur);
            int len = 0;
            const void* data = image->data(len);
            void* newBuf = malloc(len);
            memcpy(newBuf, data, len);
	        cout << __func__ << " try to save filename: " << writeFile << ", withdata: " << data << ", length: " << len << endl;
            int saveResult = storage.saveImage(writeFile.c_str(), newBuf, len, "bucket");
            if (0 != saveResult) {
                cout << __func__ << "failed to read image from gridfs filename=" << readFile << endl;
            }
            free(newBuf);
            storage.freeImage(image);
        }
    }
};

START_TEST(test_gridFS);

