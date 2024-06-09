#include "FSMachine.h"

#include "Tracer.h"

namespace zemb {
FSMachine::FSMachine() {}

FSMachine::~FSMachine() {
    m_stateHandlerMap.clear();
    m_stateListenerSet.clear();
}

bool FSMachine::initState(int state) {
    if (state < 0) {
        return false;
    }
    m_currState = state;
    return true;
}

bool FSMachine::registerState(int state,
                              std::shared_ptr<FSHandler> stateHandler) {
    if (state < 0) {
        TRACE_ERR_CLASS("state cannot be negative,state(%d)!", state);
        return false;
    }
    auto iter = m_stateHandlerMap.find(state);
    if (iter != m_stateHandlerMap.end()) {
        return false;
    }
    m_stateHandlerMap.insert({state, stateHandler});
    return true;
}

void FSMachine::registerListener(std::shared_ptr<FSListener> listener) {
    m_stateListenerSet.insert({listener});
}

int FSMachine::getState() { return m_currState; }

void FSMachine::processState() {
    if (!m_stateHandlerMap.empty()) {
        auto iter = m_stateHandlerMap.find(m_currState);
        int tmp = m_currState;
        if (iter != m_stateHandlerMap.end()) {
            m_currState = iter->second->handleState();
            for (auto& listener : m_stateListenerSet) {
                listener->onStateChanged(m_currState);
            }
        }
    }
}
}  // namespace zemb
