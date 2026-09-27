#pragma once
// Minimal, self-contained test framework: no external dependency, matching
// this project's existing zero-dependency vendoring philosophy (no package
// manager is used anywhere in this repo -- see CLAUDE.md). Each test file
// below builds into its own small executable (see tests/CMakeLists.txt),
// each registered individually with CTest, rather than one combined binary.
//
// Usage:
//   #include "test_framework.h"
//   TEST_CASE("description of what this verifies") {
//       CHECK(some_condition);
//       CHECK_MESSAGE(some_condition, "why this matters: " << detail);
//   }
//   TEST_MAIN()
//
// CHECK/CHECK_MESSAGE record a failure and keep running the rest of the
// test case (soft assert) rather than aborting, so one failing check
// doesn't hide others in the same TEST_CASE. TEST_MAIN() generates a
// main() that runs every registered test case and exits non-zero if any
// check failed anywhere, which is what CTest uses to report pass/fail.

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace testfw {

struct TestCase
{
	std::string name;
	std::function<void()> fn;
};

inline std::vector<TestCase>& registry()
{
	static std::vector<TestCase> tests;
	return tests;
}

struct Registrar
{
	Registrar(const std::string& name, std::function<void()> fn)
	{
		registry().push_back({ name, fn });
	}
};

inline int& failureCount()
{
	static int count = 0;
	return count;
}

} // namespace testfw

#define TESTFW_CONCAT_(a, b) a##b
#define TESTFW_CONCAT(a, b) TESTFW_CONCAT_(a, b)

#define TEST_CASE(name) \
	static void TESTFW_CONCAT(testfw_fn_, __LINE__)(); \
	static testfw::Registrar TESTFW_CONCAT(testfw_reg_, __LINE__)(name, TESTFW_CONCAT(testfw_fn_, __LINE__)); \
	static void TESTFW_CONCAT(testfw_fn_, __LINE__)()

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::cerr << "  CHECK FAILED: " << #cond << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
			testfw::failureCount()++; \
		} \
	} while (0)

#define CHECK_MESSAGE(cond, msg) \
	do { \
		if (!(cond)) { \
			std::cerr << "  CHECK FAILED: " << #cond << " -- " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
			testfw::failureCount()++; \
		} \
	} while (0)

#define TEST_MAIN() \
	int main() \
	{ \
		int ran = 0; \
		for (auto& t : testfw::registry()) \
		{ \
			std::cout << "[ RUN  ] " << t.name << std::endl; \
			int before = testfw::failureCount(); \
			t.fn(); \
			int after = testfw::failureCount(); \
			std::cout << (after == before ? "[ OK   ] " : "[ FAIL ] ") << t.name << std::endl; \
			ran++; \
		} \
		std::cout << ran << " test case(s) run, " << testfw::failureCount() << " check failure(s)." << std::endl; \
		return testfw::failureCount() == 0 ? 0 : 1; \
	}
