#ifndef LISHMUTIL_NODE_MEMORY_MANAGER_H
#define LISHMUTIL_NODE_MEMORY_MANAGER_H

#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <cstring>
namespace tvushm {

template<typename T>
class NodeMemoryManager {
public:
    // 节点结构定义
    struct Node {
        T data;
        bool is_used;
        size_t index;

        Node() : is_used(false), index(0) {}
        explicit Node(const T& value) : data(value), is_used(true), index(0) {}
        explicit Node(T&& value) : data(std::move(value)), is_used(true), index(0) {}
    };

private:
    std::vector<Node> nodes_;
    std::list<size_t> free_list_;  // 存储可用节点索引
    mutable std::mutex mutex_;     // 线程安全锁
    size_t capacity_;
    size_t used_count_;
    std::function<void(T&)> initializer_;  // 自定义初始化函数
    std::function<void(T&)> destructor_;   // 自定义析构函数

public:
    // 构造函数
    explicit NodeMemoryManager(size_t initial_capacity = 1)
        : capacity_(initial_capacity), used_count_(0) {
        nodes_.reserve(capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            nodes_.emplace_back();
            nodes_[i].index = i;
            free_list_.push_front(i);
        }
    }

    // 带自定义初始化和析构函数的构造函数
    NodeMemoryManager(size_t initial_capacity,
                      std::function<void(T&)> init_func,
                      std::function<void(T&)> destroy_func)
        : capacity_(initial_capacity), used_count_(0),
        initializer_(init_func), destructor_(destroy_func) {
        nodes_.reserve(capacity_);
        for (size_t i = 0; i < capacity_; ++i) {
            nodes_.emplace_back();
            nodes_[i].index = i;
            free_list_.push_front(i);
        }
    }

    // 析构函数
    ~NodeMemoryManager() {
        clear();
    }

    // 删除拷贝构造函数和赋值操作符
    NodeMemoryManager(const NodeMemoryManager&) = delete;
    NodeMemoryManager& operator=(const NodeMemoryManager&) = delete;

    // 移动构造函数
    NodeMemoryManager(NodeMemoryManager&& other) noexcept
        : nodes_(std::move(other.nodes_)),
        free_list_(std::move(other.free_list_)),
        capacity_(other.capacity_),
        used_count_(other.used_count_),
        initializer_(std::move(other.initializer_)),
        destructor_(std::move(other.destructor_)) {
        other.capacity_ = 0;
        other.used_count_ = 0;
    }

    // 移动赋值操作符
    NodeMemoryManager& operator=(NodeMemoryManager&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock1(mutex_);
            std::lock_guard<std::mutex> lock2(other.mutex_);

            nodes_ = std::move(other.nodes_);
            free_list_ = std::move(other.free_list_);
            capacity_ = other.capacity_;
            used_count_ = other.used_count_.load();
            initializer_ = std::move(other.initializer_);
            destructor_ = std::move(other.destructor_);

            other.capacity_ = 0;
            other.used_count_ = 0;
        }
        return *this;
    }

    /**
         * @brief 申请一个新的节点
         * @param data 要存储的数据
         * @return 成功返回节点指针，失败返回nullptr
         */
    Node* allocate(const T& data = T{}) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (free_list_.empty()) {
            // 如果没有可用节点，尝试扩展容量
            if (!expandCapacity()) {
                return nullptr; // 扩容失败
            }
        }

        size_t index = free_list_.front();
        free_list_.pop_front();

        Node* node = &nodes_[index];
        node->data = data;
        node->is_used = true;

        if (initializer_) {
            initializer_(node->data);
        }

        used_count_++;
        return node;
    }

    /**
         * @brief 申请一个新的节点（移动语义）
         * @param data 要存储的数据
         * @return 成功返回节点指针，失败返回nullptr
         */
    Node* allocate(T&& data) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (free_list_.empty()) {
            if (!expandCapacity()) {
                return nullptr;
            }
        }

        size_t index = free_list_.front();
        free_list_.pop_front();

        Node* node = &nodes_[index];
        node->data = std::move(data);
        node->is_used = true;

        if (initializer_) {
            initializer_(node->data);
        }

        used_count_++;
        return node;
    }

    /**
         * @brief 获取指定索引的节点
         * @param index 节点索引
         * @return 节点指针，如果索引无效或未使用则返回nullptr
         */
    Node* getNode(size_t index) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (index >= nodes_.size() || !nodes_[index].is_used) {
            return nullptr;
        }

        return &nodes_[index];
    }

    /**
         * @brief 释放指定的节点
         * @param node 要释放的节点指针
         * @return 是否成功释放
         */
    bool deallocate(Node* node) {
        if (!node) return false;

        std::lock_guard<std::mutex> lock(mutex_);

        size_t index = node->index;
        if (index >= nodes_.size() || !node->is_used) {
            return false;
        }

        if (destructor_) {
            destructor_(node->data);
        }

        node->is_used = false;
        node->data = T{}; // 重置为默认值
        free_list_.push_front(index);
        used_count_--;

        return true;
    }

    /**
         * @brief 释放指定索引的节点
         * @param index 节点索引
         * @return 是否成功释放
         */
    bool deallocate(size_t index) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (index >= nodes_.size() || !nodes_[index].is_used) {
            return false;
        }

        if (destructor_) {
            destructor_(nodes_[index].data);
        }

        nodes_[index].is_used = false;
        nodes_[index].data = T{};
        free_list_.push_front(index);
        used_count_--;

        return true;
    }

    /**
         * @brief 检查节点是否正在使用
         * @param index 节点索引
         * @return 是否正在使用
         */
    bool isUsed(size_t index) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (index < nodes_.size()) ? nodes_[index].is_used : false;
    }

    /**
         * @brief 获取已使用的节点数量
         * @return 已使用的节点数
         */
    size_t getUsedCount() const {
        return used_count_;
    }

    /**
         * @brief 获取总容量
         * @return 总容量
         */
    size_t getCapacity() const {
        return capacity_;
    }

    /**
         * @brief 清空所有节点
         */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto& node : nodes_) {
            if (node.is_used && destructor_) {
                destructor_(node.data);
            }
            node.is_used = false;
            node.data = T{};
        }

        free_list_.clear();
        for (size_t i = 0; i < nodes_.size(); ++i) {
            nodes_[i].index = i;
            free_list_.push_front(i);
        }
        used_count_ = 0;
    }

    /**
         * @brief 获取所有正在使用的节点
         * @return 正在使用的节点列表
         */
    std::vector<Node*> getUsedNodes() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Node*> result;

        for (auto& node : nodes_) {
            if (node.is_used) {
                result.push_back(&node);
            }
        }

        return result;
    }

    /**
         * @brief 遍历所有正在使用的节点并执行回调
         * @param callback 回调函数
         */
    void forEachUsedNode(std::function<void(Node&)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto& node : nodes_) {
            if (node.is_used) {
                callback(node);
            }
        }
    }

    /**
         * @brief 设置自定义初始化函数
         * @param init_func 初始化函数
         */
    void setInitializer(std::function<void(T&)> init_func) {
        std::lock_guard<std::mutex> lock(mutex_);
        initializer_ = init_func;
    }

    /**
         * @brief 设置自定义析构函数
         * @param destroy_func 析构函数
         */
    void setDestructor(std::function<void(T&)> destroy_func) {
        std::lock_guard<std::mutex> lock(mutex_);
        destructor_ = destroy_func;
    }

private:
    /**
         * @brief 扩展容量
         * @return 是否扩容成功
         */
    bool expandCapacity() {
        size_t new_capacity = capacity_ * 2;
        if (new_capacity > capacity_) {
            try {
                size_t old_size = nodes_.size();
                nodes_.resize(new_capacity);

                // 添加新节点到空闲列表
                for (size_t i = old_size; i < new_capacity; ++i) {
                    nodes_[i].index = i;
                    nodes_[i].is_used = false;
                    free_list_.push_front(i);
                }

                capacity_ = new_capacity;
                return true;
            } catch (...) {
                return false; // 内存分配失败
            }
        }
        return false;
    }
};

// 使用RAII模式的节点持有者
template<typename T>
class NodeHandle {
private:
    NodeMemoryManager<T>* manager_;
    typename NodeMemoryManager<T>::Node* node_;

public:
    NodeHandle() : manager_(nullptr), node_(nullptr) {}

    NodeHandle(NodeMemoryManager<T>* manager, typename NodeMemoryManager<T>::Node* node)
        : manager_(manager), node_(node) {}

    // 移动构造函数
    NodeHandle(NodeHandle&& other) noexcept
        : manager_(other.manager_), node_(other.node_) {
        other.manager_ = nullptr;
        other.node_ = nullptr;
    }

    // 移动赋值操作符
    NodeHandle& operator=(NodeHandle&& other) noexcept {
        if (this != &other) {
            reset();
            manager_ = other.manager_;
            node_ = other.node_;
            other.manager_ = nullptr;
            other.node_ = nullptr;
        }
        return *this;
    }

    // 拷贝构造和赋值被禁用
    NodeHandle(const NodeHandle&) = delete;
    NodeHandle& operator=(const NodeHandle&) = delete;

    ~NodeHandle() {
        reset();
    }

    T* operator->() const { return node_ ? &node_->data : nullptr; }
    T& operator*() const { return node_ ? node_->data : *static_cast<T*>(nullptr); }

    T* get() const { return node_ ? &node_->data : nullptr; }
    bool isValid() const { return manager_ && node_ && node_->is_used; }

    void reset() {
        if (manager_ && node_) {
            manager_->deallocate(node_);
        }
        manager_ = nullptr;
        node_ = nullptr;
    }

    typename NodeMemoryManager<T>::Node* release() {
        auto temp = node_;
        node_ = nullptr;
        manager_ = nullptr;
        return temp;
    }
};

}
#endif // LISHMUTIL_NODE_MEMORY_MANAGER_H

