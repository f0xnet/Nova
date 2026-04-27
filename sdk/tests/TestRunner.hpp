#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>

namespace NovaTest {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& getTests() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int runAll() {
    int passed = 0, failed = 0;
    for (auto& tc : getTests()) {
        try {
            tc.fn();
            std::cout << "[PASS] " << tc.name << "\n";
            ++passed;
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << tc.name << "\n       " << e.what() << "\n";
            ++failed;
        }
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) {
        getTests().push_back({name, fn});
    }
};

} // namespace NovaTest

// NOVA_CAT forces macro-expansion of a and b before token-paste.
#define NOVA_CAT_RAW(a, b) a##b
#define NOVA_CAT(a, b) NOVA_CAT_RAW(a, b)

// TEST_CASE passes __COUNTER__ as a normal (non-## adjacent) argument so it
// expands to its numeric value before NOVA_TC_IMPL sees it.
#define NOVA_TC_IMPL(name, ctr) \
    static void NOVA_CAT(nova_tf_, ctr)(); \
    static NovaTest::Registrar NOVA_CAT(nova_reg_, ctr)(name, NOVA_CAT(nova_tf_, ctr)); \
    static void NOVA_CAT(nova_tf_, ctr)()

#define TEST_CASE(name) NOVA_TC_IMPL(name, __COUNTER__)

#define REQUIRE(expr) \
    do { if (!(expr)) throw std::runtime_error("REQUIRE(" #expr ") failed at line " + std::to_string(__LINE__)); } while(0)

#define REQUIRE_EQ(a, b) \
    do { if (!((a) == (b))) throw std::runtime_error( \
        std::string("REQUIRE_EQ failed at line ") + std::to_string(__LINE__)); } while(0)
