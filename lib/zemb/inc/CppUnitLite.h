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
#ifndef __ZEMB_CPP_UNIT_LITE_H__
#define __ZEMB_CPP_UNIT_LITE_H__
#include <iostream>
#include <string>

#include "BaseType.h"

/**
 * @file CppUnitLite.h
 * @brief C++单元测试
 * @note 本文件拷贝于CppUnitLite(https://github.com/smikes/CppUnitLite)项目
 * @code 使用说明
 *  #include "CppUnitLite.h"
 *  class Case01{
 *  public:
 *  int size()
 *  {
 *     return 0;
 *  }
 *
 *  };
 *  //构建测试用例testCaseCase01Test
 *  TEST(Case01, testCase)
 *  {
 *      Stack s;
 *      LONGS_EQUAL(0, s.size());
 *      std::string b = "asa";
 *      CHECK_EQUAL("asa", b);
 *  }
 *  //构建测试用例testCaseCase02Test
 *  TEST(Case02, testCase)
 *  {
 *  }
 *
 *  int main()
 *  {
 *      TestResult tr;
 *      TestRegistry::runAllTests(tr);
 *      return 0;
 *  }
 * @endcode
 */
namespace zemb {

/**
 * @class SimpleString
 * @brief 简单字符串类
 */
class SimpleString {
    friend bool operator==(const SimpleString& left, const SimpleString& right);

public:
    SimpleString();
    explicit SimpleString(const char* value);
    SimpleString(const SimpleString& other);
    ~SimpleString();
    SimpleString operator=(const SimpleString& other);
    char* asCharString() const;
    int size() const;

private:
    char* buffer;
};

SimpleString StringFrom(bool value);
SimpleString StringFrom(const char* value);
SimpleString StringFrom(long value); /* NOLINT */
SimpleString StringFrom(double value);
SimpleString StringFrom(const SimpleString& other);
SimpleString StringFrom(const std::string& value);

/**
 * @class Failure
 * @brief CppUnit错误类
 */
class Failure {
public:
    Failure(const SimpleString& testName, const SimpleString& fileName,
            long lineNumber, const SimpleString& condition); /* NOLINT */
    Failure(const SimpleString& testName, const SimpleString& fileName,
            long lineNumber, const SimpleString& expected, /* NOLINT */
            const SimpleString& actual);
    SimpleString message;
    SimpleString testName;
    SimpleString fileName;
    long lineNumber; /* NOLINT */
};

/**
 * @class TestResult
 * @brief CppUnit测试结果类
 */
class TestResult {
public:
    TestResult();
    virtual ~TestResult() {}
    virtual void testsStarted();
    virtual void addFailure(const Failure& failure);
    virtual void testsEnded();

private:
    int failureCount;
};

/**
 * @class Test
 * @brief CppUnit测试类
 */
class Test {
public:
    explicit Test(const SimpleString& testName);
    virtual ~Test() {}
    virtual void run(const TestResult& result) = 0;
    Test* getNext() const;
    void setNext(Test* test);
    Test* getPrev() const;
    void setPrev(Test* test);

protected:
    bool check(long expected, long actual, TestResult* result, /* NOLINT */
               const SimpleString& fileName, long lineNumber); /* NOLINT */
    bool check(const SimpleString& expected, const SimpleString& actual,
               TestResult* result, const SimpleString& fileName,
               long lineNumber); /* NOLINT */
    SimpleString name_;
    Test* next_;
    Test* prev_;
};

#define TEST(testName, testGroup)                               \
    class testGroup##testName##Test : public Test {             \
    public:                                                     \
        testGroup##testName##Test() : Test(#testName "Test") {} \
        void run(TestResult& result_);                          \
    };                                                          \
    testGroup##testName##Test testGroup##testName##Instance =   \
        testGroup##testName##Test();                            \
    void testGroup##testName##Test::run(TestResult& result_)

#define CHECK(condition)                                         \
    {                                                            \
        if (!(condition)) {                                      \
            result_.addFailure(                                  \
                Failure(name_, __FILE__, __LINE__, #condition)); \
            return;                                              \
        }                                                        \
    }

#define CHECK_EQUAL(expected, actual)                             \
    {                                                             \
        if (!((expected) == (actual))) {                          \
            result_.addFailure(Failure(name_, __FILE__, __LINE__, \
                                       StringFrom(expected),      \
                                       StringFrom(actual)));      \
        }                                                         \
    }

#define LONGS_EQUAL(expected, actual)                             \
    {                                                             \
        long actualTemp = actual;     /* NOLINT */                \
        long expectedTemp = expected; /* NOLINT */                \
        if ((expectedTemp) != (actualTemp)) {                     \
            result_.addFailure(Failure(name_, __FILE__, __LINE__, \
                                       StringFrom(expectedTemp),  \
                                       StringFrom(actualTemp)));  \
            return;                                               \
        }                                                         \
    }
#define DOUBLES_EQUAL(expected, actual, threshold)                     \
    {                                                                  \
        double actualTemp = actual;                                    \
        double expectedTemp = expected;                                \
        if (fabs((expectedTemp) - (actualTemp)) > threshold) {         \
            result_.addFailure(                                        \
                Failure(name_, __FILE__, __LINE__,                     \
                        StringFrom(static_cast<double>(expectedTemp)), \
                        StringFrom(static_cast<double>(actualTemp)))); \
            return;                                                    \
        }                                                              \
    }

/* 添加失败信息 */
#define FAIL(text)                                                      \
    {                                                                   \
        result_.addFailure(Failure(name_, __FILE__, __LINE__, (text))); \
        return;                                                         \
    }

/**
 * @class TestRegistry
 * @brief CppUnit测试注册器类
 * @note 该类为单例类
 */
class TestRegistry {
public:
    static void addTest(Test* test);
    static void runAllTests(const TestResult& result);

private:
    static TestRegistry& instance();
    TestRegistry();
    void add(Test* test);
    void run(const TestResult& result);
    Test* tests;
};
}  // namespace zemb
#endif
