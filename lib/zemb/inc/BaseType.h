/******************************************************************************
 * This file is part of ZEMB.
 *
 * ZEMB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ZEMB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZEMB.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Project: zemb
 * Author : FergusZeng
 * Email  : cblock@126.com
 * git	  : https://gitee.com/newgolo/embedme.git
 * Copyright 2014~2022 @ ShenZhen ,China
 *******************************************************************************/
#ifndef __ZEMB_BASE_TYPE_H__
#define __ZEMB_BASE_TYPE_H__

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

// 查看GCC预定义宏:"gcc -E -dM - </dev/null"

using namespace std;  // NOLINT
/**
 *	@file  BaseType.h
 *	@brief 基础类型头文件
 */

/** 计算数组大小(元素个数).*/
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/** 判断一个字符是否是十进制数字.*/
#define IS_DIGITAL(ch) ((((ch) >= '0') && ((ch) <= '9')) ? true : false)

/** 判断一个字符是否是16进制数字.*/
#define IS_HEXDIG(ch)                                                         \
    (((((ch) >= '0') && ((ch) <= '9')) || (((ch) >= 'A') && ((ch) <= 'F')) || \
      (((ch) >= 'a') && ((ch) <= 'f')))                                       \
         ? true                                                               \
         : false)

/** 判断一个字符是否是大写字母.*/
#define IS_CAPITAL(ch) ((((ch) >= 'A') && ((ch) <= 'Z')) ? true : false)

/** 判断一个字符是否是字母.*/
#define IS_ALPHABET(ch)                                                     \
    (((((ch) >= 'A') && ((ch) <= 'Z')) || (((ch) >= 'a') && ((ch) <= 'b'))) \
         ? true                                                             \
         : false)

/** 返回 x 和 y 中较大值. */
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

/** 返回 x 和 y 中较小值. */
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/** 在 min 和 max 区间中取与 x 大小较接近的数值. */
#define CLIP(min, x, max) \
    (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

/** 求正余(避免n为负数时得到负余数)  */
#define MOD(n, m) (((n) % (m) + (m)) % (m))

/* 单位定义 */
#define UNIT_KB (1024)
#define UNIT_MB (1048576)
#define UNIT_GB (1073741824)
#define M_GA (9.8)         /**< 重力加速度(32位). */
#define M_2PI (M_PI * 2.0) /**< 圆周率使用math.h中定义的M_PI系列宏 */

/* 数学计算 */
#define SQUARE(x) ((x) * (x))        /**< x 的平方. */
#define R_AREA(r) ((PI) * (r) * (r)) /**< 计算半径为 r 的圆的面积. */
#define ROUND_DIV(divided, divider) \
    (((divided) + ((divider) >> 1)) / (divider)) /**< 整除 */
#define ROUND_UP(num, align) \
    (((num) + ((align) - 1u)) & ~((align) - 1u))        /**< 向上取整 */
#define ROUND_DOWN(num, align) ((num) & ~((align) - 1)) /**< 向下取整 */
#define F_EQUAL(f1, f2) \
    (fabs((f1) - (f2)) < 0.000001) /**< 判断浮点数是否相等 */
#define F_GREATER(f1, f2) \
    (((f1) - (f2)) > 0.000001) /**< 判断浮点数f1是否大于f2 */

/* 位计算 */
#define BIT_GET(value, bit) \
    (!!((0x0001 << (bit)) & (value))) /**< 获取value的第bit位的值 */
#define BIT_SET(value, bit) \
    ((0x0001 << (bit)) | (value)) /**< 设置value的第bit位的值 */
#define BIT_CLR(value, bit) \
    ((~(0x0001 << (bit))) & (value)) /**< 清除value的第bit位的值 */

#define UNUSED_PARAM(param) \
    { (void)(param); } /**< 消除未使用变量警告 */

/* 字符串操作 */
#define CSTR(stdstr) ((stdstr).c_str()) /**< string转换为char* */
#define VSTR(stdstr) \
    ((stdstr).empty() ? "(null)" : (stdstr).c_str()) /**< 可视化字符串 */
#define CH2STR(ch) \
    (std::string(1, static_cast<char>(ch))) /**< 字符转换为string */
#define BUILD_DATE __DATE__                 /**< 编译日期(字符串) */
#define BUILD_TIME __TIME__                 /**< 编译时间(字符串) */

/* 编译器分支程序优化 */
#define LIKELY(x) (__builtin_expect(!!(x), 1)) /**<进入此分支概率更大 */
#define UNLIKELY(x) (__builtin_expect(!!(x), 0)) /**<进入此分支概率更小 */

/** 动态链接库的符号导出控制,在声明的时候加上即可 */
#if __GNUC__ >= 4
#define SYMBOL_PUBLIC __attribute__((visibility("default")))
#define SYMBOL_LOCAL __attribute__((visibility("hidden")))
#else
#define SYMBOL_PUBLIC
#define SYMBOL_LOCAL
#endif

/** 禁止对象拷贝(删除拷贝构造函数和赋值运算符) */
#define DECL_COPY_DISABLE(...)                \
    __VA_ARGS__(const __VA_ARGS__&) = delete; \
    __VA_ARGS__& operator=(const __VA_ARGS__&) = delete;

/** 定义类名(给类添加获取类名的方法) */
#define DECL_CLASSNAME(ClassName)                                     \
public:                                                               \
    virtual std::string& className() { return this->__CLASS_NAME__; } \
                                                                      \
private:                                                              \
    std::string __CLASS_NAME__{#ClassName};

/** 生成类属性(get & set) */
#define CC_PROPERTY(varType, varName, dftVal)          \
protected:                                             \
    varType m_##varName{dftVal};                       \
                                                       \
public:                                                \
    varType get##varName(void) { return m_##varName; } \
                                                       \
public:                                                \
    void set##varName(varType var) { m_##varName = var; }

#define ENUM_STRING(enum_name) \
    { enum_name, #enum_name } /**< 定义枚举字符串 */

#define APP_VERSION_BUILD(app, major, minor, patch)                         \
    do {                                                                    \
        if (!g_appVersionBuilder) {                                         \
            AppVersion::build((app), (major), (minor), (patch), BUILD_DATE, \
                              BUILD_TIME);                                  \
        }                                                                   \
    } while (0); /**<定义软件版本 */

namespace zemb {
/**
 * @class EnumString
 * @brief 用于定义枚举字符串表
 */
struct EnumString {
    int value;
    const char* name;
};

/**
 * 注意：以下类型定义可能会和其他第三方库定义产生冲突，解决办法：
 * 在引用完第三方库后undef类型
 *  #include <zemb/BaseType.h>
 *  #define uint64 zemb_uint64
 *  #include <opencv2/opencv.hpp>
 *  #undef uint64
 */
using sint8 = signed char;   /**< 8位有符号数 */
using sint16 = int16_t;      /**< 16位有符号数 */
using sint32 = signed int;   /**< 32位有符号数 */
using sint64 = int64_t;      /**< 64位有符号数 */
using uint8 = unsigned char; /**< 8位无符号数 */
using uint16 = uint16_t;     /**< 16位无符号数 */
using uint32 = uint32_t;     /**< 32位无符号数 */
using uint64 = uint64_t;     /**< 64位无符号数 */
using fp32 = float;          /**< 32位浮点数 */
using fp64 = double;         /**< 64位浮点数 */

/* 数值最大值 */
enum MAX_INT_E {
    MAX_SINT8 = 127,
    MAX_UINT8 = 255,
    MAX_SINT16 = 32767,
    MAX_UINT16 = 65535,
    MAX_SINT32 = 2147483647,
    MAX_UINT32 = 4294967295,
    MAX_SINT64 = 9223372036854775807LL,
    MAX_UINT64 = 18446744073709551615ULL,
};

/* 定义函数返回值 */
enum RC_E {
    RC_OK = 0,       /**< 返回值:正常 */
    RC_ERROR = -1,   /**< 返回值:出错 */
    RC_TIMEOUT = -2, /**< 返回值:超时 */
};

/* 定义数据类型 */
enum BASETYPE_E {
    BASETYPE_NONE = 0,    /**< 未知类型 */
    BASETYPE_INT,         /**< 整数 */
    BASETYPE_DOUBLE,      /**< 浮点数 */
    BASETYPE_STRING,      /**< 字符串 */
    BASETYPE_INTARRAY,    /**< 整数数组 */
    BASETYPE_DOUBLEARRAY, /**< 浮点数数组 */
    BASETYPE_STRINGARRAY, /**< 字符串数组 */
    BASETYPE_TUPLE,       /**< 元组 */
};

class VersionBuilder;
extern VersionBuilder* g_appVersionBuilder;
class AppVersion {
public:
    AppVersion();
    int majorVersion();
    int minorVersion();
    int patchVersion();
    std::string version();
    std::string date();
    std::string time();
    static void build(const std::string& appName, int major, int minor,
                      int patch, const std::string& date,
                      const std::string& time);
};

/**
 * @class Bitable
 * @brief 用于对类型T进行位操作
 * @tparam T
 */
template <typename T>
class Bitable {
public:
    /**
     * @brief 构造位值类
     * @param value
     */
    explicit Bitable(const T& value) { m_value = value; }
    ~Bitable() {}
    /**
     * @brief 设置位
     * @param bit
     * @return Bitable&
     */
    Bitable& set(uint8 bit) {
        T t = 1;
        m_value = (t << bit) | m_value;
        return (*this);
    }
    /**
     * @brief 清除位
     * @param bit
     * @return Bitable&
     */
    Bitable& clr(uint8 bit) {
        T t = 1;
        m_value = (~(t << bit)) & m_value;
        return (*this);
    }

    /**
     * @brief 获取当前值
     * @return T
     */
    T value() { return m_value; }

    /**
     * @brief 获取位值
     * @param bit
     * @return bool, true:1 false:0
     */
    bool value(uint8 bit) {
        T t = 1;
        return !!((t << (bit)) & (m_value));
    }

private:
    T m_value;
};

/**
 * @class Array
 * @brief 数组基类
 * @tparam T
 */
template <typename T>
class Array {
public:
    Array() : m_type(BASETYPE_NONE) {}
    virtual ~Array() { m_array.clear(); }
    int type() { return m_type; }
    int size() { return m_array.size(); }
    void clear() { m_array.clear(); }
    T& operator[](int idx) { return m_array[idx]; }
    Array& operator=(const Array& array) {
        if (this == &array) {
            return (*this);
        }
        m_array = array.m_array;
        return (*this);
    }
    Array& append(T item) {
        m_array.push_back(item);
        return *this;
    }

protected:
    int m_type;
    std::vector<T> m_array;
};

/**
 * @class IntArray
 * @brief 整形数组类,例:[1, 2, 3]
 */
class IntArray : public Array<int> {
    DECL_CLASSNAME(IntArray)

public:
    IntArray() { m_type = BASETYPE_INTARRAY; }
    /**
     * @brief 整型数组
     * @param arrayString 初始化字符串,如"[1,2,3]"
     */
    explicit IntArray(const std::string& arrayString) {
        m_type = BASETYPE_INTARRAY;
        initWithString(arrayString);
    }
    /**
     * @brief 使用字符串初始化
     * @param arrayString 初始化字符串,如"[1,2,3]"
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initWithString(const std::string& arrayString);
    /**
     * @brief 序列化
     * @return std::string 序列化字符串
     */
    std::string serialize();

    /**
     * @brief 流输出运算符重载
     * @param os
     * @param array
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, IntArray& array) {
        os << array.serialize();
        return os;
    }
};

/**
 *  @class  DoubleArray
 *  @brief  浮点型数组类,例:[1.01, 2.02, 3.03]
 */
class DoubleArray : public Array<double> {
    DECL_CLASSNAME(DoubleArray)

public:
    DoubleArray() { m_type = BASETYPE_DOUBLEARRAY; }
    /**
     * @brief 浮点数组
     * @param arrayString 初始化字符串，如"[1.0,2.1,3.2]"
     */
    explicit DoubleArray(const std::string& arrayString) {
        m_type = BASETYPE_DOUBLEARRAY;
        initWithString(arrayString);
    }
    /**
     * @brief 使用字符串初始化
     * @param arrayString 初始化字符串,如"[1.0,2.1,3.2]"
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initWithString(const std::string& arrayString);
    /**
     * @brief 序列化
     * @return std::string 序列化字符串
     */
    std::string serialize();
    /**
     * @brief 流输出运算符重载
     * @param os
     * @param array
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, DoubleArray& array) {
        os << array.serialize();
        return os;
    }
};

/**
 * @class StringArray
 * @brief 字符串数组类
 */
class StringArray : public Array<std::string> {
    DECL_CLASSNAME(StringArray)

public:
    StringArray() { m_type = BASETYPE_STRINGARRAY; }
    /**
     * @brief 字符串数组
     * @param arrayString  初始化字符串,如"[\"one\",\"two\",\"three\"]"
     */
    explicit StringArray(const std::string& arrayString) {
        m_type = BASETYPE_STRINGARRAY;
        initWithString(arrayString);
    }
    /**
     * @brief 使用字符串初始化
     * @param arrayString 初始化字符串,如"[\"one\",\"two\",\"three\"]"
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initWithString(const std::string& arrayString);
    /**
     * @brief 序列化
     * @return std::string 序列化字符串
     */
    std::string serialize();
    /**
     * @brief 流输出运算符重载
     * @param os
     * @param array
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, StringArray& array) {
        os << array.serialize();
        return os;
    }
};

/**
 * @class TupleItem
 * @brief 元组元素类
 */
class TupleItem {
    DECL_CLASSNAME(TupleItem)

public:
    /**
     * @brief 构造整形元组元素
     */
    explicit TupleItem(int);
    /**
     * @brief 构造浮点元组元素
     */
    explicit TupleItem(double);
    /**
     * @brief 构造字符串元组元素
     */
    explicit TupleItem(std::string);
    /**
     * @brief 拷贝构造函数
     */
    TupleItem(const TupleItem&);
    virtual ~TupleItem();
    /**
     * @brief 获取元组元素类型
     * @return int
     */
    int baseType();
    /**
     * @brief 元组元素转换为整形
     * @return int
     */
    int toInt();
    /**
     * @brief 元组元素转换为浮点数
     * @return double
     */
    double toDouble();
    /**
     * @brief 元组元素转换为字符串
     * @return std::string
     */
    std::string toString();

private:
    int m_type;
    double m_value;
    std::string m_string;
};

/**
 * @class Tuple
 * @brief 元组类(可以同时存储整数,浮点数,字符串)
 */
class Tuple {
    DECL_CLASSNAME(Tuple)

public:
    Tuple();
    Tuple(const Tuple&);
    virtual ~Tuple();
    /**
     * @brief 使用字符串初始化元组
     * @param tupleString 初始化字符串,如"(1,2.345,\"three\")"
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool initWithString(const std::string& tupleString);
    /**
     * @brief 获取元组元素个数
     * @return int 元素个数
     */
    int size();
    /**
     * @brief 获取类型
     * @return int
     */
    int type();
    /**
     * @brief 清空元组
     */
    void clear();
    /**
     * @brief 序列化
     * @return std::string 序列化字符串
     */
    std::string serialize();
    /**
     * @brief 增加元素
     * @tparam T
     * @param item
     * @return Tuple&
     */
    template <typename T>
    Tuple& append(const T& item) {
        auto itemPtr = std::make_unique<TupleItem>(item);
        m_itemVect.push_back(std::move(itemPtr));
        return *this;
    }
    /**
     * @brief 插入元素
     * @tparam T
     * @param idx
     * @param item
     * @return Tuple&
     */
    template <typename T>
    Tuple& insert(int idx, const T& item) {
        auto itemPtr = std::make_unique<TupleItem>(item);
        auto iter = m_itemVect.begin();
        int vsize = m_itemVect.size();
        for (auto i = 0; i < idx && i < vsize; ++i) {
            iter++;
        }
        m_itemVect.insert(iter, std::move(itemPtr));
        return *this;
    }
    /**
     * @brief 重载[]运算阀
     * @param idx
     * @return TupleItem&
     */
    TupleItem& operator[](int idx);
    /**
     * @brief 重载赋值运算符
     * @return Tuple&
     */
    Tuple& operator=(const Tuple&);
    /**
     * @brief 重载流输出运算符
     * @param os
     * @param tuple
     * @return std::ostream&
     */
    friend std::ostream& operator<<(std::ostream& os, Tuple& tuple) {
        os << tuple.serialize();
        return os;
    }

private:
    std::vector<std::unique_ptr<TupleItem>> m_itemVect;
};

/**
 * @brief 优先队列(大顶堆,最大的在最顶端)
 * @tparam T 队列元素(T需要重载运算符"<")
 */
template <typename T>
class PrioQueue {
private:
    std::priority_queue<T, std::vector<T>, std::less<T>> m_elemQueue;

public:
    inline bool empty() const { return m_elemQueue.empty(); }

    inline void put(T item) { m_elemQueue.emplace(item); }

    inline T get() {
        auto bestItem = m_elemQueue.top();
        m_elemQueue.pop();
        return bestItem;
    }

    inline void clear() {
        while (!m_elemQueue.empty()) {
            m_elemQueue.pop();
        }
    }

    inline uint32 size() { return m_elemQueue.size(); }
};

/**
 * @brief Pair型优先队列(小顶堆,最小的在最顶端)
 * @tparam T 队列元素(T需要重载运算符">")
 */
template <typename T>
class PairPrioQueue {
private:
    using Element = std::pair<int, T>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>>
        m_elemQueue;

public:
    inline bool empty() const { return m_elemQueue.empty(); }

    inline void put(T item, int priority) {
        m_elemQueue.emplace(priority, item);
    }

    inline Element get() {
        auto bestItem = m_elemQueue.top();
        m_elemQueue.pop();
        return bestItem;
    }

    inline void clear() {
        while (!m_elemQueue.empty()) {
            m_elemQueue.pop();
        }
    }

    inline uint32 size() { return m_elemQueue.size(); }
};

#if __cplusplus < 201703L
template <typename T>
class optional {
public:
    optional() = default;

    optional(const optional&) = default;
    optional& operator=(const optional&) = default;

    optional(optional&&) = default;
    optional& operator=(optional&&) = default;

    ~optional() = default;

    template <typename... Args>
    explicit optional(Args&&... args)
        : m_value(true, T(std::forward<Args>(args)...)) {}

    explicit operator bool() const { return m_value.first; }

    T& value() { return m_value.second; }

    const T& value() const { return m_value.second; }

    T* operator->() { return &(m_value.second); }

    const T* operator->() const { return &(m_value.second); }

    T& operator*() { return m_value.second; }

    const T& operator*() const { return m_value.second; }

private:
    std::pair<bool, T> m_value;
};

template <class T>
inline optional<T> make_optional(T const& v) {
    return optional<T>(v);
}

template <class T>
inline optional<T> make_optional(bool cond, T const& v) {
    return optional<T>(cond, v);
}

struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};

// inline constexpr nullopt_t nullopt{/*unspecified*/};

#endif
}  // namespace zemb
#endif
