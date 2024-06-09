#ifndef __ZEMB_FSMACHINE_H__
#define __ZEMB_FSMACHINE_H__

#include <atomic>
#include <map>
#include <memory>
#include <set>

#include "BaseType.h"
#include "Thread.h"
/**
 * @file FSMachine.h
 * @brief 状态机
 * @code 用法
 *    //状态机用法:
 *    //1.实例化StateHandler.
 *    //2.将状态处理器注册到状态机
 *    //3.设置初始状态,启动状态机.
 *    //4.运行状态处理.
 *    FSMachine stateMachine;
 *    AStateHandler stateA;
 *    BStateHandler stateB;
 *    stateMachine.registerState(0,stateA);
 *    stateMachine.registerState(1,stateB);
 *    stateMachine.setState(0);
 *    while(1)
 *    {
 *        stateMachine.processState();
 *        Thread::msleep(100);
 *    }
 * @endcode
 */

namespace zemb {

class FSMachine;
/**
 * @class FSHandler
 * @brief 状态处理器
 */
class FSHandler {
    DECL_CLASSNAME(FSHandler)

public:
    FSHandler() {}
    virtual ~FSHandler() {}
    /**
     * @brief 状态处理函数
     * @return 返回下一个状态
     * @note 每一个子类都必须实现该方法
     */
    virtual int handleState() = 0;
};

/**
 * @class FSListener
 * @brief 状态监听器
 * @note 当状态机状态发生改变时,通知所有状态监听器
 */
class FSListener {
public:
    virtual void onStateChanged(int state) = 0;
};

/**
 * @class FSMachine
 * @brief 状态机
 */
class FSMachine {
    DECL_CLASSNAME(FSMachine)

public:
    FSMachine();
    virtual ~FSMachine();
    /**
     * @brief 初始化状态
     * @param state
     * @return true
     * @return false
     */
    bool initState(int state);
    /**
     * @brief 注册状态处理器
     * @param state,必须大于等于0，负数为非法状态
     * @param stateHandler
     * @return true
     * @return false
     */
    bool registerState(int state, std::shared_ptr<FSHandler> stateHandler);
    /**
     * @brief 注册状态监听器
     * @param listener
     */
    void registerListener(std::shared_ptr<FSListener> listener);

    /**
     * @brief 获取当前状态
     * @return int
     */
    int getState();

    /**
     * @brief 处理当前状态
     */
    void processState(void);

private:
    std::atomic<int> m_currState{-1};
    std::map<int, std::shared_ptr<FSHandler>> m_stateHandlerMap;
    std::set<std::shared_ptr<FSListener>> m_stateListenerSet;
};

}  // namespace zemb

#endif
