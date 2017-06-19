#include "localStorage.h"
#include "testMacros.h"

#include <iostream>
#include <time.h>

class test_localStorage : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_localStorage);
    CPPUNIT_TEST(test_read);
    CPPUNIT_TEST(test_write);
    CPPUNIT_TEST(test_write_rawfile);
    CPPUNIT_TEST_SUITE_END();
private:
    string fileId;
public:
    test_localStorage() {
        fileId = "user2_thumb_small";
    }
protected:
    void test_read() {
        cout << "begin testcase: " << __func__ << endl;
        // do something
        LocalStorage storage;
	storage.set_option("directory", "./");
	storage.initialize();

        string readFile = fileId + ".jpg";
        Image* image = storage.getImage(readFile.c_str(), "data");
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
        LocalStorage storage;
	storage.set_option("directory", "./");
	storage.initialize();

        string saveFile = fileId + "-orig-" + std::to_string((int64_t)cur) + ".jpg";
        string rawFile = "./data/user2_thumb_small.jpg";
        FILE* fd = fopen(rawFile.c_str(), "r");
        if (NULL == fd) {
            return;
        }
        cout << __func__ << " begin to write image into gridfs. filename=" << saveFile << endl;
        char* buffer = (char*)malloc(1024*1024);
        size_t fileLen = fread(buffer, 1, 1024*1024, fd);
        int result = storage.saveImage(saveFile.c_str(), buffer, (int)fileLen, "data");
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
        LocalStorage storage;

	storage.set_option("directory", "./");
	storage.initialize();

        string readFile = fileId + ".jpg";
        Image* image = storage.getImage(readFile.c_str(), "data");
        if (NULL == image) {
            cout << __func__ << " failed to read image from gridfs filename=" << readFile << endl;
        } else {
            cout << __func__ << " successed to read image from gridfs" << endl;
            time_t cur = time(NULL);
            string writeFile = fileId + std::to_string((int64_t)cur) + ".jpg";
            int len = 0;
            const void* data = image->data(len);
            void* newBuf = malloc(len);
            memcpy(newBuf, data, len);
	    cout << __func__ << " try to save filename: " << writeFile << ", withdata: " << data << ", length: " << len << endl;
            int saveResult = storage.saveImage(writeFile.c_str(), newBuf, len, "data");
            if (0 != saveResult) {
                cout << __func__ << "failed to read image from gridfs filename=" << readFile << endl;
            }
            free(newBuf);
            storage.freeImage(image);
        }
    }
};

START_TEST(test_localStorage);

