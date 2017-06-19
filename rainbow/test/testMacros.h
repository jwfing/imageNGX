#ifndef AVOS_BACKEND_IMAGE_SERVICE_TEST_MACROS_INCLUDE_H_
#define AVOS_BACKEND_IMAGE_SERVICE_TEST_MACROS_INCLUDE_H_

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#define CPPUNIT_NS CppUnit

using namespace log4cxx;
using namespace log4cxx::helpers;

#define START_TEST(className) \
    CPPUNIT_TEST_SUITE_REGISTRATION(className);\
int main(int argc, char** argv)\
{\
    BasicConfigurator::configure();\
    CppUnit::TextUi::TestRunner runner;\
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();\
    runner.addTest(registry.makeTest());\
    runner.run();\
    return 0;\
}

#endif
