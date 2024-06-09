/******************************************************************************
 * This project just copy from
 *CppUnitLite(https://github.com/smikes/CppUnitLite) Give My Thanks to
 *Michael(The author of CppUnit & CppUnitLite) !!!
 *
 * Project: zemb
 * Author : FergusZeng
 * Email  : cblock@126.com
 * git	  : https://gitee.com/newgolo/embedme.git
 * Copyright 2014~2022 @ ShenZhen ,China
 *******************************************************************************/
#include "CppUnitLite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BaseType.h"

namespace zemb {
static const int DEFAULT_SIZE = 20;
SimpleString::SimpleString() : buffer(new char[1]) { buffer[0] = '\0'; }

SimpleString::SimpleString(const char* otherBuffer)
    : buffer(new char[strlen(otherBuffer) + 1]) {
    snprintf(buffer, strlen(otherBuffer), "%s", otherBuffer);
}

SimpleString::SimpleString(const SimpleString& other) {
    buffer = new char[other.size() + 1];
    snprintf(buffer, other.size(), "%s", other.buffer);
}

SimpleString SimpleString::operator=(const SimpleString& other) {
    delete[] buffer;
    buffer = new char[other.size() + 1];
    snprintf(buffer, other.size(), "%s", other.buffer);
    return *this;
}

char* SimpleString::asCharString() const { return buffer; }

int SimpleString::size() const { return static_cast<int>(strlen(buffer)); }

SimpleString::~SimpleString() { delete[] buffer; }

bool operator==(const SimpleString& left, const SimpleString& right) {
    return !strcmp(left.asCharString(), right.asCharString());
}

SimpleString StringFrom(bool value) {
    char buffer[sizeof("false") + 1];
    snprintf(buffer, sizeof(buffer), "%s", value ? "true" : "false");
    return SimpleString(buffer);
}

SimpleString StringFrom(const char* value) { return SimpleString(value); }

SimpleString StringFrom(long value) { /* NOLINT */
    char buffer[DEFAULT_SIZE];
    snprintf(buffer, sizeof(buffer), "%ld", value);
    return SimpleString(buffer);
}

SimpleString StringFrom(double value) {
    char buffer[DEFAULT_SIZE];
    snprintf(buffer, sizeof(buffer), "%lf", value);
    return SimpleString(buffer);
}

SimpleString StringFrom(const SimpleString& value) {
    return SimpleString(value);
}

SimpleString StringFrom(const std::string& value) {
    return SimpleString(CSTR(value));
}

Failure::Failure(const SimpleString& testName, const SimpleString& fileName,
                 long lineNumber, const SimpleString& condition) /* NOLINT */
    : message(condition),
      testName(testName),
      fileName(fileName),
      lineNumber(lineNumber) {}

Failure::Failure(const SimpleString& testName, const SimpleString& fileName,
                 long lineNumber, const SimpleString& expected, /* NOLINT */
                 const SimpleString& actual)
    : testName(testName), fileName(fileName), lineNumber(lineNumber) {
    const char* part1 = "expected ";
    const char* part3 = " but was: ";
    int len =
        strlen(part1) + expected.size() + strlen(part3) + actual.size() + 1;
    char* stage = new char[len];
    snprintf(stage, len, "%s%s%s%s", part1, expected.asCharString(), part3,
             actual.asCharString());
    message = SimpleString(stage);
    delete[] stage;
}

TestResult::TestResult() : failureCount(0) {}

void TestResult::testsStarted() {}

void TestResult::addFailure(const Failure& failure) {
    fprintf(stdout, "\n%s%s%s%s%d%s%s\n", "Failure: \"",
            failure.message.asCharString(), "\" ", "line ", failure.lineNumber,
            " in ", failure.fileName.asCharString());
    failureCount++;
}

void TestResult::testsEnded() {
    if (failureCount > 0) {
        fprintf(stdout, "\nThere were %d failures\n", failureCount);
    } else {
        fprintf(stdout, "\nThere were no test failures\n");
    }
}

Test::Test(const SimpleString& testName) : name_(testName), next_(0), prev_(0) {
    TestRegistry::addTest(this);
}

Test* Test::getNext() const { return next_; }

void Test::setNext(Test* test) { next_ = test; }

Test* Test::getPrev() const { return prev_; }

void Test::setPrev(Test* test) { prev_ = test; }
bool Test::check(long expected, long actual, TestResult* result,  /* NOLINT */
                 const SimpleString& fileName, long lineNumber) { /* NOLINT */
    if (expected == actual) {
        return true;
    }
    result->addFailure(Failure(name_, StringFrom(__FILE__), __LINE__,
                               StringFrom(expected), StringFrom(actual)));
    return false;
}

bool Test::check(const SimpleString& expected, const SimpleString& actual,
                 TestResult* result, const SimpleString& fileName,
                 long lineNumber) { /* NOLINT */
    if (expected == actual) {
        return true;
    }
    result->addFailure(
        Failure(name_, StringFrom(__FILE__), __LINE__, expected, actual));
    return false;
}

TestRegistry::TestRegistry() : tests(0) {}
void TestRegistry::addTest(Test* test) { instance().add(test); }

void TestRegistry::runAllTests(const TestResult& result) {
    instance().run(result);
}

TestRegistry& TestRegistry::instance() {
    static TestRegistry registry;
    return registry;
}

void TestRegistry::add(Test* test) {
    if (tests == 0) {
        tests = test;
        return;
    }
    test->setPrev(tests);
    tests->setNext(test);
    tests = test;
}

void TestRegistry::run(const TestResult& result) {
    const_cast<TestResult&>(result).testsStarted();
    Test* test;
    for (test = tests; test != 0; test = test->getPrev()) {
        if (test->getPrev() == 0) {
            break;
        }
    }
    for (; test != 0; test = test->getNext()) {
        printf(".");
        fflush(stdout);
        test->run(result);
    }
    const_cast<TestResult&>(result).testsEnded();
}
}  // namespace zemb
