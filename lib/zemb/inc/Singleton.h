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
#ifndef __ZEMB_SINGLETON__H__
#define __ZEMB_SINGLETON__H__

/**
 * @file Singleton.h
 * @brief 单例
 * @code 单例使用例程
 * class Dummy: public Singleton<Dummy>{
 * DECL_SINGLETON(Dummy)
 * public:
 *     virtual ~Dummy();
 *     ...
 * };
 * Dummy::Dummy()
 * {
 * }
 * @endcode
 */
namespace zemb {

/** 单例模板类子类构造函数定义,用于定义单例模板类子类的构造函数 */
#define DECL_SINGLETON(InstanceClass)      \
private:                                   \
    friend class Singleton<InstanceClass>; \
    InstanceClass();

/**
 * @brief 单例模板类
 * @tparam T
 */
template <typename T>
class Singleton {
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
    virtual ~Singleton() {}

protected:
    Singleton() {}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

}  // namespace zemb
#endif
