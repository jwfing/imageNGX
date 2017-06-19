#include "s3Storage.h"
#include "testMacros.h"

#include <sys/stat.h>
#include <iostream>

class test_s3 : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(test_s3);
    CPPUNIT_TEST(test_init);
    CPPUNIT_TEST(test_getImage);
    CPPUNIT_TEST(test_headImage);
    // CPPUNIT_TEST(test_saveImage);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() {
        host = "s3.amazonaws.com";
        bukect = "del-dev-content";
        accessKey = "AKIAIK73HBH45D3VBURQ";
        secretKey = "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";
    }
    void test_init() {
        S3Storage storage;
        bool isInited = storage.init(host, accessKey, secretKey);
        CPPUNIT_ASSERT(isInited);
    }

    void test_headImage() {
        S3Storage storage;
        bool isInited = storage.init(host, accessKey, secretKey);
        CPPUNIT_ASSERT(isInited);
        S3Status status = storage.headImage("0000be5f7d773183a29fceabdca85bb3-orig", bukect);
        CPPUNIT_ASSERT_EQUAL(S3StatusOK, status);
        status = storage.headImage("0000be5f7d773183a29fceabdca85bb3-rig", bukect);
        CPPUNIT_ASSERT_EQUAL(S3StatusHttpErrorNotFound, status);
    }

    void test_getImage() {
        S3Storage storage;
        bool isInited = storage.init(host, accessKey, secretKey);
        CPPUNIT_ASSERT(isInited);
        //Image* image = storage.getImage("34801e3031c5e8d2c8c1144b3249835c-test");
        //CPPUNIT_ASSERT(image);
        Image* image = storage.getImage("0000be5f7d773183a29fceabdca85bb3-orig", bukect);
        CPPUNIT_ASSERT(image);

        int dataLen = 0;
        const void* data = image->data(dataLen);
        cout << "len: " << dataLen << endl;
        FILE *f = fopen("/tmp/s.jpg", "w");
        size_t wrote = fwrite(data, 1, dataLen, f);
        cout << "size_t : " << wrote << endl;
        fclose(f);
    }

    void test_saveImage() {
        S3Storage storage;
        bool isInited = storage.init(host, accessKey, secretKey);
        CPPUNIT_ASSERT(isInited);
        const char* filename = "/tmp/s.jpg";
        FILE *f = fopen(filename, "r");
        if (f != NULL) {
            struct stat statbuf;
            if (stat(filename, &statbuf) == -1) {
                cout << "ERROR: Failed to stat file :"  << filename << endl;
                return;
            }
            int contentLength = statbuf.st_size;
            char* data = NULL;
            data = (char*) malloc(contentLength);
            CPPUNIT_ASSERT(data);
            int len = fread(data, 1, contentLength, f);
            CPPUNIT_ASSERT_EQUAL(contentLength, len);
            storage.saveImage("0000be5f7d773183a29fceabdca85bb3-orig-test", data, contentLength, bukect);
            if (data != NULL) {
                free(data);
                data = NULL;
            }
            fclose(f);
        }
    }

private:
    string host;
    const char* bukect;
    string accessKey;
    string secretKey;

};

START_TEST(test_s3);
