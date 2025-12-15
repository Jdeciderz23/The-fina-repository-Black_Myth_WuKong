#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "BaseState.h"
#include <unordered_map>
#include <string>
#include <memory>

/**
 * @class StateMachine
 * @brief 状态机类，负责管理实体的状态切换和更新
 * @tparam T 状态所属的实体类型
 */
template <typename T>
class StateMachine {
public:
    /**
     * @brief 构造函数
     * @param owner 状态机所属的实体
     */
    explicit StateMachine(T* owner) : _owner(owner), _currentState(nullptr), _previousState(nullptr) {}

    /**
     * @brief 析构函数
     */
    ~StateMachine() {
        _states.clear();
    }

    /**
     * @brief 初始化状态机
     * @param initialState 初始状态
     */
    void init(BaseState<T>* initialState) {
        _currentState = initialState;
        if (_currentState) {
            _currentState->onEnter(_owner);
        }
    }

    /**
     * @brief 更新状态机
     * @param deltaTime 帧间隔时间
     */
    void update(float deltaTime) {
        if (_currentState) {
            _currentState->onUpdate(_owner, deltaTime);
        }
    }

    /**
     * @brief 注册状态
     * @param state 要注册的状态
     */
    void registerState(BaseState<T>* state) {
        if (state) {
            _states[state->getStateName()] = state;
        }
    }

    /**
     * @brief 切换到指定状态
     * @param stateName 目标状态名称
     */
    void changeState(const std::string& stateName) {
        auto it = _states.find(stateName);
        if (it == _states.end()) {
            return; // 状态不存在
        }

        changeState(it->second);
    }

    /**
     * @brief 切换到指定状态
     * @param newState 目标状态指针
     */
    void changeState(BaseState<T>* newState) {
        if (!newState || newState == _currentState) {
            return;
        }

        // 退出当前状态
        if (_currentState) {
            _currentState->onExit(_owner);
            _previousState = _currentState;
        }

        // 进入新状态
        _currentState = newState;
        _currentState->onEnter(_owner);
    }

    /**
     * @brief 返回到上一个状态
     */
    void revertToPreviousState() {
        if (_previousState) {
            changeState(_previousState);
        }
    }

    /**
     * @brief 获取当前状态
     * @return BaseState<T>* 当前状态指针
     */
    BaseState<T>* getCurrentState() const {
        return _currentState;
    }

    /**
     * @brief 获取上一个状态
     * @return BaseState<T>* 上一个状态指针
     */
    BaseState<T>* getPreviousState() const {
        return _previousState;
    }

    /**
     * @brief 检查是否处于指定状态
     * @param stateName 状态名称
     * @return bool 是否处于该状态
     */
    bool isInState(const std::string& stateName) const {
        return _currentState && _currentState->getStateName() == stateName;
    }

private:
    T* _owner; ///< 状态机所属的实体
    BaseState<T>* _currentState; ///< 当前状态
    BaseState<T>* _previousState; ///< 上一个状态
    std::unordered_map<std::string, BaseState<T>*> _states; ///< 已注册的状态映射
};

#endif // STATEMACHINE_H