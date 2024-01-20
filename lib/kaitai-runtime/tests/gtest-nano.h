// gtest-nano.h implements very minimalistic GTest-compatible API that can be used to run tests in older
// (C++98-compatible) environments.

#include <iostream>
#include <vector>
#include <math.h>

namespace testing {
    struct TestInfo {
        const char *suite;
        const char *name;
        void (*testFunc)();
    };

    std::vector<TestInfo> g_tests;
    bool g_testPass;
    bool g_allPass;

    void InitGoogleTest(int *argc, char **argv) {
        // Avoid warnings on unused arguments: we don't have implementation for parsing command line
        // right now, but we might implement it in future.
        (void) argc;
        (void) argv;

        std::cout << "[----------] gtest-nano: starting up\n";
    }

    int runAllTests() {
        std::cout << "[==========] Running " << g_tests.size() << " tests.\n";

        g_allPass = true;
        for (std::vector<TestInfo>::const_iterator it = g_tests.begin(); it != g_tests.end(); ++it) {
            const TestInfo &test = *it;
            g_testPass = true;
            std::cout << "[ RUN      ] " << test.suite << "." << test.name << "\n";
            test.testFunc();
            std::cout << (g_testPass ? "[       OK ] " : "[  FAILED  ] ") <<  test.suite << "." << test.name << "\n";
            g_allPass &= g_testPass;
        }

        return g_allPass ? 0 : 1;
    }
}

// Defines test function and registers it in global list of tests
#define TEST(suite, name)                                    \
    void Test_##suite##_##name();                            \
    namespace {                                              \
        struct Register_##suite##_##name {                   \
            Register_##suite##_##name() {                    \
                ::testing::TestInfo info = {                 \
                    #suite,                                  \
                    #name,                                   \
                    Test_##suite##_##name                    \
                };                                           \
                ::testing::g_tests.push_back(info);          \
            }                                                \
        };                                                   \
        Register_##suite##_##name register_##suite##_##name; \
    }                                                        \
    void Test_##suite##_##name()

// Assertion macro
#define EXPECT_EQ(a, b)                           \
    do {                                          \
        if ((a) == (b)) {                         \
        } else {                                  \
            ::testing::g_testPass = false;        \
        }                                         \
    } while (false)

// Floating point comparison macro
#define EXPECT_FLOAT_EQ(a, b)                     \
    do {                                          \
        if (fabs(a - b) < 1e-6) {                 \
        } else {                                  \
            ::testing::g_testPass = false;        \
        }                                         \
    } while (false)

#define EXPECT_DOUBLE_EQ(a, b) EXPECT_FLOAT_EQ(a, b)

// Failure macro
#define FAIL()                     \
    ::testing::g_testPass = false; \
    std::cerr

#define RUN_ALL_TESTS() ::testing::runAllTests()
