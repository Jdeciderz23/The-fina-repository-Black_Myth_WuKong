#ifndef BASESTATE_H
#define BASESTATE_H
#include <string>


#include<string>
/**
 * @class BaseState
 * @brief 状态机的基类，所有状态都需要继承自此类
 * @tparam T 状态所属的实体类型
 */
template <typename T>
class BaseState {
public:
    /**
     * @brief 构造函数
     */
    BaseState() = default;

    /**
     * @brief 析构函数
     */
    virtual ~BaseState() = default;

    /**
     * @brief 进入状态时调用
     * @param entity 状态所属的实体
     */
    virtual void onEnter(T* entity) = 0;

    /**
     * @brief 状态更新时调用
     * @param entity 状态所属的实体
     * @param deltaTime 帧间隔时间
     */
    virtual void onUpdate(T* entity, float deltaTime) = 0;

    /**
     * @brief 退出状态时调用
     * @param entity 状态所属的实体
     */
    virtual void onExit(T* entity) = 0;

    /**
     * @brief 获取状态名称
     * @return std::string 状态名称
     */
    virtual std::string getStateName() const = 0;
};

#endif // BASESTATE_H